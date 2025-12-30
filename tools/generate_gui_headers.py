#!/usr/bin/env python3
"""
Generate type-safe C++ widget accessor headers from .stkgui XML files.

Usage:
    python generate_gui_headers.py [--output-dir DIR] [--gui-dir DIR]

This script scans all .stkgui files in the data/gui directory and generates
corresponding C++ headers with typed widget accessor structs.

Example:
    data/gui/screens/options/options_audio.stkgui
    ->
    src/generated/gui/screens/options/options_audio_widgets.hpp

The generated struct provides:
    - Typed widget pointers (e.g., CheckBoxWidget* music_enabled)
    - A bind(Screen*) method to populate pointers from the screen
    - Compile-time safety for widget access (no more string typos)

Integration:
    - Runs automatically during CMake configure (execute_process)
    - Can be run manually: python3 tools/generate_gui_headers.py
    - Generated headers are committed to repo for IDE support

See also:
    - src/generated/gui/README.md for usage documentation
    - docs/plans/2025-01-30-stkgui-typed-widgets-design.md for design rationale
"""

import argparse
import os
import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path
from typing import Dict, List, Optional, Set, Tuple

# Mapping from XML element names to (C++ class name, header file)
WIDGET_TYPE_MAP: Dict[str, Tuple[str, str]] = {
    # Checkbox
    "checkbox": ("CheckBoxWidget", "check_box_widget.hpp"),
    # Spinners
    "spinner": ("SpinnerWidget", "spinner_widget.hpp"),
    "gauge": ("SpinnerWidget", "spinner_widget.hpp"),
    # Buttons
    "button": ("ButtonWidget", "button_widget.hpp"),
    "icon-button": ("IconButtonWidget", "icon_button_widget.hpp"),
    "icon": ("IconButtonWidget", "icon_button_widget.hpp"),
    # Ribbons
    "ribbon": ("RibbonWidget", "ribbon_widget.hpp"),
    "buttonbar": ("RibbonWidget", "ribbon_widget.hpp"),
    "tabs": ("RibbonWidget", "ribbon_widget.hpp"),
    "vertical-tabs": ("RibbonWidget", "ribbon_widget.hpp"),
    # Labels
    "label": ("LabelWidget", "label_widget.hpp"),
    "bright": ("LabelWidget", "label_widget.hpp"),
    "header": ("LabelWidget", "label_widget.hpp"),
    "small-header": ("LabelWidget", "label_widget.hpp"),
    "tiny-header": ("LabelWidget", "label_widget.hpp"),
    # Other widgets
    "bubble": ("BubbleWidget", "bubble_widget.hpp"),
    "list": ("ListWidget", "list_widget.hpp"),
    "textbox": ("TextBoxWidget", "text_box_widget.hpp"),
    "model": ("ModelViewWidget", "model_view_widget.hpp"),
    "progressbar": ("ProgressBarWidget", "progress_bar_widget.hpp"),
    "ratingbar": ("RatingBarWidget", "rating_bar_widget.hpp"),
    # Dynamic ribbons
    "ribbon_grid": ("DynamicRibbonWidget", "dynamic_ribbon_widget.hpp"),
    "scrollable_ribbon": ("DynamicRibbonWidget", "dynamic_ribbon_widget.hpp"),
    "scrollable_toolbar": ("DynamicRibbonWidget", "dynamic_ribbon_widget.hpp"),
}

# Elements that are layout containers, not widgets to expose
SKIP_ELEMENTS = {"div", "box", "spacer", "stkgui"}

# C++ reserved keywords that can't be used as identifiers
CPP_KEYWORDS = {
    "alignas", "alignof", "and", "and_eq", "asm", "auto", "bitand", "bitor",
    "bool", "break", "case", "catch", "char", "char8_t", "char16_t", "char32_t",
    "class", "compl", "concept", "const", "consteval", "constexpr", "constinit",
    "const_cast", "continue", "co_await", "co_return", "co_yield", "decltype",
    "default", "delete", "do", "double", "dynamic_cast", "else", "enum",
    "explicit", "export", "extern", "false", "float", "for", "friend", "goto",
    "if", "inline", "int", "long", "mutable", "namespace", "new", "noexcept",
    "not", "not_eq", "nullptr", "operator", "or", "or_eq", "private", "protected",
    "public", "register", "reinterpret_cast", "requires", "return", "short",
    "signed", "sizeof", "static", "static_assert", "static_cast", "struct",
    "switch", "template", "this", "thread_local", "throw", "true", "try",
    "typedef", "typeid", "typename", "union", "unsigned", "using", "virtual",
    "void", "volatile", "wchar_t", "while", "xor", "xor_eq",
}


def snake_to_pascal(name: str) -> str:
    """Convert snake_case or kebab-case to PascalCase."""
    # Replace hyphens with underscores, then convert
    name = name.replace("-", "_")
    return "".join(word.capitalize() for word in name.split("_"))


def make_safe_identifier(name: str) -> str:
    """Convert widget ID to a safe C++ identifier."""
    # Replace hyphens with underscores
    safe_name = name.replace("-", "_")
    # Append suffix if it's a C++ keyword
    if safe_name in CPP_KEYWORDS:
        safe_name = safe_name + "_"
    return safe_name


