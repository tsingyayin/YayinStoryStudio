#include <atomic>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <DbgHelp.h>
#else
#include <ctime>
#include <sys/stat.h>
#include <unistd.h>
#endif
#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qstring.h>
#include "General/CrashGateway.h"
#include "General/Exception.h"
#include "General/VIApplication.h"

namespace Visindigo::__Private__ {
	using namespace Visindigo::General;

	struct CrashStackFrame {
		enum Limits {
			FunctionLength = 512,
			BinaryLength = 128,
			SourceLength = 260,
		};
		quintptr Address = 0;
		quint64 Line = 0;
		bool Resolved = false;
		bool AddressIsAbsolute = false;
		wchar_t Function[FunctionLength] = {};
		wchar_t BinaryFile[BinaryLength] = {};
		wchar_t SourceFile[SourceLength] = {};
	};

	struct CrashSnapshot {
		enum Limits {
			PathLength = 260,
			MessageLength = 512,
			FunctionLength = 512,
			MaxFrames = 64,
		};
		bool Native = false;
		bool Critical = true;
		qint32 Type = 0;
		quint32 NativeCode = 0;
		quint32 ProcessId = 0;
		quint32 ThreadId = 0;
		quintptr FaultAddress = 0;
		qint32 Line = 0;
		quint32 FrameCount = 0;
		quint32 MiniDumpType = 0;
		void* NativeInfo = nullptr;
		wchar_t Message[MessageLength] = {};
		wchar_t File[PathLength] = {};
		wchar_t Function[FunctionLength] = {};
		wchar_t ReportPath[PathLength] = {};
		wchar_t DumpPath[PathLength] = {};
		CrashStackFrame Frames[MaxFrames];
	};

	struct TimeParts {
		int Year = 0;
		int Month = 0;
		int Day = 0;
		int Hour = 0;
		int Minute = 0;
		int Second = 0;
		int Millisecond = 0;
	};

	struct CrashGatewayState {
		std::atomic<bool> Installed = false;
		std::atomic<bool> Handling = false;
		std::atomic<bool> DumpEnabled = true;
		std::atomic<bool> ResolveSymbols = true;
		std::atomic<bool> SymbolsReady = false;
		std::atomic<quint32> MiniDumpType = 0;
		std::atomic<quint32> MainThreadId = 0;
		std::atomic<ApplicationExceptionMessageHandler*> MessageHandler = nullptr;
		wchar_t ReportFolder[CrashSnapshot::PathLength] = {};
		wchar_t LogFolder[CrashSnapshot::PathLength] = {};
		wchar_t LogFileName[CrashSnapshot::PathLength] = {};
		wchar_t ProductInfo[CrashSnapshot::MessageLength] = {};
		wchar_t HardwareInfo[8192] = {};
	};
	static CrashGatewayState state;

	struct HandlingGate {
		bool Acquired = false;
		HandlingGate() {
			bool expected = false;
			Acquired = state.Handling.compare_exchange_strong(expected, true);
		}
		~HandlingGate() {
			if (Acquired) {
				state.Handling.store(false);
			}
		}
	};

	// ---------- 无分配字符串工具 ----------

	static void copyW(wchar_t* dest, size_t destLen, const wchar_t* src) {
		if (dest == nullptr || destLen == 0) {
			return;
		}
		if (src == nullptr) {
			dest[0] = L'\0';
			return;
		}
		size_t count = wcslen(src);
		if (count > destLen - 1) {
			count = destLen - 1;
		}
		wmemcpy(dest, src, count);
		dest[count] = L'\0';
	}

	static void copyW(wchar_t* dest, size_t destLen, const QString& src) {
		if (dest == nullptr || destLen == 0) {
			return;
		}
		size_t count = static_cast<size_t>(src.size());
		if (count > destLen - 1) {
			count = destLen - 1;
		}
		src.left(static_cast<int>(count)).toWCharArray(dest);
		dest[count] = L'\0';
	}

	static void formatV(wchar_t* dest, size_t destLen, const wchar_t* format, va_list args) {
		if (dest == nullptr || destLen == 0) {
			return;
		}
		dest[0] = L'\0';
#ifdef Q_OS_WIN
		_vsnwprintf_s(dest, destLen, _TRUNCATE, format, args);
#else
		va_list copied;
		va_copy(copied, args);
		vswprintf(dest, destLen, format, copied);
		va_end(copied);
#endif
	}

	static void formatW(wchar_t* dest, size_t destLen, const wchar_t* format, ...) {
		va_list args;
		va_start(args, format);
		formatV(dest, destLen, format, args);
		va_end(args);
	}

	// 崩溃期把宽字符串转成 UTF-8：不依赖本地化设置，也不做任何分配。
	static size_t encodeUtf8(const wchar_t* source, char* dest, size_t destLen) {
		if (dest == nullptr || destLen == 0) {
			return 0;
		}
		dest[0] = '\0';
#ifdef Q_OS_WIN
		int written = WideCharToMultiByte(CP_UTF8, 0, source, -1, dest, static_cast<int>(destLen), nullptr, nullptr);
		return written > 0 ? static_cast<size_t>(written - 1) : 0;
#else
		size_t used = 0;
		for (const wchar_t* cursor = source; *cursor != 0; ++cursor) {
			unsigned int code = static_cast<unsigned int>(*cursor);
			char encoded[4];
			size_t length = 0;
			if (code < 0x80) {
				encoded[length++] = static_cast<char>(code);
			}
			else if (code < 0x800) {
				encoded[length++] = static_cast<char>(0xC0 | (code >> 6));
				encoded[length++] = static_cast<char>(0x80 | (code & 0x3F));
			}
			else {
				encoded[length++] = static_cast<char>(0xE0 | (code >> 12));
				encoded[length++] = static_cast<char>(0x80 | ((code >> 6) & 0x3F));
				encoded[length++] = static_cast<char>(0x80 | (code & 0x3F));
			}
			if (used + length + 1 > destLen) {
				break;
			}
			memcpy(dest + used, encoded, length);
			used += length;
		}
		dest[used] = '\0';
		return used;
#endif
	}

	// ---------- 时间（不经过 QDateTime） ----------

