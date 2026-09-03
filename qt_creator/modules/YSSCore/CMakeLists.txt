# ============================================================================
#  YSSCore — 编辑器核心库
#
#  生产(Windows)形态为 DLL；本工程在 Android 下产出 libYSSCore.so。
#  依赖 Visindigo（其公开头如 <General/Plugin.h>、<Widgets/BorderFrame.h> 来自 Visindigo）。
#
#  Qt 依赖: Core / Gui / Widgets
#  导出宏:  YSSCore_EXPORT
# ============================================================================
set(_mod_dir "${YSS_SRC_ROOT}/YSSCore")

# 排除模块根 main.cpp（桌面单跑调试入口）
yss_collect_sources(_src "${_mod_dir}" "main\\.cpp$")

# automoc 误报处理：ColorThemeProvider.cpp 的 Q_GADGET 字样仅在其 \note 文档注释中，
# 真实宏在 ColorThemeProvider.h；跳过对该 .cpp 运行 automoc。
set_source_files_properties(
    "${_mod_dir}/Editor/cpp/ColorThemeProvider.cpp"
    PROPERTIES SKIP_AUTOMOC TRUE
)

add_library(YSSCore SHARED ${_src})

target_include_directories(YSSCore PUBLIC "${_mod_dir}")

target_compile_definitions(YSSCore PRIVATE YSSCore_EXPORT)

target_link_libraries(YSSCore PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Visindigo
)
