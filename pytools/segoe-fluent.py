#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import re
import html
import urllib.request
from collections import Counter

URL = "https://learn.microsoft.com/en-us/windows/apps/design/iconography/segoe-fluent-icons-font"


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


def main():
    print("Downloading page...")

    page = download_page(URL)

    entries = find_icon_entries(page)

    print(f"Found {len(entries)} icon entries.\n")

    with open("SegoeFluentIcons.h", "w", encoding="utf-8") as f:
        f.write("enum IconName : quint32 {\n")

        for enum_name, codepoint, _ in entries:
            f.write(f"    {enum_name} = 0x{codepoint},\n")

        f.write("};\n")

    print("\n\n// Duplicate enum names:\n")

    counter = Counter(
        enum_name
        for enum_name, _, _ in entries
    )

    dup_count = 0

    for name, count in sorted(counter.items()):
        if count > 1:
            dup_count += 1
            print(f"// {name} ({count} occurrences)")

    if dup_count == 0:
        print("// No duplicates found.")


if __name__ == "__main__":
    main()