#!/usr/bin/env python3

import pyqtgraph as pg
import json
import re
import numpy as np


class BenchmarkVis:
    def __init__(self,
                 window,
                 bench_file,
                 pen=pg.mkPen()):
        self.rgx = re.compile(".*Base64.*decode.*")
        self.key_x = lambda bench: int(bench["name"].split("/")[2])
        self.key_y = lambda x: x["cpu_time"]
        self.pen = pen
        self.window = window
        self.data = json.loads(open(bench_file).read())
        self.draw(window)

    def draw(self, window, throughput=True):
        if hasattr(self, "line"):
            if self.line is not None:
                window.removeItem(self.line)

        self.benchmarks = np.array(sorted(list(self.benchmarks_to_xy(
            self.data["benchmarks"], self.rgx, throughput))))
        self.benchmarks = np.transpose(self.benchmarks)
        self.line = window.plot(
            self.benchmarks[0], self.benchmarks[1], pen=self.pen)
        self.vertlines = list()

    def setPen(self, pen):
        self.line.setData(self.benchmarks[0], self.benchmarks[1], pen=pen)

    def drawCacheBoundaries(self, height):
        self.drawVertLine(32 * 1024, height)
        self.drawVertLine(512 * 1024, height)
        self.drawVertLine(4 * 1024 * 1024, height)

    def drawVertLine(self, x, height):
        self.vertlines.append(self.window.plot(
            [x, x], [0, height], pen=pg.mkPen(color=(0, 0, 255))))

    def benchmarks_to_xy(self, benchmarks, rgx: re.Pattern, throughput=True):
        for bench in benchmarks:
            if not rgx.match(bench["name"]):
                continue
            x = self.key_x(bench)
            y = self.key_y(bench)
            if throughput and x != 0:
                y /= x
            yield x, y
