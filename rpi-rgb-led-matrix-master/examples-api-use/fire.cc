// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Small example how to use the library.
// For more examples, look at demo-main.cc
//
// This code is public domain
// (but note, that the led-matrix library this depends on is GPL v2)

#include "led-matrix.h"

#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <signal.h>

using rgb_matrix::GPIO;
using rgb_matrix::RGBMatrix;
using rgb_matrix::Canvas;

class RGB
{
public:
	unsigned int R;
	unsigned int G;
	unsigned int B;

	RGB()
	{
		R = 0;
		G = 0;
		B = 0;
	}

	RGB(unsigned int r, unsigned int g, unsigned int b)
	{
		R = r;
		G = g;
		B = b;
	}

	bool Equals(RGB rgb)
	{
		return (R == rgb.R) && (G == rgb.G) && (B == rgb.B);
	}
};

class HSL
{
public:
	int H;
	float S;
	float L;

	HSL(int h, float s, float l)
	{
		H = h;
		S = s;
		L = l;
	}

	bool Equals(HSL hsl)
	{
		return (H == hsl.H) && (S == hsl.S) && (L == hsl.L);
	}
};

static float HueToRGB(float v1, float v2, float vH) {
	if (vH < 0)
		vH += 1;

	if (vH > 1)
		vH -= 1;

	if ((6 * vH) < 1)
		return (v1 + (v2 - v1) * 6 * vH);

	if ((2 * vH) < 1)
		return v2;

	if ((3 * vH) < 2)
		return (v1 + (v2 - v1) * ((2.0f / 3) - vH) * 6);

	return v1;
}

static RGB HSLToRGB(HSL hsl) {
	unsigned char r = 0;
	unsigned char g = 0;
	unsigned char b = 0;

	if (hsl.S == 0)
	{
		r = g = b = (unsigned char)(hsl.L * 255);
	}
	else
	{
		float v1, v2;
		float hue = (float)hsl.H / 360;

		v2 = (hsl.L < 0.5) ? (hsl.L * (1 + hsl.S)) : ((hsl.L + hsl.S) - (hsl.L * hsl.S));
		v1 = 2 * hsl.L - v2;

		r = (unsigned int)(255 * HueToRGB(v1, v2, hue + (1.0f / 3)));
		g = (unsigned int)(255 * HueToRGB(v1, v2, hue));
		b = (unsigned int)(255 * HueToRGB(v1, v2, hue - (1.0f / 3)));
	}

	return RGB(r, g, b);
}

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
	interrupt_received = true;
}

const int w = 64;
const int h = 64;
RGB palette[256];
int fire[h][w];  //this buffer will contain the fire

static void GetPalette() {
	RGB color = RGB(0, 0, 0);
	HSL hsl = HSL(0, 0.0, 0.0);
	//generate the palette
	for (int x = 0; x < 256; x++)
	{
		//HSLtoRGB is used to generate colors:
		//Hue goes from 0 to 85: red to yellow
		//Saturation is always the maximum: 255
		//Lightness is 0..255 for x=0..128, and 255 for x=128..255
		hsl = HSL(x / 3, 1.0, (std::min(255, x * 2)) / 255.0);
		color = HSLToRGB(hsl);
		//set the palette to the calculated RGB value
		palette[x] = color;
	}
}

static void DrawOnCanvas(Canvas *canvas) {
	/*
	 * Let's create a simple animation. We use the canvas to draw
	 * pixels. We wait between each step to have a slower animation.
	 */
	RGB rgb = palette[85];
	//canvas->Fill(rgb.R, rgb.G, rgb.B);

	int center_x = canvas->width() / 2;
	int center_y = canvas->height() / 2;

	//randomize the bottom row of the fire buffer
	for (int x = 0; x < w; x++) fire[h - 1][x] = abs(32768 + rand()) % 256;
	//do the fire calculations for every pixel, from top to bottom
	for (int y = 0; y < h - 1; y++)
	{
		for (int x = 0; x < w; x++)
		{
			fire[y][x] =
				((fire[(y + 1) % h][(x - 1 + w) % w]
					+ fire[(y + 1) % h][(x) % w]
					+ fire[(y + 1) % h][(x + 1) % w]
					+ fire[(y + 2) % h][(x) % w])
					* 32) / 129;
		}
	}

	//set the drawing buffer to the fire buffer, using the palette colors
	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			RGB c = palette[fire[y][x]];
			canvas->SetPixel(x, y, c.R, c.G, c.B);
		}
	}
	usleep(20 * 1000);  // wait a little to slow down things.
}

int main(int argc, char *argv[]) {

	RGBMatrix::Options defaults;
	defaults.hardware_mapping = "regular";  // or e.g. "adafruit-hat"
	defaults.rows = 32;
	defaults.chain_length = 1;
	defaults.parallel = 1;
	defaults.show_refresh_rate = true;
	Canvas *canvas = rgb_matrix::CreateMatrixFromFlags(&argc, &argv, &defaults);
	if (canvas == NULL)
		return 1;

	// It is always good to set up a signal handler to cleanly exit when we
	// receive a CTRL-C for instance. The DrawOnCanvas() routine is looking
	// for that.
	signal(SIGTERM, InterruptHandler);
	signal(SIGINT, InterruptHandler);

	GetPalette();
	while (!interrupt_received)
	{
		DrawOnCanvas(canvas);    // Using the canvas.
	}
	// Animation finished. Shut down the RGB matrix.
	canvas->Clear();
	delete canvas;

	return 0;
}

