import sys
import os
import datetime

AutoVersionFilePath = "\\YSSCore\\General\\private\\AUTO_VERSION.h"
VersionNickName = "Technical Preview 4"

def getCommitCount():
    gitCommitsCount = os.popen("git log --oneline | find /v /c \"\"").read()
    gitCommitsCount = int(gitCommitsCount)+1
    print("git commits count now is : "+ str(gitCommitsCount))
    return gitCommitsCount

def spawnHeadFile(path):
    if os.path.exists(path+AutoVersionFilePath):
        os.remove(path+AutoVersionFilePath)
    versionFile = open(path+AutoVersionFilePath, "w", encoding="utf-8-sig")
    versionFile.write("#pragma execution_character_set(\"utf-8\")\n")
    versionFile.write("#pragma once\n")
    versionFile.write("// This file is auto generated by YSSCore/pytools/AutoVersionFromGIT.py\n")
    versionFile.write("// Do not modify this file manually\n")
    versionFile.write("#define YSSCore_VERSION_BUILD "+str(getCommitCount())+"\n")
    versionFile.write("#define YSSCore_VERSION_BUILD_DATE \""+datetime.date.today().strftime("%Y-%m-%d")+"\"\n")
    versionFile.write("#define YSSCore_VERSION_BUILD_TIME \""+datetime.datetime.now().strftime("%H:%M:%S")+"\"\n")
    versionFile.write("#define YSSCore_VERSION_NICKNAME \""+VersionNickName+"\"\n")
    versionFile.close()


if __name__ == "__main__":
    path:str = sys.path[0]
    print(path)
    os.chdir(path+"/../../")
    path = os.getcwd()
    spawnHeadFile(path)



