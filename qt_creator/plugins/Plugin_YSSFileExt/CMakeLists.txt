# ============================================================================
#  Plugin_YSSFileExt — 通用文本文件类型插件 (ID: cn.yxgeneral.yayinstorystudio.plugin.yssfileext)
#
#  纯 Qt、无 Win32，是三个插件中可移植性最好、最干净的样例。
#  桌面形态为 *.vpl 动态库；Android 形态产出 libPlugin_YSSFileExt.so。
#
#  Qt 依赖: Core / Gui / Widgets
#  导出宏:  YSSFileExt_EXPORT（main.cpp 中入口函数受该宏保护）
# ============================================================================
set(_mod_dir "${YSS_SRC_ROOT}/Plugin_YSSFileExt")

yss_collect_sources(_src "${_mod_dir}")

add_library(Plugin_YSSFileExt SHARED ${_src} "${_mod_dir}/Plugin_YSSFileExt.qrc")

target_include_directories(Plugin_YSSFileExt PRIVATE "${_mod_dir}")

# 必须定义：main.cpp 的 extern "C" VisindigoPluginMain 入口在 #ifdef YSSFileExt_EXPORT 内
target_compile_definitions(Plugin_YSSFileExt PRIVATE YSSFileExt_EXPORT)

target_link_libraries(Plugin_YSSFileExt PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Visindigo
    YSSCore
)

# 构建后导出到 qt_creator/plugin_build/YSSFileExt/
yss_export_plugin(Plugin_YSSFileExt YSSFileExt
    "${CMAKE_CURRENT_LIST_DIR}/Plugin_YSSFileExt.vpl.json")
