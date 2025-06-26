#pragma once
#include "../Macro.h"

namespace YSSCore::General {
	class Version;
	class YSSCoreAPI YayinStoryStudio
	{
	public:
		static Version getVersion();
		static Version getABIVersion();
	};
}