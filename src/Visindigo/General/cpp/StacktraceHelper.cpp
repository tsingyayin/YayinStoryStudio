#include "General/StacktraceHelper.h"
#include <QtCore/qregularexpression.h>
#include "General/Log.h"

#ifdef VI_HAS_STD_STACKTRACE
	#include <stacktrace>
	#include <iostream>
#endif

namespace Visindigo::General {
	class StacktraceFramePrivate {
		friend class StacktraceFrame;
	protected:
		QString functionName = "<unknown>";
		QString sourceFileName = "<unknown>";
		QString binaryFileName = "<unknown>";
		quint64 address = 0x00;
		quint64 lineNumber = 0;
	};

	/*!
		\class Visindigo::General::StacktraceFrame
		\inheaderfile General/StacktraceHelper.h
		\brief 堆栈跟踪帧数据类。
		\inmodule Visindigo
		\ingroup VDebug
		\since Visindigo 0.13.0

		StacktraceFrame 类表示程序执行堆栈中的单个帧信息。它包含函数名称、源文件名、
		二进制文件名、内存地址和行号等信息。

		此类主要用于与 Visindigo::General::StacktraceHelper 类配合使用，以获取和表示
		当前执行堆栈的详细信息。

		请注意，此类是数据类，不包含任何复杂的逻辑或行为。它仅用于存储和传递堆栈帧信息。
		因此它与任何特定的堆栈跟踪实现均解耦，例如无论VI_HAS_STD_STACKTRACE是否被定义，
		这类本身都可以使用。

		此外，这类的实例是不可变的，一旦创建，其内容就不能更改。
	*/

	/*!
		\since Visindigo 0.13.0
		默认构造函数，创建一个空的堆栈跟踪帧对象。

		因为此类是不可变的，使用这个构造函数基本等同于表示一个无效的堆栈帧。
	*/
	StacktraceFrame::StacktraceFrame() : d(new StacktraceFramePrivate()) {
		
	}

	/*!
		\since Visindigo 0.13.0
		构造函数，使用指定的参数创建一个堆栈跟踪帧对象。

		\a functionName 函数名称。
		\a sourceFileName 源文件名。
		\a binaryFileName 二进制文件名。
		\a address 内存地址。
		\a lineNumber 行号。
	*/
	StacktraceFrame::StacktraceFrame(const QString& functionName, const QString& sourceFileName, const QString& binaryFileName, quint64 address, quint64 lineNumber)
		: d(new StacktraceFramePrivate()) {
		d->functionName = functionName;
		d->sourceFileName = sourceFileName;
		d->binaryFileName = binaryFileName;
		d->address = address;
		d->lineNumber = lineNumber;
	}

	/*!
		\fn Visindigo::General::StacktraceFrame::StacktraceFrame(const StacktraceFrame& other)
		\since Visindigo 0.13.0

		拷贝构造函数
	*/

	/*!
		\fn Visindigo::General::StacktraceFrame& Visindigo::General::StacktraceFrame::operator=(const StacktraceFrame& other)
		\since Visindigo 0.13.0

		拷贝赋值运算符
	*/

	/*!
		\fn Visindigo::General::StacktraceFrame::StacktraceFrame(StacktraceFrame&& other) noexcept
		\since Visindigo 0.13.0

		移动构造函数
	*/

	/*!
		\fn Visindigo::General::StacktraceFrame& Visindigo::General::StacktraceFrame::operator=(StacktraceFrame&& other) noexcept
		\since Visindigo 0.13.0

		移动赋值运算符
	*/
	VICopyable_Impl(StacktraceFrame);
	VIMoveable_Impl(StacktraceFrame);

	/*!
		\since Visindigo 0.13.0
		析构函数，销毁堆栈跟踪帧对象。
	*/
	StacktraceFrame::~StacktraceFrame() {
		if (d != nullptr){
			delete d;
		}
	}

	/*!
		\since Visindigo 0.13.0
		return 函数名称
	*/
	QString StacktraceFrame::getFunctionName() const {
		return d->functionName;
	}

	/*!
		\since Visindigo 0.13.0
		return 源文件名
	*/
	QString StacktraceFrame::getSourceFileName() const {
		return d->sourceFileName;
	}

	/*!
		\since Visindigo 0.13.0
		return 二进制文件名
	*/
	QString StacktraceFrame::getBinaryFileName() const {
		return d->binaryFileName;
	}

	/*!
		\since Visindigo 0.13.0
		return 内存地址
	*/
	quint64 StacktraceFrame::getAddress() const {
		return d->address;
	}

	/*!
		\since Visindigo 0.13.0
		return 行号
	*/
	quint64 StacktraceFrame::getLineNumber() const {
		return d->lineNumber;
	}

