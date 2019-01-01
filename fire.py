import matplotlib

import time
import numpy as np
from samplebase import SampleBase
import os
import re


class File_Output(SampleBase):
    def __init__(self, *args, **kwargs):
        super(File_Output, self).__init__(*args, **kwargs)
        self.parser.add_argument("-fr", "--framerate", help="Set the Framerate", default=0.1, type=float)

    def get_fire(self):
        return np.array([[(0,0,0) if x % 2 == 0 else (255,255,255) for x in range(64)] for y in range(64)])

    def run(self):
        offscreen_canvas = self.matrix.CreateFrameCanvas()

        m = self.get_fire()

        rate = self.args.framerate

        while True:
            offscreen_canvas.Clear()
            for r in range(len(m)):
                for c in range(len(m[r])):
                    color = m[r][c]
                    if len(color) <= 3:
                        offscreen_canvas.SetPixel(c, r, color[0], color[2], color[1])  # rbg
                    elif color[3] > 0:
                        offscreen_canvas.SetPixel(c, r, color[0], color[2], color[1])  # rbg

            offscreen_canvas = self.matrix.SwapOnVSync(offscreen_canvas)
            time.sleep(rate)


if __name__ == '__main__':
    file_output = File_Output()
    if (not file_output.process()):
        file_output.print_help()

