#pragma once
#include <QtCore>
#include <QJsonValue>
#include <QJsonDocument>
#include "JsonValueRef.h"

namespace YSS::Utility {

	class JsonConfigPrivate;

	class JsonConfig
	{
		friend class JsonConfigPrivate;
	public:
		JsonConfig();
		JsonConfig(const QString& jsonStr);
		JsonConfig(const QJsonDocument& json);
		JsonConfig(const JsonConfig& other) noexcept;
		JsonConfig(JsonConfig&& other) noexcept;
		~JsonConfig();
		JsonConfig& operator=(const JsonConfig& other) noexcept;
		JsonConfig& operator=(JsonConfig&& other) noexcept;
		JsonValueRef operator[](const QString& key);
		QStringList keys(const QString& key = "");
		QJsonParseError parse(const QString& jsonStr);
		QString toString(QJsonDocument::JsonFormat format = QJsonDocument::Indented);
		QJsonValue getValue(const QString& key = "", bool* ok = nullptr);
		qint64 getInt(const QString& key = "", bool* ok = nullptr);
		QString getString(const QString& key = "", bool* ok = nullptr);
		bool getBool(const QString& key = "", bool* ok = nullptr);
		double getDouble(const QString& key = "", bool* ok = nullptr);
		JsonConfig getObject(const QString& key = "", bool* ok = nullptr);
		QList<JsonConfig> getArray(const QString& key = "", bool* ok = nullptr);
		bool contains(const QString& key);
		void setValue(const QString& key, const QJsonValue& value);
		void setInt(const QString& key, qint64 value);
		void setString(const QString& key, const QString& value);
		void setBool(const QString& key, bool value);
		void setDouble(const QString& key, double value);
		void setObject(const QString& key, const JsonConfig& value);
		void setArray(const QString& key, const QList<JsonConfig>& value);
		bool isEmpty(const QString& key = "");
		bool isNull(const QString& key = "");
		bool isObject(const QString& key = "");
		bool isArray(const QString& key = "");
		bool isString(const QString& key = "");
		bool isBool(const QString& key = "");
		bool isInt(const QString& key = "");
		bool isDouble(const QString& key = "");
	private:
		JsonConfigPrivate* d;
	};
}