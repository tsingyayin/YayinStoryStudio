#pragma once
#include "../Macro.h"

namespace Visindigo::General {
	class Version;
	class VisindigoAPI YayinStoryStudio
	{
	public:
		static Version getVersion();
		static Version getABIVersion();
	};
}