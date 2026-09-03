# ============================================================================
#  YayinStoryStudio — Qt for Android 工程 (Qt Creator / CMake)
#
#  本工程以仓库真实源码 (../src) 作为编译输入，目录结构镜像 Windows 方案的
#  模块依赖图：
#
#      Visindigo(基础框架库)
#           │
#      YSSCore(编辑器核心库, 依赖 Visindigo)
#           │
#      YayinStoryStudio(主程序)          Plugin_* (三个运行时插件)
#
#  用法：
#    1. 用 Qt Creator 打开本文件 (File → Open File or Project → 选本 CMakeLists.txt)
#    2. 选择 Qt for Android Kit，例如 "Qt 6.10.x Android arm64-v8a"
#    3. 直接构建/运行；androiddeployqt 会负责打 APK
#
#  详细说明 / 已知阻断点见同目录 README.md
# ============================================================================
cmake_minimum_required(VERSION 3.21)

project(YayinStoryStudio
    VERSION 0.16.1
    DESCRIPTION "Yayin Story Studio (Qt Creator / Android CMake port)"
    LANGUAGES CXX
)

# ---------- 全局编译设置 ----------
set(CMAKE_CXX_STANDARD 23)          # 与桌面工程一致 (stdcpp23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

# ---------- Qt6 ----------
find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network
    Sql        # Visindigo::Utility::VirtualStorage 依赖 QtSql (QSQLITE)
)
# 自动开启 AUTOMOC / AUTORCC 等 Qt6 默认策略
qt_standard_project_setup()
# 显式确保 .qrc 被 rcc 编译(某些 Qt 版本 qt_standard_project_setup 默认不启用 AUTORCC，
# 曾导致所有 :/ 资源未打包——Android/CMake 下 editorTheme.json、内置字体读不到而崩溃)。
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC OFF)
# 声明 Android 部署路径属性可用（Qt 6.8+），消除 androiddeployqt 的 QTP0002 dev 警告
qt_policy(SET QTP0002 NEW)

# ---------- 路径 ----------
# qt_creator/ 位于仓库根目录下，源码在 ../src
set(YSS_REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(YSS_SRC_ROOT  "${YSS_REPO_ROOT}/src")

include(cmake/YSSModule.cmake)

# ---------- 模块 ----------
# 构建顺序即依赖顺序：Visindigo -> YSSCore -> 主程序
add_subdirectory(modules/Visindigo)
add_subdirectory(modules/YSSCore)
add_subdirectory(app/YayinStoryStudio)

# 运行时插件（桌面形态为 *.vpl，Android 形态为 lib*.so 共享库）
option(YSS_BUILD_PLUGINS "构建三个运行时插件 (ASERStudio / ImageViewer / YSSFileExt)" ON)
if(YSS_BUILD_PLUGINS)
    add_subdirectory(plugins/Plugin_YSSFileExt)
    add_subdirectory(plugins/Plugin_ImageViewer)
    add_subdirectory(plugins/Plugin_ASERStudio)
endif()
