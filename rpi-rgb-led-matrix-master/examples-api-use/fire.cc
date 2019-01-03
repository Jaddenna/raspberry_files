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

int palette[256];

static void GetPalette() {
	RGB color = new RGB(0, 0, 0);
	HSL hsl = new HSL(0, 0.0, 0.0);
	//generate the palette
	for (int x = 0; x < 256; x++)
	{
		//HSLtoRGB is used to generate colors:
		//Hue goes from 0 to 85: red to yellow
		//Saturation is always the maximum: 255
		//Lightness is 0..255 for x=0..128, and 255 for x=128..255
		hsl = new HSL(x / 3, 1.0, (std::min(255, x * 2)) / 255.0);
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
	GetPalette();
	RGB rgb = palette[0];
	canvas->Fill(rgb.R, rgb.G, rgb.B);

	int center_x = canvas->width() / 2;
	int center_y = canvas->height() / 2;
	float radius_max = canvas->width() / 2;
	float angle_step = 1.0 / 360;
	for (float a = 0, r = 0; r < radius_max; a += angle_step, r += angle_step) {
		if (interrupt_received)
			return;
		float dot_x = cos(a * 2 * M_PI) * r;
		float dot_y = sin(a * 2 * M_PI) * r;
		canvas->SetPixel(center_x + dot_x, center_y + dot_y,
			255, 0, 0);
		usleep(1 * 1000);  // wait a little to slow down things.
	}
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

	DrawOnCanvas(canvas);    // Using the canvas.

	// Animation finished. Shut down the RGB matrix.
	canvas->Clear();
	delete canvas;

	return 0;
}

