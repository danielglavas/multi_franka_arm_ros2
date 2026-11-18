#!/usr/bin/env python3
"""Claude Code PostToolUse hook: checks a file right after Claude edited it.

Claude Code passes a JSON description of the tool call on stdin. We read the edited
file's path from it, run cheap checks, and report problems on stderr with exit code 2,
which makes Claude Code show the message to Claude so it fixes the file.
Exit code 0 (silent) means the file looks fine or is not a file we check.
"""
import json
import re
import sys
import xml.etree.ElementTree as ElementTree
from pathlib import Path

import yaml

# File endings checked by each parser.
XML_SUFFIXES = {".xml", ".urdf", ".xacro", ".sdf", ".launch"}
YAML_SUFFIXES = {".yaml", ".yml"}
# A private key header, e.g. "-----BEGIN OPENSSH PRIVATE KEY-----".
PRIVATE_KEY_PATTERN = re.compile(r"-----BEGIN [A-Z ]*PRIVATE KEY-----")


def find_problems(file_path: Path, text: str) -> list:
    """Return a list of human-readable problems found in the file's text."""
    problems = []
    if text and not text.endswith("\n"):
        problems.append("no line break at the end of the file")
    if PRIVATE_KEY_PATTERN.search(text):
        problems.append("contains a private key; it must not be in the repo")
    suffix = file_path.suffix.lower()
    try:
        if suffix == ".json":
            json.loads(text)
        elif suffix in YAML_SUFFIXES:
            list(yaml.safe_load_all(text))  # list() forces every document to be parsed
        elif suffix in XML_SUFFIXES:
            ElementTree.fromstring(text)
    except Exception as parse_error:  # any parser error means the file is broken
        problems.append(f"does not parse as {suffix}: {parse_error}")
    return problems


def main() -> int:
    """Read the hook input, check the edited file, return the exit code."""
    hook_input = json.load(sys.stdin)
    edited_path = hook_input.get("tool_input", {}).get("file_path")
    if not edited_path or not Path(edited_path).is_file():
        return 0  # nothing to check (e.g. the file was not written)
    try:
        text = Path(edited_path).read_text(encoding="utf-8")
    except UnicodeDecodeError:
        return 0  # binary file such as a mesh or an image: skip
    problems = find_problems(Path(edited_path), text)
    if not problems:
        return 0
    for problem in problems:
        print(f"{edited_path}: {problem}", file=sys.stderr)
    return 2  # exit code 2: Claude Code feeds stderr back to Claude


if __name__ == "__main__":
    sys.exit(main())
