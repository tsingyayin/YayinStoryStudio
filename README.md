
<img width="1800" height="600" alt="yayinstorystudio" src="https://github.com/user-attachments/assets/f5df1f1f-92c7-476e-8792-621b03a48606" />

![](https://img.shields.io/badge/Project-Visindigo-purple)
![](https://img.shields.io/badge/Python-3.10%2B-blue)
![](https://img.shields.io/badge/Qt-6.8.3%2B-lightgreen)
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

若要从源代码构建，当前只支持在Windows上使用Visual Studio 2022进行编译。请确保安装了Qt 6.8.3及其开发工具，并且在Visual Studio中配置了Qt的路径。

除此之外，用户还应有一个安装了PySide6的Python3.10或更改版本的Python环境。Yayin Story Studio使用Python作为辅助编译工具。

虽然本仓库只提供Visual Studio解决方案，但Yayin Story Studio的核心代码中考虑了跨平台问题，因此用户也可以尝试用Qt VS Tools的功能将Visual Studio解决方案转换为Qt Creator项目，从而在其他平台上编译。

# 参与开发
本项目暂时还没准备好接受外部直接代码贡献，但欢迎任何人提出建议或意见。下面的内容有助于您快速了解本项目：

## 项目结构
* .vscode - VSCode 配置文件。它可能会在日后被用于创建一个仅用于查看的VSCode工作区，现在其中的内容没有实际用途。
* Plugin_* - 各插件项目。每个项目都是个独立的Visual Studio项目，可以单独编译。
* YayinStoryStudio - 主程序项目。它是一个纯编辑器，各插件的扩展本质上在扩展此项目。
* YSSCore - Yayin Story Studio的内核，要开发插件，则需要引用此项目。它包含了所有的核心功能和API。
* YSSInstaller - YSS的安装程序项目。它提供一个可视化的页面，供用户调整和更新YSS。（暂未正式开发）
* config - 可供整个解决方案引用的，用于配置项目或进行自动化的配置文件。
* design - 用于存放早期创意说明或其他临时性设计文件。
  
## 对于每个Visual Studio项目
* compiled - 存放要参与编译的资源，他们应该都被记录在qrc文件内。
* resource - 存放运行时需要的资源文件。
* pytools(如果有) - 存放用于辅助开发的Python脚本，它们不参与到最终程序，只在开发和编译过程中使用。
* doc(如果有) - 用于存放QDoc的配置和文档。
Yayin Story Studio的cpp与h文件的管理采用相对紧密的模式，即所有的cpp文件都在h文件所在目录的`cpp`子目录下。这样做的目的是为了便于管理和查找相关文件。

若要开发插件，可以用相应的PyTools提出所有头文件，或者从Release下载相应版本的头文件目录。

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
