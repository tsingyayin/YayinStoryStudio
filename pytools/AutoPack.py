# -*- coding: utf-8 -*-
"""
AutoPack.py - YSS 自动打包脚本

将 x64/Release 打包为发布版本，流程：
  1. 复制 x64/Release → x64/YSS_V<版本号>[ <后缀>]
       后缀规则（依据 src/Visindigo/General/private/AUTO_VERSION.h 的 Visindigo_VERSION_NICKNAME）：
         - 开发版本：使用完整昵称（如 "EA TP1"、"EA Beta 1"）→ YSS_V0.16.0.193 EA Beta 1
         - EA Release（昵称等于 "EA" 或含 "Release"）：只保留 "EA" → YSS_V0.16.0.193 EA
         - 历史兼容：昵称等于 "SEA" 时只保留 "SEA"（日后不再使用）
  2. 清空 user_data/logs、user_data/repos、user_data/third_party 的内容（保留目录本身）
  3. 删除新文件夹中的 *.ilk；将 *.exp / *.lib / *.map / *.pdb 收集并压缩为
     YSSDbg_V<版本号>_MSVC_AMD64.zip，之后从新文件夹中移除
  4. 用 Bandizip (bz.exe) 将新文件夹压缩为同名 .zip
     （压缩包内为根目录内容，不含顶层文件夹，与既有发布包一致）

用法：
  python AutoPack.py            # 常规打包（目标已存在时会询问是否覆盖）
  python AutoPack.py --force    # 目标已存在时直接覆盖，不询问
  python AutoPack.py --dry-run  # 仅预览计划与将要执行的操作，不实际执行

依赖：Bandizip 命令行工具 bz.exe（需在 PATH 中，或通过环境变量 BZ_EXE 指定完整路径）。
本脚本仅使用 Python 标准库，不安装任何第三方库。
"""

import os
import re
import sys
import shutil
import subprocess
import tempfile

# ---------------------------------------------------------------- 路径常量
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.normpath(os.path.join(SCRIPT_DIR, ".."))
X64_DIR = os.path.join(REPO_ROOT, "x64")
RELEASE_DIR = os.path.join(X64_DIR, "Release")

AUTO_VERSION_H = os.path.join(
    REPO_ROOT, "src", "Visindigo", "General", "private", "AUTO_VERSION.h"
)

# Bandizip 命令行工具：优先取环境变量 BZ_EXE，其次 PATH 中的 bz.exe
BZ_EXE = os.environ.get("BZ_EXE") or shutil.which("bz.exe") or "bz.exe"

# 需要从发布文件夹剥离、单独收入调试包的调试文件扩展名
DBG_EXTS = (".exp", ".lib", ".map", ".pdb")
# 需要清空内容但保留目录的 user_data 子目录
CLEAR_DIRS = ("logs", "repos", "third_party")


# ---------------------------------------------------------------- 工具函数
def log(msg):
    print("[AutoPack] " + msg)


def read_version_info():
    """从 AUTO_VERSION.h 读取版本字符串与昵称（由 AutoVersionFromGIT.py 生成）。"""
    if not os.path.exists(AUTO_VERSION_H):
        raise RuntimeError(
            "未找到 %s ，请先构建以生成 AUTO_VERSION.h（由 AutoVersionFromGIT.py 自动生成）。"
            % AUTO_VERSION_H
        )
    with open(AUTO_VERSION_H, "r", encoding="utf-8-sig") as f:
        content = f.read()

    def grab(pattern):
        m = re.search(pattern, content)
        return m.group(1) if m else None

    version = grab(r'#define\s+Visindigo_VERSION_STRING\s+"([^"]+)"')
    nickname = grab(r'#define\s+Visindigo_VERSION_NICKNAME\s+"([^"]+)"') or ""
    if not version:
        raise RuntimeError("AUTO_VERSION.h 中未找到 Visindigo_VERSION_STRING。")
    return version, nickname


