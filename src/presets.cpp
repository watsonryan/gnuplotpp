#include "gnuplotpp/presets.hpp"

namespace gnuplotpp {

PresetDefaults preset_defaults(Preset preset) {
  PresetDefaults defaults;

  switch (preset) {
    case Preset::IEEE_SingleColumn:
      defaults.size = FigureSizeInches{.w = 3.5, .h = 2.5};
      defaults.style.font = "Times";
      defaults.style.font_pt = 8.5;
      defaults.style.line_width_pt = 1.0;
      defaults.style.point_size = 0.6;
      defaults.style.grid = false;
      break;
    case Preset::IEEE_DoubleColumn:
      defaults.size = FigureSizeInches{.w = 7.16, .h = 2.8};
      defaults.style.font = "Times";
      defaults.style.font_pt = 8.5;
      defaults.style.line_width_pt = 1.0;
      defaults.style.point_size = 0.6;
      defaults.style.grid = false;
      break;
    case Preset::AIAA_Column:
      defaults.size = FigureSizeInches{.w = 3.25, .h = 2.4};
      defaults.style.font = "Times";
      defaults.style.font_pt = 8.0;
      defaults.style.line_width_pt = 1.0;
      defaults.style.point_size = 0.6;
      defaults.style.grid = false;
      break;
    case Preset::AIAA_Page:
      defaults.size = FigureSizeInches{.w = 7.0, .h = 2.8};
      defaults.style.font = "Times";
      defaults.style.font_pt = 8.0;
      defaults.style.line_width_pt = 1.0;
      defaults.style.point_size = 0.6;
      defaults.style.grid = false;
      break;
    case Preset::IEEE_Tran:
      defaults.size = FigureSizeInches{.w = 3.5, .h = 2.4};
      defaults.style.font = "Times";
      defaults.style.font_pt = 8.0;
      defaults.style.line_width_pt = 1.0;
      defaults.style.point_size = 0.6;
      defaults.style.grid = false;
      break;
    case Preset::Nature_1Col:
      defaults.size = FigureSizeInches{.w = 3.54, .h = 2.5};
      defaults.style.font = "Arial";
      defaults.style.font_pt = 8.0;
      defaults.style.line_width_pt = 1.0;
      defaults.style.point_size = 0.7;
      defaults.style.grid = false;
      break;
    case Preset::Elsevier_1Col:
      defaults.size = FigureSizeInches{.w = 3.35, .h = 2.4};
      defaults.style.font = "Times";
      defaults.style.font_pt = 8.0;
      defaults.style.line_width_pt = 1.0;
      defaults.style.point_size = 0.6;
      defaults.style.grid = false;
      break;
    case Preset::Custom:
      defaults.size = FigureSizeInches{.w = 3.5, .h = 2.5};
      defaults.style = Style{};
      break;
  }

  return defaults;
}

void apply_preset_defaults(FigureSpec& spec,
                           bool overwrite_size,
                           bool overwrite_style) {
  const auto defaults = preset_defaults(spec.preset);

  if (overwrite_size) {
    spec.size = defaults.size;
  }
  if (overwrite_style) {
    spec.style = defaults.style;
  }

  if ((spec.preset == Preset::AIAA_Column || spec.preset == Preset::AIAA_Page) &&
      spec.style.font_pt < 8.0) {
      spec.style.font_pt = 8.0;
  }
}

void apply_style_profile(FigureSpec& spec, StyleProfile profile) {
  switch (profile) {
    case StyleProfile::Science:
      spec.style.font = "Times";
      spec.style.font_pt = 12.0;
      spec.style.line_width_pt = 1.5;
      spec.style.grid = true;
      spec.style.tick_font_scale = 1.0;
      spec.style.label_font_scale = 4.0 / 3.0;
      spec.style.title_font_scale = 4.0 / 3.0;
      spec.style.title_bold = true;
      spec.palette = ColorPalette::Tab10;
      break;
    case StyleProfile::IEEE_Strict:
      spec.style.font = "Times";
      spec.style.font_pt = 8.5;
      spec.style.line_width_pt = 1.0;
      spec.style.grid = false;
      spec.style.tick_font_scale = 1.0;
      spec.style.label_font_scale = 1.0;
      spec.style.title_font_scale = 1.0;
      spec.style.title_bold = false;
      spec.palette = ColorPalette::Grayscale;
      break;
    case StyleProfile::AIAA_Strict:
      spec.style.font = "Times";
      spec.style.font_pt = 8.0;
      spec.style.line_width_pt = 1.0;
      spec.style.grid = false;
      spec.style.tick_font_scale = 1.0;
      spec.style.label_font_scale = 1.0;
      spec.style.title_font_scale = 1.0;
      spec.style.title_bold = false;
      spec.palette = ColorPalette::Default;
      break;
    case StyleProfile::Presentation:
      spec.style.font = "Helvetica";
      spec.style.font_pt = 12.0;
      spec.style.line_width_pt = 2.0;
      spec.style.grid = true;
      spec.style.tick_font_scale = 1.0;
      spec.style.label_font_scale = 1.15;
      spec.style.title_font_scale = 1.25;
      spec.style.title_bold = true;
      spec.palette = ColorPalette::Viridis;
      break;
    case StyleProfile::DarkPrintSafe:
      spec.style.font = "Times";
      spec.style.font_pt = 9.0;
      spec.style.line_width_pt = 1.6;
      spec.style.grid = true;
      spec.style.tick_font_scale = 1.0;
      spec.style.label_font_scale = 1.08;
      spec.style.title_font_scale = 1.15;
      spec.style.title_bold = true;
      spec.palette = ColorPalette::Grayscale;
      break;
    case StyleProfile::Tufte_Minimal:
      spec.style.font = "Helvetica";
      spec.style.font_pt = 12.5;
      spec.style.line_width_pt = 1.8;
      spec.style.point_size = 0.6;
      spec.style.grid = false;
      spec.style.tick_font_scale = 1.0;
      spec.style.label_font_scale = 1.30;
      spec.style.title_font_scale = 1.55;
      spec.style.title_bold = true;
      spec.palette = ColorPalette::Grayscale;
      break;
  }
}

}  // namespace gnuplotpp
