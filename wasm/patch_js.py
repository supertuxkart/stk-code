import re
import sys
import pathlib

match_regex = r'/\* (.+?)\n(.+?)\n\*/\n(.*?)(\n\n|$)'

fragments_dir = sys.argv[1]
target_dir = sys.argv[2]
fragments_path = pathlib.Path(fragments_dir)
target_path = pathlib.Path(target_dir)
target_text = target_path.read_text()

for fragment_file in fragments_path.iterdir():
  print(f"applying patch from {fragment_file.name}")
  fragment_text = fragment_file.read_text()
  matches = re.findall(match_regex, fragment_text, re.S)

  for mode, patch_regex, patch_text, _ in matches:
    fragment_matches = re.findall(patch_regex, target_text)
    if not fragment_matches:
      print(f"warning: regex did not match anything for '{patch_regex}'");
    if mode == "DELETE":
      target_text = re.sub(patch_regex, "", target_text)
    elif mode == "REPLACE":
      target_text = re.sub(patch_regex, patch_text, target_text)
    elif mode == "INSERT":
      target_text = re.sub("("+patch_regex+")", r'\1'+patch_text, target_text)
    elif mode == "INSERT_BEFORE":
      target_text = re.sub("("+patch_regex+")", patch_text+r'\1', target_text)

target_path.write_text(target_text)