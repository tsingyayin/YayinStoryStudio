#include <QtCore/qdir.h>
#ifdef Q_OS_ANDROID
#include <QtCore/qjniobject.h>
#endif
#include "General/Exception.h"
#include "General/Log.h"
#include "General/Placeholder.h"
#include "General/private//VIGeneral_p.h"
#include "General/Version.h"
#include "Utility/SevenZipBinder.h"

namespace Visindigo::__Private__ {
	class VisindigoCorePrivate {
		friend class VisindigoCore;
	protected:
		// nothing for now
	};

	VisindigoCore::VisindigoCore() :Visindigo::General::Plugin("cn.yxgeneral.visindigo.builtin.core") {
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
		delete VI7zBinder;
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
				{zh_TW, ":/resource/cn.yxgeneral.visindigo/i18n/zh_TW.json"},
				{en, ":/resource/cn.yxgeneral.visindigo/i18n/en.json"},
				{ja, ":/resource/cn.yxgeneral.visindigo/i18n/ja.json"},
				{jp_less_loanword, ":/resource/cn.yxgeneral.visindigo/i18n/jp_less_loanword.json"},
				{ko, ":/resource/cn.yxgeneral.visindigo/i18n/ko.json"},
				{ru, ":/resource/cn.yxgeneral.visindigo/i18n/ru.json"},
				{de, ":/resource/cn.yxgeneral.visindigo/i18n/de.json"},
				{fr, ":/resource/cn.yxgeneral.visindigo/i18n/fr.json"}
			}
		);
	}

	VIGeneralCommandHandler::VIGeneralCommandHandler(Visindigo::General::Plugin* parent)
		:General::CommandHandler(parent, "vigeneral", "vigeneral", { "vig" }) {}

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
		else if (unnamedArgs.first() == "craSHvi") {
			VI_Throw(General::Exception::InternalError, "This is a test crash triggered by the craSHvi command.");
				}
		else if (unnamedArgs.first() == "craSHc") {
			throw;
				}
		return true;
	}

	namespace VisindigoBuiltinPlaceholderProvider {
		VisindigoCore::VisindigoCore(Visindigo::General::Plugin* parent) :
			Visindigo::General::PlaceholderProvider(parent, "ph_visindigo", "visindigo") {}
		VisindigoCore::~VisindigoCore() {}
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
			else if (name == "userDataPath") {
#ifdef Q_OS_ANDROID
				// Android：用户共享存储的 YayinStoryStudio 目录(与 logs/config/plugins/themes 同级)。
				// 模板等默认路径经 $(visindigo::userDataPath)/… 落到这里，用户文件管理器可见、可管理。
				QJniObject file = QJniObject::callStaticObjectMethod("android/os/Environment",
					"getExternalStorageDirectory", "()Ljava/io/File;");
				if (file.isValid()) {
					const QString path = file.callObjectMethod("getAbsolutePath", "()Ljava/lang/String;").toString();
					if (!path.isEmpty()) {
						return path + QStringLiteral("/YayinStoryStudio");
					}
				}
				return QStringLiteral("/storage/emulated/0/YayinStoryStudio");
#else
				// 桌面：维持既有语义 <当前目录>/user_data，使 $(visindigo::userDataPath)/repos == 原 $(visindigo::programPath)/user_data/repos。
				return QDir::currentPath() + QStringLiteral("/user_data");
#endif
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
			Visindigo::General::PlaceholderProvider(parent, "ph_viformat", "viformat") {}
		VIFormat::~VIFormat() {}
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