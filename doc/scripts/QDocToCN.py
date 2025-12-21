import re
from pathlib import Path

# ===============================
# 替换规则（按顺序执行）
# ===============================

REPLACEMENTS = [

    # ---------- <title> ----------
    # <title>ClassName Class | Module</title>
    (
        re.compile(
            r"<title>\s*(?P<name>.+?)\s+Class\s*\|\s*(?P<module>.+?)\s*</title>",
            re.IGNORECASE
        ),
        r"<title>类 \g<name> | \g<module></title>"
    ),

    # ---------- (since ...) ----------
    (
        re.compile(r"\(since\s+([^)]+)\)", re.IGNORECASE),
        r"(自 \1 引入)"
    ),

    # ---------- This ... was introduced in ... ----------
    (
        re.compile(
            r"<p>\s*This\s+(function|enum|class|module|typedef)\s+was\s+introduced\s+in\s+(.+?)\.\s*</p>",
            re.IGNORECASE
        ),
        r"<p>这个 \1 从 \2 开始支持。</p>"
    ),

    # ---------- 导航链接 ----------
    (re.compile(r'>Public Functions<', re.IGNORECASE), '>公开成员函数<'),
    (re.compile(r'>Protected Functions<', re.IGNORECASE), '>受保护成员函数<'),
    (re.compile(r'>Private Functions<', re.IGNORECASE), '>私有成员函数<'),

    (re.compile(r'>Public Types<', re.IGNORECASE), '>公开类型<'),
    (re.compile(r'>Protected Types<', re.IGNORECASE), '>受保护类型<'),
    (re.compile(r'>Private Types<', re.IGNORECASE), '>私有类型<'),

    (re.compile(r'>Static Public Members<', re.IGNORECASE), '>静态公开成员<'),
    (re.compile(r'>Static Protected Members<', re.IGNORECASE), '>静态受保护成员<'),
    (re.compile(r'>Static Private Members<', re.IGNORECASE), '>静态私有成员<'),

    (re.compile(r'>Detailed Description<', re.IGNORECASE), '>详细说明<'),
    (re.compile(r'>Member Type Documentation<', re.IGNORECASE), '>成员类型文档<'),
    (re.compile(r'>Member Function Documentation<', re.IGNORECASE), '>成员函数文档<'),

    (re.compile(r'>Namespaces<', re.IGNORECASE),'>命名空间<'),
    (re.compile(r'>Classes<', re.IGNORECASE), '>类<'),

    (re.compile(r'>Related Non-Members<', re.IGNORECASE), '>相关的非成员内容<'),

    (re.compile(r">Macros<", re.IGNORECASE), ">宏<"),
    (re.compile(r'>Macro Documentation<', re.IGNORECASE), '>宏文档<'),
    (re.compile(r'>Signals<', re.IGNORECASE), '>信号<'),
    (re.compile(r'>Slots<', re.IGNORECASE), '>槽<'),
    (re.compile(r'>Types<', re.IGNORECASE), '>类型别名<'),

    # ---------- 表格左侧 ----------
    (re.compile(r'>\s*Header:\s*<', re.IGNORECASE), '>头文件：<'),
    (re.compile(r'>\s*Since:\s*<', re.IGNORECASE), '>自以下版本：<'),
    (re.compile(r'>\s*Inherits:\s*<', re.IGNORECASE), '>继承自：<'),
    (re.compile(r'>\s*Inherited by:\s*<', re.IGNORECASE), '>被继承：<'),

    # ---------- 其他 ----------
    (re.compile(r' is part of <', re.IGNORECASE), ' 是该集合的一部分：<'),
    (re.compile(r'>More...<', re.IGNORECASE), '>详情...<'),
    (re.compile(r'<b>See also </b>', re.IGNORECASE), '<b>另请参阅 </b>'),

    # ---------- 成员列表 ----------
    (
        re.compile(
            r'List of all members, including inherited members',
            re.IGNORECASE
        ),
        '所有成员列表（包含继承成员）'
    ),
    (re.compile(r">List of All Members for (?P<name>.+?)<", re.IGNORECASE), r">所有 \g<name> 的成员列表<"),
    (re.compile(r">This is the complete list of members for <", re.IGNORECASE), ">这是完整的成员列表：<"),
    (re.compile(r">, including inherited members.<", re.IGNORECASE), ">，包含继承成员。<"),
]


# ===============================
# 处理函数
# ===============================

def process_html_file(path: Path):
    text = path.read_text(encoding="utf-8", errors="ignore")
    original = text

    for pattern, replacement in REPLACEMENTS:
        text = pattern.sub(replacement, text)

    if text != original:
        path.write_text(text, encoding="utf-8")
        print(f"[OK] {path}")
    else:
        print(f"[SKIP] {path}")


def main(root_dir: str):
    root = Path(root_dir)
    if not root.is_dir():
        print("不是有效目录:", root_dir)
        return

    for html in root.rglob("*.html"):
        process_html_file(html)
