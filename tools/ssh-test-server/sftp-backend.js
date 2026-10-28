'use strict';
/*
	给测试服务器用的 SFTP 子系统实现。

	只实现编辑器/测试会用到的子集：stat / lstat / fstat / open / read / write /
	close / opendir / readdir / unlink / rename / mkdir / rmdir / realpath。
	所有路径都被限制在 rootDir 之内，越界一律当作权限不足。

	注意：这是测试用的最小实现，不做并发保护，也不要用于任何真实场景。
*/

const fs = require('fs');
const path = require('path');
const { utils: { sftp: { STATUS_CODE, OPEN_MODE } } } = require('ssh2');

// 把 SFTP 的 OPEN 标志翻译成 Node 的打开模式。
//
// ssh2 自带的 flagsToString 只认几种固定组合，而分块写入需要
// "WRITE + CREAT（不截断、从偏移处继续写）"这类组合，因此这里自己判断。
function flagsToNodeMode(flags) {
	const canRead = (flags & OPEN_MODE.READ) !== 0;
	const canWrite = (flags & OPEN_MODE.WRITE) !== 0;
	const truncate = (flags & OPEN_MODE.TRUNC) !== 0;
	const append = (flags & OPEN_MODE.APPEND) !== 0;
	if (canRead && canWrite) {
		return truncate ? 'w+' : 'r+';
	}
	if (canWrite) {
		if (append) {
			return 'a';
		}
		// 不截断时保留已有内容，从当前位置写；文件是否存在由调用方按 CREAT 决定。
		return truncate ? 'w' : 'r+';
	}
	return 'r';
}

function toAttrs(stats) {
	return {
		mode: stats.mode,
		uid: stats.uid || 0,
		gid: stats.gid || 0,
		size: stats.size,
		atime: Math.floor(stats.atimeMs / 1000),
		mtime: Math.floor(stats.mtimeMs / 1000)
	};
}

