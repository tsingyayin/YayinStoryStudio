# ============================================================================
#  Plugin_ASERStudio — ASER Studio 插件 (ID: cn.yxgeneral.aserstudio)
#
#  说明：本插件与 Windows 专属 Unity 引擎 ASE-Remake.exe 的联动
#  (ASERProgram.cpp 的进程/窗口/命名管道桥接) 全部已包在 Q_OS_WIN 内并有
#  非 Windows 桩实现，因此 Android 上可编译、AStoryX 语言工具可保留，
#  但 ASE-Remake 引擎联动在 Android 上不可用（无对应引擎）。
#
#  Qt 依赖: Core / Gui / Widgets / Network
#  导出宏:  ASER_EXPORT
#
#  注：ASERStudio_assets1.qrc 在桌面由 rcc 输出为旁路二进制 ASERStudio_assets1.rcc，
#  运行时用 QResource::registerResource 动态加载；本工程暂不内嵌它（体积很大且
#  语义是"与插件 .vpl 同目录的外部文件"），如需内嵌可自行加入 AUTORCC。
# ============================================================================
set(_mod_dir "${YSS_SRC_ROOT}/Plugin_ASERStudio")

yss_collect_sources(_src "${_mod_dir}")

set(_plugin_aserstudio_qrcs "${_mod_dir}/Plugin_ASERStudio.qrc")
# Android：把 ASER 项目模板资源(Plugin_ASERStudio_assets1.qrc, ~21MB)直接内嵌进插件 .so。
# 桌面版把该 qrc 用 rcc 输出为旁路外部 ASERStudio_assets1.rcc、运行时 registerResource；
# Android 没有该外部文件，故改为 qrc 内嵌——插件随进程(DT_NEEDED)加载时 qrc 即自动注册。
if(ANDROID)
	list(APPEND _plugin_aserstudio_qrcs "${_mod_dir}/Plugin_ASERStudio_assets1.qrc")
endif()
add_library(Plugin_ASERStudio SHARED ${_src} ${_plugin_aserstudio_qrcs})

target_include_directories(Plugin_ASERStudio PRIVATE "${_mod_dir}")

target_compile_definitions(Plugin_ASERStudio PRIVATE ASER_EXPORT)

target_link_libraries(Plugin_ASERStudio PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Visindigo
    YSSCore
)

# 构建后导出到 qt_creator/plugin_build/ASERStudio/
yss_export_plugin(Plugin_ASERStudio ASERStudio
    "${CMAKE_CURRENT_LIST_DIR}/Plugin_ASERStudio.vpl.json")
