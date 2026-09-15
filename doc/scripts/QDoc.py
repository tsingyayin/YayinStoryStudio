import yaml
import os
import re
import sys
import shutil
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

NAV_PATTERN = re.compile(r'(<nav class="yss-nav">)(.*?)(</nav>)', re.DOTALL)

def readModules(srcPath:str):
    # 从 module.qdoc 的 \module 声明读出模块列表，用来生成顶栏导航。
    # 模块页文件名是 QDoc 按模块名小写生成的（YSSFileExt -> yssfileext-module.html），
    # 所以这里不手写列表，免得以后加了模块忘改模板（之前就漏过 YayinStoryStudio，
    # 反而多列了 pytools / yssplugindev 这两个普通页面）。
    items = [("主页", "index.html")]
    path = os.path.join(srcPath, "module.qdoc")
    if not os.path.exists(path):
        print("找不到 module.qdoc，顶栏导航保持模板里的兜底内容。")
        return items
    with open(path, "r", encoding="utf-8", errors="ignore") as f:
        for line in f:
            matched = re.match(r"\s*\\module\s+(\S+)\s*$", line)
            if matched:
                name = matched.group(1)
                items.append((name, name.lower() + "-module.html"))
    return items

def writeNavigation(htmlPath:str, srcPath:str):
    # 把每个页面顶栏里 <nav class="yss-nav"> 的内容重写成模块列表。
    # 模板里的静态列表与这里的内容一致，所以这步只是“保持同步”，
    # 万一这步没跑成，页面上的静态列表仍然是一份可用的导航。
    items = readModules(srcPath)
    links = "\n".join(
        "      <a href=\"" + href + "\">" + title + "</a>" for title, href in items
    )
    pages = 0
    for name in os.listdir(htmlPath):
        if not name.endswith(".html"):
            continue
        path = os.path.join(htmlPath, name)
        with open(path, "r", encoding="utf-8", errors="ignore") as f:
            text = f.read()
        replaced, count = NAV_PATTERN.subn(
            lambda m: m.group(1) + "\n" + links + "\n    " + m.group(3), text, count=1
        )
        if count and replaced != text:
            with open(path, "w", encoding="utf-8", newline="") as f:
                f.write(replaced)
            pages += 1
    print("顶栏导航已同步：" + str(len(items)) + " 项，更新 " + str(pages) + " 个页面。")

def writeSiteFiles(htmlPath:str, baseUrl:str):
    # 生成站点根目录下的 robots.txt 与 sitemap.xml。
    # 爬虫不会自己去发现一个没有外链的新域名，这两个文件是它唯一的“地图”，
    # 也是提交到 Google Search Console / Bing 站长平台时必需的东西。
    baseUrl = baseUrl.rstrip("/")
    # 搜索结果页只是个 JS 壳子，没有索引价值，不进 sitemap
    pages = sorted(
        name for name in os.listdir(htmlPath)
        if name.endswith(".html") and name != "search-results.html"
    )
    today = datetime.date.today().isoformat()

    with open(os.path.join(htmlPath, "robots.txt"), "w", encoding="utf-8", newline="\n") as f:
        f.write("User-agent: *\n")
        f.write("Allow: /\n\n")
        f.write("Sitemap: " + baseUrl + "/sitemap.xml\n")

    with open(os.path.join(htmlPath, "sitemap.xml"), "w", encoding="utf-8", newline="\n") as f:
        f.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n")
        f.write("<urlset xmlns=\"http://www.sitemaps.org/schemas/sitemap/0.9\">\n")
        for name in pages:
            f.write("  <url><loc>" + baseUrl + "/" + name + "</loc>"
                    + "<lastmod>" + today + "</lastmod></url>\n")
        f.write("</urlset>\n")

    print("已生成 robots.txt 与 sitemap.xml，收录页面数：" + str(len(pages)))

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
    htmlPath = path + "\\..\\html"
    if os.path.exists(htmlPath):
        # qdoc 不会清理输出目录，残留的旧页面会一直留在文档里，因此在生成前先整个删除。
        shutil.rmtree(htmlPath)
    print("QDocPath: " + QDocPath)
    print("QDocConfig: " + QDocConfig)
    docPara:str = QDocPath+" "+QDocConfig + " -indexdir D:/Qt/Docs/Qt-6.10.3"
    os.system(docPara)
    if para["qt"]["translationCN"]:
        QDocToCN.main(htmlPath)
    else:
        print("跳过中文翻译步骤。")
    copyFolder(path + "\\..\\src\\template\\images", htmlPath + "\\images")
    copyFolder(path + "\\..\\src\\template\\style", htmlPath + "\\style")
    copyFolder(path + "\\..\\src\\template\\scripts", htmlPath + "\\scripts")
    writeNavigation(htmlPath, path + "\\..\\src")
    writeSiteFiles(htmlPath, para.get("site", {}).get("baseUrl", "http://prts.site"))
    end = datetime.datetime.now()
    print("完成于 " + end.strftime("%Y-%m-%d %H:%M:%S") + "，耗时 " + str((end - start).seconds) + " 秒。")