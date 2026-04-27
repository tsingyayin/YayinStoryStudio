#include "General/private//VIGeneral_p.h"
#include "General/Placeholder.h"
#include "General/Log.h"
#include "General/Version.h"
#include <QtCore/qdir.h>

namespace Visindigo::__Private__ {
	class VisindigoCorePrivate {
		friend class VisindigoCore;
	protected:
		// nothing for now
	};

	VisindigoCore::VisindigoCore():Visindigo::General::Plugin(){

	}

	VisindigoCore::~VisindigoCore() {

	}

	void VisindigoCore::onPluginEnable() {
		registerPluginModule(new VisindigoTranslator(this));
		registerPluginModule(new VIGeneralCommandHandler(this));
		registerPluginModule(new VisindigoBuiltinPlaceholderProvider::VisindigoCore(this));
		registerPluginModule(new VisindigoBuiltinPlaceholderProvider::VIFormat(this));
		vgInfo << "VisindigoCore plugin enabled.";
	}

	void VisindigoCore::onApplicationInit() {
		vgInfo << "VisindigoCore application init.";
	}

	void VisindigoCore::onPluginDisable() {
		vgInfo << "VisindigoCore plugin disabled.";
	}

	void VisindigoCore::onTest() {
		vgInfo << "VisindigoCore plugin test.";
	}

	VisindigoTranslator::VisindigoTranslator(Visindigo::General::Plugin* parent) 
		: Visindigo::General::Translator(parent, "Visindigo")
	{
		setDefaultLang(zh_CN);
		setLangFilePath(
			{
				{zh_CN, ":/resource/cn.yxgeneral.visindigo/i18n/zh_CN.json"},
				{en, ":/resource/cn.yxgeneral.visindigo/i18n/en.json"}
			}
		);
	}

	VIGeneralCommandHandler::VIGeneralCommandHandler(Visindigo::General::Plugin* parent)
		:General::CommandHandler(parent, "vigeneral", "vigeneral", {"vig"}) {
	}

	VIGeneralCommandHandler::~VIGeneralCommandHandler() {
		
	}
	General::CommandErrorData VIGeneralCommandHandler::onCommand(const QString& entryName, const QStringList& unnamedArgs, const QMap<QString, QString>& namedArgs) {
		if (unnamedArgs.isEmpty()) {
			return true;
		}
		if (unnamedArgs.first() == "parseph") {
			QStringList params = unnamedArgs.mid(1);
			QString placeHolderString = params.join(' ');
			QString result = General::PlaceholderManager::getInstance()->requestPlaceholder(placeHolderString);
			vgInfo << "Parsed placeholder: " << result;
		}
		return true;
	}

	namespace VisindigoBuiltinPlaceholderProvider {
		VisindigoCore::VisindigoCore(Visindigo::General::Plugin* parent) : 
			Visindigo::General::PlaceholderProvider(parent, "ph_visindigo", "visindigo") {
		}
		VisindigoCore::~VisindigoCore() {
		}
		QString VisindigoCore::onPlaceholderRequest(const QString& name, const QString& param) {
			if (name == "apiVersion") {
				return ::Visindigo::General::Version::getAPIVersion().toString();
			}
			else if (name == "abiVersion") {
				return ::Visindigo::General::Version::getABIVersion().toString();
			}
			else if (name == "programPath") {
				return QDir::currentPath();
			}
			else if (name == "unixEpoch") {
				return QString::number(QDateTime::currentSecsSinceEpoch());
			}
			else if (name == "dateTime") {
				if (param.isEmpty()) {
					return QDateTime::currentDateTime().toString();
				}
				else {
					return QDateTime::currentDateTime().toString(param);
				}
			}
			return QString();
		}

		VIFormat::VIFormat(Visindigo::General::Plugin* parent) : 
			Visindigo::General::PlaceholderProvider(parent, "ph_viformat", "viformat") {
		}
		VIFormat::~VIFormat() {
		}
		QString VIFormat::onPlaceholderRequest(const QString& name, const QString& param) {
			if (name == "round") {
				QStringList params = param.split(',');
				if (params.size() != 2) {
					return QString();
				}
				bool ok;
				double num = params[0].toDouble(&ok);
				int precision = params[1].toInt(&ok);
				if (ok) {
					return QString::number(num, 'f', precision);
				}
			}
			else if (name == "upper") {
				return param.toUpper();
			}
			else if (name == "lower") {
				return param.toLower();
			}
			else if (name == "convert") {
				QStringList params = param.split(',');
				if (params.size() != 3) {
					return QString();
				}
				bool ok;
				int num = params[0].toInt(&ok);
				int fromBase = params[1].toInt(&ok);
				int toBase = params[2].toInt(&ok);
				if (ok && fromBase >= 2 && fromBase <= 36 && toBase >= 2 && toBase <= 36) {
					return QString::number(num, toBase);
				}
			}
			return QString();
		}
	}
}