def resolve_folder_suffix(nickname):
    """
    根据昵称决定发布文件夹后缀：
      - 昵称等于 "EA" 或包含 "Release"（如 "EA Release"）→ 只保留 "EA"（EA Release 发布版）
      - 昵称等于 "SEA" → 只保留 "SEA"（历史超级早期体验版，日后不再使用）
      - 其余情况（开发版本）→ 使用完整昵称
    """
    n = (nickname or "").strip()
    if not n:
        return ""
    if n == "EA" or "Release" in n:
        return "EA"
    if n == "SEA":
        return "SEA"
    return n


def build_plan():
    version, nickname = read_version_info()
    suffix = resolve_folder_suffix(nickname)
    folder_name = "YSS_V" + version
    if suffix:
        folder_name += " " + suffix
    return {
        "version": version,
        "nickname": nickname,
        "suffix": suffix,
        "folder_name": folder_name,
        "new_folder": os.path.join(X64_DIR, folder_name),
        "rel_zip": os.path.join(X64_DIR, folder_name + ".zip"),
        "dbg_zip": os.path.join(X64_DIR, "YSSDbg_V" + version + "_MSVC_AMD64.zip"),
    }


def run_bz(args, cwd=None):
    """驱动 Bandizip 命令行工具。"""
    cmd = [BZ_EXE] + args
    log("执行: " + " ".join('"%s"' % a if " " in a else a for a in cmd) + ("  (工作目录: %s)" % cwd if cwd else ""))
    result = subprocess.run(cmd, cwd=cwd, capture_output=True)
    out = result.stdout.decode("utf-8", errors="replace").rstrip()
    err = result.stderr.decode("utf-8", errors="replace").rstrip()
    if out:
        print(out)
    if err:
        print(err, file=sys.stderr)
    if result.returncode != 0:
        raise RuntimeError("bz.exe 执行失败，返回码 %d。" % result.returncode)
    return result


# ---------------------------------------------------------------- 各打包步骤
def step1_copy_release(plan):
    """复制 x64/Release → 新版本文件夹。"""
    if os.path.exists(plan["new_folder"]):
        log("删除已存在的文件夹: %s" % plan["new_folder"])
        shutil.rmtree(plan["new_folder"])
    log("复制 %s → %s" % (RELEASE_DIR, plan["new_folder"]))
    shutil.copytree(RELEASE_DIR, plan["new_folder"])


def step2_clear_user_data(plan):
    """清空 user_data/logs、repos、third_party 的内容，保留目录本身。"""
    for sub in CLEAR_DIRS:
        p = os.path.join(plan["new_folder"], "user_data", sub)
        if not os.path.isdir(p):
            log("跳过（目录不存在）: %s" % os.path.relpath(p, plan["new_folder"]))
            continue
        count = 0
        for entry in os.listdir(p):
            ep = os.path.join(p, entry)
            if os.path.isdir(ep) and not os.path.islink(ep):
                shutil.rmtree(ep)
            else:
                os.remove(ep)
            count += 1
        log("已清空 %s （移除 %d 项）" % (os.path.relpath(p, plan["new_folder"]), count))