def collect_widgets(element: ET.Element) -> List[Tuple[str, str, str]]:
    """
    Recursively collect all widgets with id attributes.
    Returns list of (id, xml_element_name, cpp_type).
    """
    widgets = []

    tag = element.tag.lower()
    widget_id = element.get("id")

    if widget_id and tag in WIDGET_TYPE_MAP:
        cpp_type, _ = WIDGET_TYPE_MAP[tag]
        widgets.append((widget_id, tag, cpp_type))

    # Recurse into children
    for child in element:
        widgets.extend(collect_widgets(child))

    return widgets


def get_required_headers(widgets: List[Tuple[str, str, str]]) -> Set[str]:
    """Get the set of header files needed for the widget types used."""
    headers = set()
    for _, xml_tag, _ in widgets:
        if xml_tag in WIDGET_TYPE_MAP:
            _, header = WIDGET_TYPE_MAP[xml_tag]
            headers.add(header)
    return headers


def generate_header(
    stkgui_path: Path,
    output_path: Path,
    relative_stkgui: str
) -> bool:
    """
    Generate a C++ header file for the given .stkgui file.
    Returns True if successful, False otherwise.
    """
    try:
        tree = ET.parse(stkgui_path)
        root = tree.getroot()
    except ET.ParseError as e:
        print(f"Warning: Failed to parse {stkgui_path}: {e}", file=sys.stderr)
        return False

    widgets = collect_widgets(root)

    if not widgets:
        # No widgets with IDs found, skip generating header
        return True

    # Derive struct name from filename
    stem = stkgui_path.stem  # e.g., "options_audio"
    struct_name = snake_to_pascal(stem) + "Widgets"

    # Get required headers
    required_headers = get_required_headers(widgets)

    # Generate the header content
    lines = [
        "// Auto-generated from " + relative_stkgui,
        "// Do not edit manually - regenerate with tools/generate_gui_headers.py",
        "#pragma once",
        "",
        '#include "guiengine/screen.hpp"',
    ]

    # Add widget headers
    for header in sorted(required_headers):
        lines.append(f'#include "guiengine/widgets/{header}"')

    lines.extend([
        "",
        "namespace GUIEngine {",
        "",
        f"struct {struct_name}",
        "{",
    ])

    # Add member declarations
    for widget_id, _, cpp_type in widgets:
        member_name = make_safe_identifier(widget_id)
        lines.append(f"    {cpp_type}* {member_name} = nullptr;")

    lines.extend([
        "",
        "    void bind(Screen* screen)",
        "    {",
    ])

    # Add bind statements
    for widget_id, _, cpp_type in widgets:
        member_name = make_safe_identifier(widget_id)
        lines.append(f'        {member_name} = screen->getWidget<{cpp_type}>("{widget_id}");')

    lines.extend([
        "    }",
        "};",
        "",
        "}  // namespace GUIEngine",
        "",
    ])

    # Write the header file
    output_path.parent.mkdir(parents=True, exist_ok=True)

    content = "\n".join(lines)

    # Only write if content changed (avoid unnecessary rebuilds)
    if output_path.exists():
        existing = output_path.read_text()
        if existing == content:
            return True

    output_path.write_text(content)
    print(f"Generated: {output_path}")
    return True


def main():
    parser = argparse.ArgumentParser(
        description="Generate type-safe C++ widget accessor headers from .stkgui files"
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=None,
        help="Output directory for generated headers (default: src/generated/gui)"
    )
    parser.add_argument(
        "--gui-dir",
        type=Path,
        default=None,
        help="Input directory containing .stkgui files (default: data/gui)"
    )
    args = parser.parse_args()

    # Determine project root (script is in tools/)
    script_dir = Path(__file__).parent
    project_root = script_dir.parent

    gui_dir = args.gui_dir or (project_root / "data" / "gui")
    output_dir = args.output_dir or (project_root / "src" / "generated" / "gui")

    if not gui_dir.exists():
        print(f"Error: GUI directory not found: {gui_dir}", file=sys.stderr)
        return 1

    # Find all .stkgui files
    stkgui_files = list(gui_dir.rglob("*.stkgui"))

    if not stkgui_files:
        print(f"Warning: No .stkgui files found in {gui_dir}", file=sys.stderr)
        return 0

    print(f"Processing {len(stkgui_files)} .stkgui files...")

    success_count = 0
    skip_count = 0

    for stkgui_path in sorted(stkgui_files):
        # Compute relative path from gui_dir
        relative = stkgui_path.relative_to(gui_dir)

        # Compute output path: screens/options/options_audio.stkgui -> screens/options/options_audio_widgets.hpp
        output_path = output_dir / relative.parent / (relative.stem + "_widgets.hpp")

        # Relative path for the comment in generated file
        relative_stkgui = str(relative)

        if generate_header(stkgui_path, output_path, relative_stkgui):
            success_count += 1
        else:
            skip_count += 1

    print(f"Done: {success_count} generated, {skip_count} skipped")
    return 0


if __name__ == "__main__":
    sys.exit(main())
