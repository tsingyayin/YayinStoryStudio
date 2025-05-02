#pragma once
#include "JsonConfig.h"

namespace YSS::Utility {

	class JsonDocumentPrivate;

	class JsonDocument
	{
		friend class JsonConfigPrivate;
	public:
		JsonDocument();
		JsonDocument(const QString& configPath, const QString& defaultConfigPath);
		JsonDocument(const JsonDocument& other) = delete;
		JsonDocument(JsonDocument&& other) = delete;
		~JsonDocument();
		QJsonParseError load(const QString& configPath = "", const QString& defaultConfigPath = "", uchar* whichError = nullptr, bool configAutoCreate = true);
		void save();
		QStringList keys(const QString& key = "");
		JsonValueRef operator[](const QString& key);
		QJsonValue getValue(const QString& key = "");
		qint64 getInt(const QString& key = "");
		QString getString(const QString& key = "");
		bool getBool(const QString& key = "");
		double getDouble(const QString& key = "");
		JsonConfig getObject(const QString& key = "");
		QList<JsonConfig> getArray(const QString& key = "");
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
		JsonDocumentPrivate* d;
	};
}