#!/usr/bin/env python3
from __future__ import annotations

import re
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
MAIN_PATTERN = re.compile(r"(?m)^int\s+main\s*\(")
USING_ALIAS_PATTERN = re.compile(r"^using\s+\w+\s*=")

LIBRARIES = {
    "dfs": "DFS/dfs.cpp",
    "dij": "Dijkstra/dijksura.cpp",
    "uf": "UnionFind/nomal_UF.cpp",
    "ruf": "UnionFind/rollback_UF.cpp",
    "scc": "SCC/scc.cpp",
    "twosat": "TwoSat/two_sat.cpp",
    "seg": "SegmentTree/segtree.cpp",
    "lseg": "SegmentTree/lazySegTree.cpp",
    "pseg": "SegmentTree/persistentSegTree.cpp",
    "plseg": "SegmentTree/persistentLazySegTree.cpp",
    "treap": "Treap/implicitTreap.cpp",
    "trie": "Trie/trie.cpp",
    "btrie": "Trie/binaryTrie.cpp",
    "aho": "AhoCorasick/ahoCorasick.cpp",
    "bit": "BIT/fenwickTree.cpp",
    "mf": "Flow/maxflow.cpp",
    "dll": "DoublyLinkedList/dll.cpp",
    "era": "prime/Eratosthenes.cpp",
    "gcd": "GCD/gcd.cpp",
    "lcm": "LCM/lcm.cpp",
    "dig": "Digit/digit.cpp",
    "lca": "LCA/lca.cpp",
    "hld": "HLD/hld.cpp",
    "reroot": "Rerooting/rerooting.cpp",
    "rh": "RollingHash/rollingHash.cpp",
    "zh": "ZobristHash/zobrist_hash.cpp",
    "mo": "Mo/mo.cpp",
    "mp": "ModPow/modpow.cpp",
    "mpd": "ModPow/division.cpp",
    "comb": "combinatorics/combinatorics.cpp",
    "comp": "Compress/compress.cpp",
    "rs": "RangeSet/rangeSet.cpp",
    "grid": "Grid/grid.cpp",
    "mat": "Matrix/matrix.cpp",
    "geom": "Geometry/geometry.cpp",
}


def normalize_name(raw: str) -> str:
    return raw.lower()


def print_usage() -> None:
    print("usage: lib NAME [NAME ...]")
    print("       lib --list")
    print()
    print("examples:")
    print("  lib seg")
    print("  lib UF")


def print_library_list() -> None:
    for name in sorted(LIBRARIES):
        print(f"{name:7} {LIBRARIES[name]}")


def strip_common_prologue(source: str) -> str:
    lines = source.splitlines()
    while lines and lines[0] in {"#include <bits/stdc++.h>", "using namespace std;"}:
        lines.pop(0)
        while lines and lines[0] == "":
            lines.pop(0)
    return "\n".join(lines).strip() + "\n"


def library_path(name: str) -> Path:
    return ROOT / "my-library" / LIBRARIES[name]


def library_body(name: str) -> str:
    path = library_path(name)
    if not path.is_file():
        raise FileNotFoundError(f"lib: library file not found for {name}: {path}")

    body = strip_common_prologue(path.read_text(encoding="utf-8"))
    if not body.strip():
        raise ValueError(f"lib: library file is empty for {name}: {path}")

    return body.rstrip() + "\n"


def is_prologue_line(line: str) -> bool:
    stripped = line.strip()
    return stripped.startswith("#include ") or stripped.startswith("#define ") or stripped.startswith("using ")


def comparable_prologue_line(line: str) -> str:
    stripped = line.strip()
    if USING_ALIAS_PATTERN.match(stripped):
        return stripped.replace("atcoder::", "")
    return stripped


def starts_with_prologue(text: str) -> bool:
    for line in text.splitlines():
        if line.strip():
            return is_prologue_line(line)
    return False


def remove_duplicate_prologue_lines(body: str, source: str) -> str:
    source_lines = {comparable_prologue_line(line) for line in source.splitlines()}
    lines = body.splitlines()
    index = 0

    while index < len(lines):
        line = lines[index]
        if not line.strip():
            index += 1
            continue
        if not is_prologue_line(line):
            break
        if comparable_prologue_line(line) in source_lines:
            lines.pop(index)
            while index < len(lines) and not lines[index].strip():
                lines.pop(index)
            continue
        index += 1

    return "\n".join(lines).strip() + "\n"


def insert_text(path: Path, offset: int, text: str) -> None:
    with path.open("r+", encoding="utf-8") as file:
        file.seek(offset)
        suffix = file.read().lstrip("\n")
        file.seek(offset)
        file.write(text)
        file.write(suffix)
        file.truncate()


def insert_libraries(main_cpp: Path, names: list[str]) -> int:
    if not main_cpp.is_file():
        print("lib: main.cpp not found in current directory", file=sys.stderr)
        return 1

    source = main_cpp.read_text(encoding="utf-8")
    match = MAIN_PATTERN.search(source)
    if match is None:
        print("lib: int main(...) not found in main.cpp", file=sys.stderr)
        return 1

    blocks: list[str] = []
    for name in names:
        body = remove_duplicate_prologue_lines(library_body(name), source)
        if body.rstrip() in source:
            print(f"already inserted: {name}")
            continue
        blocks.append(body)
        print(f"inserted: {name}")

    if not blocks:
        return 0

    prefix = source[: match.start()].rstrip()
    insert_at = len(prefix)
    blocks_text = "\n".join(blocks)
    padding = "\n" if prefix and starts_with_prologue(blocks_text) else "\n\n" if prefix else ""
    insert_text(main_cpp, insert_at, f"{padding}{blocks_text}\n")
    return 0


def main(argv: list[str]) -> int:
    if not argv or argv in (["-h"], ["--help"]):
        print_usage()
        return 0

    if argv == ["--list"]:
        print_library_list()
        return 0

    option_like = [arg for arg in argv if arg.startswith("-")]
    if option_like:
        print(f"lib: library names must not start with '-': {option_like[0]}", file=sys.stderr)
        print("lib: run 'lib --list' to see available libraries", file=sys.stderr)
        return 1

    names = [normalize_name(arg) for arg in argv]
    unknown = [name for name in names if name not in LIBRARIES]
    if unknown:
        print(f"lib: unknown library: {unknown[0]}", file=sys.stderr)
        print("lib: run 'lib --list' to see available libraries", file=sys.stderr)
        return 1

    try:
        return insert_libraries(Path.cwd() / "main.cpp", names)
    except (FileNotFoundError, ValueError) as exc:
        print(exc, file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
