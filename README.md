
<img width="1800" height="600" alt="yayinstorystudio" src="https://github.com/user-attachments/assets/f5df1f1f-92c7-476e-8792-621b03a48606" />

![](https://img.shields.io/badge/Project-Visindigo-purple)
![](https://img.shields.io/badge/Python-3.12%2B-blue)
![](https://img.shields.io/badge/Qt-6.10.3%2B-lightgreen)
![](https://img.shields.io/badge/LGPL-2.1-green)

# Yayin Story Studio

Yayin Story Studio 是适用于ASE的故事创作工具，旨在通过可视化、代码补全以及联想等功能，帮助用户更高效地编写故事脚本。

Yayin Story Studio 被设计为高度插件化的应用程序，因此它可以通过安装插件来扩展功能。理论上，Yayin Story Studio 经过适当的扩展后也可以用于编写代码。

# COFFEE!
如果您喜欢这个项目并希望支持它，可以通过以下方式捐赠：
* [爱发电](https://afdian.com/a/tsingyayin)

我们坚信开发者是某种将咖啡因转换为代码的生物，您的支持将帮助我们持续从咖啡因中汲取代码之力！

# 运行、部署与编译
建议非技术用户直接从Release下载已编译的版本。

若要从源代码构建，当前只支持在Windows上使用Visual Studio 2022/2026进行编译。请确保安装了Qt 6.10.3 及其开发工具，并且在Visual Studio中配置了Qt的路径。

除此之外，用户还应有一个安装了PySide6的Python3.12或更高版本的Python环境。Yayin Story Studio使用Python作为辅助编译工具。

请注意，本项目的Qt版本和C++版本都非常新，因此需要支持C++23或更高版本的编译器。

本项目目前并未完全遵照Qt的编码标准以进行适配不同编译器和平台的开发，但正尝试努力解决这些问题。请注意，YSS目前并不把全平台支持作为其开发的首要目标，只是为了将来潜在的跨平台需求尽量编写平台无关/编译器无关的代码，并尽可能的降低使用Qt工具将其转换为QtCreator项目后需要进行的修改量。

# 版本：SEA 与 TP
由于开发代码编辑器的复杂性，当前Yayin Story Studio仍然处于超级早期开发阶段（Super Early Access），直到我们完成了所有核心功能的开发与测试后，SEA阶段才会结束。

对于每个Minor版本更新，我们遵循`TP（技术预览）`->`Beta`->`Release`的发布流程。
* TP版本主要用于开发过程中预览，通常会包含极其粗糙的功能与API变更，有时甚至可能不会提供构建后的二进制文件。
* Beta版在功能上相对稳定，追求对现有内容的修复与优化，但也可能会出现新内容。
* Release版本是经过充分测试的稳定版本，通常会包含所有已知问题的修复。Release期间一般不会添加新功能。
  
当一个Minor版本进入Release阶段后，一个Fork会被创建，用于维护该版本的稳定性和安全性。之后，主分支会进入下一个Minor版本的开发阶段。

# 参与开发
本项目暂时还没准备好接受外部直接代码贡献，但欢迎任何人提出建议或意见。下面的内容有助于您快速了解本项目：

## 项目结构
* .vscode - VSCode 配置文件。它可能会在日后被用于创建一个仅用于查看的VSCode工作区，现在其中的内容没有实际用途。
* binary - 存放导出的库文件，它不在git记录中，但运行之后会自动生成。目前只包括作为库使用的Visindigo和YSSCore的dll、lib和pdb等文件，以及全部配套的头文件。这部分内容可以从pytools/deriveLib.py脚本中生成，但需要你在config中正确配置deriveConfig.json文件。
* config - 用于存放和Visual Studio解决方案无关的其他配置文件，多为本项目的Python辅助工具所用的配置文件。
* design - 用于存放早期创意说明或其他临时性设计文件。
* doc - YSS项目配套的文档子项目
    * html - 存放输出结果，它不在git记录中，但运行之后会自动生成。
    * scripts - 用于生成文档并进行汉化的Python脚本，以及一个基于Node.js http-server的本地服务器脚本。
    * src - 用于存档不和任何.cpp文件关联的.qdoc文档内容，以及用于引导QDoc工作的qdocconf文件，和QDoc
    生成时依赖的资源文件。
* pytools - 存放用于辅助开发的Python脚本，这些脚本不参与到最终程序，只在开发和编译过程中使用。
* src - 存放Visual Studio项目的目录。每个Visual Studio项目都在src目录下有一个独立的子目录。
    * Plugin_ASERStudio - 为ASE-Remake开发提供支持的插件，提供对AStoryX的解析、着色和纠正功能。
    * Plugin_YSSFileExt - 为YSS本体提供一些常见文件格式（图片除外）的打开支持
    * Plugin_ImageViewer - 为YSS提供一个通用的图片查看器插件，支持多种图片格式的打开和预览。
    * Visindigo - Visindigo库项目。Visindigo是一个通用的Qt库，提供了日志记录、异常处理、文件操作等功能。YSSCore和YayinStoryStudio都依赖于此项目。
    * YSSCore - Yayin Story Studio的内核，要开发插件，则需要引用此项目。它包含了所有的核心功能和API。
    * YayinStoryStudio - 主程序项目。它是一个纯编辑器，各插件的扩展本质上在扩展此项目。
    * (其他未提及的项目) - 有设想，但暂未投入开发的项目，这里不再赘述。
  
## 对于每个Visual Studio项目
* resource - 存放要参与编译的资源，他们应该都被记录在qrc文件内。请注意，目前所有项目都遵守Visindigo的资源约定，这就需要在resource目录下直接创建一个和该项目ID（比如cn.yxgeneral.aserstudio）相同名称的子目录，然后再在其中放置资源文件。
* pytools(如果有) - 存放用于辅助开发的Python脚本，它们不参与到最终程序，只在开发和编译过程中使用。
Yayin Story Studio的cpp与h文件的管理采用相对紧密的模式，即所有的cpp文件都在h文件所在目录的`cpp`子目录下。这样做的目的是为了便于管理和查找相关文件。

# 致谢

* TsingYayin和他的三位高中&大学同学对本项目做出了重要支持和贡献：

[Ayano_Aishi 爱诗绫里](https://space.bilibili.com/475519346) - YSP的创意贡献者，YSP是Tsing Yayin在Ayano_Aishi的启发下开发的。YSP的极其早期版本也被称为AASP。
> 注：Ayano_Aishi 理应翻译为爱诗绫野，但由于Ayano_Aishi在提出YSP创想的时候正在没日没夜的玩《逆转裁判》，所以Tsing Yayin决定强行将其翻译为爱诗绫里。

> 注2：Ayano_Aishi 也是本项目的首席现代C++语言语法顾问。

[Gra_dus 谷雨暮歌](https://space.bilibili.com/144798380) - Arknights Story Editor的作者，ASE在YSP衰落后及时出现，接替Yayin Story Player成为了方舟社区最好的剧情播放工具。

[v0v_tempest](https://space.bilibili.com/178835858) - 他没有直接参与YSP、ASE以及YSS的开发，但在这位超级活爹的辅导之下，Tsing Yayin通过了考研数学二考试。并且他还帮助Tsing Yayin运营了一段时间他的Minecraft服务器。

* 除此之外，YSP、ASE的几位早期用户对软件社区的贡献也功不可没：

[The_Universe寰宇](https://space.bilibili.com/108130502) - YSP的重要早期用户，在YSP0.6-0.9版本中提供了大量的反馈和建议。

[小姐婕拉啦](https://space.bilibili.com/32201824) - 在YSP0.6-0.9版本间提供了重要数据帮助改进UI效果，为程序标准化做出了重要贡献。

[三叔](https://space.bilibili.com/37691) - 制作了ASE的新手教程视频，时至今日仍然是初学者的重要参考。

* 也感谢全体YSP和ASE用户，你们是我们不断前进的动力。

* 特别致谢：南京工业大学 计算机与信息工程学院 徐新艳 导师。她在本科阶段教授Tsing Yayin的Python课程对项目的早期发展起到了重要作用。
