# gnuplotpp

Pure C++20 plotting API with a gnuplot backend for publication-ready IEEE/AIAA figures.

## Build

```bash
cmake --preset dev-debug
cmake --build --preset build-debug
ctest --preset test-debug
```

## CMake Presets

- `dev-debug`
- `dev-release`
- `dev-cpm` (enables CPM dependency management)

## CPM Dependencies (inside CMake)

CPM is optional and controlled by `GNUPLOTPP_ENABLE_CPM`.
When enabled, CMake downloads `CPM.cmake` and adds `nlohmann_json`.

## Example Plots

```bash
./build/dev-debug/two_window_example --out out/two_window
./build/dev-debug/layout_2x2_example --out out/layout_2x2
```

Outputs are written under:

- `out/<name>/figures/tmp/*.dat`
- `out/<name>/figures/tmp/figure.gp`

If `gnuplot` is installed, rendered vector files are also produced:

- `out/<name>/figures/figure.pdf`
- `out/<name>/figures/figure.svg`
