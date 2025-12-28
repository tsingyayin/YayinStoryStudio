#ifndef Visindigo_Utility_NJsonConfig_h
#define Visindigo_Utility_NJsonConfig_h
#include "Utility/GeneralConfig.h"

namespace Visindigo::Utility {
	class VisindigoAPI GeneralConfigParser {
	public:
		static GeneralConfig* parseFromJson(const QString& jsonString, bool* ok = nullptr);
		static QString serializeToJson(GeneralConfig* config, GeneralConfig::StringFormat format = GeneralConfig::StringFormat::Formatted, qint32 formatIndent = 4);
		static GeneralConfig parseFromYaml(const QString& yamlString, bool* ok = nullptr);
		static QString serializeToYaml(const GeneralConfig& config, GeneralConfig::StringFormat format = GeneralConfig::StringFormat::Formatted, qint32 formatIndent = 4);
	};
}
#endif // Visindigo_Utility_NJsonConfig_h