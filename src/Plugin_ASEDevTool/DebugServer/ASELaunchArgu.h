#ifndef Plugin_ASEDevTool_DebugServer_ASELaunchArgu_h
#define Plugin_ASEDevTool_DebugServer_ASELaunchArgu_h
#include <VIMacro.h>
class QString;
class ASELaunchArguPrivate;
class ASELaunchArgu {
public:
	enum SizeMode {
		W1024H576 = 0,
		W1280H720,
		W1366H768,
		W1600H900,
		W1920H1080,
		W2560H1440,
		W3840H2160
	};
	VICopyable(ASELaunchArgu);
	VIMoveable(ASELaunchArgu);

	ASELaunchArgu();
	ASELaunchArgu(const QString& workingFolder, const QString& mainFileName, SizeMode mode = SizeMode::W1920H1080);
	void setWorkingFolder(const QString& folder);
	void setMainFileName(const QString& fileName);
	void setSizeMode(SizeMode mode);
	QString getWorkingFolder() const;
	QString getMainFileName() const;
	SizeMode getSizeMode() const;
	QString toString();
private:
	ASELaunchArguPrivate* d;
};
#endif // Plugin_ASEDevTool_DebugServer_ASELaunchArgu_h
