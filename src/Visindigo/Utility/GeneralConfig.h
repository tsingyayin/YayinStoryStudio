#ifndef Visindigo_Utility_GeneralConfig_h
#define Visindigo_Utility_GeneralConfig_h
#include "../Macro.h"
#include <QtCore/qmap.h>
#include <QtCore/qlist.h>
#include <QtCore/qstring.h>

namespace Visindigo::__Private__ {
	class GeneralConfigPrivate;
	class GeneralConfigMemPool;
}

namespace Visindigo::Utility {
	class GeneralConfig;
	using GeneralConfigMap = QMap<QString, GeneralConfig*>;
	using GeneralConfigList = QList<GeneralConfig*>;

	class VisindigoAPI GeneralConfig {
		friend class Visindigo::__Private__::GeneralConfigPrivate;
		friend class Visindigo::__Private__::GeneralConfigMemPool;
	public:
		enum Type {
			None, Integer, Real, Bool,
			String, List, Map, Reference,
			Number = Real,
			Array = List,
			Dict = Map,
			Null = None
		};
		enum StringFormat {
			Stream, Formatted
		};
		
		GeneralConfig();
		GeneralConfig(qint64 value);
		GeneralConfig(qreal value);
		GeneralConfig(bool value);
		GeneralConfig(const QString& value, bool isRef = false);
		GeneralConfig(const GeneralConfigList& value);
		GeneralConfig(const GeneralConfigMap& value);
		virtual ~GeneralConfig();

		static GeneralConfig* copyFrom(GeneralConfig* other);

		qint64 getInt(const QString& key = "", qint64 def = 0, bool* ok = nullptr);
		qreal getReal(const QString& key = "", qreal def = 0.0, bool* ok = nullptr);
		bool getBool(const QString& key = "", bool def = false, bool* ok = nullptr);
		QString getString(const QString& key = "", const QString& def = "", bool* ok = nullptr);
		QString getRefNode(const QString& key = "", const QString& def = "", bool* ok = nullptr);
		GeneralConfigList getList(const QString& key = "", const GeneralConfigList& def = {}, bool* ok = nullptr);
		GeneralConfigMap getMap(const QString& key = "", const GeneralConfigMap& def = {}, bool* ok = nullptr);
		QString getComment(const QString& key = "", bool* ok = nullptr, bool isHeadComment = false);
		GeneralConfig* getChildNode(const QString& key);
		QStringList getKeys(const QString& key = "", bool* ok = nullptr);

		void setInt(qint64 value, const QString& key = "");
		void setDouble(double value, const QString& key = "");
		void setBool(bool value, const QString& key = "");
		void setString(const QString& value, const QString& key = "");
		void setRefNode(const QString& value, const QString& key = "");
		void setList(const GeneralConfigList& value, const QString& key = "");
		void setMap(const GeneralConfigMap& value, const QString& key = "");
		void setComment(const QString& comment, const QString& key = "", bool isHeadComment = false);
		

		bool contains(const QString& key);
		Type getNodeType(const QString& key = "", bool refNodeGetFinalType = false);
		bool isNone(const QString& key = "");
		bool isInt(const QString& key = "");
		bool isReal(const QString& key = "");
		bool isString(const QString& key = "");
		bool isList(const QString& key = "");
		bool isMap(const QString& key = "");
		bool isReference(const QString& key = "");

	protected:
		Visindigo::__Private__::GeneralConfigPrivate* d;
	};

	
}

#endif