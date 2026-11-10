"""Remove sizer flag combinations rejected by wxWidgets 3.x from XRC files.

wxWidgets >= 3.0 reports "wxALIGN_xxx has no effect when combined with wxEXPAND"
and "wxALIGN_xxx can't be used in a vertical/horizontal sizer". In wx 2.8 these
flags were silently ignored, so removing them does not change the layout.

Usage: python fix-xrc-flags.py <dir-with-xrc-files>
"""
import glob
import os
import re
import sys
import xml.etree.ElementTree as ET

ALIGN_H = {"wxALIGN_RIGHT", "wxALIGN_CENTRE_HORIZONTAL", "wxALIGN_CENTER_HORIZONTAL", "wxALIGN_LEFT"}
ALIGN_V = {"wxALIGN_BOTTOM", "wxALIGN_CENTRE_VERTICAL", "wxALIGN_CENTER_VERTICAL", "wxALIGN_TOP"}
CENTRE = {"wxALIGN_CENTRE", "wxALIGN_CENTER"}
BOX_SIZERS = {"wxBoxSizer", "wxStaticBoxSizer"}


def local(tag):
    return tag.split("}", 1)[-1]


def orientation(sizer):
    if sizer is None:
        return None
    cls = sizer.get("class")
    if cls == "wxStdDialogButtonSizer":
        return "wxHORIZONTAL"
    if cls not in BOX_SIZERS:
        return None
    for child in sizer:
        if local(child.tag) == "orient":
            return (child.text or "").strip()
    return "wxHORIZONTAL"


def fix_flags(flags, orient):
    tokens = [t.strip() for t in flags.split("|") if t.strip()]
    if "wxEXPAND" in tokens or "wxGROW" in tokens:
        tokens = [t for t in tokens if not t.startswith("wxALIGN_")]
    elif orient == "wxVERTICAL":
        tokens = ["wxALIGN_CENTRE_HORIZONTAL" if t in CENTRE else t for t in tokens]
        tokens = [t for t in tokens if t not in ALIGN_V]
    elif orient == "wxHORIZONTAL":
        tokens = ["wxALIGN_CENTRE_VERTICAL" if t in CENTRE else t for t in tokens]
        tokens = [t for t in tokens if t not in ALIGN_H]
    # keep a stable, de-duplicated order
    seen, out = set(), []
    for t in tokens:
        if t not in seen:
            seen.add(t)
            out.append(t)
    return "|".join(out) if out else "0"


def new_flag_values(path):
    root = ET.parse(path).getroot()
    parents = {child: parent for parent in root.iter() for child in parent}
    values = []
    for elem in root.iter():
        if local(elem.tag) != "flag":
            continue
        item = parents.get(elem)
        sizer = parents.get(item) if item is not None else None
        if item is not None and item.get("class") in ("sizeritem", "spacer"):
            values.append(fix_flags(elem.text or "", orientation(sizer)))
        else:
            values.append(None)  # not a sizer flag: leave untouched
    return values


def main(directory):
    changed_total = 0
    for path in sorted(glob.glob(os.path.join(directory, "*.xrc"))):
        values = iter(new_flag_values(path))
        with open(path, encoding="utf-8", newline="") as f:
            text = f.read()
        changed = 0

        def repl(m):
            nonlocal changed
            new = next(values)
            if new is None or new == m.group(2).strip():
                return m.group(0)
            changed += 1
            return m.group(1) + new + m.group(3)

        text = re.sub(r"(<flag>)(.*?)(</flag>)", repl, text, flags=re.S)
        if changed:
            with open(path, "w", encoding="utf-8", newline="") as f:
                f.write(text)
            print(f"{os.path.basename(path)}: {changed} flags fixed")
            changed_total += changed
    print(f"total: {changed_total}")


if __name__ == "__main__":
    main(sys.argv[1])
