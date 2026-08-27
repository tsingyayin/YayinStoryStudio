# CONTRIBUTING — YayinStoryStudio 代码风格约定

所有本仓库贡献者，无论是人类还是AI Agent，都应尽可能遵守本规定。


## 1. 声明的位置

### 1.1 模块目录结构
- 项目基本全部采用PIMPL模式，公共头文件放在模块根目录或对应子目录：`src/Visindigo/General/`、`src/Visindigo/Widgets/`、`src/YSSCore/General/`、`src/YSSCore/Editor/`、`src/YayinStoryStudio/Editor/` 等。
- 源文件统一放在**与头文件同名模块的 `folder/cpp/` 子目录**下：如 `Widgets/cpp/MultiButton.cpp`、`YSSCore/General/cpp/YSSProject.cpp`、`Editor/MainEditor/cpp/MainWin.cpp`、`Plugin_ASERStudio/cpp/*.cpp`。
- 对于PIMPL模式中的私有类，如果：
    - **仅在单个 `.cpp` 内使用**，则直接内联写在 `.cpp` 里。
    - **有多个 `.cpp` 需要引用，或私有类也是QObject，需要MOC生成代码**，则需要单独写一个私有实现头文件（`*_p.h`），并放在 `folder/private/` 子目录下。
- 请注意，除PIMPL之外的，其他外部不常直接使用（如测试代码的头文件、部分特殊的宏定义等）但某些时候确有引用必要的文件，也应放在`folder/private`目录中，但不应该以`_p.h`结尾，以便打包工具可以正常将其输出。（这种情况极少）
- `*_p.h`的实现可直接写在对应外部实现的 `.cpp` 里，也可以写在`*_p.cpp`里，但无论如何都必须放置在`folder/cpp/`目录下。

### 1.2 头文件保护（include guard）
- 一律用 `#ifndef`（**不用 `#pragma once`**）。
- guard 名 = 文件相对 `src/` 的路径下划线化 + `_h`：

  ```cpp
  #ifndef Visindigo_General_Translator_h
  #define Visindigo_General_Translator_h
  // ...
  #endif // Visindigo_General_Translator_h
  ```

- 结尾必须 `#endif // <同样的 guard 名>`。

### 1.3 include 语句
- 对本项目（按子项目计）的头文件，从子项目根目录开始使用双引号引用头文件，例如对于Visindigo子项目，应有`#include "General/Translator.h"`，而不是`#include "Visindigo/General/Translator.h"`。
- 对本项目（按子项目计）引用其他子项目的头文件，从其他子项目根目录开始使用尖括号引用头文件，例如对于YSSCore子项目引入上例头文件，则应写为`#include <General/Translator.h>`
- 对于第三方库的头文件，使用尖括号引用头文件，例如`#include <QtWidgets/qframe.h>`。
- 对于Qt库，使用详细直接路径引用`.h`结尾的头文件，例如`#include <QtWidgets/qframe.h>`，而不是`#include <QFrame>`。
- 推荐按如下两种顺序之一引用头文件：
    - 从近到远顺序：
        - 本项目当前模块的头文件
        - 本项目其他模块的头文件
        - Qt 库的头文件
        - 其他第三方库的头文件
    - 从远到近顺序：
        - 其他第三方库的头文件
        - Qt 库的头文件
        - 本项目其他模块的头文件
        - 本项目当前模块的头文件
### 1.4 前置声明
- 头文件内尽量前置声明而非 include 重型头：`class QString;`、`namespace Visindigo::Utility { class JsonConfig; }`。
- 用 `// Forward declarations` 与 `// Main` 分节注释标出（见 `src/Visindigo/General/Translator.h`、`Plugin.h`）。

