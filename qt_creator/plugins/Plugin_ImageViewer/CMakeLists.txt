# ============================================================================
#  Plugin_ImageViewer — 图片查看插件 (ID: cn.yxgeneral.yayinstorystudio.plugin.imageviewer)
#
#  纯 Qt(QGraphicsView)，无 Win32。Android 形态产出 libPlugin_ImageViewer.so。
#
#  Qt 依赖: Core / Gui / Widgets
#  导出宏:  IMAGEVIEWER_EXPORT
# ============================================================================
set(_mod_dir "${YSS_SRC_ROOT}/Plugin_ImageViewer")

yss_collect_sources(_src "${_mod_dir}")

add_library(Plugin_ImageViewer SHARED ${_src} "${_mod_dir}/Plugin_ImageViewer.qrc")

target_include_directories(Plugin_ImageViewer PRIVATE "${_mod_dir}")

target_compile_definitions(Plugin_ImageViewer PRIVATE IMAGEVIEWER_EXPORT)

target_link_libraries(Plugin_ImageViewer PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Visindigo
    YSSCore
)

# 构建后导出到 qt_creator/plugin_build/ImageViewer/
yss_export_plugin(Plugin_ImageViewer ImageViewer
    "${CMAKE_CURRENT_LIST_DIR}/Plugin_ImageViewer.vpl.json")
