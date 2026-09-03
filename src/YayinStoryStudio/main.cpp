#include <QtCore/qresource.h>
#include <QtWidgets/QApplication>
#include <QtWidgets/qmessagebox.h>
#include <General/Log.h>
#include <General/LoggerMsgHandler.h>
#include <General/Version.h>
#include <General/VIApplication.h>
#include <General/YSSLogger.h>
#include <Utility/FileUtility.h>
#include "Editor/MainEditor/MainEditorBuiltinPlugin.h"
#include "Editor/TitlePage/TitlePage.h"
#include "YayinStoryStudio.h"

#ifdef Q_OS_ANDROID

#include <QtCore/qcoreapplication.h>
#include <QtCore/qdir.h>
#include <QtCore/qjniobject.h>
#include <QtGui/qaccessible.h>
#include <General/PluginManager.h>

extern "C" Visindigo::General::Plugin* VisindigoPluginMain_YSSFileExt(void);
extern "C" Visindigo::General::Plugin* VisindigoPluginMain_ImageViewer(void);
extern "C" Visindigo::General::Plugin* VisindigoPluginMain_ASERStudio(void);

namespace YSSAndroid {
	static QString externalStorageRoot() {
		QJniObject file = QJniObject::callStaticObjectMethod("android/os/Environment",
			"getExternalStorageDirectory", "()Ljava/io/File;");
		if (file.isValid()) {
			const QString path = file.callObjectMethod("getAbsolutePath", "()Ljava/lang/String;").toString();
			if (!path.isEmpty()) {
				return path;
			}
		}
		return QStringLiteral("/storage/emulated/0");
	}

	static bool hasAllFilesAccess() {
		if (QNativeInterface::QAndroidApplication::sdkVersion() < 30) {
			return true;
		}
		return QJniObject::callStaticMethod<jboolean>("android/os/Environment",
			"isExternalStorageManager", "()Z") != 0;
	}

	static void openAllFilesAccessSettings() {
		QJniObject context = QNativeInterface::QAndroidApplication::context();
		if (!context.isValid()) {
			return;
		}
		const QString packageName =
			context.callObjectMethod("getPackageName", "()Ljava/lang/String;").toString();
		QJniObject action = QJniObject::fromString(
			QStringLiteral("android.settings.MANAGE_APP_ALL_FILES_ACCESS_PERMISSION"));
		QJniObject intent("android/content/Intent", "(Ljava/lang/String;)V", action.object());
		QJniObject uri = QJniObject::callStaticObjectMethod("android/net/Uri", "parse",
			"(Ljava/lang/String;)Landroid/net/Uri;",
			QJniObject::fromString(QStringLiteral("package:") + packageName).object());
		intent.callObjectMethod("setData", "(Landroid/net/Uri;)Landroid/content/Intent;", uri.object());
		context.callMethod<void>("startActivity", "(Landroid/content/Intent;)V", intent.object());
	}

	static void setupPublicUserFolder() {
		const QString base = externalStorageRoot() + QStringLiteral("/YayinStoryStudio");
		VISetEnv(Visindigo::General::VIApplication::LogFolderPath, base + QStringLiteral("/logs"));
		VISetEnv(Visindigo::General::VIApplication::PluginFolderPath, base + QStringLiteral("/plugins"));
		VISetEnv(Visindigo::General::VIApplication::ConfigPath, base + QStringLiteral("/config"));
		VISetEnv(Visindigo::General::VIApplication::ThemeFolderPath, base + QStringLiteral("/themes"));
		QDir().mkpath(base + QStringLiteral("/logs"));
		QDir().mkpath(base + QStringLiteral("/plugins"));
		QDir().mkpath(base + QStringLiteral("/config"));
		QDir().mkpath(base + QStringLiteral("/themes"));
		QDir().mkpath(base + QStringLiteral("/repos"));
	}
}
#endif // Q_OS_ANDROID

int main(int argc, char* argv[])
{
#ifdef Q_OS_ANDROID
	Q_INIT_RESOURCE(YayinStoryEditor);
	Q_INIT_RESOURCE(YSSBundledPluginsMeta);
	YSSAndroid::setupPublicUserFolder();
	if (not YSSAndroid::hasAllFilesAccess()) {
		YSSAndroid::openAllFilesAccessSettings();
	}
#endif
	VISetEnv(Visindigo::General::VIApplication::MinimumLoadingTimeMS, 1500);
	VISetEnv(Visindigo::General::VIApplication::UseVirtualTerminal, true);
	VISetEnv(Visindigo::General::VIApplication::SaveCommandHistory, true);
	Visindigo::General::VIApplication app(argc, argv, Visindigo::General::VIApplication::WidgetApp);
	QStringList prts = Visindigo::Utility::FileUtility::readLines(":/resource/cn.yxgeneral.visindigo/IWillFindU.txt");
	for (auto p : prts) {
		yMessage << p;
	}
	yInfo << "Yayin Story Studio " << Visindigo::General::Version::getAPIVersion();
	/*
	QMessageBox::information(nullptr, "SEA TP 技术预览版本警告",
R"(警告！
您当前正在使用技术预览版本！

技术预览版本仅供用户体验和测试新功能，并帮助我们对正式版程序进行完善。技术预览版本很可能非常不稳定，也许会导致数据丢失或其他严重问题。请勿在重要项目或生产环境中使用技术预览版本。

敬请期待Yayin Story Studio后续的开发。)", QMessageBox::Ok);
*/
	YSS::Editor::MainEditorBuiltinPlugin* mainEditorBuiltinPlugin = new YSS::Editor::MainEditorBuiltinPlugin();
	app.addDependencyPlugin(mainEditorBuiltinPlugin);

#ifdef Q_OS_ANDROID
	{
		Visindigo::General::PluginManager* pluginManager = Visindigo::General::PluginManager::getInstance();
        pluginManager->addPluginEntryPoint(&VisindigoPluginMain_YSSFileExt,
			QStringLiteral(":/plugins/Plugin_YSSFileExt.vpl.json"));
        pluginManager->addPluginEntryPoint(&VisindigoPluginMain_ImageViewer,
			QStringLiteral(":/plugins/Plugin_ImageViewer.vpl.json"));
        pluginManager->addPluginEntryPoint(&VisindigoPluginMain_ASERStudio,
			QStringLiteral(":/plugins/Plugin_ASERStudio.vpl.json"));
	}
#endif

	YSS::Main* mainPlugin = new YSS::Main();
	app.setMainPlugin(mainPlugin);
	app.setLoadingMessageHandler(new YSS::TitlePage::TitlePage());

	int c = app.start();
	return c;
}