module.exports = function attachSftp(sftp, rootDir, log) {
	const root = path.resolve(rootDir);
	fs.mkdirSync(root, { recursive: true });

	const handles = new Map();
	let nextHandle = 1;

	function makeHandle(state) {
		const buffer = Buffer.alloc(4);
		const id = nextHandle++;
		handles.set(id, state);
		buffer.writeUInt32BE(id, 0, true);
		return buffer;
	}

	function getHandle(buffer) {
		if (buffer.length !== 4) {
			return null;
		}
		const id = buffer.readUInt32BE(0, true);
		return handles.get(id) || null;
	}

	// 把客户端的绝对 POSIX 路径映射到沙箱内的真实路径；越界返回 null。
	function resolve(input) {
		const normalized = path.posix.normalize('/' + String(input || '/').replace(/^\/+/, ''));
		const real = path.resolve(path.join(root, normalized));
		if (real !== root && !real.startsWith(root + path.sep)) {
			return null;
		}
		return real;
	}

	function replyStat(reqid, target, follow) {
		const real = resolve(target);
		if (real === null) {
			return sftp.status(reqid, STATUS_CODE.PERMISSION_DENIED);
		}
		try {
			const stats = follow ? fs.statSync(real) : fs.lstatSync(real);
			sftp.attrs(reqid, toAttrs(stats));
		}
		catch (error) {
			sftp.status(reqid, error.code === 'ENOENT' ? STATUS_CODE.NO_SUCH_FILE : STATUS_CODE.FAILURE);
		}
	}

	sftp.on('REALPATH', (reqid, target) => {
		const real = resolve(target);
		if (real === null) {
			return sftp.status(reqid, STATUS_CODE.PERMISSION_DENIED);
		}
		const shown = '/' + path.relative(root, real).split(path.sep).join('/');
		sftp.name(reqid, [{ filename: shown === '/' ? '/' : shown, longname: shown, attrs: {} }]);
	});

	sftp.on('STAT', (reqid, target) => replyStat(reqid, target, true));
	sftp.on('LSTAT', (reqid, target) => replyStat(reqid, target, false));

	sftp.on('FSTAT', (reqid, handle) => {
		const state = getHandle(handle);
		if (!state || state.fd === undefined) {
			return sftp.status(reqid, STATUS_CODE.FAILURE);
		}
		try {
			sftp.attrs(reqid, toAttrs(fs.fstatSync(state.fd)));
		}
		catch (error) {
			sftp.status(reqid, STATUS_CODE.FAILURE);
		}
	});

	sftp.on('OPEN', (reqid, filename, flags) => {
		const real = resolve(filename);
		if (real === null) {
			return sftp.status(reqid, STATUS_CODE.PERMISSION_DENIED);
		}
		try {
			let mode = flagsToNodeMode(flags);
			if ((flags & OPEN_MODE.EXCL) !== 0 && fs.existsSync(real)) {
				return sftp.status(reqid, STATUS_CODE.FAILURE);
			}
			let fd;
			try {
				fd = fs.openSync(real, mode, 0o644);
			}
			catch (error) {
				// 只写不截断时用 'r+' 打开；文件不存在且允许创建时退回到 'w+'。
				if (error.code === 'ENOENT' && (flags & OPEN_MODE.CREAT) !== 0) {
					mode = 'w+';
					fd = fs.openSync(real, mode, 0o644);
				}
				else {
					throw error;
				}
			}
			log(`sftp open ${filename} (${mode})`);
			sftp.handle(reqid, makeHandle({ fd }));
		}
		catch (error) {
			sftp.status(reqid, error.code === 'ENOENT' ? STATUS_CODE.NO_SUCH_FILE : STATUS_CODE.FAILURE);
		}
	});

	sftp.on('READ', (reqid, handle, offset, length) => {
		log('sftp read ' + length + '@' + offset);
		const state = getHandle(handle);
		if (!state || state.entries || state.fd === undefined) {
			return sftp.status(reqid, STATUS_CODE.FAILURE);
		}
		const buffer = Buffer.alloc(length);
		try {
			const read = fs.readSync(state.fd, buffer, 0, length, offset);
			if (read === 0) {
				return sftp.status(reqid, STATUS_CODE.EOF);
			}
			sftp.data(reqid, buffer.subarray(0, read));
		}
		catch (error) {
			sftp.status(reqid, STATUS_CODE.FAILURE);
		}
	});

	sftp.on('WRITE', (reqid, handle, offset, data) => {
		log('sftp write ' + data.length + '@' + offset);
		const state = getHandle(handle);
		if (!state || state.entries || state.fd === undefined) {
			return sftp.status(reqid, STATUS_CODE.FAILURE);
		}
		try {
			fs.writeSync(state.fd, data, 0, data.length, offset);
			sftp.status(reqid, STATUS_CODE.OK);
		}
		catch (error) {
			sftp.status(reqid, STATUS_CODE.FAILURE);
		}
	});

	sftp.on('OPENDIR', (reqid, target) => {
		const real = resolve(target);
		if (real === null) {
			return sftp.status(reqid, STATUS_CODE.PERMISSION_DENIED);
		}
		try {
			const names = fs.readdirSync(real);
			const entries = [];
			for (const name of names) {
				const entryPath = path.join(real, name);
				let attrs = {};
				try {
					attrs = toAttrs(fs.lstatSync(entryPath));
				}
				catch (error) {
					attrs = {};
				}
				entries.push({ filename: name, longname: name, attrs });
			}
			sftp.handle(reqid, makeHandle({ entries }));
		}
		catch (error) {
			sftp.status(reqid, error.code === 'ENOENT' ? STATUS_CODE.NO_SUCH_FILE : STATUS_CODE.FAILURE);
		}
	});

	sftp.on('READDIR', (reqid, handle) => {
		const state = getHandle(handle);
		if (!state || !state.entries) {
			return sftp.status(reqid, STATUS_CODE.FAILURE);
		}
		if (state.entries.length === 0) {
			return sftp.status(reqid, STATUS_CODE.EOF);
		}
		sftp.name(reqid, state.entries.splice(0, Math.min(state.entries.length, 16)));
	});

	sftp.on('CLOSE', (reqid, handle) => {
		const state = getHandle(handle);
		if (!state) {
			return sftp.status(reqid, STATUS_CODE.FAILURE);
		}
		if (state.fd !== undefined) {
			try {
				fs.closeSync(state.fd);
			}
			catch (error) {
				// 关闭失败不影响结果
			}
		}
		handles.delete(handle.readUInt32BE(0, true));
		sftp.status(reqid, STATUS_CODE.OK);
	});

	sftp.on('REMOVE', (reqid, target) => {
		const real = resolve(target);
		if (real === null) {
			return sftp.status(reqid, STATUS_CODE.PERMISSION_DENIED);
		}
		try {
			fs.unlinkSync(real);
			sftp.status(reqid, STATUS_CODE.OK);
		}
		catch (error) {
			sftp.status(reqid, error.code === 'ENOENT' ? STATUS_CODE.NO_SUCH_FILE : STATUS_CODE.FAILURE);
		}
	});

	sftp.on('RENAME', (reqid, from, to) => {
		const realFrom = resolve(from);
		const realTo = resolve(to);
		if (realFrom === null || realTo === null) {
			return sftp.status(reqid, STATUS_CODE.PERMISSION_DENIED);
		}
		try {
			fs.renameSync(realFrom, realTo);
			sftp.status(reqid, STATUS_CODE.OK);
		}
		catch (error) {
			sftp.status(reqid, error.code === 'ENOENT' ? STATUS_CODE.NO_SUCH_FILE : STATUS_CODE.FAILURE);
		}
	});

	sftp.on('MKDIR', (reqid, target) => {
		const real = resolve(target);
		if (real === null) {
			return sftp.status(reqid, STATUS_CODE.PERMISSION_DENIED);
		}
		try {
			fs.mkdirSync(real);
			sftp.status(reqid, STATUS_CODE.OK);
		}
		catch (error) {
			sftp.status(reqid, error.code === 'EEXIST' ? STATUS_CODE.FAILURE : STATUS_CODE.FAILURE);
		}
	});

	sftp.on('RMDIR', (reqid, target) => {
		const real = resolve(target);
		if (real === null) {
			return sftp.status(reqid, STATUS_CODE.PERMISSION_DENIED);
		}
		try {
			fs.rmdirSync(real);
			sftp.status(reqid, STATUS_CODE.OK);
		}
		catch (error) {
			sftp.status(reqid, error.code === 'ENOENT' ? STATUS_CODE.NO_SUCH_FILE : STATUS_CODE.FAILURE);
		}
	});

	log('sftp subsystem attached');
};
