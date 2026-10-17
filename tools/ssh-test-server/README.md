# 本地 SSH 测试服务器

用于在没有管理员权限、也不动系统 sshd 的前提下，于 `127.0.0.1` 上对
`Visindigo::Network::SshSession` 做真实的握手、认证与 exec 通道测试。

## 为什么不用系统自带的 OpenSSH Server

不是不行，而是**不好控**。项目内置的 libssh2 用 WinCNG 加密后端，算法面比现代
OpenSSH/OpenSSL 构建窄很多。用本脚本可以把服务端算法精确钉死成 WinCNG 支持的那一组，
失败时也能立刻看到是哪一方拒绝的。

WinCNG 的能力表来自 [`third_party/libssh2/src/wincng.h`](../../third_party/libssh2/src/wincng.h)：

| 能力宏 | 值 | 后果 |
|---|---|---|
| `LIBSSH2_ED25519` | `0` | 不能用 `curve25519-sha256` kex；不能用 `ssh-ed25519` 主机密钥 |
| `LIBSSH2_ECDSA` | 默认关闭 | 不能用 `ecdh-sha2-*` kex；不能用 `ecdsa-*` 主机密钥 |
| `LIBSSH2_AES_GCM` | `0` | 不能用 `aes*-gcm@openssh.com`，CBC/CTR 之外没有 AEAD |
| `LIBSSH2_RSA_SHA2` | `1` | 可以用 `rsa-sha2-256` / `rsa-sha2-512` |

因此在 `ssh2` 服务端里，我们只保留这一组：

```
kex           diffie-hellman-group14-sha256 / group16-sha512 / group14-sha1
serverHostKey rsa-sha2-512 / rsa-sha2-256 / ssh-rsa
cipher        aes128/192/256-ctr, aes128/192/256-cbc
mac           hmac-sha2-256 / hmac-sha2-512 / hmac-sha1
```

> 注意：`ssh2` 的服务端实现**没有** group exchange，而 libssh2 客户端偏好 gex。
> 若把 `diffie-hellman-group-exchange-sha256` 放进列表，协商会选中它然后服务端直接
> 报 `Group exchange not implemented for server`，握手失败。所以列表里必须排除 gex。

## 运行

```powershell
cd tools\ssh-test-server
npm install --omit=optional   # 跳过需要 node-gyp 的原生可选依赖
npm start
```

默认监听 `127.0.0.1:2222`，账号 `tester` / `tester123`。可用环境变量覆盖：

| 变量 | 默认值 |
|---|---|
| `SSH_TEST_HOST` | `127.0.0.1` |
| `SSH_TEST_PORT` | `2222` |
| `SSH_TEST_USER` | `tester` |
| `SSH_TEST_PASS` | `tester123` |
| `SSH_TEST_VERBOSE` | `1`（设为 `0` 关闭日志） |

主机密钥在首次启动时自动生成到 `hostkey_rsa.pem`（RSA 2048，已加入 `.gitignore`）。
删掉该文件即可得到一份新密钥，用来测试 `HostKeyChanged`。

## 支持的命令

`exec` 通道只模拟以下几个，其余一律退出码 127，避免被误当成真 shell：

| 命令 | 行为 |
|---|---|
| `echo <text>` | 把 `<text>` 写到 stdout，退出码 0 |
| `pwd` | `/home/<user>` |
| `whoami` | 用户名 |
| `stderr <text>` | 把 `<text>` 写到 stderr，退出码 0 |
| `fail` | stderr 输出一行，退出码 1 |
| `sleep <seconds>` | 延迟后以退出码 0 结束（测试超时/取消） |

## SFTP 子系统

`sftp-backend.js` 提供了一套以 `sftp-root/` 为沙箱的文件系统后端，实现
`stat` / `lstat` / `fstat` / `open` / `read` / `write` / `close` /
`opendir` / `readdir` / `unlink` / `rename` / `mkdir` / `rmdir` / `realpath`。
越界路径一律按 `PERMISSION_DENIED` 处理；`sftp-root/` 与主机密钥一样已加入 `.gitignore`。

沙箱在每次启动时自动创建，但**不会自动清空**——重复跑测试脚本时，若上一次留下的
目录还在，`mkdir` 会按预期失败。需要干净环境时手动删掉 `sftp-root/`。

## 验证记录

### 握手 / 认证 / 主机密钥（`SshSession`）

用 `third_party/libssh2`（WinCNG）+ 本项目探针实测通过：

```
socket connected to 127.0.0.1:2222
handshake ok
host key type=1 len=279 sha256=dYDSX/1dVvj+VOqaCMhDHpFhErXSkaIwcX+PEOZqDVs=
auth methods: password
authenticated as tester
exec started: echo hello-from-libssh2
stdout=hello-from-libssh2
exit status=0
```

服务端侧协商结果：

```
kex=diffie-hellman-group16-sha512 hostkey=rsa-sha2-512 cipher=aes256-ctr mac=hmac-sha2-256
```

### 通道与 exec（`SshChannel` / `SshExecReply`）

36 条断言、10 个场景，连续 3 轮全过：普通命令、stdout/stderr 分路、非零退出码、
未知命令（127）、执行超时、通用通道流式数据、无效参数与未就绪会话拒绝、
同一会话上并发两条 exec、会话关闭时中止通道、断开后重连再执行。

服务端日志可见同一连接上并发执行了两条命令：

```
[ssh-test-server] exec: "echo first"
[ssh-test-server] exec: "echo second"
```

### SFTP（`SftpReply`）

34 条断言、11 个场景，连续 3 轮全过：建目录、写文件、stat（大小/时间/类型）、
读文件、列根目录与子目录（含条目的尺寸与目录标志）、重命名后内容不变、
300 KiB 大文件往返、不存在的文件报 `FileNotFound`、删除文件与目录、
参数校验（空请求 / 未就绪会话）、会话关闭时中止未完成操作。

服务端可见 SFTP 请求序列（片段）：

```
[ssh-test-server] sftp open /sub/big.bin (w)
[ssh-test-server] sftp write 16384@0
...
[ssh-test-server] sftp open /sub/big.bin (r)
[ssh-test-server] sftp read 30000@0
```

## 警告

**仅供本地测试。** 私钥在本地自动生成、密码写死在代码里、没有任何权限隔离，
不要绑定到 `0.0.0.0`，不要暴露到局域网或公网。
