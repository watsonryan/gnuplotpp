# Design Doc: Pure C++ Plotting Module for Publication-Ready Figures (IEEE / AIAA)

name: gnuplotpp

## Status (Current Repo)

Implemented now:
- gnuplot-driven backend with generated `.gp` + `.dat` and direct render to `PDF/SVG/EPS`
- strict IEEE-oriented styling profile for IEEE presets (monochrome + dashed series styles)
- C++ examples for `1x2`, `2x2`, and single-axes 3-line IEEE plot
- axis controls including limits and log axes via `AxesSpec`

Not currently active in repo:
- native PNG backend (removed to keep workflow strictly gnuplot/vector-first)
- JSON config loader (`plot_config.json`) integration

## 0. Objective

Provide a **pure C++20** plotting experience for the refactored estimation repo:

- Example executables call a **C++ plotting API** directly (no Python).
- The system generates **publication-ready** vector figures (**PDF/SVG/EPS**) suitable for IEEE and AIAA submissions.
- Styling and layout are **easily configurable**:
  - presets: IEEE / AIAA
  - figure widths: single-column / double-column (IEEE), column / page width (AIAA)
  - subplot layouts (e.g., **1×2 “two-window” plots**, 2×1, 2×2, …)

**Key design choice:** Use **gnuplot as an external renderer backend**, driven by auto-generated scripts/data from C++.
- This keeps the developer experience “pure C++” (your C++ program produces the final PDF/SVG directly).
- You avoid embedding Python.
- You get stable, high-quality vector output and built-in multiplot layout support.

---

## 1. Requirements

### 1.1 Output formats
Must support:
- **PDF** (primary for submission)
- **SVG** (vector for web/docs, or conversion)
- **EPS** (optional legacy workflows)
- **PNG** (optional preview raster)

### 1.2 Publication presets
Provide built-in presets based on common author guidance:

**IEEE graphics sizing:**
- One column width: **3.5 in**
- Two columns width: **7.16 in**
- IEEE recommends vector formats (PS/EPS/PDF) for flexibility.

**AIAA sizing / readability:**
- Column width: **3.25 in**
- Page width: **7 in**
- Lettering should remain at least **~8 pt** when reduced to column width.

(These appear in official IEEE Author Center and AIAA journal figure/table guidance; see references.)

### 1.3 Layout
Support at least:
- `layout(1,1)`, `layout(1,2)`, `layout(2,1)`, `layout(2,2)`
- shared axes (optional), but consistent subplot spacing/margins
- per-subplot titles and labels
- global figure title (optional)

### 1.4 Styling controls
Configurable per figure or globally:
- font family, font size
- line width, point size
- grid on/off
- legend placement
- color palette (include a colorblind-friendly palette)
- axis limits, log axes
- tick formatting (scientific notation)

### 1.5 “Pure C++ experience”
From a user workflow perspective:
- `./app_dynamic_compare --out out/run1` produces:
  - `out/run1/figures/*.pdf` (and optional svg/png)
  - `out/run1/*.csv` (optional; recommended for reproducibility)

No manual scripts required.

---

## 2. Non-goals (v1)

- Interactive GUI plotting
- Complex annotation editor
- Full TeX layout engine (optional later)
- 3D plotting (optional later)

---

## 3. High-level architecture

### 3.1 Components

1. **Plot specification (C++ data model)**
   - `FigureSpec`: size, output formats, preset/theme, layout
   - `AxesSpec`: labels, limits, grid, legend policy
   - `SeriesSpec`: line/scatter/errorbar/band series + styling

2. **Renderer backend interface**
   - `IPlotBackend`: converts a `FigureSpec` + series data into output files

3. **Gnuplot backend (v1)**
   - Generates:
     - data files (`.dat`) per series
     - a gnuplot script (`.gp`)
   - Executes `gnuplot` to render PDF/SVG/EPS/PNG

4. **Plot manager**
   - Loads plot configuration from file (`plot_config.json`)
   - Exposes a small API used by **all examples**
   - Centralizes figure naming conventions and output directories

---

## 4. Public C++ API (recommended)

Design the plotting module as `bierman::viz` (or `est::viz`) so all examples can use it.

