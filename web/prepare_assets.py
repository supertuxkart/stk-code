#!/usr/bin/env python3
"""Merge external STK assets with this revision's data without editing either.

Pass an extracted asset directory. Use the official full asset set matching the
game when available; the official 1.5-rc1 mobile LQ pack is a smaller test option.
"""
import argparse
from pathlib import Path
import shutil

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("assets", type=Path)
    parser.add_argument("--output", type=Path, default=ROOT / "build-web-tools/data")
    args = parser.parse_args()
    source = args.assets.resolve()
    if (source / "data/karts").is_dir():
        source /= "data"
    for name in ("karts", "tracks", "textures", "sfx", "music", "models", "library"):
        if not (source / name).is_dir():
            parser.error(f"Missing {source / name}; pass the directory containing the game assets.")
    output = args.output.resolve()
    if output == source or output in source.parents or source in output.parents:
        parser.error("Output must be separate from the source assets.")
    if output == ROOT or output in ROOT.parents or output == ROOT / "data":
        parser.error("Output must not overwrite repository source data.")
    # Merge external media first, then prefer code-owned XML/shaders/UI. Fresh
    # output prevents removed assets lingering from an earlier build.
    if output.exists():
        parser.error(f"{output} exists; choose a fresh --output or reuse it with build.py --assets.")
    shutil.copytree(source, output, ignore=shutil.ignore_patterns(".git", ".svn"))
    shutil.copytree(ROOT / "data", output, dirs_exist_ok=True,
                    ignore=shutil.ignore_patterns(".git", ".svn"))
    print(f"Prepared browser data: {output}")


if __name__ == "__main__":
    main()
