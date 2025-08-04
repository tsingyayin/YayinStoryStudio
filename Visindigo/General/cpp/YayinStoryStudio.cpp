#include "../YayinStoryStudio.h"
#include "../Version.h"
#include "../private/AUTO_VERSION.h"
namespace Visindigo::General {
	Version YayinStoryStudio::getVersion()
	{
		return Version(Visindigo_VERSION_MAJOR,
			Visindigo_VERSION_MINOR,
			Visindigo_VERSION_PATCH,
			true,
			Visindigo_VERSION_BUILD,
			Visindigo_VERSION_NICKNAME);
	}
	Version YayinStoryStudio::getABIVersion()
	{
		return Version(Visindigo_ABI_VERSION_MAJOR, Visindigo_ABI_VERSION_MINOR, 0);
	}
}