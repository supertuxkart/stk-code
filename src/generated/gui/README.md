# Generated GUI Widget Headers

This directory contains auto-generated C++ headers that provide type-safe access to widgets defined in `.stkgui` XML files.

## Why?

Without these headers, widget access uses string IDs and runtime type casting:

```cpp
// Old way - typos and wrong types are runtime crashes
SpinnerWidget* volume = getWidget<SpinnerWidget>("music_volme");  // typo!
CheckBoxWidget* cb = getWidget<CheckBoxWidget>("music_volume");   // wrong type!
```

With generated headers, the compiler catches these errors:

```cpp
// New way - compile-time safety
m_widgets.music_volme;  // Compiler error: no member named 'music_volme'
```

## Usage

### 1. Include the generated header

```cpp
#include "generated/gui/screens/options/options_audio_widgets.hpp"
```

### 2. Add a member variable

```cpp
class OptionsScreenAudio : public Screen {
private:
    GUIEngine::OptionsAudioWidgets m_widgets;
};
```

### 3. Bind widgets in init()

```cpp
void OptionsScreenAudio::init() {
    Screen::init();
    m_widgets.bind(this);  // One-time lookup

    // Now use typed members directly
    m_widgets.music_enabled->setState(true);
    m_widgets.music_volume->setValue(5);
}
```

### 4. Use pointer comparison in callbacks

```cpp
void OptionsScreenAudio::eventCallback(Widget* widget, ...) {
    if (widget == m_widgets.music_enabled) {
        // Handle music toggle
    }
}
```

## Regenerating

Headers are regenerated automatically when you run `cmake ..`.

To regenerate manually:

```bash
python3 tools/generate_gui_headers.py
```

## File Mapping

| Source | Generated |
|--------|-----------|
| `data/gui/screens/options/options_audio.stkgui` | `src/generated/gui/screens/options/options_audio_widgets.hpp` |
| `data/gui/dialogs/confirm_dialog.stkgui` | `src/generated/gui/dialogs/confirm_dialog_widgets.hpp` |

## Reserved Keywords

Widget IDs that are C++ keywords (like `new`, `delete`, `class`) are escaped with a trailing underscore:

| XML ID | C++ Member |
|--------|------------|
| `new` | `new_` |
| `delete` | `delete_` |
| `class` | `class_` |

## Do Not Edit

These files are auto-generated. Any manual changes will be overwritten.

To modify widget definitions, edit the corresponding `.stkgui` file in `data/gui/`.