### 1.5 pimpl（d 指针）
- 本项目虽然属于Qt生态，但不使用Qt的PIMPL规范，直接使用裸指针d，不使用QScopedPointer或QSharedPointer。
- 如果没有必要，不向私有类传递公共类的指针q。
- 公共类尾部（`protected:`/`private:` 区）声明 `FooPrivate* d;`（星号贴类型）。
- 私有类必须将全部成员声明为`protected`，并将可能访问它的其他类声明为`friend class`，以便访问私有成员。

---

## 2. 类的声明结构

### 2.1 整体结构
- PIMPL中的外部类
    ```cpp
    class ClassNamePrivate;
    class EXPORT_MACRO ClassName :public QtBaseClass, OtherInterface {
        Q_OBJECT; // if QObject
        Q_PROPERTY(...); // if needed
    signals:
        void eachSignal();
    public:
        enum IfHaveEnum { ... };
        Q_ENUM(IfHaveEnum); // if needed
    public:
        static void staticMethod();
    public:
        ClassName(...);
        virtual ~ClassName(); // if final, virtual can be omitted
    public:
        void getAndSet();
        void doSomething();
    public:
        virtual void virtualMethod();
        virtual void pureVirtualMethod() = 0;
    private:
        ClassNamePrivate* d;
    };
    ```
    如果类是final类，或者实现上就不需要继承才能使用的类，允许析构函数省略 `virtual` 关键字。
    本项目在语义上将没有virtual关键字的类视为final类，禁止继承。

    此外，部分外部类的d指针可能还需要其他类操作，此时将这些类声明为友元，并将private d更改为protected d。

- PIMPL中的内部类
    ```cpp
    class ClassNamePrivate :public QObject {
        Q_OBJECT; // if QObject
        friend class ClassName;
    protected:
        ClassName* q; // pointer to public class, if needed
        Type memberVariable;
    protected:
        void privateMethod();
    };
    ```
    值得指出的是，如无必要，一般不为私有类提供构造函数和析构函数，除非有特殊需求。
    一般也不为成员访问提供对应的GetSet函数。
- 其他类
    ```cpp
    class ClassName :public QtBaseClass, OtherInterface {
        Q_OBJECT; // if QObject
        Q_PROPERTY(...); // if needed
    signals:
        void eachSignal();
    public:
        enum IfHaveEnum { ... };
        Q_ENUM(IfHaveEnum); // if needed
    private:
        Type memberVariable;
    public:
        static void staticMethod();
    public:
        ClassName(...);
        virtual ~ClassName(); // if final, virtual can be omitted
    public:
        void getAndSet();
        void doSomething();
    public:
        virtual void virtualMethod();
        virtual void pureVirtualMethod() = 0;
    private:
        void privateMethod();
    };
    ```
    请注意，对于非PIMPL的类，私有成员变量放在比较靠前的位置，私有成员函数则放在末尾。本项目中也有私有成员变量放在私有成员函数之后的情况，但不推荐。

### 2.2 Qt 宏
- `Q_OBJECT;`、 `Q_GADGET;`、`Q_ENUM(...)`、`Q_PROPERTY(...)` 等 Qt 宏 末尾应带分号。

### 2.3 访问限定符
- `public:` / `protected:` / `private:` / `signals:` **顶格**（与类名同级），类内成员缩进 1 个 tab。
- 常用顺序：`friend` → Qt 宏 → `signals:` → `public:` → `protected:` → `private:`。
- 同一限定符可按功能分段；`public:` 优先。
- Qt 小写关键字直接使用（`signals:`、`public slots:`）。

### 2.4 虚函数、override
- 基类虚函数声明带 `virtual`；空实现可 inline 在头文件并带分号：`virtual void onPluginEnable() {};`，也可以在 cpp 中实现。
- 子类重写**必须同时写** `virtual` 与 `override`：`virtual void onThemeChanged() override;`。重写不加override视同非法。