	static void readLocalTime(TimeParts& parts) {
#ifdef Q_OS_WIN
		SYSTEMTIME time;
		GetLocalTime(&time);
		parts.Year = time.wYear;
		parts.Month = time.wMonth;
		parts.Day = time.wDay;
		parts.Hour = time.wHour;
		parts.Minute = time.wMinute;
		parts.Second = time.wSecond;
		parts.Millisecond = time.wMilliseconds;
#else
		struct timespec now;
		clock_gettime(CLOCK_REALTIME, &now);
		struct tm broken;
		localtime_r(&now.tv_sec, &broken);
		parts.Year = broken.tm_year + 1900;
		parts.Month = broken.tm_mon + 1;
		parts.Day = broken.tm_mday;
		parts.Hour = broken.tm_hour;
		parts.Minute = broken.tm_min;
		parts.Second = broken.tm_sec;
		parts.Millisecond = static_cast<int>(now.tv_nsec / 1000000);
#endif
	}

	// 与 VIApplication::LogFileNameTimeFormat 的默认值保持同形，便于人工把报告与日志对上。
	static void formatStamp(const TimeParts& parts, wchar_t* dest, size_t destLen) {
		formatW(dest, destLen, L"%04d-%02d-%02d_%02d_%02d_%02d",
			parts.Year, parts.Month, parts.Day, parts.Hour, parts.Minute, parts.Second);
	}

	static void formatDisplay(const TimeParts& parts, wchar_t* dest, size_t destLen) {
		formatW(dest, destLen, L"%04d-%02d-%02d %02d:%02d:%02d.%03d",
			parts.Year, parts.Month, parts.Day, parts.Hour, parts.Minute, parts.Second, parts.Millisecond);
	}

	// ---------- 快照 ----------

	static CrashSnapshot& snapshotStorage() {
		static CrashSnapshot buffer;
		return buffer;
	}

	static void resetSnapshot(CrashSnapshot& snapshot, bool native, bool critical) {
		snapshot.Native = native;
		snapshot.Critical = critical;
		snapshot.NativeCode = 0;
		snapshot.NativeInfo = nullptr;
		snapshot.FaultAddress = 0;
		snapshot.Line = 0;
		snapshot.FrameCount = 0;
		snapshot.Type = static_cast<qint32>(Exception::Unknown);
		snapshot.MiniDumpType = state.MiniDumpType.load();
		snapshot.ReportPath[0] = L'\0';
		snapshot.DumpPath[0] = L'\0';
		snapshot.Message[0] = L'\0';
		snapshot.File[0] = L'\0';
		snapshot.Function[0] = L'\0';
		for (quint32 i = 0; i < CrashSnapshot::MaxFrames; ++i) {
			snapshot.Frames[i].Address = 0;
			snapshot.Frames[i].Line = 0;
			snapshot.Frames[i].Resolved = false;
			snapshot.Frames[i].AddressIsAbsolute = false;
			snapshot.Frames[i].Function[0] = L'\0';
			snapshot.Frames[i].BinaryFile[0] = L'\0';
			snapshot.Frames[i].SourceFile[0] = L'\0';
		}
		TimeParts parts;
		readLocalTime(parts);
#ifdef Q_OS_WIN
		snapshot.ProcessId = static_cast<quint32>(GetCurrentProcessId());
		snapshot.ThreadId = static_cast<quint32>(GetCurrentThreadId());
#else
		snapshot.ProcessId = static_cast<quint32>(getpid());
		snapshot.ThreadId = 0;
#endif
	}

	static void fillFramesFromException(CrashSnapshot& snapshot, const Exception& ex) {
		const QList<StacktraceFrame> frames = ex.getStacktrace();
		quint32 count = 0;
		for (const StacktraceFrame& frame : frames) {
			if (count >= CrashSnapshot::MaxFrames) {
				break;
			}
			CrashStackFrame& target = snapshot.Frames[count];
			target.Address = static_cast<quintptr>(frame.getAddress());
			target.Line = static_cast<quint64>(frame.getLineNumber());
			target.Resolved = true;
			target.AddressIsAbsolute = false;
			copyW(target.Function, CrashStackFrame::FunctionLength, frame.getFunctionName());
			copyW(target.BinaryFile, CrashStackFrame::BinaryLength, frame.getBinaryFileName());
			copyW(target.SourceFile, CrashStackFrame::SourceLength, frame.getSourceFileName());
			count++;
		}
		snapshot.FrameCount = count;
	}

	static void fillSnapshotFromException(CrashSnapshot& snapshot, const Exception& ex) {
		resetSnapshot(snapshot, false, ex.isCritical());
		snapshot.Type = static_cast<qint32>(ex.getType());
		snapshot.Line = static_cast<qint32>(ex.getLine());
		copyW(snapshot.Message, CrashSnapshot::MessageLength, ex.getMessage());
		copyW(snapshot.File, CrashSnapshot::PathLength, ex.getFile());
		copyW(snapshot.Function, CrashSnapshot::FunctionLength, ex.getFunction());
		fillFramesFromException(snapshot, ex);
	}

	// 快照里没有堆栈时才去采集：来自异常的堆栈已经带齐了信息，不需要再走一次符号解析。
	static void captureFrames(CrashSnapshot& snapshot);

	static bool captureFramesIfNeeded(CrashSnapshot& snapshot) {
		if (snapshot.FrameCount == 0) {
			captureFrames(snapshot);
		}
		return snapshot.FrameCount > 0;
	}

	// ---------- 堆栈采集（与符号解析） ----------

