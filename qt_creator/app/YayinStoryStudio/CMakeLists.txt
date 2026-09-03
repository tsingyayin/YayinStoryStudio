# ============================================================================
#  YayinStoryStudio — 主程序 (Android 上由 androiddeployqt 打成 APK)
#
#  主程序自身源码不含任何 WIN32/Q_OS_WIN 平台宏，全部走 Qt 抽象层，因此
#  Android 上可直接编译。已对 src/YayinStoryStudio/YayinStoryStudio.cpp 的
#  Main::onPluginEnable() 加了 Q_OS_ANDROID 守卫：桌面安装器(更新服务)通信、
#  ExtTool 注册表文件关联等逻辑在 Android 上跳过。
#
#  Qt 依赖: Core / Gui / Widgets / Network
# ============================================================================
set(_mod_dir "${YSS_SRC_ROOT}/YayinStoryStudio")

yss_collect_sources(_src "${_mod_dir}")

qt_add_executable(YayinStoryStudio ${_src} "${_mod_dir}/YayinStoryEditor.qrc")

target_include_directories(YayinStoryStudio PRIVATE "${_mod_dir}")

target_link_libraries(YayinStoryStudio PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Visindigo
    YSSCore
)

# ---- Android 打包 ----
# 提供自定义 AndroidManifest.xml（声明 MANAGE_EXTERNAL_STORAGE / INTERNET / 旧版外存权限）。
# androiddeployqt 会合并本目录 android/ 下的清单/资源并填充 %%INSERT_*%% 占位符。
# 包名/图标/版本如需自定义可继续在该清单与属性中调整。
if(ANDROID)
    set_target_properties(YayinStoryStudio PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
    )

    # 插件随 APK 的 lib/<abi>/ 打包(Android 官方加载方式)：把三个插件链接进主程序，
    # 使其成为 DT_NEEDED 并被 androiddeployqt 收进 APK；运行时由 main.cpp 用各插件的
    # 唯一导出入口(VisindigoPluginMain_*)注册为依赖插件(依赖序：YSSFileExt → ImageViewer → ASERStudio)。
    target_link_libraries(YayinStoryStudio PRIVATE
        Plugin_YSSFileExt
        Plugin_ImageViewer
        Plugin_ASERStudio
    )

    # 内嵌三个插件的元数据 json(qrc prefix /plugins)：运行时 PluginManager 经
    # setPluginLoadPath(..., ":/plugins/Plugin_*.vpl.json") 读取 ID/Dependencies 做依赖排序/去活。
    # 绝对路径的资源需配 QT_RESOURCE_ALIAS，才能以相对名(即 /plugins/<文件名>)访问。
    set_source_files_properties(
        "${CMAKE_SOURCE_DIR}/plugins/Plugin_YSSFileExt/Plugin_YSSFileExt.vpl.json"
        PROPERTIES QT_RESOURCE_ALIAS "Plugin_YSSFileExt.vpl.json")
    set_source_files_properties(
        "${CMAKE_SOURCE_DIR}/plugins/Plugin_ImageViewer/Plugin_ImageViewer.vpl.json"
        PROPERTIES QT_RESOURCE_ALIAS "Plugin_ImageViewer.vpl.json")
    set_source_files_properties(
        "${CMAKE_SOURCE_DIR}/plugins/Plugin_ASERStudio/Plugin_ASERStudio.vpl.json"
        PROPERTIES QT_RESOURCE_ALIAS "Plugin_ASERStudio.vpl.json")
    qt_add_resources(YayinStoryStudio "YSSBundledPluginsMeta"
        PREFIX "/plugins"
        FILES
            "${CMAKE_SOURCE_DIR}/plugins/Plugin_YSSFileExt/Plugin_YSSFileExt.vpl.json"
            "${CMAKE_SOURCE_DIR}/plugins/Plugin_ImageViewer/Plugin_ImageViewer.vpl.json"
            "${CMAKE_SOURCE_DIR}/plugins/Plugin_ASERStudio/Plugin_ASERStudio.vpl.json"
    )
endif()