### 2.5 静态成员与单例
- 单例：`public:` 区声明 `static Foo* getInstance();`，类内 `static Foo* Instance;`。
- cpp 中的定义与赋值：

  ```cpp
  Foo* Foo::Instance = nullptr;
  Foo::Foo() {
      Instance = this;
      // ...
  }
  Foo* Foo::getInstance() { return Instance; }
  ```

- 常用宏辅助：`VIPlugin(PluginClass)`（见 `src/Visindigo/General/Plugin.h`）。

### 2.6 友元
- `friend class` 常用，集中在类体顶部（公共类 friend 私有类、私有类 friend 公共类，双向）。

### 2.7 导出宏
- 每个库/插件一个 `XxxCompileMacro.h`，定义 `VisindigoAPI` / `YSSCoreAPI` / `ASERAPI` 等（见 `src/Visindigo/VICompileMacro.h`、`src/YSSCore/YSSCoreCompileMacro.h`）。
- 类声明前写：`class VisindigoAPI Foo {`。

---

## 3. 约定俗成的写法习惯

### 3.1 缩进、换行、编码
| 项目 | 规则 |
|---|---|
| C++ 缩进 | **Tab**（不展开为空格） |
| Python 缩进 | **4 空格** |
| 换行 | **CRLF**（Windows） |
| 源文件编码 | **UTF-8 无 BOM** |
| 资源文件（含中文的 .rc 等） | **UTF-8 with BOM** + `#pragma code_page(65001)`，否则 rc.exe 中文乱码 |

### 3.2 命名规范（自定义为主，注意与 Qt 惯例不同）
- **类名**：PascalCase（`MultiButton`、`YSSProject`、`PluginManager`）。
- **函数/方法**：**首字母大写**（与 Qt 小写开头相反）：`getInstance()`、`loadProject()`、`toString()`；槽函数用 `onXxxChanged`。
- **成员变量**：**PascalCase 无前缀**（`CentralWidget`、`PluginID`、`ConfigPath`）；**禁用 `m_` 前缀**。
- **常量/宏**：全大写下划线（`VisindigoAPI`、`Compiled_VIAPI_Version`）。
- **命名空间**：首字母大写 + C++17 `::` 嵌套：`namespace Visindigo::General`、`namespace YSSCore::General`、`namespace YSS::Editor`；私有实现放 `namespace Visindigo::__Private__`。
  - ⚠️ 命名空间易混淆：核心库用 `YSSCore::General` / `YSSCore::Editor`，主程序编辑器用 `YSS::Editor`。
- **枚举**：枚举值 PascalCase，显式 `Unknown = 0` 起头：

  ```cpp
  enum LangID {
      Unknown = 0,
      zh_CN,
      // ...
  };
  Q_ENUM(LangID);
  ```

- **文件命名**：PascalCase（`MultiButton.h`、`YSSProject.cpp`），私有头 `Foo_p.h`。

### 3.3 指针 / 引用 / 默认参数
- 星号贴类型：`QWidget* parent`、`MainWin* getInstance()`（**不写** `T *x`）。
- 空指针默认 `= nullptr`；字符串默认 `= ""`；布尔默认 `= false`。

### 3.4 大括号
- **K&R**：`{` 与声明/函数同行（`class Foo {`、`Foo::Foo() {`）。
- 长初始化列表换行后 `{` 独占一行（少数例外，允许）。

### 3.5 Lambda
- 以 `[this](...)` 为主，用于 `connect`；无捕获的算法 lambda 用 `[](...)`。

### 3.6 空行与分节
- 头文件用 `// Forward declarations` / `// Main` 分节并夹空行；同一命名空间可拆多个 `namespace ... { }` 块，中间空行分隔。
- 方法区段间通常不空行；类尾部 `d` 指针前常空一行。

