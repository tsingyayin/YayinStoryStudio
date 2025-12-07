#ifndef Plugin_ASEDevTool_DebugServer_ASELaunchArgu_h
#define Plugin_ASEDevTool_DebugServer_ASELaunchArgu_h
#include <VIMacro.h>
class QString;
class ASELaunchArguPrivate;
class ASELaunchArgu {
public:
	enum SizeMode {
		Auto = 0,
		W3840H2160 = 1,
		W2560H1440 = 2,
		W1920H1080 = 3,
		W1600H900 = 4,
		W1368H768 = 5,
		W1024H576 = 6,
		W576H324 = 7
	};
	VICopyable(ASELaunchArgu);
	VIMoveable(ASELaunchArgu);

	ASELaunchArgu();
	ASELaunchArgu(const QString& workingFolder, const QString& mainFileName, SizeMode mode = SizeMode::W1600H900);
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
