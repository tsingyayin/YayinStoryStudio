import sys
import os
import json

def deleteAllFilesInDir(dirPath):
    print(f"Deleting all files in directory: {dirPath}")
    if not os.path.exists(dirPath):
        return
    for item in os.listdir(dirPath):
        itemPath =  os.path.join(dirPath, item)
        if os.path.isfile(itemPath):
            os.remove(itemPath)
        elif os.path.isdir(itemPath):
            deleteAllFilesInDir(itemPath)
            os.rmdir(itemPath)

def copyFileToDir(filePath, dirPath):
    print(f"Copying file {filePath} to directory {dirPath}")
    if not os.path.exists(dirPath):
        os.makedirs(dirPath)
    if os.path.isfile(filePath):
        fileName = os.path.basename(filePath)
        destPath = os.path.join(dirPath, fileName)
        with open(filePath, "rb") as fsrc:
            with open(destPath, "wb") as fdst:
                fdst.write(fsrc.read())

def createDirIfNotExists(dirPath):
    if not os.path.exists(dirPath):
        print(f"Creating directory: {dirPath}")
        os.makedirs(dirPath)
    return dirPath

class DeriveProject:
    def __init__(self, config):
        self.config = config
        self.proj_path:str = os.path.abspath(config["proj_path"])
        print(f"Initialized DeriveProject with path: {self.proj_path}")
        self.modules:list = config.get("modules", [])
        self.binMainName:str = config.get("bin_main_name", "")
        self.binExtNames:list = config.get("bin_ext_names", [])
        self.ignoreHeaderSuffixes:list = config.get("ignore_header_suffixes", [])
        self.platforms:list = config.get("platforms", ["x64"])
        self.releasePaths:list = config.get("release_build_paths", [])
        self.debugPaths:list = config.get("debug_build_paths", [])

    def deriveBinary(self):
        if len(self.releasePaths) > 0:
            for platform, relPath in zip(self.platforms, self.releasePaths):
                buildPath = os.path.abspath(relPath)
                outputDir = os.path.abspath(os.path.join(deriveConfig["output"], self.binMainName, "bin", platform, "Release",))
                print(f"Deriving release binaries from {buildPath} to {outputDir}")
                createDirIfNotExists(outputDir)
                deleteAllFilesInDir(outputDir)
                for ext in self.binExtNames:
                    binFile = f"{self.binMainName}.{ext}"
                    binFilePath = os.path.join(buildPath, binFile)
                    copyFileToDir(binFilePath, outputDir)
        if len(self.debugPaths) > 0:
            for platform, dbgPath in zip(self.platforms, self.debugPaths):
                buildPath = os.path.abspath(dbgPath)
                outputDir = os.path.abspath(os.path.join(deriveConfig["output"], self.binMainName, "bin", platform, "Debug",))
                print(f"Deriving debug binaries from {buildPath} to {outputDir}")
                createDirIfNotExists(outputDir)
                deleteAllFilesInDir(outputDir)
                for ext in self.binExtNames:
                    binFile = f"{self.binMainName}.{ext}"
                    binFilePath = os.path.join(buildPath, binFile)
                    copyFileToDir(binFilePath, outputDir)

    def copyHeaders(self):
        includeDir = os.path.abspath(os.path.join(deriveConfig["output"], self.binMainName, "include"))
        print(f"Copying header files to {includeDir}")
        createDirIfNotExists(includeDir)
        deleteAllFilesInDir(includeDir)
        for module in self.modules:
            modulePath = os.path.join(self.proj_path, module)
            if not os.path.isdir(modulePath):
                print(f"Module path does not exist: {modulePath}")
                continue
            for root, dirs, files in os.walk(modulePath):
                for file in files:
                    if file.endswith(".h") or file.endswith(".hpp"):
                        mainName, ext = os.path.splitext(file)
                        if any(mainName.endswith(suffix) for suffix in self.ignoreHeaderSuffixes):
                            print(f"Ignoring header file due to suffix: {file}")
                            continue
                        filePath = os.path.join(root, file)
                        relativePath = os.path.relpath(filePath, self.proj_path)
                        destPath = os.path.join(includeDir, relativePath)
                        destDir = os.path.dirname(destPath)
                        createDirIfNotExists(destDir)
                        copyFileToDir(filePath, destDir)
        # also copy root headers
        for file in os.listdir(self.proj_path):
            if file.endswith(".h") or file.endswith(".hpp"):
                mainName, ext = os.path.splitext(file)
                if any(mainName.endswith(suffix) for suffix in self.ignoreHeaderSuffixes):
                    print(f"Ignoring root header file due to suffix: {file}")
                    continue
                filePath = os.path.join(self.proj_path, file)
                copyFileToDir(filePath, includeDir)
        

if __name__ == "__main__":
    os.chdir(os.path.join(os.path.dirname(__file__), '..'))
    print("current dir:", os.getcwd())
    deriveConfig = json.load(open("config/deriveConfig.json", "r", encoding="utf-8"))
    for projName, projConfig in deriveConfig["projects"].items():
        print(f"Processing project: {projName}")
        deriveProj = DeriveProject(projConfig)
        deriveProj.deriveBinary()
        deriveProj.copyHeaders()