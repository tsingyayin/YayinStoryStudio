#include "../YayinStoryStudio.h"
#include "../Version.h"
#include "../private/AUTO_VERSION.h"
namespace YSSCore::General {
	Version YayinStoryStudio::getVersion()
	{
		return Version(YSSCore_VERSION_MAJOR,
			YSSCore_VERSION_MINOR,
			YSSCore_VERSION_PATCH,
			true,
			YSSCore_VERSION_BUILD,
			YSSCore_VERSION_NICKNAME);
	}
	Version YayinStoryStudio::getABIVersion()
	{
		return Version(YSSCore_ABI_VERSION_MAJOR, YSSCore_ABI_VERSION_MINOR, 0);
	}
}