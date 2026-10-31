#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
从微软 Segoe Fluent Icons 文档页面抓取图标清单，导出 Visindigo 使用的 IconName 枚举。

用法：
    python pytools/segoe-fluent.py [输出路径]

不传输出路径时直接覆盖 src/Visindigo/Widgets/private/IconNameEnum.hpp。
"""

import re
import sys
import html
import urllib.request
from collections import Counter
from pathlib import Path

URL = "https://learn.microsoft.com/en-us/windows/apps/design/iconography/segoe-fluent-icons-font"

SCRIPT_DIR = Path(__file__).resolve().parent
REPO_ROOT = SCRIPT_DIR.parent
DEFAULT_OUTPUT = REPO_ROOT / "src" / "Visindigo" / "Widgets" / "private" / "IconNameEnum.hpp"

ENUM_HEADER = """/*
\tWarning: This File COULD ONLY be included by Widgets/IconFontRouter.h
*/
enum IconName : quint32 {
"""


def download_page(url: str) -> str:
    req = urllib.request.Request(
        url,
        headers={
            "User-Agent": (
                "Mozilla/5.0 "
                "(Windows NT 10.0; Win64; x64) "
                "AppleWebKit/537.36 "
                "(KHTML, like Gecko) "
                "Chrome/137.0 Safari/537.36"
            )
        }
    )

    with urllib.request.urlopen(req) as resp:
        return resp.read().decode("utf-8", errors="ignore")


def strip_tags(text: str) -> str:
    text = re.sub(r"<.*?>", "", text, flags=re.S)
    text = html.unescape(text)
    return text.strip()


def sanitize_enum_name(desc: str) -> str:
    """
    尽可能保留微软原文。

    C++ 枚举名不能包含空格和特殊字符，
    所以统一替换为下划线。
    """

    name = desc.strip()

    name = re.sub(r"\s+", "_", name)
    name = re.sub(r"[^A-Za-z0-9_]", "_", name)

    if re.match(r"^\d", name):
        name = "_" + name

    return name


def extract_tables(page: str):
    return re.findall(
        r"<table.*?>(.*?)</table>",
        page,
        flags=re.S | re.I,
    )


def extract_rows(table_html: str):
    return re.findall(
        r"<tr.*?>(.*?)</tr>",
        table_html,
        flags=re.S | re.I,
    )


def extract_cells(row_html: str):
    cells = re.findall(
        r"<(?:td|th).*?>(.*?)</(?:td|th)>",
        row_html,
        flags=re.S | re.I,
    )

    return [strip_tags(x) for x in cells]


def find_icon_entries(page: str):
    results = []

    for table in extract_tables(page):
        rows = extract_rows(table)

        if not rows:
            continue

        header = extract_cells(rows[0])

        unicode_idx = None
        desc_idx = None

        for i, col in enumerate(header):
            col_lower = col.lower()

            if "unicode" in col_lower:
                unicode_idx = i

            if "description" in col_lower:
                desc_idx = i

        if unicode_idx is None or desc_idx is None:
            continue

        for row in rows[1:]:
            cells = extract_cells(row)

            if len(cells) <= max(unicode_idx, desc_idx):
                continue

            unicode_text = cells[unicode_idx]
            desc_text = cells[desc_idx]

            match = re.search(
                r"\b([EF][0-9A-F]{3,4})\b",
                unicode_text,
                flags=re.I,
            )

            if not match:
                continue

            codepoint = match.group(1).upper()

            enum_name = sanitize_enum_name(desc_text)

            if not enum_name:
                continue

            results.append(
                (enum_name, codepoint, desc_text)
            )

    return results


def merge_entries(entries):
    """
    按码位去重并升序排列。

    页面在图标总表之前还有一张“新增图标”汇总表，同一码位会出现多次；
    总表在页面中位置更靠后，因此以后出现的记录为准。
    """

    by_codepoint = {}

    for enum_name, codepoint, desc in entries:
        by_codepoint[codepoint] = (enum_name, codepoint, desc)

    return sorted(by_codepoint.values(), key=lambda e: int(e[1], 16))


def resolve_names(entries):
    """
    微软文档中个别图标会以同一个名称占据两个码位（如 DefenderApp 的 E83D 与 F08A），
    C++ 枚举不允许重名，保留码位较小的一个，其余记录在案。
    """

    used = {}
    icons = []
    skipped = []

    for entry in entries:
        enum_name, codepoint, _ = entry

        if enum_name in used:
            skipped.append((enum_name, codepoint, used[enum_name]))
            continue

        used[enum_name] = codepoint
        icons.append(entry)

    return icons, skipped


def write_enum(entries, output_path):
    with open(output_path, "w", encoding="utf-8") as f:
        f.write(ENUM_HEADER)

        for enum_name, codepoint, _ in entries:
            f.write(f"    {enum_name} = 0x{codepoint},\n")

        f.write("};\n")


def main():
    output_path = Path(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_OUTPUT

    print("Downloading page...")

    page = download_page(URL)

    entries = find_icon_entries(page)

    print(f"Found {len(entries)} table rows.")

    icons, skipped = resolve_names(merge_entries(entries))

    write_enum(icons, output_path)

    print(f"Wrote {len(icons)} icons to {output_path}")

    print(f"\nDuplicate enum names ({len(skipped)}):")

    if not skipped:
        print("// No duplicates found.")
    else:
        for enum_name, codepoint, kept_codepoint in skipped:
            print(f"// {enum_name}: 0x{codepoint} skipped, 0x{kept_codepoint} kept")

    counter = Counter(
        enum_name
        for enum_name, _, _ in entries
    )

    repeated = sorted(
        (name, count) for name, count in counter.items() if count > 1
    )

    print(f"\nNames repeated across tables: {len(repeated)}")


if __name__ == "__main__":
    main()