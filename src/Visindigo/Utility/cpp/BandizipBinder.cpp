#include "Utility/BandizipBinder.h"
#include <QtCore/qprocess.h>

#include <QtCore/qstringlist.h>
namespace Visindigo::Utility {
	class BandizipBinderPrivate {
		friend class BandizipBinder;
	protected:
		QString BandizipExePath = "./bz.exe";
		QProcess* BandizipProcess = nullptr;
		static BandizipBinder* Instance;
		bool isCompressing = false;
		QString targetName = "";
		BandizipBinderPrivate() {
			BandizipProcess = new QProcess();
		}

		~BandizipBinderPrivate(){
			delete BandizipProcess;
		}

		static QString enumToString(BandizipBinder::CompressFormat format) {
			switch (format) {
			case BandizipBinder::zip:
				return "zip";
			case BandizipBinder::zipx:
				return "zipx";
			case BandizipBinder::exe:
				return "exe";
			case BandizipBinder::tar:
				return "tar";
			case BandizipBinder::tgz:
				return "tgz";
			case BandizipBinder::lzh:
				return "lzh";
			case BandizipBinder::iso:
				return "iso";
			case BandizipBinder::sevenz:
				return "7z";
			case BandizipBinder::gz:
				return "gz";
			case BandizipBinder::xz:
				return "xz";
			default:
				return "zip";
			}
		}
	};

	BandizipBinder* BandizipBinderPrivate::Instance = nullptr;

	BandizipBinder::BandizipBinder()
		:QObject(nullptr), d(new BandizipBinderPrivate) {
		connect(d->BandizipProcess, &QProcess::readyReadStandardOutput, [this](){
			QString output = d->BandizipProcess->readAllStandardOutput();
			if (output.startsWith("Compressing:")) {
				QString fileName = output.mid(QString("Compressing:").length()).trimmed();
				emit compressingFile(fileName);
			}
			else if (output.startsWith("Compressed:")) {
				QString fileName = output.mid(QString("Compressed:").length()).trimmed();
				emit compressedFile(fileName);
			}
			});
		connect(d->BandizipProcess, &QProcess::stateChanged, [this](QProcess::ProcessState newState){
			if (newState == QProcess::NotRunning) {
				if (d->isCompressing) {
					emit compressedFile(d->targetName);
				}
				else {
					emit decompressedFile(d->targetName);
				}
			}
			});
	}

	BandizipBinder* BandizipBinder::getInstance() {
		if (BandizipBinderPrivate::Instance == nullptr) {
			BandizipBinderPrivate::Instance = new BandizipBinder();
		}
		return BandizipBinderPrivate::Instance;
	}

	BandizipBinder::~BandizipBinder() {
		delete d;
	}

	void BandizipBinder::bindBandizipBinary(const QString& bandizipExePath) {
		d->BandizipExePath = bandizipExePath;
	}

	QString BandizipBinder::getBandizipBinaryPath() const {
		return d->BandizipExePath;
	}

	bool BandizipBinder::compressFilesToZip(const QStringList& filePaths, const QString& arcPath, CompressFormat format, const QString& password) {
		if (isBusy()) {
			return false;
		}
		QStringList arguments;
		arguments << "c";
		arguments << QString("-fmt:%1").arg(BandizipBinderPrivate::enumToString(format));
		if (!password.isEmpty()) {
			arguments << QString("-p:%1").arg(password);
		}
		arguments << arcPath;
		arguments.append(filePaths);
		d->isCompressing = true;
		d->targetName = arcPath;
		d->BandizipProcess->start(d->BandizipExePath, arguments);
		return true;
	}

	bool BandizipBinder::decompressFile(const QString& arcPath, const QString& outputPath, CompressFormat format, const QString& password) {
		if (isBusy()) {
			return false;
		}
		QStringList arguments;
		arguments << "x";
		arguments << QString("-fmt:%1").arg(BandizipBinderPrivate::enumToString(format));
		if (!password.isEmpty()) {
			arguments << QString("-p:%1").arg(password);
		}
		arguments << arcPath;
		arguments << QString("-o:%1").arg(outputPath);
		d->isCompressing = false;
		d->targetName = outputPath;
		d->BandizipProcess->start(d->BandizipExePath, arguments);
		return true;
	}

	bool BandizipBinder::isBusy() const {
		return d->BandizipProcess->state() == QProcess::Running;
	}

}