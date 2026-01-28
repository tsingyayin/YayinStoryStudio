import yaml
import os
import sys
import QDocToCN
import datetime
def copyFolder(src:str, dest:str):
    if not os.path.exists(dest):
        os.makedirs(dest)
    for item in os.listdir(src):
        s = os.path.join(src, item)
        d = os.path.join(dest, item)
        if os.path.isdir(s):
            copyFolder(s, d)
        else:
            if not os.path.exists(d):
                with open(s, "rb") as fsrc:
                    with open(d, "wb") as fdest:
                        fdest.write(fsrc.read())

if __name__ == "__main__":
    start = datetime.datetime.now()
    path:str = sys.path[0]
    os.chdir(path)
    print(path)
    yaml_file = open("../../config/para.yml", "r", encoding="utf-8")
    para = yaml.load(yaml_file, Loader=yaml.FullLoader)
    yaml_file.close()
    QtInstallPath = para["qt"]["installPath"]
    QDocPath = QtInstallPath + "\\bin\\qdoc.exe"
    QDocConfig = path + "\\..\\src\\config.qdocconf"
    print("QDocPath: " + QDocPath)
    print("QDocConfig: " + QDocConfig)
    docPara:str = QDocPath+" "+QDocConfig + " -indexdir D:/Qt/Docs/Qt-6.8.3"
    os.system(docPara)
    if para["qt"]["translationCN"]:
        QDocToCN.main(path + "\\..\\html")
    else:
        print("跳过中文翻译步骤。")
    copyFolder(path + "\\..\\src\\template\\images", path + "\\..\\html\\images")
    copyFolder(path + "\\..\\src\\template\\style", path + "\\..\\html\\style")
    copyFolder(path + "\\..\\src\\template\\scripts", path + "\\..\\html\\scripts")
    end = datetime.datetime.now()
    print("完成于 " + end.strftime("%Y-%m-%d %H:%M:%S") + "，耗时 " + str((end - start).seconds) + " 秒。")