def step3_strip_debug_files(plan):
    """
    删除 *.ilk；将 *.exp / *.lib / *.map / *.pdb 收集压缩进
    YSSDbg_..._MSVC_AMD64.zip（扁平结构），并从新文件夹中移除。
    """
    # 1) 删除 .ilk（不进入调试包）
    ilk_count = 0
    for root, _dirs, files in os.walk(plan["new_folder"]):
        for f in files:
            if f.lower().endswith(".ilk"):
                os.remove(os.path.join(root, f))
                ilk_count += 1
    log("已删除 .ilk 文件 %d 个" % ilk_count)

    # 2) 收集调试文件
    dbg_files = [
        os.path.join(root, f)
        for root, _dirs, files in os.walk(plan["new_folder"])
        for f in files
        if f.lower().endswith(DBG_EXTS)
    ]
    if not dbg_files:
        log("未发现调试文件（.exp/.lib/.map/.pdb），跳过调试包。")
        return

    log("收集到调试文件 %d 个，打包 → %s" % (len(dbg_files), os.path.basename(plan["dbg_zip"])))
    staging = tempfile.mkdtemp(prefix="yss_dbg_")
    try:
        # 以文件名为准扁平化存放，避免同名冲突
        used = set()
        for src in dbg_files:
            base = os.path.basename(src)
            dst = os.path.join(staging, base)
            if dst in used or os.path.exists(dst):
                stem, ext = os.path.splitext(base)
                i = 1
                while os.path.exists(dst):
                    dst = os.path.join(staging, "%s_%d%s" % (stem, i, ext))
                    i += 1
            used.add(dst)
            shutil.copy2(src, dst)
        run_bz(["a", "-r", "-y", plan["dbg_zip"], "."], cwd=staging)
    finally:
        shutil.rmtree(staging, ignore_errors=True)

    # 3) 从新文件夹移除调试文件
    for f in dbg_files:
        os.remove(f)
    log("已从发布文件夹移除调试文件 %d 个" % len(dbg_files))


def step4_create_release_zip(plan):
    """用 Bandizip 将新文件夹内容压缩为同名 .zip（根目录内容，无顶层文件夹）。"""
    log("压缩发布包 → %s" % os.path.basename(plan["rel_zip"]))
    run_bz(["a", "-r", "-y", plan["rel_zip"], "."], cwd=plan["new_folder"])


# ---------------------------------------------------------------- 入口
def main():
    args = set(sys.argv[1:])
    force = "--force" in args
    dry_run = "--dry-run" in args

    if not os.path.isdir(RELEASE_DIR):
        raise RuntimeError("未找到发布目录: %s" % RELEASE_DIR)

    plan = build_plan()

    print("=" * 64)
    print("YSS 自动打包计划")
    print("=" * 64)
    print("版本号  : V%s" % plan["version"])
    print("昵称    : %s" % (plan["nickname"] if plan["nickname"] else "(空)"))
    print("文件夹  : %s" % plan["folder_name"])
    print("发布包  : %s" % os.path.basename(plan["rel_zip"]))
    print("调试包  : %s" % os.path.basename(plan["dbg_zip"]))
    print("=" * 64)

    if dry_run:
        log("预览模式（--dry-run），不执行任何操作。")
        return

    # 已存在目标检查
    existing = [
        (t, d)
        for t, d in (
            (plan["new_folder"], "版本文件夹"),
            (plan["rel_zip"], "发布压缩包"),
            (plan["dbg_zip"], "调试压缩包"),
        )
        if os.path.exists(t)
    ]
    if existing:
        if not force:
            log("检测到已存在的目标：")
            for t, d in existing:
                log("  [%s] %s" % (d, t))
            ans = input("是否删除并重建上述目标？[y/N] ").strip().lower()
            if ans not in ("y", "yes"):
                log("已取消。")
                return
        for t, _d in existing:
            if os.path.isdir(t) and not os.path.islink(t):
                shutil.rmtree(t)
            else:
                os.remove(t)
            log("已删除旧目标: %s" % t)

    step1_copy_release(plan)
    step2_clear_user_data(plan)
    step3_strip_debug_files(plan)
    step4_create_release_zip(plan)

    print("=" * 64)
    log("打包完成。")
    for t, d in (
        (plan["new_folder"], "版本文件夹"),
        (plan["dbg_zip"], "调试包"),
        (plan["rel_zip"], "发布包"),
    ):
        if os.path.exists(t):
            size = os.path.getsize(t) if os.path.isfile(t) else dir_size(t)
            log("[%s] %s (%.1f MB)" % (d, t, size / 1024.0 / 1024.0))


def dir_size(path):
    total = 0
    for root, _dirs, files in os.walk(path):
        for f in files:
            try:
                total += os.path.getsize(os.path.join(root, f))
            except OSError:
                pass
    return total


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n[AutoPack] 已取消。")
        sys.exit(1)
    except Exception as e:
        print("[AutoPack] 错误: %s" % e, file=sys.stderr)
        sys.exit(1)
