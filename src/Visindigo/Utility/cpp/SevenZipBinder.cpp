#include <QtCore/qfile.h>
#include <QtCore/qprocess.h>
#include <QtCore/qregularexpression.h>
#include <QtCore/qstringlist.h>
#include "General/Log.h"
#include "Utility/SevenZipBinder.h"

namespace Visindigo::Utility {
	class SevenZipBinderPrivate {
		friend class SevenZipBinder;
	protected:
		QString exePath = "./7za.exe";
		QProcess* exeProcess = nullptr;
		static SevenZipBinder* Instance;
		bool isCompressing = false;
		QString targetName = "";
		SevenZipBinderPrivate() {
			exeProcess = new QProcess();
		}

		~SevenZipBinderPrivate() {
			if (exeProcess->state() == QProcess::Running) {
				exeProcess->kill();
			}
			delete exeProcess;
		}

		static QString enumToString(SevenZipBinder::CompressFormat format) {
			switch (format) {
			case SevenZipBinder::sevenz:
				return "7z";
			default:
				return QMetaEnum::fromType<SevenZipBinder::CompressFormat>().valueToKey(static_cast<int>(format));
			}
		}
	};

	SevenZipBinder* SevenZipBinderPrivate::Instance = nullptr;

	/*!
		\class Visindigo::Utility::SevenZipBinder
		\brief SevenZipBinder提供对7za.exe的简单绑定，以便压缩和解压文件.
		\since Visindigo 0.16.0
		\inmodule Visindigo

		SevenZipBinder类提供了对7za.exe的简单绑定，允许用户通过Qt的信号槽机制来压缩和解压文件，并获取处理进度和结果。
		这个类使用单例模式，确保在整个应用程序中只有一个SevenZipBinder实例。用户可以通过getInstance()函数来获取这个实例，
		并使用它来执行压缩和解压操作。

		要正确使用它，请确保在调用任何压缩或解压函数之前，已经通过bind7zaBinary()函数绑定了7za.exe的路径。默认情况下，路径为"./7za.exe"。
		也就是说，只要软件的执行目录下拥有此文件，就可以直接使用而无需额外配置。

		出于授权考虑，Visindigo的源代码仓库中不包含此文件，由开发者自行提供。
	*/

	/*!
		\enum Visindigo::Utility::SevenZipBinder::CompressFormat
		\brief 定义了支持的压缩格式。
		\value __Both__ 支持压缩和解压的格式的起始符，不应直接使用。
		\value sevenz 7z格式。
		\value xz xz格式。
		\value bzip2 bzip2格式。
		\value gzip gzip格式。
		\value tar tar格式。
		\value zip zip格式。
		\value wim wim格式。
		\value __DecompOnly__ 仅支持解压的格式的起始符，不应直接使用。

		下列取值由7za支持但只能用于解压，因此不单独编写格式化说明：

		\value ar ar格式，仅支持解压。
		\value arj arj格式，仅支持解压。
		\value cab cab格式，仅支持解压。
		\value chm chm格式，仅支持解压。
		\value cpio cpio格式，仅支持解压。
		\value cramfs cramfs格式，仅支持解压。
		\value dmg dmg格式，仅支持解压。
		\value ext ext格式，仅支持解压。
		\value fat fat格式，仅支持解压。
		\value gpt gpt格式，仅支持解压。
		\value hfs hfs格式，仅支持解压。
		\value ihex ihex格式，仅支持解压。
		\value iso iso格式，仅支持解压。
		\value lzh lzh格式，仅支持解压。
		\value lzma lzma格式，仅支持解压。
		\value mbr mbr格式，仅支持解压。
		\value msi msi格式，仅支持解压。
		\value nsis nsis格式，仅支持解压。
		\value ntfs ntfs格式，仅支持解压。
		\value qcow2 qcow2格式，仅支持解压。
		\value rar rar格式，仅支持解压。
		\value rpm rpm格式，仅支持解压。
		\value squashfs squashfs格式，仅支持解压。
		\value udf udf格式，仅支持解压。
		\value vhd vhd格式，仅支持解压。
		\value vmdk vmdk格式，仅支持解压。
		\value xar xar格式，仅支持解压。
		\value z z格式，仅支持解压。
		\value zstd zstd格式，仅支持解压。
	*/

	/*!
		\fn Visindigo::Utility::SevenZipBinder::processing(const QString& fileName, float percent)
		\since Visindigo 0.16.0

		当7za.exe正在处理文件时发出此信号，fileName是当前正在处理的文件名，percent是处理进度的百分比。

		由于7za.exe的压缩逻辑，文件名的提示进度远远快于实际的压缩进度，譬如如果有100个文件，则可能会出现
		从50%开始就只显示第100个文件的名字，因此请依赖百分比来判断实际的处理进度。fileName仅供显示参考。

		\a fileName 当前正在处理的文件名，仅供显示参考。
		\a percent 处理进度的百分比。
	*/

	/*!
		\fn Visindigo::Utility::SevenZipBinder::processed(bool success)
		\since Visindigo 0.16.0

		当7za.exe完成处理时发出此信号，success表示处理是否成功。

		\a success 处理是否成功。
	*/

