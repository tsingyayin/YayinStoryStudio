
<img width="1800" height="600" alt="yayinstorystudio" src="https://github.com/user-attachments/assets/f5df1f1f-92c7-476e-8792-621b03a48606" />

![](https://img.shields.io/badge/Project-Visindigo-purple)
![](https://img.shields.io/badge/Python-3.12%2B-blue)
![](https://img.shields.io/badge/Qt-6.10.3%2B-lightgreen)
![](https://img.shields.io/badge/LGPL-2.1-green)
![](https://img.shields.io/badge/Support-CMake-yellow)
![](https://img.shields.io/badge/Support-Visual_Studio_2026-purple)
![](https://img.shields.io/badge/Support-Qt_Creator-green)

# Yayin Story Studio 项目简介
Yayin Story Studio （简称YSS）是一个基于Qt/Visindigo的代码编辑器。目前以支持明日方舟剧情编辑器重制版（Arknights Story Editor Remake）（简称ASE-Remake 或 ASER）的AStoryX脚本为主要开发目标。未来引入Git支持，AI Agent支持后，YSS将成为一个通用的代码编辑器。

## Project Visindigo - 全面的明日方舟剧情二创工具集
整个Project Visindigo项目自2021年启动以来，先后产出了Yayin Story Player（YSP）、Arknights Story Editor（ASE）、Arknights Story Editor Remake（ASER）和Yayin Story Studio（YSS）四个主要项目。时至今日，它们仍然是整个明日方舟二创社区最好用、最全面的剧情编辑工具集。

| <img width="128" height="128" alt="YSP" src="https://github.com/user-attachments/assets/1161e562-410b-4093-8caa-32423c23c22e" /><br>**YSP** | <img width="128" height="128" alt="ASE" src="https://github.com/user-attachments/assets/b10641f0-29b2-4dd8-ab89-20e4cd4fa3a3" /><br>**ASE** |
| --- | --- |
| <img width="128" height="128" alt="ASER" src="https://github.com/user-attachments/assets/9ff66188-0126-4835-8abb-8d050fa90a4b" /><br>**ASER** | <img width="128" height="128" alt="YSS" src="https://github.com/user-attachments/assets/75b619a8-2ee9-4c58-a07a-1ab3e644670a" /><br>**YSS** |


YSP从Python脚本和明日方舟官方剧情脚本中汲取灵感，首创了专用于明日方舟二创的SPOL脚本语言，并在SPOL中定义和规范了数种常用的演出功能为“控制器”。YSP的出现让明日方舟二创社区的剧情创作门槛大幅降低，极大地丰富了社区的剧情创作内容。基于这些概念基础，谷雨暮歌的新作ASE则进一步实现了更多的播放功能——其基于Unity而非Qt的技术栈天然兼容来自明日方舟的各种官方资源，并在AStory脚本的自定义Rules设计中实现了对SPOL的兼容。ASER则在ASE基础上进一步发展出AStoryX，虽然没有太多新特性，语法也与AStory本身近似，但ASER在播放功能、性能与素材兼容性上则有十分显著的提升。与此同时，为了解决自ASE时代（2023年以后）以来，AStory/AStoryX编辑体验不佳的问题，Project Visindigo开发了Yayin Story Studio，转为AStoryX编写提供支持。

## 获取Project Visindigo系列软件以及使用教程
YSP项目已经于2023年底停止维护（Python版本早在2021年中旬就不再更新），但其源代码仍然可以在[cYSP](https://github.com/tsingyayin/cYSP)和[YSP-Yayin_Story_Player](https://github.com/tsingyayin/YSP-Yayin_Story_Player)中获取。Release中也有最新的程序文件。

ASE/ASER项目是闭源项目，且有限授权，要获取相关软件，以及获得使用授权，要求使用者必须有一个本人身份实名认证的社交平台账号在紫靛社区群内，并在多媒体平台有明日方舟的二创投稿。

目前紫靛社区群只在QQ上开设。群号（1群，原2群）563449926、（2群，原3群）1072976010、（水群，原1群）737797713。请注意，考虑到ASE/ASER软件的专业性与专用性，为了避免滥传滥用，紫靛社区群审核极其严格。不满足[关于申请ASER/YSS社区的一些说明](https://www.bilibili.com/opus/1216043483647705089)中提到的条件的用户将无法加入紫靛社区群，也无法获取ASE/ASER软件。如确有明日方舟剧情二创需要，请考虑使用YSP或其他开源工具。

YSP的使用教程在其程序文件包中，以PDF文档的形式提供。

ASE/ASER的使用教程，请参见[Arknight Story Editor 使用教程](https://www.yuque.com/tsingyayin/ase)和[Arknight Story Editor Remake 使用教程](https://www.yuque.com/tsingyayin/aser)。请注意，ASE已经于2026年6月停止维护，请务必不要搞错。

YSS的使用教程请参见[Yayin Story Studio 使用教程](https://www.yuque.com/tsingyayin/yss)、开发文档请参见[Yayin Story Studio 开发文档](http://prts.site)（由于没钱买证书，这是个http页面）。

# COFFEE!
如果您喜欢这个项目并希望支持它，可以通过以下方式捐赠：
* [爱发电](https://afdian.com/a/tsingyayin)

我们坚信开发者是某种将咖啡因转换为代码的生物，您的支持将帮助我们持续从咖啡因中汲取代码之力！

# YSS 使用指南
## 运行程序
**建议非技术用户直接从Release下载已编译的版本。**，并主要参考[Yayin Story Studio 使用教程](https://www.yuque.com/tsingyayin/yss)进行使用。下面的内容主要面向开发者，介绍如何从源代码构建YSS。

## 构建YSS

若要从源代码构建，有两种环境方案：

### Visual Studio 2026
在Windows上，推荐使用Visual Studio 2026进行编译。请确保安装了Qt 6.10.3 及其开发工具，并且在Visual Studio中配置了Qt的路径。

除此之外，用户还应有一个安装了PySide6的Python3.12或更高版本的Python环境。Yayin Story Studio的Visual Studio解决方案使用Python作为辅助编译工具。

### Qt Creator (CMake)
从0.17.0开始，YSS初步提供CMake构建支持，用户可以在Qt Creator中使用任何支持CMake的编译器，向任何Qt兼容的目标平台构建YSS。不过目前只针对Android平台进行了部分调整，没有在其他平台上进行过测试。请注意，CMake构建方式目前仍处于实验阶段，可能存在一些问题。

### 所有方案都要注意的事情
请注意，本项目的Qt版本和C++版本都非常新，因此需要支持C++23或更高版本的编译器。

此外，部分Py脚本的驱动还未从Visual Studio解决方案迁移到CMake构建系统中，因此在使用CMake构建时，可能需要手动运行部分Python脚本来生成必要的文件。

## 版本选择：EA、TP、Beta、Release
有关 YSS 各版本的固定后缀（SEA/EA）的介绍，以及开发过程中的技术预览版、Beta测试版、Beta RC测试版、发行版的区别，请参见使用教程中的[选择适合你的YSS版本](https://www.yuque.com/tsingyayin/yss/tergn7u16it0sn65)章节。

# 参与 YSS 的开发
## 编码风格与规范

为了保证代码的可读性和可维护性，YSS项目采用了统一的编码风格和规范。请在提交代码前仔细阅读本仓库中的[编码规范](CONTRIBUTING.md)文档，并遵循其中的要求。

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
  
### 对于每个Visual Studio项目
* resource - 存放要参与编译的资源，他们应该都被记录在qrc文件内。请注意，目前所有项目都遵守Visindigo的资源约定，这就需要在resource目录下直接创建一个和该项目ID（比如cn.yxgeneral.aserstudio）相同名称的子目录，然后再在其中放置资源文件。
* pytools(如果有) - 存放用于辅助开发的Python脚本，它们不参与到最终程序，只在开发和编译过程中使用。
Yayin Story Studio的cpp与h文件的管理采用相对紧密的模式，即所有的cpp文件都在h文件所在目录的`cpp`子目录下。这样做的目的是为了便于管理和查找相关文件。

## AI 与 Vibe Coding
本项目编码中有AI参与。本项目也接受合并来自AI生成的代码，但需遵守以下前提：
1. AI生成的代码必须经过人工审核，确保其符合项目的编码规范和质量要求。无论代码是人工编写还是AI生成，提交者必须完全知道它们在做什么，并对其行为负责。
2. AI生成的代码不得包含对已有ABI的破坏性更改，除非经过充分的测试和验证，并且获得了项目维护者的批准。
3. 包含AI生成的代码的分支，合并时不能与主线相差超过5000行或50个文件（以较多者为准），以便于维护者进行人工审核。（这里5000行按add + del计算）
4. AI不得参与API设计，也不得参与功能的架构设计，只能在具体实现层面提供辅助。
5. 一个功能被来自多个分支的代码同时修改时，优先选择人工编写或经过完全人工复核并修正的AI代码。
6. 向本项目多次申请合并低质量AI代码的行为将被视为滥用，可能导致该贡献者被禁止参与本项目的开发。

本项目不被禁止用于AI训练，但不建议使用本项目的源码进行AI训练。我们认为将参杂有AI生成的代码用于二次训练是极其不负责的行为，且可能导致AI生成的代码质量下降。

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
