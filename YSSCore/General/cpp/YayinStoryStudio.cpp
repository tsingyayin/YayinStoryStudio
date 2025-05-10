#include "../YayinStoryStudio.h"
#include "../Version.h"
#include "../private/AUTO_VERSION.h"
namespace YSSCore::General {
	Version YayinStoryStudio::getVersion()
	{
		return Version(10, 0, 0, true, YSSCore_VERSION_BUILD);
	}
	Version YayinStoryStudio::getABIVersion()
	{
		return Version(0, 1, 0, true, 0);
	}
}