### 3.7 Python 工具脚本（`pytools/`、`doc/scripts/`）
- 4 空格缩进；`# -*- coding: utf-8 -*-` 可省略（历史遗留可有可无）。
- 函数名 **snake_case** 为主（`read_version_info`）；现存个别驼峰（`copyFolder`、`deleteAllFilesInDir`）属历史遗留，**新代码用 snake_case**。
- 类 PascalCase，实例属性 snake_case。
- 模块级常量全大写：`SCRIPT_DIR`、`REPO_ROOT`、`BZ_EXE`。
- 类型注解部分使用：`def f(src: str) -> None:`，公共函数签名建议标注。
- 入口惯例：`if __name__ == "__main__":` + `def main():` 收口。
- docstring 用中文；模块头用 `"""..."""` 说明用途与用法；输出用 f-string。
- 只用标准库（如 `AutoPack.py` 仅依赖 Bandizip 命令行 `bz.exe`）。

### 3.8 资源与插件
- 资源路径用域名式前缀：`:/resource/cn.yxgeneral.yayinstorystudio/icon.png`。
- 插件入口导出固定函数：`extern "C" ASERAPI Visindigo::General::Plugin* VisindigoPluginMain() { ... }`。

### 3.9 QObject 与构建登记（硬性）
- **带 `Q_OBJECT` 的类（含 `*_p.h` 私有头）必须在对应 `.vcxproj` 登记 `<QtMoc Include="...">`**，漏登记则 moc 不生成、链接报错；`.vcxproj.filters` 同步加条目。

---

## 4. 注释的使用

### 4.1 语言
- 文档注释**只用中文**。
- 结构性短注释用英文或中文（`// Forward declarations`、`// Main`）。
- 代码行内注释推荐使用英文。

### 4.2 QDoc 文档注释（公共 API）
- 格式：`/*! ... */` + **反斜杠命令**（`\class` `\brief` `\since` `\inmodule` `\fn` `\enum` `\a` `\property`）。
- **不要**用 `/**` 或 `@param` / `@brief`（Javadoc 风格）。
- 所有QtDoc注释只能写在`.cpp`文件中，`.h`中的注释QtDoc不能识别。
- 所有实体都使用`\since` 记录引入版本（`Visindigo 0.13.0`、`YSS 0.13.0`）。
- 函数文档直接写在函数体上方。类文档写在Private类定义之后，构造函数定义之前。
- 类内其他在cpp文件没有对应代码的实体（如枚举、属性、信号）文档写在类文档之后，构造函数定义之前。
- 类内其他在cpp文件没有对应代码的函数或模板函数，按声明时的顺序写在对应位置。譬如
  ```cpp
  class Foo {
      Q_OBJECT;
  public:
      Foo();
      virtual void bar() = 0;
      template<typename T> void baz(T arg);
      void qux();
  };
  ```
  由于这里的 `bar()` 和 `baz()` 在cpp文件中没有对应的实现代码，所以它们的文档应该按顺序排列在构造函数文档之后，`qux()`的文档之前。

### 4.3 行注释与块注释
- 代码内用 `//` 行注释为主；`/* */` 块注释罕见。
- 不用 `///` 三斜杠（个别历史文件有，非惯例）。

### 4.4 版本/作者/许可证头
- 所有文件都**不写** Copyright / Author / License 头（仓库根 `LICENSE.txt` 是唯一许可证声明）。
- 由AI设计API、AI主要实现的文件，必须通过注释加入如下内容：
    ```plain
    Visindigo Project Full AI Generated Declaration

    1. This file was entirely generated by AI. There was almost no human
    involvement in the design of its API and implementation.
    2. Provided that the usage characteristics of the program corresponding
    to this code are not substantially changed, anyone is free to make
    fundamental modifications to this code.
    3. This code does not guarantee implementation consistency across
    generations; it only guarantees that the corresponding program
    features remain basically stable.
    ```
    注：由人类设计API，人类指导大部分细节的文件，即使由AI完成实现，也不需要加入上述声明。
### 4.5 TODO
- 用 `// TODO: ...` 行注释，中英皆可；不使用 FIXME / XXX。
