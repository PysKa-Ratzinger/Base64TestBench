
# Simple base64 benchmarking challenge

To start the challenge, just clone the repository and run make.

You need to have clang++ installed.

# Generating the performance graph

First you need to output the benchmark results into a readable json file, like so:

```
make -j4 && ./challenge_unittests && ./challenge_bench --benchmark_out_format=json --benchmark_out=./results.json
```

Then, run an interactive python shell inside the tools folder, and generate the performance graph:

```
# Setup
import pyqtgraph as pg
import results_visualizer

win = pg.plot()

# Load the graph
p1 = results_visualizer.BenchmarkVis(win, "../results.json")
```

In the opened window, you can also right click the graph and change the display
options of the graph. In particular, it might be useful to view the X axis with
a logarithmic scale.

You can also draw some vertical lines representing some standard boundaries for
CPU cache. These may not correspond exactly to the CPU cache of your computer.

```
p1.drawCacheBoundaries(30)
```