	static void captureFrames(CrashSnapshot& snapshot) {
#ifdef Q_OS_WIN
		quint32 count = 0;
		if (snapshot.NativeInfo != nullptr) {
			EXCEPTION_POINTERS* info = static_cast<EXCEPTION_POINTERS*>(snapshot.NativeInfo);
			CONTEXT context;
			if (info->ContextRecord != nullptr) {
				context = *info->ContextRecord;
			}
			else {
				RtlCaptureContext(&context);
			}
			HANDLE thread = nullptr;
			bool closeThread = false;
			if (GetCurrentThreadId() == snapshot.ThreadId) {
				thread = GetCurrentThread();
			}
			else {
				thread = OpenThread(THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME, FALSE, snapshot.ThreadId);
				closeThread = true;
				if (thread != nullptr) {
					SuspendThread(thread);
				}
			}
			if (thread != nullptr) {
				STACKFRAME64 frame;
				memset(&frame, 0, sizeof(frame));
				DWORD machine = IMAGE_FILE_MACHINE_UNKNOWN;
#if defined(_M_X64) || defined(__x86_64__)
				machine = IMAGE_FILE_MACHINE_AMD64;
				frame.AddrPC.Offset = context.Rip;
				frame.AddrFrame.Offset = context.Rbp;
				frame.AddrStack.Offset = context.Rsp;
#elif defined(_M_ARM64)
				machine = IMAGE_FILE_MACHINE_ARM64;
				frame.AddrPC.Offset = context.Pc;
				frame.AddrFrame.Offset = context.Fp;
				frame.AddrStack.Offset = context.Sp;
#endif
				frame.AddrPC.Mode = AddrModeFlat;
				frame.AddrFrame.Mode = AddrModeFlat;
				frame.AddrStack.Mode = AddrModeFlat;
				HANDLE process = GetCurrentProcess();
				while (count < CrashSnapshot::MaxFrames) {
					if (!StackWalk64(machine, process, thread, &frame, &context, nullptr,
						SymFunctionTableAccess64, SymGetModuleBase64, nullptr)) {
						break;
					}
					if (frame.AddrPC.Offset == 0) {
						break;
					}
					CrashStackFrame& target = snapshot.Frames[count];
					target.Address = static_cast<quintptr>(frame.AddrPC.Offset);
					target.AddressIsAbsolute = true;
					count++;
				}
				if (closeThread) {
					ResumeThread(thread);
					CloseHandle(thread);
				}
			}
		}
		if (count == 0) {
			void* addresses[CrashSnapshot::MaxFrames];
			USHORT captured = RtlCaptureStackBackTrace(0, CrashSnapshot::MaxFrames, addresses, nullptr);
			for (quint32 i = 0; i < captured && i < CrashSnapshot::MaxFrames; ++i) {
				snapshot.Frames[i].Address = reinterpret_cast<quintptr>(addresses[i]);
				snapshot.Frames[i].AddressIsAbsolute = true;
				count++;
			}
		}
		snapshot.FrameCount = count;
#endif
	}

#ifdef Q_OS_WIN
	// 该函数会进入堆已不可信的区域：只允许栈上 POD，绝不声明任何带析构的局部量，
	// 否则 MSVC 会因为需要栈展开而拒绝 __try（C2712）。
	static void resolveFramesGuarded(CrashSnapshot& snapshot) {
		HANDLE process = GetCurrentProcess();
		for (quint32 i = 0; i < snapshot.FrameCount; ++i) {
			CrashStackFrame& frame = snapshot.Frames[i];
			if (frame.Resolved || frame.Address == 0) {
				continue;
			}
			DWORD64 address = static_cast<DWORD64>(frame.Address);
			IMAGEHLP_MODULEW64 moduleInfo;
			memset(&moduleInfo, 0, sizeof(moduleInfo));
			moduleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULEW64);
			if (SymGetModuleInfoW64(process, address, &moduleInfo)) {
				const wchar_t* module = moduleInfo.ImageName[0] != 0 ? moduleInfo.ImageName : moduleInfo.LoadedImageName;
				copyW(frame.BinaryFile, CrashStackFrame::BinaryLength, module);
				if (frame.AddressIsAbsolute && moduleInfo.BaseOfImage != 0) {
					frame.Address = static_cast<quintptr>(address - moduleInfo.BaseOfImage);
				}
			}
			char symbolStorage[sizeof(SYMBOL_INFOW) + 256 * sizeof(wchar_t)];
			memset(symbolStorage, 0, sizeof(symbolStorage));
			SYMBOL_INFOW* symbol = reinterpret_cast<SYMBOL_INFOW*>(symbolStorage);
			symbol->SizeOfStruct = sizeof(SYMBOL_INFOW);
			symbol->MaxNameLen = 255;
			DWORD64 displacement = 0;
			if (SymFromAddrW(process, address, &displacement, symbol)) {
				copyW(frame.Function, CrashStackFrame::FunctionLength, symbol->Name);
			}
			IMAGEHLP_LINEW64 lineInfo;
			memset(&lineInfo, 0, sizeof(lineInfo));
			lineInfo.SizeOfStruct = sizeof(IMAGEHLP_LINEW64);
			DWORD lineDisplacement = 0;
			if (SymGetLineFromAddrW64(process, address, &lineDisplacement, &lineInfo)) {
				copyW(frame.SourceFile, CrashStackFrame::SourceLength, lineInfo.FileName);
				frame.Line = static_cast<quint64>(lineInfo.LineNumber);
			}
			frame.Resolved = true;
		}
	}
#endif

	static void resolveFrames(CrashSnapshot& snapshot) {
#ifdef Q_OS_WIN
		if (!state.SymbolsReady.load()) {
			return;
		}
		__try {
			resolveFramesGuarded(snapshot);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			// 符号化失败不影响转储与报告，地址依然有效。
		}
#endif
	}

	// ---------- 平台后端 ----------

