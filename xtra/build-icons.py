"""Build pgAdmin's SVG icons from Tabler Icons (https://tabler.io/icons, MIT).

Each pgAdmin icon (pgadmin/include/images/<name>.png) listed in ICONS gets a
coloured SVG in pgadmin/include/images/svg/<name>.svg. At build time png2c
prefers that SVG over the PNG, so the code keeps using <name>_png_img etc.
Icons that are not listed keep their original PNG.

Usage: python build-icons.py <tabler-icons-package-dir>
       (the "package" folder of the @tabler/icons npm tarball)
"""
import os
import re
import shutil
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
IMAGES = os.path.join(HERE, "..", "pgadmin", "include", "images")
OUT = os.path.join(IMAGES, "svg")

# Mid-tone colours, readable on both light and dark backgrounds
C = {
    "server": "#3B82F6", "database": "#F59E0B", "schema": "#8B5CF6",
    "table": "#0EA5E9", "view": "#14B8A6", "column": "#64748B",
    "key": "#EAB308", "constraint": "#22C55E", "index": "#10B981",
    "function": "#A855F7", "trigger": "#F97316", "role": "#EC4899",
    "type": "#06B6D4", "textsearch": "#84CC16", "foreign": "#0D9488",
    "agent": "#6366F1", "slony": "#0891B2", "neutral": "#6B7280",
    "ok": "#22C55E", "bad": "#EF4444", "warn": "#F59E0B", "off": "#9CA3AF",
}

# name: (tabler icon, colour key, badge)   badge: None | "x" | "sync"
ICONS = {}


def add(names, icon, colour, badge=None):
    for n in names.split():
        ICONS[n] = (icon, colour, badge)


# Servers, databases, schemas
add("server server-sm", "server", "server")
add("serverbad serverbad-sm", "server", "off")
add("servers", "server-2", "server")
add("database database-sm databases", "database", "database")
add("closeddatabase closeddatabase-sm", "database", "off")
add("baddatabase", "database", "database", "x")
add("tablespace tablespaces", "building-warehouse", "database")
add("namespace namespace-sm namespaces", "schema", "schema")
add("catalog catalog-sm catalogs", "books", "schema")
add("catalogobject catalogobject-sm catalogobjects", "book", "schema")
add("extension extension-sm extensions", "puzzle", "schema")
# Tables, views, columns
add("table table-sm tables", "table", "table")
add("table-repl table-repl-sm", "table", "table", "sync")
add("exttable exttable-sm exttables", "table-export", "table")
add("view view-sm views", "eye", "view")
add("mview mview-sm", "eye-table", "view")
add("column column-sm columns", "columns", "column")
# Constraints and indexes
add("constraints", "shield-check", "constraint")
add("check", "circle-check", "constraint")
add("checkbad", "circle-check", "constraint", "x")
add("exclude", "layers-subtract", "constraint")
add("unique", "circle-dot", "constraint")
add("primarykey key", "key", "key")
add("foreignkey", "link", "key")
add("foreignkeybad", "link", "key", "x")
add("index indexes", "list-search", "index")
# Rules and triggers
add("rule rules", "route", "trigger")
add("rulebad", "route", "trigger", "x")
add("trigger triggers", "bolt", "trigger")
add("triggerbad", "bolt", "trigger", "x")
add("triggerfunction triggerfunctions", "math-function", "trigger")
# Functions and friends
add("function functions", "math-function", "function")
add("procedure procedures", "terminal-2", "function")
add("aggregate aggregate-sm aggregates", "sum", "function")
add("operator operators", "math-symbols", "function")
add("operatorclass operatorclasses", "category", "function")
add("operatorfamily operatorfamilies", "category-2", "function")
add("package packages", "package", "function")
add("language language-sm languages", "code", "function")
# Types
add("type types", "typography", "type")
add("domain domain-sm domains", "box", "type")
add("enumeration enumerations", "list-numbers", "type")
add("cast cast-sm casts", "arrows-exchange", "type")
add("conversion conversions", "transfer", "type")
add("collation collation-sm collations", "sort-a-z", "type")
add("sequence sequences", "hash", "type")
add("sequence-repl", "hash", "type", "sync")
add("synonym synonyms", "tags", "type")
# Full text search
add("textsearch", "abc", "textsearch")
add("dictionary dictionaries", "vocabulary", "textsearch")
add("parser parsers", "text-recognition", "textsearch")
add("template templates", "template", "textsearch")
add("configuration configurations", "settings", "textsearch")
# Foreign data
add("foreigndatawrapper foreigndatawrapper-sm foreigndatawrappers", "plug", "foreign")
add("foreignserver foreignserver-sm foreignservers", "server-2", "foreign")
add("foreigntable foreigntables", "table-share", "foreign")
add("usermapping usermapping-sm usermappings", "user-shield", "foreign")
# Roles
add("user loginroles", "user", "role")
add("users roles", "users", "role")
add("group groups", "users-group", "role")
add("resourcegroup resourcegroups", "cpu", "agent")
# pgAgent
add("job jobs", "clock", "agent")
add("jobdisabled", "clock", "off")
add("step steps", "subtask", "agent")
add("schedule schedules", "calendar-time", "agent")
# Slony-I
add("slcluster slclusters", "topology-star-3", "slony")
add("slnode slnodes", "hierarchy", "slony")
add("slnode-local", "hierarchy", "ok")
add("slnode-disabled", "hierarchy", "off")
add("slpath slpaths", "route", "slony")
add("sllisten sllistens", "network", "slony")
add("slset slset2 slsets", "stack-2", "slony")
add("slsubscription slsubscription2 slsubscriptions", "download", "slony")
# Query tool
add("query_execute", "player-play", "ok")
add("query_execfile", "file-code", "ok")
add("query_pgscript", "code", "ok")
add("query_explain", "analyze", "warn")
add("query_cancel stop", "player-stop", "bad")
add("query_commit", "circle-check", "ok")
add("query_rollback", "arrow-back-up", "bad")
add("sql-16", "sql", "server")
add("readdata", "table", "table")
add("storedata", "device-floppy", "neutral")
# Debugger
add("debugger", "bug", "trigger")
add("continue", "player-play", "ok")
add("stepInto", "step-into", "neutral")
add("stepOver", "player-track-next", "neutral")
add("setBreak", "circle-dot", "bad")
add("variable variables", "variable", "neutral")
# Editing and files
add("file_new", "file-plus", "neutral")
add("file_open folder", "folder-open", "neutral")
add("file_save", "device-floppy", "neutral")
add("clip_copy", "copy", "neutral")
add("clip_cut", "scissors", "neutral")
add("clip_paste", "clipboard", "neutral")
add("edit_undo", "arrow-back-up", "neutral")
add("edit_redo", "arrow-forward-up", "neutral")
add("edit_find", "search", "neutral")
add("edit_clear clearAll", "eraser", "neutral")
add("sortfilter", "filter", "neutral")
add("delete", "trash", "bad")
add("ddcancel", "x", "bad")
add("ddnewdiagram", "file-plus", "neutral")
add("ddgendiagram", "file-code", "neutral")
# Misc
add("backup", "database-export", "neutral")
add("restore", "database-import", "neutral")
add("statistics", "chart-bar", "neutral")
add("property", "list-details", "neutral")
add("favourite", "star", "warn")
add("help", "help-circle", "server")
add("hint2", "bulb", "warn")
add("terminate_backend", "hand-stop", "bad")
add("checked", "square-check", "neutral")
add("unchecked uncheck gqbColNotSel", "square", "neutral")
add("gqbColSel", "square-check", "ok")
add("disabled", "circle-x", "off")
# Toolbar (32 px) and navigation (24 px)
add("connect", "plug-connected", "server")
add("create", "square-plus", "ok")
add("drop", "trash", "bad")
add("refresh reload", "refresh", "server")
add("properties", "adjustments", "neutral")
add("sql-32", "sql", "server")
add("viewdata", "table", "table")
add("viewfiltereddata", "filter", "table")
add("vacuum", "tool", "neutral")
add("plugins", "puzzle", "schema")
add("help2", "help-circle", "server")
add("hint", "bulb", "warn")
add("ddmodel-32", "schema", "schema")
add("back", "arrow-left", "neutral")
add("forward", "arrow-right", "neutral")