	/*!
		\since Visindigo 0.13.0
		return 将堆栈跟踪帧信息格式化为字符串表示形式。

		这是个用于调试的辅助函数，Visindigo输出日志时不依赖此函数，因此调用此函数
		输出的结果和在日志上看到的结果有所不同。
	*/
	QString StacktraceFrame::toString() const {
		return QString("Function: %1, Source File: %2, Binary File: %3, Address: 0x%4, Line: %5")
			.arg(d->functionName)
			.arg(d->sourceFileName)
			.arg(d->binaryFileName)
			.arg(QString::number(d->address, 16))
			.arg(QString::number(d->lineNumber));
	}

#ifdef VI_HAS_STD_STACKTRACE
#ifdef Q_CC_MSVC
	static StacktraceFrame std_stacktrace_entryToStacktraceData(const std::stacktrace_entry& frame) {
		QString rawDescription = QString(frame.description().c_str());
		QString functionName = "<unknown>";
		QString fileName = QString(frame.source_file().c_str());
		fileName = fileName.isEmpty() ? "<unknown>" : fileName;
		QString binaryFileName = "<unknown>";
		quint64 sourceLine = static_cast<quint64>(frame.source_line());
		quint64 address = 0;
		if (rawDescription.isEmpty()) {
			return StacktraceFrame("<unknown>", fileName, "<unknown>", 0x00, sourceLine);
		}
		// output format: "SourceFile(line): BinaryFile!Function+0xAddress"
		// SourceFile, line, function are optional
		// eg from https://en.cppreference.com/w/cpp/utility/basic_stacktrace.html
		QString binaryData = "";
		if (rawDescription.contains(": ")) {
			QStringList sourceFile_Binary = rawDescription.split(": ");
			fileName = sourceFile_Binary.size() > 1 ? sourceFile_Binary[0] : "<unknown>";
			QRegularExpression sourceLineRegex("\\(\\d+\\)");
			QRegularExpressionMatch match = sourceLineRegex.match(fileName);
			if (match.hasMatch()) {
				QString lineStr = match.captured(0);
				lineStr.remove('(');
				lineStr.remove(')');
				sourceLine = lineStr.toULongLong();
				fileName = fileName.replace(match.captured(0), "");
			}
			binaryData = sourceFile_Binary.size() > 1 ? sourceFile_Binary[1] : "";
		}
		else {
			binaryData = rawDescription;
		}
		QStringList binary_address = binaryData.split('+');
		address = binary_address.size() > 1 ? binary_address[1].toULongLong(nullptr, 16) : 0x00;
		QStringList binary_function = binary_address.size() > 0 ? binary_address[0].split('!') : QStringList();
		binaryFileName = binary_function.size() > 0 ? binary_function[0] : "<unkown>";
		functionName = binary_function.size() > 1 ? binary_function[1] : "<unknown>";
		return StacktraceFrame(functionName, fileName, binaryFileName, address, sourceLine);
	}
#else
	static StacktraceFrame std_stacktrace_entryToStacktraceData(const std::stacktrace_entry& frame) {
		return StacktraceFrame("<unkown>", "<unkown>", "<unkown>", 0x00, 0);
	}
#endif
#else
	static StacktraceFrame std_stacktrace_entryToStacktraceData(const std::stacktrace_entry& frame) {
		return StacktraceFrame("<unkown>", "<unkown>", "<unkown>", 0x00, 0);
	}
#endif

	/*!
		\class Visindigo::General::StacktraceHelper
		\inheaderfile General/StacktraceHelper.h
		\brief 堆栈跟踪帮助类。
		\inmodule Visindigo
		\ingroup VDebug
		\since Visindigo 0.13.0

		StacktraceHelper 类提供获取当前执行堆栈跟踪信息的静态方法。
		它允许开发者在调试和错误处理过程中获取程序执行路径的详细信息。

		请注意，StacktraceHelper 类仅包含静态方法，因此不需要创建其实例。
	
	*/

	/*!
		\fn QList<StacktraceFrame> Visindigo::General::StacktraceHelper::getStacktrace(qint32 maxFrames = 63, qint32 skipFrames = 0)
		\since Visindigo 0.13.0

		\a maxFrames 最大帧数，默认值为63。
		\a skipFrames 跳过的帧数，默认值为0。

		这个函数自动跳过本身的堆栈帧，因此不必考虑调用此函数带来的额外堆栈帧。

		这个函数在定义了VI_HAS_STD_STACKTRACE宏时，使用C++标准库的std::stacktrace来获取堆栈信息。

		但由于不同编译器对std::stacktrace的支持程度不同，实际获取的信息也有所差异，因此需要对每个
		编译器单独处理，当前只在Q_CC_MSVC下实现了对std::stacktrace的解析，其他编译器下将返回空的堆栈帧列表。

		VI_HAS_STD_STACKTRACE未定义时，此函数也将返回空的堆栈帧列表。

		return 获取当前执行堆栈的堆栈跟踪帧列表。
	*/
#ifdef VI_HAS_STD_STACKTRACE
	QList<StacktraceFrame> StacktraceHelper::getStacktrace(qint32 maxFrames, qint32 skipFrames) {
		QList<StacktraceFrame> stacktraceList;
		auto stack = std::stacktrace::current(skipFrames + 1, maxFrames + 1); // +1 to skip this function frame
		for (const auto& frame : stack) {
			stacktraceList.append(std_stacktrace_entryToStacktraceData(frame));
		}
		return stacktraceList;
	}
#else
	QList<StacktraceFrame> StacktraceHelper::getStacktrace(qint32 maxFrames, qint32 skipFrames) {
		QList<StacktraceFrame> stacktraceList;
		return stacktraceList;
	}
#endif
}

 