#ifdef Q_OS_WIN
	typedef BOOL(WINAPI* MiniDumpWriteDumpProc)(HANDLE, DWORD, HANDLE, MINIDUMP_TYPE,
		PMINIDUMP_EXCEPTION_INFORMATION, PMINIDUMP_USER_STREAM_INFORMATION, PMINIDUMP_CALLBACK_INFORMATION);

	struct Win32Backend {
		HMODULE Module = nullptr;
		MiniDumpWriteDumpProc WriteDump = nullptr;
		LPTOP_LEVEL_EXCEPTION_FILTER PreviousFilter = nullptr;
	};
	static Win32Backend backend;

	static const wchar_t* exceptionCodeName(DWORD code) {
		switch (code) {
		case EXCEPTION_ACCESS_VIOLATION: return L"EXCEPTION_ACCESS_VIOLATION";
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return L"EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
		case EXCEPTION_BREAKPOINT: return L"EXCEPTION_BREAKPOINT";
		case EXCEPTION_DATATYPE_MISALIGNMENT: return L"EXCEPTION_DATATYPE_MISALIGNMENT";
		case EXCEPTION_FLT_DIVIDE_BY_ZERO: return L"EXCEPTION_FLT_DIVIDE_BY_ZERO";
		case EXCEPTION_FLT_OVERFLOW: return L"EXCEPTION_FLT_OVERFLOW";
		case EXCEPTION_ILLEGAL_INSTRUCTION: return L"EXCEPTION_ILLEGAL_INSTRUCTION";
		case EXCEPTION_IN_PAGE_ERROR: return L"EXCEPTION_IN_PAGE_ERROR";
		case EXCEPTION_INT_DIVIDE_BY_ZERO: return L"EXCEPTION_INT_DIVIDE_BY_ZERO";
		case EXCEPTION_INT_OVERFLOW: return L"EXCEPTION_INT_OVERFLOW";
		case EXCEPTION_INVALID_DISPOSITION: return L"EXCEPTION_INVALID_DISPOSITION";
		case EXCEPTION_NONCONTINUABLE_EXCEPTION: return L"EXCEPTION_NONCONTINUABLE_EXCEPTION";
		case EXCEPTION_PRIV_INSTRUCTION: return L"EXCEPTION_PRIV_INSTRUCTION";
		case EXCEPTION_SINGLE_STEP: return L"EXCEPTION_SINGLE_STEP";
		case EXCEPTION_STACK_OVERFLOW: return L"EXCEPTION_STACK_OVERFLOW";
		case 0xC0000409: return L"STATUS_STACK_BUFFER_OVERRUN (fail fast)";
		case 0xE06D7363: return L"MSVC C++ exception";
		default: return L"unknown";
		}
	}

	static bool loadDbgHelp() {
		// 崩溃后再 LoadLibrary 可能因为 loader lock 卡死，因此在 install 阶段就把它拉起来。
		backend.Module = LoadLibraryW(L"dbghelp.dll");
		if (backend.Module == nullptr) {
			return false;
		}
		backend.WriteDump = reinterpret_cast<MiniDumpWriteDumpProc>(
			GetProcAddress(backend.Module, "MiniDumpWriteDump"));
		return backend.WriteDump != nullptr;
	}

	static bool initializeSymbols() {
		SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES
			| SYMOPT_NO_PROMPTS | SYMOPT_FAIL_CRITICAL_ERRORS);
		if (!SymInitializeW(GetCurrentProcess(), nullptr, TRUE)) {
			return false;
		}
		state.SymbolsReady.store(true);
		return true;
	}

	static void cleanupSymbols() {
		if (state.SymbolsReady.exchange(false)) {
			SymCleanup(GetCurrentProcess());
		}
	}

	static bool fileExists(const wchar_t* path) {
		return GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES;
	}
#else
	static bool fileExists(const wchar_t* path) {
		char narrow[CrashSnapshot::PathLength * 3];
		if (encodeUtf8(path, narrow, sizeof(narrow)) == 0) {
			return false;
		}
		FILE* file = fopen(narrow, "rb");
		if (file == nullptr) {
			return false;
		}
		fclose(file);
		return true;
	}
