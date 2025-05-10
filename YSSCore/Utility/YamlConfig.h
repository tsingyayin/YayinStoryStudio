#pragma once
#include "../Macro.h"
#include <QtCore/qtypes.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
namespace YAML {
	class Node;
}
namespace YSSCore::Utility {
	class YamlConfigPrivate;
	class YSSCoreAPI YamlConfig {
		friend YamlConfigPrivate;
	public:
		YamlConfig();
		YamlConfig(const QString& yamlStr);;
		YamlConfig(const YamlConfig& other) noexcept;
		YamlConfig(YamlConfig&& other) noexcept;
		~YamlConfig();
		YamlConfig& operator=(const YamlConfig& other) noexcept;
		YamlConfig& operator=(YamlConfig&& other) noexcept;
		YAML::Node operator[](const QString& key);
		QStringList keys(const QString& key = "");
		bool parse(const QString& YamlStr);
		QString toString();
		qint64 getInt(const QString& key = "", bool* ok = nullptr);
		QString getString(const QString& key = "", bool* ok = nullptr);
		bool getBool(const QString& key = "", bool* ok = nullptr);
		double getDouble(const QString& key = "", bool* ok = nullptr);
		YAML::Node getObject(const QString& key = "", bool* ok = nullptr);
		QList<YAML::Node> getArray(const QString& key = "", bool* ok = nullptr);
		bool contains(const QString& key);
		void setInt(const QString& key, qint64 value);
		void setString(const QString& key, const QString& value);
		void setBool(const QString& key, bool value);
		void setDouble(const QString& key, double value);
		void setObject(const QString& key, const YAML::Node& value);
		void setArray(const QString& key, const QList<YAML::Node>& value);
		bool isEmpty(const QString& key = "");
		bool isNull(const QString& key = "");
		bool isObject(const QString& key = "");
		bool isArray(const QString& key = "");
		bool isString(const QString& key = "");
		bool isBool(const QString& key = "");
		bool isInt(const QString& key = "");
		bool isDouble(const QString& key = "");
	private:
		YamlConfigPrivate* d;
	};
}