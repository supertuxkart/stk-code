# STKGUI Typed Widget Accessors

## Overview

Auto-generate type-safe C++ headers from `.stkgui` XML files to catch widget access errors at compile time instead of runtime.

## Problem

Current code uses stringly-typed widget access:
```cpp
getWidget<CheckBoxWidget>("music_enabled")  // Typo? Wrong type? Runtime crash.
```

## Solution

Generate cached, typed widget structs from XML:
```cpp
struct OptionsAudioWidgets {
    CheckBoxWidget* music_enabled = nullptr;
    SpinnerWidget* music_volume = nullptr;

    void bind(GUIEngine::Screen* s) {
        music_enabled = s->getWidget<CheckBoxWidget>("music_enabled");
        music_volume = s->getWidget<SpinnerWidget>("music_volume");
    }
};
```

## Design Decisions

### Approach: Cached Bind-Once
- XML still parsed at runtime (modders can edit `.stkgui` files)
- Generated struct caches widget pointers after `bind()` call
- Compile-time type safety + one-time lookup cost

### Generation Timing
- `execute_process()` at CMake configure time (headers exist for IDE)
- `add_custom_target()` at build time (regenerates if `.stkgui` changes)
- Generated headers committed to repo as fallback

### File Organization
- Input: `data/gui/screens/options/options_audio.stkgui`
- Output: `src/generated/gui/screens/options/options_audio_widgets.hpp`
- Struct: `OptionsAudioWidgets`

### Widget Type Mapping
| XML Element | C++ Widget Type | Header |
|-------------|-----------------|--------|
| `checkbox` | `CheckBoxWidget` | `check_box_widget.hpp` |
| `spinner` | `SpinnerWidget` | `spinner_widget.hpp` |
| `gauge` | `SpinnerWidget` | `spinner_widget.hpp` |
| `button` | `ButtonWidget` | `button_widget.hpp` |
| `icon-button` | `IconButtonWidget` | `icon_button_widget.hpp` |
| `icon` | `IconButtonWidget` | `icon_button_widget.hpp` |
| `ribbon` | `RibbonWidget` | `ribbon_widget.hpp` |
| `buttonbar` | `RibbonWidget` | `ribbon_widget.hpp` |
| `tabs` | `RibbonWidget` | `ribbon_widget.hpp` |
| `vertical-tabs` | `RibbonWidget` | `ribbon_widget.hpp` |
| `label` | `LabelWidget` | `label_widget.hpp` |
| `bright` | `LabelWidget` | `label_widget.hpp` |
| `header` | `LabelWidget` | `label_widget.hpp` |
| `small-header` | `LabelWidget` | `label_widget.hpp` |
| `tiny-header` | `LabelWidget` | `label_widget.hpp` |
| `bubble` | `BubbleWidget` | `bubble_widget.hpp` |
| `list` | `ListWidget` | `list_widget.hpp` |
| `textbox` | `TextBoxWidget` | `text_box_widget.hpp` |
| `model` | `ModelViewWidget` | `model_view_widget.hpp` |
| `progressbar` | `ProgressBarWidget` | `progress_bar_widget.hpp` |
| `ratingbar` | `RatingBarWidget` | `rating_bar_widget.hpp` |
| `ribbon_grid` | `DynamicRibbonWidget` | `dynamic_ribbon_widget.hpp` |
| `scrollable_ribbon` | `DynamicRibbonWidget` | `dynamic_ribbon_widget.hpp` |
| `scrollable_toolbar` | `DynamicRibbonWidget` | `dynamic_ribbon_widget.hpp` |

### Elements Skipped (no widget generated)
- `div`, `box`, `spacer` - layout containers, rarely accessed by id
- Elements without `id` attribute

## Usage Example

```cpp
// In screen header
#include "generated/gui/screens/options/options_audio_widgets.hpp"

class OptionsScreenAudio : public Screen {
    OptionsAudioWidgets m_widgets;

    void init() override {
        m_widgets.bind(this);
        m_widgets.music_enabled->setState(UserConfigParams::m_music);
    }

    void eventCallback(Widget* w) override {
        if (w == m_widgets.music_enabled) {
            // handle toggle
        }
    }
};
```

## Files

- `tools/generate_gui_headers.py` - Generator script
- `src/generated/gui/**/*_widgets.hpp` - Generated headers
- `CMakeLists.txt` - Build integration
