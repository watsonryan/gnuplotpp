#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${1:-$ROOT_DIR/build/dev-debug}"

run_example() {
  local exe="$1"
  shift
  "$BUILD_DIR/$exe" "$@"
}

echo "[refresh] using build dir: $BUILD_DIR"

run_example three_line_ieee_example --out "$ROOT_DIR/out/three_line_ieee_readme"
run_example stats_plot_examples --out "$ROOT_DIR/out/stats_plot_examples"
run_example tufte_minimal_example --out "$ROOT_DIR/out/tufte_minimal_example"
run_example monte_carlo_alpha_example --out "$ROOT_DIR/out/monte_carlo_alpha_example" --npaths 1000 --lw 2 --alpha 0.3

cp "$ROOT_DIR/out/three_line_ieee_readme/figures/figure.svg" "$ROOT_DIR/docs/images/three_line_ieee_example.svg"
cp "$ROOT_DIR/out/stats_plot_examples/qq_plot/figures/figure.png" "$ROOT_DIR/docs/images/stats_qq_plot.png"
cp "$ROOT_DIR/out/stats_plot_examples/violin_profile/figures/figure.png" "$ROOT_DIR/docs/images/stats_violin_profile.png"
cp "$ROOT_DIR/out/stats_plot_examples/confidence_ellipse/figures/figure.png" "$ROOT_DIR/docs/images/stats_confidence_ellipse.png"
cp "$ROOT_DIR/out/stats_plot_examples/autocorrelation/figures/figure.png" "$ROOT_DIR/docs/images/stats_autocorrelation.png"
cp "$ROOT_DIR/out/tufte_minimal_example/figures/figure.png" "$ROOT_DIR/docs/images/tufte_minimal_example.png"
cp "$ROOT_DIR/out/monte_carlo_alpha_example/figures/figure.png" "$ROOT_DIR/docs/images/monte_carlo_alpha_example.png"

echo "[refresh] README assets updated under docs/images"
