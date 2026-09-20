import os
import re
import shutil
import sys

# 部署目标根目录，最终部署到 <DeployRoot>/<MAJOR.MINOR.PATCH>/
DefaultDeployRoot = "E:/Visindigo"
SourceDir = os.path.join("binary", "Visindigo")
VersionMacroPath = os.path.join("src", "Visindigo", "General", "private", "VersionMacro.h")


def readVersionMacros():
    """从 VersionMacro.h 读取主/次/修订版本号，保证版本号单一来源。"""
    macros = {"MAJOR": "0", "MINOR": "0", "PATCH": "0"}
    if not os.path.exists(VersionMacroPath):
        print("warning: VersionMacro.h not found: " + VersionMacroPath)
        return macros
    with open(VersionMacroPath, "r", encoding="utf-8-sig") as versionMacroFile:
        content = versionMacroFile.read()
    for name in ("MAJOR", "MINOR", "PATCH"):
        match = re.search(r"#define\s+Visindigo_VERSION_" + name + r"\s+(\d+)", content)
        if match:
            macros[name] = match.group(1)
    return macros


def getVersionString():
    macros = readVersionMacros()
    return macros["MAJOR"] + "." + macros["MINOR"] + "." + macros["PATCH"]


def deployVisindigo(deployRoot, clean=False):
    version = getVersionString()
    sourcePath = os.path.abspath(SourceDir)
    targetPath = os.path.abspath(os.path.join(deployRoot, version))
    print("Deploying Visindigo " + version + " from " + sourcePath + " to " + targetPath)
    if not os.path.isdir(sourcePath):
        print("error: source directory does not exist: " + sourcePath)
        print("       run pytools/deriveLib.py first to refresh " + SourceDir)
        return False
    if clean and os.path.isdir(targetPath):
        print("Cleaning target directory: " + targetPath)
        shutil.rmtree(targetPath)
    shutil.copytree(sourcePath, targetPath, dirs_exist_ok=True)
    includePath = os.path.join(targetPath, "include")
    binPath = os.path.join(targetPath, "bin", "x64")
    print("Deploy finished:")
    print("  include : " + includePath)
    print("  Release : " + os.path.join(binPath, "Release"))
    print("  Debug   : " + os.path.join(binPath, "Debug"))
    return True


if __name__ == "__main__":
    os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
    print("current dir:", os.getcwd())
    arguments = sys.argv[1:]
    deployRoots = [arg for arg in arguments if not arg.startswith("-")]
    targetRoot = deployRoots[0] if len(deployRoots) > 0 else DefaultDeployRoot
    if not deployVisindigo(targetRoot, clean=("--clean" in arguments)):
        sys.exit(1)
