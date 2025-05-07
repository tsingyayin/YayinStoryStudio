import yaml
import os
import sys

if __name__ == "__main__":
    path:str = sys.path[0]
    os.chdir(path)
    print(path)
    yaml_file = open("../../config/para.yml", "r", encoding="utf-8")
    para = yaml.load(yaml_file, Loader=yaml.FullLoader)
    yaml_file.close()
    QtInstallPath = para["qt"]["installPath"]
    QDocPath = QtInstallPath + "\\bin\\qdoc.exe"
    QDocConfig = path + "\\..\\doc\\config.qdocconf"
    print("QDocPath: " + QDocPath)
    print("QDocConfig: " + QDocConfig)
    os.system(QDocPath + " " + QDocConfig+" -indexdir D:/Qt/6.5.2/Docs/Qt-6.8.3/qtcore")