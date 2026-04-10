import os
import sys

def getCodeLines(path):
    totalLines = 0
    totalSize = 0
    for root, dirs, files in os.walk(path):
        for file in files:
            if file.endswith(".h") or file.endswith(".cpp"):
                if file.startswith("moc_") or file.startswith("ui_") or file.startswith("qrc_"):
                    continue
                with open(os.path.join(root, file), "r", encoding="utf-8", errors="ignore") as f:
                    lines = f.readlines()
                    totalLines += len(lines)
                    totalSize += os.path.getsize(os.path.join(root, file))
                    print(f"File: {os.path.join(root, file)}, Lines: {len(lines)}, Size: {os.path.getsize(os.path.join(root, file))} bytes")
    return totalLines, totalSize

if __name__ == "__main__":
    lines, size = getCodeLines("../")
    print(f"Total code lines: {lines}")
    print(f"Total file size: {size} bytes")