### 4.1 Core types
```cpp
namespace viz {

enum class OutputFormat { Pdf, Svg, Eps, Png };

enum class Preset {
  IEEE_SingleColumn,
  IEEE_DoubleColumn,
  AIAA_Column,
  AIAA_Page,
  Custom
};

struct FigureSizeInches {
  double w = 3.5;
  double h = 2.5;
};

struct Style {
  std::string font = "Times-New-Roman"; // configurable
  double font_pt = 9.0;
  double line_width_pt = 1.0;
  double point_size = 0.6;
  bool grid = false;
};

struct FigureSpec {
  Preset preset = Preset::IEEE_SingleColumn;
  FigureSizeInches size{};
  Style style{};
  int rows = 1;
  int cols = 1;
  std::vector<OutputFormat> formats{OutputFormat::Pdf};
  std::string title;
};

struct AxesSpec {
  std::string title;
  std::string xlabel;
  std::string ylabel;
  bool grid = false;
  bool legend = true;

  // optional limits
  bool has_xlim = false;
  double xmin = 0, xmax = 0;
  bool has_ylim = false;
  double ymin = 0, ymax = 0;

  bool xlog = false;
  bool ylog = false;
};

enum class SeriesType { Line, Scatter, ErrorBars, Band };

struct SeriesSpec {
  SeriesType type = SeriesType::Line;
  std::string label;
  // style overrides (optional)
  bool has_line_width = false;
  double line_width_pt = 1.0;
};

} // namespace viz
```

### 4.2 Data passing (Eigen-friendly)
All plotting calls should accept:
- `std::span<const double>`
- `Eigen::Ref<const Eigen::VectorXd>` (or `Eigen::VectorXd`)

Example API:
```cpp
namespace viz {

class Figure {
public:
  explicit Figure(FigureSpec spec);

  // Access subplot by (row, col) or linear index.
  class Axes& axes(int r, int c);
  class Axes& axes(int idx);

  void save(const std::filesystem::path& out_dir) const;
};

class Axes {
public:
  void set(const AxesSpec& spec);

  void add_series(const SeriesSpec& spec,
                  Eigen::Ref<const Eigen::VectorXd> x,
                  Eigen::Ref<const Eigen::VectorXd> y);

  // Optional: error bars and bands
  void add_errorbars(...);
  void add_band(...);
};

} // namespace viz
```

### 4.3 Example usage (two-window plot)
```cpp
viz::FigureSpec fs;
fs.preset = viz::Preset::IEEE_DoubleColumn;   // good for 1×2 layout
fs.rows = 1; fs.cols = 2;
fs.formats = {viz::OutputFormat::Pdf, viz::OutputFormat::Svg};

viz::Figure fig(fs);

viz::AxesSpec ax0;
ax0.title = "Position Error";
ax0.xlabel = "t [s]";
ax0.ylabel = "||e_p|| [m]";
ax0.grid = true;

viz::AxesSpec ax1 = ax0;
ax1.title = "Velocity Error";
ax1.ylabel = "||e_v|| [m/s]";

fig.axes(0,0).set(ax0);
fig.axes(0,1).set(ax1);

fig.axes(0,0).add_series({.type=viz::SeriesType::Line, .label="SRIF"}, t, ep_norm);
fig.axes(0,1).add_series({.type=viz::SeriesType::Line, .label="SRIF"}, t, ev_norm);

fig.save(out_dir / "figures");
```

---

## 5. Presets: IEEE and AIAA defaults

### 5.1 IEEE defaults
Preset behavior:
- `IEEE_SingleColumn`:
  - width = **3.5 in**
  - height default ~2.5 in (configurable)
  - font ~8–10 pt
- `IEEE_DoubleColumn`:
  - width = **7.16 in**
  - height default ~2.5–3.0 in (configurable)

### 5.2 AIAA defaults
Preset behavior:
- `AIAA_Column`:
  - width = **3.25 in**
  - enforce default font >= 8 pt (to remain legible at column width)
- `AIAA_Page`:
  - width = **7.0 in**

**Note:** Presets should be *defaults*, not hard-coded requirements. Users can override sizes and fonts.

---

## 6. Backend design

### 6.1 Backend interface
```cpp
namespace viz {

struct RenderResult {
  bool ok = true;
  std::string message;
  std::filesystem::path script_path;
  std::vector<std::filesystem::path> outputs;
};

class IPlotBackend {
public:
  virtual ~IPlotBackend() = default;
  virtual RenderResult render(const Figure& fig,
                              const std::filesystem::path& out_dir) = 0;
};

} // namespace viz
```

### 6.2 Gnuplot backend (v1)

#### Responsibilities
- Create `out_dir` and `out_dir/tmp/`
- For each subplot and series:
  - write a `.dat` file with columns appropriate to the series type:
    - line/scatter: `x y`
    - errorbars: `x y yerr`
    - band: `x y_low y_high`
