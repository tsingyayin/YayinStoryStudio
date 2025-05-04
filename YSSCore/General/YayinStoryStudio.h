#pragma once
#include <QtCore>
#include "../Macro.h"
#include "Version.h"
namespace YSSCore::General {
	class YayinStoryStudio
	{
	public:
		static Version getVersion();
		static Version getABIVersion();
	};
}