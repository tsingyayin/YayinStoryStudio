# libssh2（仓库内第三方依赖）

本目录是 **原样收录** 的 libssh2 源码，供 `Visindigo` 的 SSH 支持（`Network/SshSession`）静态链接使用。
除本文件与 `libssh2.vcxproj` / `libssh2.vcxproj.filters` 外，`include/`、`src/`、`LICENSE` 均为上游文件，未做任何修改。

## 版本

| 项 | 值 |
|---|---|
| 版本 | libssh2 1.11.1 |
| 来源 | https://github.com/libssh2/libssh2/releases/tag/libssh2-1.11.1 |
| 归档 | `libssh2-1.11.1.tar.gz` |
| 许可证 | BSD-3-Clause，见 `LICENSE` |

只保留了 `include/` 与 `src/`（`docs/`、`example/`、`tests/`、`cmake/`、autotools 脚本均未收录）。
`src/` 中其它 crypto 后端（`openssl.*`、`mbedtls.*`、`libgcrypt.*`、`os400qc3.*`）保留但**不参与编译**，以便日后切换后端时不必重新下载。

## 构建配置

本仓库不使用 CMake，`libssh2.vcxproj` 是一个手写的静态库工程。

| 设置 | 值 | 原因 |
|---|---|---|
| `ConfigurationType` | `StaticLibrary` | 外部只需要一个 `libssh2.lib`，不额外分发 DLL |
| 加密后端 | `LIBSSH2_WINCNG` | 使用 Windows CNG，避免引入 OpenSSL 及其 DLL |
| `HAVE_CONFIG_H` | **不定义** | 让 `src/libssh2_setup.h` 的 `_WIN32` 内建配置生效（`HAVE_SELECT` / `HAVE_SNPRINTF` / `HAVE_STRTOLL` 等） |
| `LIBSSH2_EXPORTS` / `_WINDLL` / `LIBSSH2_LIBRARY` | **不定义** | 三者都不定义时 `LIBSSH2_API` 展开为空，即静态链接语义 |
| `CompileAs` | `CompileAsC` | libssh2 是 C 代码，不要按 C++ 编译 |
| `RuntimeLibrary` | Debug `/MDd`、Release `/MD` | 必须与消费方（`Visindigo`）一致 |

### 参与编译的源文件（26 个）

即上游 `src/Makefile.inc` 的 `CSOURCES`。以下三个 `.c` **被 `#include` 进别的翻译单元，不能单独编译**：

- `wincng.c` ← 由 `crypto.c` 包含
- `blowfish.c` ← 由 `bcrypt_pbkdf.c` 包含
- `agent_win.c` ← 由 `agent.c` 在 `_WIN32` 下包含

### 链接依赖

- `bcrypt.lib`、`crypt32.lib`：`wincng.c` 自带 `#pragma comment(lib, ...)`，消费方无需再写
- `ws2_32.lib`：由最终链接目标（`Visindigo`）提供

## 如何升级

1. 下载新版 `libssh2-<version>.tar.gz`
2. 用新归档的 `include/`、`src/`、`COPYING` 覆盖本目录（`COPYING` 改名为 `LICENSE`）
3. 若 `src/Makefile.inc` 的 `CSOURCES` 有变化，同步修改 `libssh2.vcxproj` 与 `.filters` 的 `<ClCompile>` 列表
4. 更新本文件顶部的版本表
