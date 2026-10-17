'use strict';
/*
	本地 SSH 测试服务器。

	存在的理由：项目内置的 libssh2 使用 WinCNG 加密后端，它支持的算法集合比现代
	OpenSSH/OpenSSL 构建窄得多（见 README.md 的算法表）。用本脚本可以精确地把
	服务端算法钉死成 WinCNG 能接受的那一组，从而在 127.0.0.1 上复现真实握手、
	认证与 exec 通道，而不必去改动系统 sshd 的配置。

	只用于测试，**绝不能**用于任何真实环境：私钥在本地自动生成，账号密码写死在代码里。
*/

const crypto = require('crypto');
const fs = require('fs');
const path = require('path');
const { Server } = require('ssh2');
const attachSftp = require('./sftp-backend');

const PORT = Number(process.env.SSH_TEST_PORT || 2222);
const HOST = process.env.SSH_TEST_HOST || '127.0.0.1';
const USER = process.env.SSH_TEST_USER || 'tester';
const PASS = process.env.SSH_TEST_PASS || 'tester123';
const VERBOSE = process.env.SSH_TEST_VERBOSE !== '0';

// WinCNG 后端可用的算法（见 third_party/libssh2/src/wincng.h）：
//   LIBSSH2_ED25519 0    -> 不能用 curve25519-sha256 kex，也不能用 ssh-ed25519 主机密钥
//   LIBSSH2_ECDSA 0      -> 未开 ECDSA_WINCNG 时不能用 ecdh-sha2-* 与 ecdsa-* 主机密钥
//   LIBSSH2_AES_GCM 0    -> 只能用 AES-CTR / AES-CBC
//   LIBSSH2_RSA_SHA2 1   -> 可以用 rsa-sha2-256 / rsa-sha2-512
const ALGORITHMS = {
	kex: [
		// ssh2 的服务端实现没有做 group exchange（会直接报
		// "Group exchange not implemented for server"），而 libssh2 客户端
		// 又偏好 gex，所以这里只留固定组，让协商必须落到 group14-sha256。
		'diffie-hellman-group14-sha256',
		'diffie-hellman-group16-sha512',
		'diffie-hellman-group14-sha1'
	],
	serverHostKey: ['rsa-sha2-512', 'rsa-sha2-256', 'ssh-rsa'],
	cipher: [
		'aes128-ctr',
		'aes192-ctr',
		'aes256-ctr',
		'aes128-cbc',
		'aes192-cbc',
		'aes256-cbc'
	],
	hmac: ['hmac-sha2-256', 'hmac-sha2-512', 'hmac-sha1'],
	compress: ['none']
};

function log(message) {
	if (VERBOSE) {
		console.log(`[ssh-test-server] ${message}`);
	}
}

function loadOrCreateHostKey() {
	const keyPath = path.join(__dirname, 'hostkey_rsa.pem');
	if (fs.existsSync(keyPath)) {
		return fs.readFileSync(keyPath);
	}
	const { privateKey } = crypto.generateKeyPairSync('rsa', {
		modulusLength: 2048,
		privateKeyEncoding: { type: 'pkcs1', format: 'pem' },
		publicKeyEncoding: { type: 'pkcs1', format: 'pem' }
	});
	fs.writeFileSync(keyPath, privateKey, { mode: 0o600 });
	log(`generated a throwaway RSA host key at ${keyPath}`);
	return Buffer.from(privateKey);
}

/*
	把一条命令翻译成 (stdout, stderr, exitCode)。
	只实现测试需要的几个，其余一律 127，避免让人误以为这是个真 shell。
*/
function runCommand(command) {
	const trimmed = command.trim();
	const firstWordEnd = trimmed.search(/\s/);
	const name = (firstWordEnd < 0 ? trimmed : trimmed.slice(0, firstWordEnd)).toLowerCase();
	const rest = firstWordEnd < 0 ? '' : trimmed.slice(firstWordEnd + 1).trim();

	switch (name) {
		case 'echo':
			return { stdout: rest + '\n', stderr: '', code: 0 };
		case 'pwd':
			return { stdout: `/home/${USER}\n`, stderr: '', code: 0 };
		case 'whoami':
			return { stdout: `${USER}\n`, stderr: '', code: 0 };
		case 'sleep': {
			const seconds = Number(rest) || 0;
			return { stdout: '', stderr: '', code: 0, delayMs: Math.min(seconds * 1000, 60000) };
		}
		case 'fail':
			return { stdout: '', stderr: 'requested failure\n', code: 1 };
		case 'stderr':
			return { stdout: '', stderr: rest + '\n', code: 0 };
		default:
			return { stdout: '', stderr: `${name}: command not found\n`, code: 127 };
	}
}

const hostKey = loadOrCreateHostKey();

const server = new Server({ hostKeys: [hostKey], algorithms: ALGORITHMS }, (client) => {
	log('client connected');

	client.on('handshake', (details) => {
		log(`negotiated kex=${details.kex} hostkey=${details.serverHostKey} ` +
			`cipher=${details.cs.cipher} mac=${details.cs.mac}`);
	});

	client.on('authentication', (ctx) => {
		if (ctx.method === 'password' && ctx.username === USER && ctx.password === PASS) {
			log(`password auth accepted for ${ctx.username}`);
			ctx.accept();
			return;
		}
		log(`auth rejected: method=${ctx.method} user=${ctx.username}`);
		ctx.reject(['password']);
	});

	client.on('ready', () => {
		log('session ready');
	});

	client.on('session', (accept) => {
		const session = accept();

		session.on('sftp', (acceptSftp, rejectSftp) => {
			log('sftp subsystem requested');
			const sftp = acceptSftp();
			attachSftp(sftp, path.join(__dirname, 'sftp-root'), log);
		});

		session.on('exec', (acceptExec, rejectExec, info) => {
			log(`exec: ${JSON.stringify(info.command)}`);
			const stream = acceptExec();
			const result = runCommand(info.command);
			const finish = () => {
				if (result.stdout) {
					stream.write(result.stdout);
				}
				if (result.stderr) {
					stream.stderr.write(result.stderr);
				}
				stream.exit(result.code);
				stream.end();
			};
			if (result.delayMs) {
				setTimeout(finish, result.delayMs);
			}
			else {
				finish();
			}
		});
	});

	client.on('close', () => {
		log('client disconnected');
	});

	client.on('error', (error) => {
		console.error(`[ssh-test-server] client error: ${error.message}`);
	});
});

server.on('error', (error) => {
	console.error(`[ssh-test-server] server error: ${error.message}`);
});

server.listen(PORT, HOST, () => {
	console.log(`[ssh-test-server] listening on ${HOST}:${PORT}`);
	console.log(`[ssh-test-server] user=${USER} password=${PASS}`);
	console.log('[ssh-test-server] this server is for local testing only; never expose it');
});