BADGES = {
    "x": '<circle cx="18" cy="18" r="5.5" fill="#EF4444" stroke="none" />'
         '<path d="M16 16l4 4M20 16l-4 4" stroke="#FFFFFF" stroke-width="1.6" fill="none" />',
    "sync": '<circle cx="18" cy="18" r="5.5" fill="#06B6D4" stroke="none" />'
            '<circle cx="18" cy="18" r="2" fill="#FFFFFF" stroke="none" />',
}


def png_size(name):
    with open(os.path.join(IMAGES, name + ".png"), "rb") as f:
        return struct.unpack(">II", f.read(24)[16:24])


def build(tabler_dir):
    outline = os.path.join(tabler_dir, "icons", "outline")
    os.makedirs(OUT, exist_ok=True)
    for old in os.listdir(OUT):
        if old.endswith(".svg"):
            os.remove(os.path.join(OUT, old))
    missing = []
    for name, (icon, colour, badge) in sorted(ICONS.items()):
        if not os.path.exists(os.path.join(IMAGES, name + ".png")):
            missing.append("pgAdmin:" + name)
            continue
        src = os.path.join(outline, icon + ".svg")
        if not os.path.exists(src):
            missing.append("tabler:" + icon)
            continue
        w, h = png_size(name)
        with open(src, encoding="utf-8") as f:
            svg = f.read()
        body = svg[svg.index(">", svg.index("<svg")) + 1:svg.rindex("</svg>")]
        body = "".join(line.strip() for line in body.splitlines())
        # thinner strokes look better at larger sizes; 16 px needs some weight
        stroke = "1.75" if max(w, h) <= 16 else "1.5"
        out = ('<svg xmlns="http://www.w3.org/2000/svg" width="%d" height="%d" '
               'viewBox="0 0 24 24" fill="none" stroke="%s" stroke-width="%s" '
               'stroke-linecap="round" stroke-linejoin="round">%s%s</svg>\n'
               % (w, h, C[colour], stroke, body, BADGES.get(badge, "")))
        out = out.replace("currentColor", C[colour])
        with open(os.path.join(OUT, name + ".svg"), "w", encoding="utf-8", newline="\n") as f:
            f.write(out)
    shutil.copyfile(os.path.join(tabler_dir, "LICENSE"), os.path.join(OUT, "LICENSE-tabler-icons.txt"))
    print("%d icons written to %s" % (len(ICONS) - len(missing), os.path.normpath(OUT)))
    if missing:
        print("missing:", " ".join(missing))
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(build(sys.argv[1]))