	/*!
		\since Visindigo 0.16.0
		
		构造函数
	*/
	SevenZipBinder::SevenZipBinder()
		:QObject(nullptr), d(new SevenZipBinderPrivate) {
		connect(d->exeProcess, &QProcess::readyReadStandardOutput, this, [this]() {
			static QRegularExpression progressRegex(R"([\s]?([0-9]+%)[\s]?([0-9]+)\s\+\s([\s\S]+))");
			QByteArray output = d->exeProcess->readAllStandardOutput();
			QString outputStr = QString::fromLocal8Bit(output);
			outputStr.replace("\r", ""); // Remove carriage return characters
			auto match = progressRegex.match(outputStr);
			if (match.hasMatch()) {
				QString percentStr = match.captured(1);
				QString fileName = match.captured(3);
				float percent = percentStr.left(percentStr.length() - 1).toFloat() / 100.0f;
				emit processing(fileName, percent);
			}
			});
			
		connect(d->exeProcess, &QProcess::stateChanged, this, [this](QProcess::ProcessState newState) {
			if (newState == QProcess::NotRunning) {
				if (d->isCompressing) {
					emit processed(true);
				}
				else {
					emit processed(true);
				}
			}
			});
	}

	/*!
		\since Visindigo 0.16.0

		获取SevenZipBinder实例
	*/
	SevenZipBinder* SevenZipBinder::getInstance() {
		if (SevenZipBinderPrivate::Instance == nullptr) {
			SevenZipBinderPrivate::Instance = new SevenZipBinder();
		}
		return SevenZipBinderPrivate::Instance;
	}

	/*!
		\since Visindigo 0.16.0

		析构函数。由VisindigoCore在Disable时自动调用。
	*/
	SevenZipBinder::~SevenZipBinder() {
		delete d;
		SevenZipBinderPrivate::Instance = nullptr;
	}

	/*!
		\since Visindigo 0.16.0

		绑定7za.exe的路径，如果不设置，则默认为"./7za.exe"，即软件执行目录下的7za.exe。

		\a exePath 7za.exe的路径。
	*/
	void SevenZipBinder::bind7zaBinary(const QString& exePath) {
		d->exePath = exePath;
	}

	/*!
		\since Visindigo 0.16.0

		获取当前绑定的7za.exe路径。
	*/
	QString SevenZipBinder::get7zaBinaryPath() const {
		return d->exePath;
	}

	/*!
		\since Visindigo 0.16.0

		压缩文件到指定的压缩包路径，支持多种压缩格式。filePaths可以是多个文件或一个文件夹（会压缩整个文件夹）。如果正在处理其他任务，则返回false。

		\a filePaths 待压缩的文件或文件夹路径列表。
		\a arcPath 输出压缩包的路径。
		\a format 压缩格式，不能是仅支持解压的格式。
		\a password 压缩包密码，为空表示不加密。

		return 任务已开始返回true；正在处理其他任务或格式不支持压缩时返回false。
	*/
	bool SevenZipBinder::compressFilesToZip(const QStringList& filePaths, const QString& arcPath, CompressFormat format, const QString& password) {
		if (isBusy()) {
			return false;
		}
		if ((qint32)format >= (qint32)SevenZipBinder::__DecompOnly__) {
			return false; // Compression format cannot be a decompression-only format
		}
		QStringList arguments;
		arguments << "a" << "-bsp1";
		arguments << QString("-t%1").arg(SevenZipBinderPrivate::enumToString(format));
		if (!password.isEmpty()) {
			arguments << QString("-p%1").arg(password);
		}
		arguments << arcPath;
		arguments.append(filePaths);
		d->isCompressing = true;
		d->targetName = arcPath;
		d->exeProcess->start(d->exePath, arguments);
		return true;
	}

	/*!
		\since Visindigo 0.16.0

		解压压缩包到指定的输出路径，支持多种压缩格式。如果正在处理其他任务，则返回false。

		\a arcPath 待解压的压缩包路径。
		\a outputPath 解压输出目录。
		\a format 压缩包格式。
		\a password 压缩包密码，为空表示未加密。

		return 任务已开始返回true；正在处理其他任务时返回false。
	*/
	bool SevenZipBinder::decompressFile(const QString& arcPath, const QString& outputPath, CompressFormat format, const QString& password) {
		if (isBusy()) {
			return false;
		}
		QStringList arguments;
		arguments << "x" << "-bsp1";
		arguments << QString("-t%1").arg(SevenZipBinderPrivate::enumToString(format));
		if (!password.isEmpty()) {
			arguments << QString("-p%1").arg(password);
		}
		arguments << arcPath;
		arguments << QString("-o%1").arg(outputPath);
		d->isCompressing = false;
		d->targetName = outputPath;
		d->exeProcess->start(d->exePath, arguments);
		return true;
	}

	/*!
		\since Visindigo 0.16.0

		检查7za.exe是否正在处理任务。

		return 正在处理任务时返回true。
	*/
	bool SevenZipBinder::isBusy() const {
		return d->exeProcess->state() == QProcess::Running;
	}

	/*!
		\since Visindigo 0.17.0
		检查压缩功能是否可用。

		请注意，这个函数只是单纯检查目标文件是否存在，如果设置的是无效的7za.exe路径，或者7za.exe无法正常运行，
		这个函数也不会检测到。请确保7za.exe是可执行的，并且在调用压缩或解压函数之前已经绑定了正确的路径。

		return 已绑定的7za.exe文件存在时返回true。
	*/
	bool SevenZipBinder::isValid() const {
		return QFile::exists(d->exePath);
	}
}