- Generate a `.gp` script:
  - choose terminal based on output format:
    - PDF: `set terminal pdfcairo ...`
    - SVG: `set terminal svg ...`
    - EPS: `set terminal epscairo ...`
    - PNG: `set terminal pngcairo ...`
  - apply font, line width, size
  - use multiplot layout:
    - `set multiplot layout <rows>,<cols> ...`
  - emit per-axes commands:
    - titles/labels
    - grid on/off
    - ranges/log axes
    - plot commands referencing the `.dat` series files

#### Execution model
- Prefer running `gnuplot` in **batch mode**:
  - write script file
  - call: `gnuplot <script.gp>`
- Capture stdout/stderr for diagnostics.
- If `gnuplot` not found:
  - still emit `.gp` + `.dat` so user can run later
  - return `RenderResult.ok=false` with a helpful message

#### Why gnuplot works well here
- Vector terminals (PDF/EPS/SVG) are mature
- Multiplot layout is a first-class feature: `set multiplot layout rows,cols`
- Terminals allow explicit font selection (e.g., Times New Roman) and figure size

---

## 7. Configuration system (easy and consistent)

### 7.1 Plot configuration file
Provide `plot_config.json` at repo root or per-run directory.

Example:
```json
{
  "preset": "IEEE_SingleColumn",
  "formats": ["pdf", "svg"],
  "font": "Times-New-Roman",
  "font_pt": 9,
  "line_width_pt": 1.0,
  "grid": true,
  "default_height_in": 2.5
}
```

### 7.2 Override policy
- CLI flags override config file.
- Per-figure overrides override global defaults.

### 7.3 CMake integration
- `ENABLE_PLOTTING=ON`
- `find_program(GNUPLOT_EXECUTABLE gnuplot)`
- Compile-time define `VIZ_HAS_GNUPLOT` if found.

---

## 8. Integration with examples (one unified path)

### 8.1 Mandatory: shared plotting entry point
Every example should call a shared helper:
- `make_static_figures(run, config)`
- `make_dynamic_figures(run, config)`
- `make_ukf_figures(run, config)`

Where `run` contains:
- time vector
- truth and estimate trajectories
- error norms
- method names

### 8.2 Output layout
```
out/<example>/<runid>/
  figures/
    error_norm_1x2.pdf
    traj_xyz.pdf
  tmp/
    error_norm_1x2.gp
    ax0_series0.dat
    ax1_series0.dat
```

---

## 9. Testing and validation

### 9.1 Unit tests (no gnuplot required)
- Script generation “golden file” tests:
  - given a known `FigureSpec`, verify the emitted `.gp` contains expected commands:
    - terminal selection
    - `set multiplot layout 1,2` for two-window plots
    - correct data file references
- Data file formatting tests:
  - verify numeric formatting, header policy (typically no headers for gnuplot data files)

### 9.2 Integration tests (gnuplot required)
If `gnuplot` available:
- render a small figure to PDF
- assert output file exists and is non-empty
- optionally validate the file signature:
  - PDF begins with `%PDF`

---

## 10. Implementation plan (Codex-friendly)

1. Implement `FigureSpec`, `AxesSpec`, `SeriesSpec`, and `Figure/Axes` containers.
2. Implement `GnuplotBackend` that can render:
   - single plot, single series → PDF
3. Add multiplot grid and subplot support.
4. Add presets (IEEE/AIAA widths, font defaults).
5. Add JSON config loader and CLI overrides.
6. Integrate into one example app (dynamic compare).
7. Propagate to all apps so all examples generate figures uniformly.
8. Add tests (golden script + optional integration test).

---

## 11. References (guidelines and docs)

> Keep these as documentation references; presets are defaults and should remain overridable.

- IEEE Author Center (graphics size: 3.5 in one column, 7.16 in two columns; vector formats PS/EPS/PDF recommended):  
  https://journals.ieeeauthorcenter.ieee.org/create-your-ieee-journal-article/create-graphics-for-your-article/resolution-and-size/

- IEEE Proceedings (similar figure width guidance):  
  https://proceedingsoftheieee.ieee.org/resources/guidelines-for-figures-and-tables/

- AIAA guidelines for journal figures/tables (column width 3 1/4 in, page width 7 in; lettering at least 8 pt at column width):  
  https://aiaa.org/publications/journals/journal-author/guidelines-for-journal-figures-and-tables/

- gnuplot documentation (multiplot layout):  
  https://gnuplot.info/docs_6.0/loc13474.html

- gnuplot documentation (pdfcairo terminal):  
  https://gnuplot.info/docs/loc21789.html