#endif

	// ---------- 报告写入 ----------

	struct ReportWriter {
#ifdef Q_OS_WIN
		HANDLE File = INVALID_HANDLE_VALUE;
#else
		FILE* File = nullptr;
#endif
		wchar_t Line[2048] = {};
		char Bytes[8300] = {};

		~ReportWriter() {
			Close();
		}

		bool Open(const wchar_t* path, bool overwrite = false) {
#ifdef Q_OS_WIN
			File = CreateFileW(path, GENERIC_WRITE, 0, nullptr,
				overwrite ? CREATE_ALWAYS : CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
			return File != INVALID_HANDLE_VALUE;
#else
			char narrow[CrashSnapshot::PathLength * 3];
			if (encodeUtf8(path, narrow, sizeof(narrow)) == 0) {
				return false;
			}
			File = fopen(narrow, overwrite ? "wb" : "wbx");
			return File != nullptr;
#endif
		}

		void Close() {
#ifdef Q_OS_WIN
			if (File != INVALID_HANDLE_VALUE) {
				FlushFileBuffers(File);
				CloseHandle(File);
				File = INVALID_HANDLE_VALUE;
			}
#else
			if (File != nullptr) {
				fclose(File);
				File = nullptr;
			}
#endif
		}

		bool Valid() const {
#ifdef Q_OS_WIN
			return File != INVALID_HANDLE_VALUE;
#else
			return File != nullptr;
#endif
		}

		void WriteText(const wchar_t* text) {
			if (!Valid() || text == nullptr) {
				return;
			}
			size_t length = encodeUtf8(text, Bytes, sizeof(Bytes));
			if (length == 0) {
				return;
			}
#ifdef Q_OS_WIN
			DWORD written = 0;
			WriteFile(File, Bytes, static_cast<DWORD>(length), &written, nullptr);
#else
			fwrite(Bytes, 1, length, File);
#endif
		}

		void WriteLine(const wchar_t* text) {
			WriteText(text);
			WriteText(L"\r\n");
		}

		void WriteFormatLine(const wchar_t* format, ...) {
			va_list args;
			va_start(args, format);
			formatV(Line, 2048, format, args);
			va_end(args);
			WriteLine(Line);
		}

		void WriteRaw(const wchar_t* text) {
			if (text == nullptr) {
				return;
			}
			const wchar_t* cursor = text;
			while (*cursor != 0) {
				const wchar_t* lineEnd = wcschr(cursor, L'\n');
				size_t length = lineEnd == nullptr ? wcslen(cursor) : static_cast<size_t>(lineEnd - cursor);
				while (length > 0 && (cursor[length - 1] == L'\r' || cursor[length - 1] == L' ')) {
					length--;
				}
				size_t copied = length < 2047 ? length : 2047;
				wmemcpy(Line, cursor, copied);
				Line[copied] = L'\0';
				WriteLine(Line);
				if (lineEnd == nullptr) {
					break;
				}
				cursor = lineEnd + 1;
			}
		}
	};

	static bool writeCrashReport(const CrashSnapshot& snapshot) {
		ReportWriter writer;
		if (!writer.Open(snapshot.ReportPath)) {
			return false;
		}
		wchar_t typeText[128] = {};
		if (!snapshot.Native && snapshot.Type != static_cast<qint32>(Exception::Unknown)) {
			copyW(typeText, 128, Exception::typeToString(static_cast<Exception::Type>(snapshot.Type)));
		}
		TimeParts parts;
		readLocalTime(parts);
		wchar_t display[64];
		formatDisplay(parts, display, 64);

		writer.WriteLine(L"Visindigo Crash Report");
		writer.WriteFormatLine(L"Generated on %s", display);
		if (state.ProductInfo[0] != 0) {
			writer.WriteFormatLine(L"Product: %s", state.ProductInfo);
		}
		writer.WriteLine(L"");
		writer.WriteFormatLine(L"Crash Source: %hs", snapshot.Native ? "native fault" : "caught exception");
		bool crashOnMainThread = snapshot.ThreadId == 0
			|| snapshot.ThreadId == state.MainThreadId.load();
		writer.WriteFormatLine(L"Process ID: %u / Thread ID: %u (%s)", snapshot.ProcessId, snapshot.ThreadId,
			crashOnMainThread ? L"main thread" : L"worker thread");
		writer.WriteFormatLine(L"Report Folder: %s", state.ReportFolder);
		writer.WriteLine(L"");

		if (snapshot.Function[0] != 0 || snapshot.File[0] != 0) {
			writer.WriteLine(L"When executing function:");
			writer.WriteFormatLine(L"%s (%s:%d)", snapshot.Function, snapshot.File, snapshot.Line);
			writer.WriteLine(L"");
		}

		if (typeText[0] != 0 || snapshot.Message[0] != 0) {
			writer.WriteLine(L"The following exception was thrown:");
			writer.WriteFormatLine(L"Critical: %s", snapshot.Critical ? L"Yes" : L"No");
			if (typeText[0] != 0) {
				writer.WriteFormatLine(L"Exception Type: %s", typeText);
			}
			if (snapshot.Message[0] != 0) {
				writer.WriteFormatLine(L"Exception Message: %s", snapshot.Message);
			}
			writer.WriteLine(L"");
		}

		if (snapshot.Native) {
			writer.WriteLine(L"Native Information:");
#ifdef Q_OS_WIN
			writer.WriteFormatLine(L"Exception Code: 0x%08lX (%s)", static_cast<unsigned long>(snapshot.NativeCode),
				exceptionCodeName(snapshot.NativeCode));
#else
			writer.WriteFormatLine(L"Exception Code: 0x%08X", snapshot.NativeCode);
#endif
			writer.WriteFormatLine(L"Fault Address: 0x%016llX", static_cast<unsigned long long>(snapshot.FaultAddress));
			writer.WriteLine(L"");
		}

		writer.WriteLine(L"Stacktrace:");
		writer.WriteLine(L"Index\tBinary File ! Function (+Address) in Source File at Line Number");
		if (snapshot.FrameCount == 0) {
			writer.WriteLine(L"(no stacktrace available)");
		}
		for (quint32 i = 0; i < snapshot.FrameCount; ++i) {
			const CrashStackFrame& frame = snapshot.Frames[i];
			const wchar_t* function = frame.Function[0] != 0 ? frame.Function : L"<unknown>";
			const wchar_t* binary = frame.BinaryFile[0] != 0 ? frame.BinaryFile : L"<unknown>";
			const wchar_t* source = frame.SourceFile[0] != 0 ? frame.SourceFile : L"<unknown>";
			writer.WriteFormatLine(L"%u\t%s ! %s (+%016llX) in %s at %llu", i, binary, function,
				static_cast<unsigned long long>(frame.Address), source,
				static_cast<unsigned long long>(frame.Line));
		}
		writer.WriteLine(L"");

		if (snapshot.DumpPath[0] != 0) {
			writer.WriteLine(L"Artifacts:");
			writer.WriteFormatLine(L"Dump: %s", snapshot.DumpPath);
			writer.WriteLine(L"");
		}

		writer.WriteLine(L"=========================================");
		if (state.HardwareInfo[0] != 0) {
			writer.WriteRaw(state.HardwareInfo);
		}
		writer.Close();
		return true;
	}

	// ---------- 转储与产物 ----------

	static bool writeMiniDump(CrashSnapshot& snapshot) {
#ifdef Q_OS_WIN
		if (backend.WriteDump == nullptr || !state.DumpEnabled.load()) {
			return false;
		}
		HANDLE file = CreateFileW(snapshot.DumpPath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL, nullptr);
		if (file == INVALID_HANDLE_VALUE) {
			return false;
		}
		MINIDUMP_EXCEPTION_INFORMATION information;
		memset(&information, 0, sizeof(information));
		bool hasInformation = false;
		if (snapshot.NativeInfo != nullptr) {
			EXCEPTION_POINTERS* pointers = static_cast<EXCEPTION_POINTERS*>(snapshot.NativeInfo);
			if (pointers->ExceptionRecord != nullptr) {
				information.ThreadId = snapshot.ThreadId;
				information.ExceptionPointers = pointers;
				information.ClientPointers = FALSE;
				hasInformation = true;
			}
		}
		BOOL written = FALSE;
		__try {
			written = backend.WriteDump(GetCurrentProcess(), GetCurrentProcessId(), file,
				static_cast<MINIDUMP_TYPE>(snapshot.MiniDumpType),
				hasInformation ? &information : nullptr, nullptr, nullptr);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			written = FALSE;
		}
		FlushFileBuffers(file);
		CloseHandle(file);
		return written != FALSE;
#else
		return false;
#endif
	}

	// 崩溃产物目录固定为日志目录下的 crashreports：解析成绝对路径并建好目录。
	static void applyReportFolder(const QString& path) {
		QString absolute = QFileInfo(path).absoluteFilePath();
		QDir().mkpath(absolute);
		copyW(state.ReportFolder, CrashSnapshot::PathLength, QDir::toNativeSeparators(absolute));
	}

	static bool ensureReportFolder() {
		if (state.ReportFolder[0] == 0) {
			return false;
		}
#ifdef Q_OS_WIN
		CreateDirectoryW(state.ReportFolder, nullptr);
#else
		mkdir(state.ReportFolder, 0777);
#endif
		return true;
	}

	// last_crash.txt 里的路径一律写成相对于当前目录的形式（路径不在当前目录下时保持绝对路径），
	// 方便外部工具直接按相对路径去取文件。
	static void toRelativePath(const wchar_t* absolute, wchar_t* dest, size_t destLen) {
		if (absolute == nullptr || absolute[0] == 0) {
			if (dest != nullptr && destLen > 0) {
				dest[0] = L'\0';
			}
			return;
		}
#ifdef Q_OS_WIN
		wchar_t current[CrashSnapshot::PathLength];
		DWORD length = GetCurrentDirectoryW(CrashSnapshot::PathLength, current);
		if (length == 0 || length >= CrashSnapshot::PathLength) {
			copyW(dest, destLen, absolute);
			return;
		}
		size_t currentLength = wcslen(current);
		while (currentLength > 1 && (current[currentLength - 1] == L'\\' || current[currentLength - 1] == L'/')) {
			currentLength--;
		}
		if (_wcsnicmp(absolute, current, currentLength) == 0
			&& (absolute[currentLength] == L'\\' || absolute[currentLength] == L'/')) {
			copyW(dest, destLen, absolute + currentLength + 1);
			return;
		}
#endif
		copyW(dest, destLen, absolute);
	}

	// 把本次运行的日志、崩溃报告与转储三条路径摆到当前目录下，供人工或外部工具一步定位。
	static void writeLastCrashFile(const CrashSnapshot& snapshot) {
		ReportWriter writer;
		if (!writer.Open(L"last_crash.txt", true)) {
			return;
		}
		wchar_t relative[CrashSnapshot::PathLength];
		wchar_t logPath[CrashSnapshot::PathLength];
		logPath[0] = L'\0';
		if (state.LogFolder[0] != 0 && state.LogFileName[0] != 0) {
#ifdef Q_OS_WIN
			formatW(logPath, CrashSnapshot::PathLength, L"%s\\%s", state.LogFolder, state.LogFileName);
#else
			formatW(logPath, CrashSnapshot::PathLength, L"%s/%s", state.LogFolder, state.LogFileName);
#endif
		}
		toRelativePath(logPath, relative, CrashSnapshot::PathLength);
		writer.WriteLine(relative);
		toRelativePath(snapshot.ReportPath, relative, CrashSnapshot::PathLength);
		writer.WriteLine(relative);
		toRelativePath(snapshot.DumpPath, relative, CrashSnapshot::PathLength);
		writer.WriteLine(relative);
		writer.Close();
	}

	// ---------- 统一存证 ----------

	// 两条路径的共同出口：先把现场落盘（报告必写，转储按需），再交还给调用方。
	static bool saveCrashReport(CrashSnapshot& snapshot, bool withDump) {
		if (!ensureReportFolder()) {
			return false;
		}
		TimeParts parts;
		readLocalTime(parts);
		wchar_t stamp[64];
		formatStamp(parts, stamp, 64);
		wchar_t prefix[96];
		quint32 attempt = 0;
		for (;;) {
			if (attempt == 0) {
				formatW(prefix, 96, L"%s", stamp);
			}
			else {
				formatW(prefix, 96, L"%s_%u", stamp, attempt);
			}
#ifdef Q_OS_WIN
			formatW(snapshot.ReportPath, CrashSnapshot::PathLength, L"%s\\%s_crashreport.log",
				state.ReportFolder, prefix);
#else
			formatW(snapshot.ReportPath, CrashSnapshot::PathLength, L"%s/%s_crashreport.log",
				state.ReportFolder, prefix);
#endif
			if (!fileExists(snapshot.ReportPath) || attempt > 1000) {
				break;
			}
			attempt++;
		}
		if (withDump && state.DumpEnabled.load()) {
#ifdef Q_OS_WIN
			formatW(snapshot.DumpPath, CrashSnapshot::PathLength, L"%s\\%s.dmp", state.ReportFolder, prefix);
#else
			formatW(snapshot.DumpPath, CrashSnapshot::PathLength, L"%s/%s.dmp", state.ReportFolder, prefix);
#endif
		}
		else {
			snapshot.DumpPath[0] = L'\0';
		}
		captureFramesIfNeeded(snapshot);
		resolveFrames(snapshot);
		bool reportWritten = writeCrashReport(snapshot);
		if (!reportWritten) {
			snapshot.ReportPath[0] = L'\0';
		}
		if (snapshot.DumpPath[0] != 0 && !writeMiniDump(snapshot)) {
			snapshot.DumpPath[0] = L'\0';
		}
		// 只在进程确实要结束时才留这份索引：可恢复的异常不值得覆盖上一次真实崩溃的记录。
		if (snapshot.Critical) {
			writeLastCrashFile(snapshot);
		}
		return reportWritten;
	}

#ifdef Q_OS_WIN
	// 采样堆栈与生成转储都有可能在只剩下守护页的栈上失败（栈溢出崩溃时尤其如此），
	// 因此过滤器本身只收集现场，真正的工作放到一个新线程上做。
	static DWORD WINAPI crashWorker(LPVOID parameter);

	// 把快照升级成语义异常对象：会分配内存，只在环境可控时调用。
	static Exception exceptionFromSnapshot(const CrashSnapshot& snapshot, const wchar_t* messageOverride = nullptr) {
		QList<StacktraceFrame> frames;
		for (quint32 i = 0; i < snapshot.FrameCount; ++i) {
			const CrashStackFrame& frame = snapshot.Frames[i];
			frames.append(StacktraceFrame(QString::fromWCharArray(frame.Function),
				QString::fromWCharArray(frame.SourceFile),
				QString::fromWCharArray(frame.BinaryFile),
				static_cast<quint64>(frame.Address),
				frame.Line));
		}
		return Exception(static_cast<Exception::Type>(snapshot.Type),
			messageOverride != nullptr ? QString::fromWCharArray(messageOverride) : QString::fromWCharArray(snapshot.Message),
			snapshot.Critical,
			QString::fromWCharArray(snapshot.File),
			snapshot.Line,
			QString::fromWCharArray(snapshot.Function),
			frames);
	}

	// 与捕获路径同一套回调序列。原生崩溃在故障线程也就是主线程上执行它：
	// 处理器几乎必然要建 Qt 界面，放到工作线程上不成立。
	static void showNativeCrashDialog(const CrashSnapshot& snapshot) {
		ApplicationExceptionMessageHandler* handler = state.MessageHandler.load();
		if (handler == nullptr) {
			return;
		}
		wchar_t message[CrashSnapshot::MessageLength];
		formatW(message, CrashSnapshot::MessageLength, L"Native fault: %s (0x%08lX) at 0x%016llX",
			exceptionCodeName(snapshot.NativeCode), static_cast<unsigned long>(snapshot.NativeCode),
			static_cast<unsigned long long>(snapshot.FaultAddress));
		if (snapshot.ReportPath[0] != 0) {
			size_t offset = wcslen(message);
			if (offset + 2 < CrashSnapshot::MessageLength) {
				formatW(message + offset, CrashSnapshot::MessageLength - offset, L"\r\n%s", snapshot.ReportPath);
			}
		}
		Exception ex = exceptionFromSnapshot(snapshot, message);
		handler->enableHandler();
		handler->onExceptionMessage(ex);
		handler->exec();
		handler->disableHandler();
	}

	// 处理器跑在堆可能已经损坏的进程里，它自己再崩一次也属正常，兜住即可。
	static void showNativeCrashDialogGuarded(const CrashSnapshot& snapshot) {
		__try {
			showNativeCrashDialog(snapshot);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
		}
	}

	static LONG WINAPI unhandledExceptionFilter(EXCEPTION_POINTERS* pointers) {
		CrashSnapshot& snapshot = snapshotStorage();
		resetSnapshot(snapshot, true, true);
		snapshot.NativeInfo = pointers;
		if (pointers != nullptr && pointers->ExceptionRecord != nullptr) {
			snapshot.NativeCode = static_cast<quint32>(pointers->ExceptionRecord->ExceptionCode);
			snapshot.FaultAddress = reinterpret_cast<quintptr>(pointers->ExceptionRecord->ExceptionAddress);
			snapshot.ThreadId = static_cast<quint32>(GetCurrentThreadId());
		}
		HandlingGate gate;
		if (gate.Acquired) {
			HANDLE worker = CreateThread(nullptr, 1 << 20, &crashWorker, &snapshot, 0, nullptr);
			if (worker != nullptr) {
				WaitForSingleObject(worker, 60000);
				CloseHandle(worker);
			}
			// 存证已经落盘，再通知界面：处理器只在主线程上调用，栈溢出时跳过。
			if (GetCurrentThreadId() == state.MainThreadId.load()
				&& snapshot.NativeCode != EXCEPTION_STACK_OVERFLOW) {
				showNativeCrashDialogGuarded(snapshot);
			}
		}
		return EXCEPTION_EXECUTE_HANDLER;
	}

	static DWORD WINAPI crashWorker(LPVOID parameter) {
		CrashSnapshot* snapshot = static_cast<CrashSnapshot*>(parameter);
		saveCrashReport(*snapshot, true);
		return 0;
	}
#endif
}

