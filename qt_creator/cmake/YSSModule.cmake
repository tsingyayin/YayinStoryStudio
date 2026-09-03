# ============================================================================
#  YSSModule.cmake — YayinStoryStudio CMake 通用辅助
# ============================================================================

# ---------------------------------------------------------------------------
# yss_collect_sources(<out_var> <module_dir> [EXCLUDE_REGEX ...])
#
# 递归收集 <module_dir> 下需要参与编译的 .cpp 与 .h。
#  .h 一并纳入目标源列表，是为了让 Qt6 的 AUTOMOC 能正确为 PIMPL 私有头
#  (*_p.h，含 Q_OBJECT) 与纯头 Q_OBJECT 类生成 moc。
#
# 自动剔除：
#   - 各模块 src 下自带的 x64/ MSBuild 中间产物目录
# 额外的排除规则通过正则以参数传入。
# ---------------------------------------------------------------------------
function(yss_collect_sources out_var module_dir)
    file(GLOB_RECURSE _files CONFIGURE_DEPENDS
        "${module_dir}/*.cpp"
        "${module_dir}/*.h"
    )
    # file(GLOB) 返回的路径统一为正斜杠，可放心用正则过滤
    list(FILTER _files EXCLUDE REGEX "/x64/")
    foreach(_pat IN LISTS ARGN)
        list(FILTER _files EXCLUDE REGEX "${_pat}")
    endforeach()
    set(${out_var} ${_files} PARENT_SCOPE)
endfunction()

# ---------------------------------------------------------------------------
# yss_add_qt(<target> <modules...>)
# 为 <target> 链接一组 Qt6 模块（PUBLIC，会向使用者传播 include/define）
# ---------------------------------------------------------------------------
function(yss_add_qt target)
    foreach(_m IN LISTS ARGN)
        target_link_libraries(${target} PUBLIC "Qt6::${_m}")
    endforeach()
endfunction()

# ---------------------------------------------------------------------------
# yss_export_plugin(<target> <pkg_dir> <vpl_json>)
# 插件构建完成后，按发布布局复制到 ${CMAKE_SOURCE_DIR}/plugin_build/<pkg_dir>/：
#   <target>.vpl        -- 沿用 .vpl 后缀约定(内容为当前平台插件二进制：
#                           Android 为 lib*.so 改名，桌面为 Plugin_*.dll 改名)
#   <target>.vpl.json   -- 插件元数据(ID/Dependencies)
# 之后即可把 plugin_build/<pkg_dir> 整体复制到插件目录。
# ---------------------------------------------------------------------------
function(yss_export_plugin target pkg_dir vpl_json)
    set(_out_dir "${CMAKE_SOURCE_DIR}/plugin_build/${pkg_dir}")
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${_out_dir}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:${target}>" "${_out_dir}/${target}.vpl"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${vpl_json}" "${_out_dir}/${target}.vpl.json"
        COMMENT "Exporting plugin '${target}' -> plugin_build/${pkg_dir}"
        VERBATIM)
endfunction()
