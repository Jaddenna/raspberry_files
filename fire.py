import matplotlib

import time
import numpy as np
from samplebase import SampleBase
import random
import colorsys


class File_Output(SampleBase):
    def __init__(self, *args, **kwargs):
        super(File_Output, self).__init__(*args, **kwargs)

    def get_palette(self):
        colors = []
        for i in range(255):
            h = (int(i / 3.5) / 360.0)
            l = i / 255.0  # ((min(50, i * 2)) / 255 )
            s = 1.0
            color = colorsys.hls_to_rgb(h, l, s)
            color = (color[0] * 255, color[1] * 255, color[2] * 255)
            colors.append(color)
        return colors

    def get_fire(self, m):
        for x in range(len(m[len(m) - 1])):
            m[x, len(m) - 1] = random.randrange(100, 255)

        base = pow(2, random.randrange(4, 6))
        summand = random.randrange(1, 30)
        multiplicator = random.uniform(4.0, 4.2)
        for y in range(len(m) - 1):
            for x in range(len(m[y])):
                down2rows = m[x, y + 2] if y < len(m) - 3 else 0
                downleft = m[x - 1, y + 1] if x > 0 else 0
                down = m[x, y + 1]
                downright = m[x + 1, y + 1] if x < len(m[y]) - 1 else 0
                m[x, y] = min(((down2rows + downleft + down + downright) * base) // (base * multiplicator + summand),
                              254)
        return m

    def run(self):
        offscreen_canvas = self.matrix.CreateFrameCanvas()
        width, height = (self.matrix.width, self.matrix.height)
        pxwidth = 1
        palette = self.get_palette()
        fire = self.get_fire(np.array([[0 for _ in range(width // pxwidth)] for _ in range(height // pxwidth)]))

        while True:
            fire = self.get_fire(fire)
            fire_colors = np.array([[palette[fire[y][x]] for x in range(len(fire[y]))] for y in range(len(fire))])
            offscreen_canvas.Clear()
            for r in range(len(fire_colors)):
                for c in range(len(fire_colors[r])):
                    color = fire_colors[r][c]
                    if len(color) <= 3:
                        offscreen_canvas.SetPixel(c, r, color[0], color[2], color[1])  # rbg
                    elif color[3] > 0:
                        offscreen_canvas.SetPixel(c, r, color[0], color[2], color[1])  # rbg

            offscreen_canvas = self.matrix.SwapOnVSync(offscreen_canvas)


if __name__ == '__main__':
    file_output = File_Output()
    if (not file_output.process()):
        file_output.print_help()