namespace Visindigo::General {
	/*!
		\class Visindigo::General::CrashGateway
		\inheaderfile General/CrashGateway.h
		\since Visindigo 0.17.0
		\inmodule Visindigo
		\brief 崩溃存证网关.

		CrashGateway 为两类崩溃留下同格式的现场记录：事件循环里捕获到的异常，
		以及操作系统报告的原生未处理异常（例如访问违例、栈溢出）。

		两类崩溃都会在报告目录下生成一份 \c{_crashreport.log} 崩溃报告，命名同源；
		原生崩溃与致命异常还会额外生成一份进程转储，供事后用调试器定位现场。
		进程即将结束时还会在当前目录下刷新一份 \c{last_crash.txt}，其中分三行记录本次运行的
		日志、崩溃报告与转储路径（相对于当前目录，取不到相对路径时为绝对路径）。

		\note 网关只负责存证，不决定进程的去向：捕获到的异常由调用方决定是否退出，
		原生未处理异常交回操作系统处理。
		\note 原生崩溃在存证完成后也会调用异常消息处理器，但仅当崩溃发生在主线程、
		且故障线程的栈仍然可用时；未设置处理器时不弹任何界面。
		\note 转储开关、产品信息与硬件信息均随存证一起生效，可在任意时刻设置；
		崩溃产物目录固定为日志目录下的 \c{crashreports}，由 LogCenter 在创建日志文件时告知。
	*/

