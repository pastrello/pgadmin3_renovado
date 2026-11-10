"""Minimal gettext .po -> .mo compiler (no GNU gettext needed on Windows).

Supports msgctxt, plural forms and skips fuzzy and obsolete entries, like
GNU msgfmt's default behaviour.

Usage: python msgfmt.py input.po output.mo
       python msgfmt.py --dump file.mo      (prints the catalogue, for checks)
"""
import ast
import struct
import sys


def parse_po(path):
    entries = {}
    with open(path, encoding="utf-8") as f:
        lines = f.read().splitlines()
    cur, section, fuzzy = {}, None, False

    def flush():
        nonlocal cur, fuzzy
        if "msgid" in cur and not fuzzy:
            msgid = cur["msgid"]
            if "msgctxt" in cur:
                msgid = cur["msgctxt"] + "\x04" + msgid
            if "msgid_plural" in cur:
                msgid += "\x00" + cur["msgid_plural"]
                msgstr = "\x00".join(cur[k] for k in sorted(k for k in cur if k.startswith("msgstr[")))
            else:
                msgstr = cur.get("msgstr", "")
            # untranslated entries are not written (the header is kept)
            if msgstr.replace("\x00", "") or msgid == "":
                entries[msgid] = msgstr
        cur, fuzzy = {}, False

    for line in lines:
        line = line.strip()
        if line.startswith("#~"):
            continue
        if line.startswith("#,") and "fuzzy" in line:
            if "msgid" in cur:
                flush()
            fuzzy = True
            continue
        if not line or line.startswith("#"):
            if "msgstr" in cur or any(k.startswith("msgstr[") for k in cur):
                flush()
            elif not line and "msgid" not in cur:
                # a blank line ends an entry: a "#, fuzzy" that only applied
                # to obsolete (#~) lines must not leak into the next entry
                fuzzy = False
            continue
        if line.startswith('"'):
            cur[section] += ast.literal_eval(line)
            continue
        key, _, value = line.partition(" ")
        if key in ("msgctxt", "msgid") and ("msgstr" in cur or any(k.startswith("msgstr[") for k in cur)):
            flush()
        section = key
        cur[key] = ast.literal_eval(value)
    flush()
    return entries


def write_mo(entries, path):
    keys = sorted(k.encode("utf-8") for k in entries)
    values = [entries[k.decode("utf-8")].encode("utf-8") for k in keys]
    n = len(keys)
    header_size = 7 * 4
    orig_tab = header_size
    trans_tab = orig_tab + n * 8
    data_start = trans_tab + n * 8
    ids = b"".join(k + b"\x00" for k in keys)
    strs = b"".join(v + b"\x00" for v in values)
    orig, trans, off = [], [], data_start
    for k in keys:
        orig.append((len(k), off))
        off += len(k) + 1
    for v in values:
        trans.append((len(v), off))
        off += len(v) + 1
    out = struct.pack("<7I", 0x950412DE, 0, n, orig_tab, trans_tab, 0, 0)
    out += b"".join(struct.pack("<2I", *t) for t in orig)
    out += b"".join(struct.pack("<2I", *t) for t in trans)
    out += ids + strs
    with open(path, "wb") as f:
        f.write(out)


def read_mo(path):
    data = open(path, "rb").read()
    magic, _, n, o, t = struct.unpack("<5I", data[:20])
    assert magic == 0x950412DE
    cat = {}
    for i in range(n):
        ln, off = struct.unpack("<2I", data[o + 8 * i:o + 8 * i + 8])
        tl, toff = struct.unpack("<2I", data[t + 8 * i:t + 8 * i + 8])
        cat[data[off:off + ln].decode("utf-8")] = data[toff:toff + tl].decode("utf-8")
    return cat


if __name__ == "__main__":
    if sys.argv[1] == "--dump":
        for k, v in sorted(read_mo(sys.argv[2]).items()):
            print(repr(k), "=>", repr(v))
    else:
        write_mo(parse_po(sys.argv[1]), sys.argv[2])
