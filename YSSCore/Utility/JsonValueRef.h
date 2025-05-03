#pragma once
#include <QtCore>
#include "../Macro.h"

namespace YSSCore::Utility {

	class JsonConfig;
	class JsonDocument;
	class VIJsonValueRefPrivate;

	class YSSCoreAPI JsonValueRef {
		friend class JsonConfig;
		friend class JsonDocument;
		friend class VIJsonValueRefPrivate;
	protected:
		JsonValueRef(JsonConfig* config, const QString& key);
	public:
		JsonValueRef(const JsonValueRef& other);
		JsonValueRef(JsonValueRef&& other) noexcept;
		~JsonValueRef();
		void operator=(const QJsonValue& value);
		QJsonValue operator->() const;
		operator QJsonValue() const;
		operator bool() const;
		operator int() const;
		operator double() const;
		operator QString() const;
		QJsonValue::Type type() const;
		inline bool isNull() const { return type() == QJsonValue::Type::Null; }
		inline bool isBool() const { return type() == QJsonValue::Type::Bool; }
		inline bool isDouble() const { return type() == QJsonValue::Type::Double; }
		inline bool isString() const { return type() == QJsonValue::Type::String; }
		inline bool isArray() const { return type() == QJsonValue::Type::Array; }
		inline bool isObject() const { return type() == QJsonValue::Type::Object; }
		inline bool isUndefined() const { return type() == QJsonValue::Type::Undefined; }
		bool toBool() const;
		int toInt() const;
		double toDouble() const;
		QString toString() const;
		QList<JsonConfig> toArray() const;
		JsonConfig toObject() const;
	private:
		VIJsonValueRefPrivate* d;
	};
}