	/*!
		\since Visindigo 0.17.0

		安装崩溃存证网关，在当前进程内注册原生未处理异常过滤器。

		此函数可重复调用，重复调用不会重复注册。为了在崩溃时能够安全工作，
		它还会预先加载符号解析组件并预热内部缓冲，因此应当在应用启动的最早阶段调用。

		不支持的操作系统上此函数不做任何事。
	*/
	void CrashGateway::install() {
		if (__Private__::state.Installed.exchange(true)) {
			return;
		}
		__Private__::state.MainThreadId.store(static_cast<quint32>(
#ifdef Q_OS_WIN
			GetCurrentThreadId()
#else
			0
#endif
			));
		__Private__::state.MiniDumpType.store(defaultMiniDumpType());
		__Private__::state.SymbolsReady.store(false);
		if (__Private__::state.ReportFolder[0] == 0) {
			__Private__::applyReportFolder(QDir::currentPath() + QStringLiteral("/user_data/logs/crashreports"));
		}
		__Private__::snapshotStorage();
#ifdef Q_OS_WIN
		__Private__::backend.PreviousFilter = SetUnhandledExceptionFilter(&__Private__::unhandledExceptionFilter);
		__Private__::loadDbgHelp();
		if (__Private__::state.ResolveSymbols.load()) {
			__Private__::initializeSymbols();
		}
#endif
	}

	/*!
		\since Visindigo 0.17.0

		卸载崩溃存证网关，把异常过滤器恢复为安装前的状态。主要用于测试场景。
	*/
	void CrashGateway::uninstall() {
		if (!__Private__::state.Installed.exchange(false)) {
			return;
		}
#ifdef Q_OS_WIN
		SetUnhandledExceptionFilter(__Private__::backend.PreviousFilter);
		__Private__::cleanupSymbols();
#endif
	}

	/*!
		\since Visindigo 0.17.0
		return 网关是否已经安装。
	*/
	bool CrashGateway::installed() {
		return __Private__::state.Installed.load();
	}

	/*!
		\since Visindigo 0.17.0
		return 当前是否正在处理一次崩溃。

		可用于在自己的崩溃处理代码里避免触发二次处理。
	*/
	bool CrashGateway::handling() noexcept {
		return __Private__::state.Handling.load();
	}

