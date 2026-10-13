import argparse
import os
import re

HEADER = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                      "..", "..", "include", "angelscript.h")

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--num", action="store_true", help="Print numeric version")
    args = parser.parse_args()

    if args.num:
        regex = re.compile(r'^#define ANGELSCRIPT_VERSION\s+(\d+)')
    else:
        regex = re.compile(r'^#define ANGELSCRIPT_VERSION_STRING\s+"(\d+\.\d+\.\d+.*)"')

    with open(HEADER, "r") as fobj:
        for l in fobj:
            match = re.match(regex, l)
            if match is not None:
                print(match.group(1))
                return

    assert False, "Can't find version"

if __name__ == "__main__":
    main()