# ============================================================================
#  Visindigo — 基础框架库
#
#  生产(Windows DllRelease)形态为 DLL；本工程在 Android 下产出 libVisindigo.so，
#  目录结构与依赖全部沿用真实源码 src/Visindigo/。
#
#  Qt 依赖: Core / Gui / Widgets / Network(LoggerManager 硬件上报) / Sql(VirtualStorage)
#  导出宏:  Visindigo_EXPORT (MSVC 下 = __declspec(dllexport)；非 MSVC 为空)
# ============================================================================
set(_mod_dir "${YSS_SRC_ROOT}/Visindigo")

# 排除模块根 main.cpp —— 那是桌面单跑调试用的独立入口，不随库编译
yss_collect_sources(_src "${_mod_dir}" "main\\.cpp$")

# DesktopHacker.cpp：Win32-only 且全仓无实际调用方(MainWin.cpp 中仅为注释掉的代码)，
# 且与 Qt6 CMake 注入的 UNICODE 宏不相容(窄字符串 Win32 API)。本工程统一不编译。
list(FILTER _src EXCLUDE REGEX "DesktopHacker\\.cpp$")

# automoc 误报处理：JsonConfig.cpp 中的 Q_GADGET/Q_OBJECT 仅出现在文档注释里，
# 真实元对象宏都在头文件；跳过对该 .cpp 运行 automoc（不影响 Qt VS Tools 桌面构建）。
set_source_files_properties(
    "${_mod_dir}/Utility/cpp/JsonConfig.cpp"
    PROPERTIES SKIP_AUTOMOC TRUE
)

# MessageCenter.h：有头无实现、全仓无使用者的废弃预留类（虚析构仅声明未定义）。
# AUTOMOC 对纯头 Q_OBJECT 强制生成 moc 后，在 Clang(Android) 下会强引用其 vtable/析构
# 导致链接失败；桌面 .sln 本就不编译它。故从本工程编译源排除。
list(FILTER _src EXCLUDE REGEX "MessageCenter\\.h$")

add_library(Visindigo SHARED ${_src} "${_mod_dir}/Visindigo.qrc")

# 源码按模块根相对路径引用头文件（如 "General/Translator.h"、"Widgets/...h"）
target_include_directories(Visindigo PUBLIC "${_mod_dir}")

# 以 DLL/共享库形态导出自身符号（等价于桌面 DllRelease 配置）
target_compile_definitions(Visindigo PRIVATE Visindigo_EXPORT)

target_link_libraries(Visindigo PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Sql
)

# Windows 专属链接依赖：Exception.cpp(Q_OS_WIN 分支) 的 MiniDumpWriteDump 需要 Dbghelp
# (仅 MSVC/桌面需要；Android 目标因 Q_OS_WIN 守卫不会引用该符号)
if(WIN32)
    target_link_libraries(Visindigo PRIVATE dbghelp)
endif()