	/*!
		\since Visindigo 0.17.0
		return 当前崩溃产物目录。
	*/
	QString CrashGateway::getReportFolderPath() {
		return QString::fromWCharArray(__Private__::state.ReportFolder);
	}

	/*!
		\since Visindigo 0.17.0
		\a folder 日志目录

		设置本次运行的日志目录。崩溃产物目录会同步为它下面的 \c{crashreports}，
		与崩溃报告、命令行历史等文件的既有约定保持一致。

		LogCenter 在创建日志文件时会自动调用它，通常不需要手动设置。
	*/
	void CrashGateway::setLogFolder(const QString& folder) {
		QString absolute = QFileInfo(folder).absoluteFilePath();
		__Private__::copyW(__Private__::state.LogFolder, __Private__::CrashSnapshot::PathLength,
			QDir::toNativeSeparators(absolute));
		__Private__::applyReportFolder(absolute + QStringLiteral("/crashreports"));
	}

	/*!
		\since Visindigo 0.17.0
		\a name 本次运行的日志文件名（不含目录）

		记录本次运行的日志文件名，供 \c{last_crash.txt} 拼出完整路径使用。
		LogCenter 在创建日志文件时会自动调用它，通常不需要手动设置。

		\sa setLogFolder
	*/
	void CrashGateway::setLogFileName(const QString& name) {
		__Private__::copyW(__Private__::state.LogFileName, __Private__::CrashSnapshot::PathLength, name);
	}

	/*!
		\since Visindigo 0.17.0
		\a enabled 是否生成进程转储

		设置崩溃时是否生成进程转储文件。转储用于事后用调试器定位现场，体积较大，
		但在排查原生崩溃时通常是唯一的可靠依据，因此默认开启。
	*/
	void CrashGateway::setDumpEnabled(bool enabled) {
		__Private__::state.DumpEnabled.store(enabled);
	}

	/*!
		\since Visindigo 0.17.0
		return 是否生成进程转储。
	*/
	bool CrashGateway::isDumpEnabled() {
		return __Private__::state.DumpEnabled.load();
	}

	/*!
		\since Visindigo 0.17.0
		\a type 转储类型标志

		设置进程转储的类型标志，取值含义由平台决定。默认值见 \l{defaultMiniDumpType()}。
	*/
	void CrashGateway::setMiniDumpType(quint32 type) {
		__Private__::state.MiniDumpType.store(type);
	}

	/*!
		\since Visindigo 0.17.0
		return 当前使用的转储类型标志。
	*/
	quint32 CrashGateway::getMiniDumpType() {
		return __Private__::state.MiniDumpType.load();
	}

	/*!
		\since Visindigo 0.17.0
		return 当前平台推荐的转储类型标志。

		现行取值在"信息足够定位问题"与"崩溃时不会因为转储而卡死"之间取平衡：
		包含数据段、线程信息、句柄信息与内存布局，但不包含完整内存镜像。
	*/
	quint32 CrashGateway::defaultMiniDumpType() {
#ifdef Q_OS_WIN
		return static_cast<quint32>(
			MiniDumpWithDataSegs | MiniDumpWithProcessThreadData | MiniDumpWithHandleData
			| MiniDumpWithUnloadedModules | MiniDumpWithFullMemoryInfo | MiniDumpWithThreadInfo
			| MiniDumpIgnoreInaccessibleMemory);
#else
		return 0;
#endif
	}

	/*!
		\since Visindigo 0.17.0
		\a enabled 是否启用符号解析

		设置崩溃报告中是否解析函数名与源文件位置。符号解析在安装网关时预热，
		未预热时报告里只会出现原始地址，但转储依然可用。默认开启。
	*/
	void CrashGateway::setSymbolResolutionEnabled(bool enabled) {
		__Private__::state.ResolveSymbols.store(enabled);
	}

	/*!
		\since Visindigo 0.17.0
		return 是否启用符号解析。
	*/
	bool CrashGateway::isSymbolResolutionEnabled() {
		return __Private__::state.ResolveSymbols.load();
	}

	/*!
		\since Visindigo 0.17.0
		\a info 产品信息，例如名称与版本

		设置写入崩溃报告头部的产品信息，便于把报告与具体发行版本对应起来。
	*/
	void CrashGateway::setProductInfo(const QString& info) {
		__Private__::copyW(__Private__::state.ProductInfo, __Private__::CrashSnapshot::MessageLength, info);
	}

	/*!
		\since Visindigo 0.17.0
		\a info 硬件与系统信息

		设置写入崩溃报告尾部的硬件与系统信息。内容会在设置时被拷贝一份，
		因此崩溃时不需要再去采集，也不会因为堆损坏而丢失。
	*/
	void CrashGateway::setHardwareInfo(const QString& info) {
		__Private__::copyW(__Private__::state.HardwareInfo, 8192, info);
	}

	/*!
		\since Visindigo 0.17.0
		\a handler 异常消息处理器

		设置原生崩溃发生后要通知的异常消息处理器。网关不接管它的生命周期，
		调用方需要保证它在进程结束前一直有效。

		处理器几乎必然要建 Qt 界面，因此只有崩溃发生在主线程、且故障线程的栈仍然可用时
		才会调用它。未设置处理器时，原生崩溃只存证，不弹任何界面。

		\sa VIApplication::setExceptionMessageHandler
	*/
	void CrashGateway::setExceptionMessageHandler(ApplicationExceptionMessageHandler* handler) {
		__Private__::state.MessageHandler.store(handler);
	}

	/*!
		\since Visindigo 0.17.0
		\a ex 捕获到的异常对象

		把在确定边界上捕获到的异常交给网关存证：在报告目录下写出与其他崩溃来源同格式的
		崩溃报告，异常被标记为致命时还会一并生成进程转储。

		此函数不决定进程的去向：写完产物就返回，是否退出、如何退出由调用方决定。
	*/
	void CrashGateway::onCaughtException(const Exception& ex) {
		__Private__::HandlingGate gate;
		if (!gate.Acquired) {
			return;
		}
		__Private__::CrashSnapshot& snapshot = __Private__::snapshotStorage();
		__Private__::fillSnapshotFromException(snapshot, ex);
		// 致命异常意味着应用很可能就此结束，与原生崩溃同等对待，因此也留下转储。
		__Private__::saveCrashReport(snapshot, ex.isCritical());
	}
}
