#ifndef Visindigo_Utility_NJsonConfig_h
#define Visindigo_Utility_NJsonConfig_h
#include "Utility/GeneralConfig.h"

namespace Visindigo::Utility {
	class VisindigoAPI GeneralConfigParser {
	public:
		static GeneralConfig* parseFromJson(const QString& jsonString, bool* ok = nullptr);
		static GeneralConfig* parseFromJson_2(const QString& jsonString, bool* ok = nullptr);
		static QString serializeToJson(GeneralConfig* config, GeneralConfig::StringFormat format = GeneralConfig::StringFormat::Formatted, qint32 formatIndent = 4);
		static GeneralConfig* parseFromYaml(const QString& yamlString, bool* ok = nullptr);
		static QString serializeToYaml(GeneralConfig* config, GeneralConfig::StringFormat format = GeneralConfig::StringFormat::Formatted, qint32 formatIndent = 4);
		static GeneralConfig* parseFromXml(const QString& xml, bool* ok = nullptr);
		static QString serializeToXml(GeneralConfig* config, GeneralConfig::StringFormat format = GeneralConfig::StringFormat::Formatted, qint32 formatIndent = 4);
		static GeneralConfig* parseFromVBC(const QByteArray& vbcData, bool* ok = nullptr);
		static QByteArray serializeToVBC(GeneralConfig* config);
	};
}
#endif // Visindigo_Utility_NJsonConfig_h