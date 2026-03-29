#pragma once
#include <QtCore/qjsondocument.h>
#include <QtCore/qstring.h>
#include <QtCore/qmetaobject.h>
#include "../VICompileMacro.h"
// Forward declarations
class QJsonValue;
namespace Visindigo::Utility {
	class JsonConfigPrivate;
	class JsonValueRef;
}
// Main
namespace Visindigo::Utility {
	template<typename T>
	concept VI_Q_OBJECT = requires(T t) {
		{ t.metaobject() } -> std::same_as<const QMetaObject*>;
	};
	template<typename T>
	concept VI_Q_GADGET = requires(T t) {
		{ t.qt_check_for_QGADGET_macro() } -> std::same_as<void>;
	};

	class VisindigoAPI JsonConfig
	{
		friend class JsonConfigPrivate;
	public:
		static JsonConfig fromJson(const QString& jsonStr, QJsonParseError* error = nullptr);
		template<VI_Q_GADGET T>static JsonConfig fromMetable(const T& object);
		template<VI_Q_OBJECT T>static JsonConfig fromMetable(const T& object);
		template<VI_Q_GADGET T>static T toMetable(const JsonConfig& config, bool* ok = nullptr);
		template<VI_Q_GADGET T>static T toMetable(const QString& jsonStr, bool* ok = nullptr);
		template<VI_Q_OBJECT T>static void writeMetable(T* object, const JsonConfig& config, bool* ok = nullptr);
		template<VI_Q_OBJECT T>static void writeMetable(T* object, const QString& jsonStr, bool* ok = nullptr);
	public:
		JsonConfig();
		JsonConfig(const QVariant& variant);
		JsonConfig(const QString& jsonStr);
		JsonConfig(const QJsonDocument& json);
		JsonConfig(const JsonConfig& other) noexcept;
		JsonConfig(JsonConfig&& other) noexcept;
		~JsonConfig();
		JsonConfig& operator=(const JsonConfig& other) noexcept;
		JsonConfig& operator=(JsonConfig&& other) noexcept;
		JsonValueRef operator[](const QString& key);
		QStringList keys(const QString& key = "") const;
		bool remove(const QString& key);
		QJsonParseError parse(const QString& jsonStr);
		QString toString(QJsonDocument::JsonFormat format = QJsonDocument::Indented);
		QJsonValue getValue(const QString& key = "", bool* ok = nullptr) const;
		qint64 getInt(const QString& key = "", bool* ok = nullptr);
		QString getString(const QString& key = "", bool* ok = nullptr);
		bool getBool(const QString& key = "", bool* ok = nullptr);
		double getDouble(const QString& key = "", bool* ok = nullptr);
		JsonConfig getObject(const QString& key = "", bool* ok = nullptr);
		QList<JsonConfig> getArray(const QString& key = "", bool* ok = nullptr);
		QStringList getStringList(const QString& key = "", bool* ok = nullptr);
		bool contains(const QString& key) const;
		void clear();
		void setValue(const QString& key, const QJsonValue& value);
		void setInt(const QString& key, qint64 value);
		void setString(const QString& key, const QString& value);
		void setBool(const QString& key, bool value);
		void setDouble(const QString& key, double value);
		void setObject(const QString& key, const JsonConfig& value);
		void setArray(const QString& key, const QList<JsonConfig>& value);
		void setArray(const QString& key, const QStringList& value);
		void setStringList(const QString& key, const QStringList& value);
		bool isEmpty(const QString& key = "") const;
		bool isNull(const QString& key = "") const;
		bool isObject(const QString& key = "") const;
		bool isArray(const QString& key = "") const;
		bool isString(const QString& key = "") const;
		bool isBool(const QString& key = "") const;
		bool isInt(const QString& key = "") const;
		bool isDouble(const QString& key = "") const;
	private:
		JsonConfigPrivate* d;
	};
}

#include <QtCore/qjsonvalue.h>
namespace Visindigo::Utility {
	template<VI_Q_OBJECT T>JsonConfig JsonConfig::fromMetable(const T& object) {
		const QMetaObject* metaObject = object.metaObject();
		JsonConfig config;
		for (int i = metaObject->propertyOffset(); i < metaObject->propertyCount(); i++) {
			QMetaProperty property = metaObject->property(i);
			const char* propName = property.name();
			QVariant value = property.read(&object);
			config.setValue(QString::fromLocal8Bit(propName), QJsonValue::fromVariant(value));
		}
		return config;
	}

	template<VI_Q_GADGET T>JsonConfig JsonConfig::fromMetable(const T& object) {
		const QMetaObject* metaObject = &T::staticMetaObject;
		JsonConfig config;
		for (int i = metaObject->propertyOffset(); i < metaObject->propertyCount(); i++) {
			QMetaProperty property = metaObject->property(i);
			const char* propName = property.name();
			QVariant value = property.read(&object);
			config.setValue(QString::fromLocal8Bit(propName), QJsonValue::fromVariant(value));
		}
		return config;
	}

	template<VI_Q_GADGET T>T JsonConfig::toMetable(const JsonConfig& config, bool* ok) {
		T object;
		const QMetaObject* metaObject = &T::staticMetaObject;
		bool success = true;
		for (int i = metaObject->propertyOffset(); i < metaObject->propertyCount(); i++) {
			QMetaProperty property = metaObject->property(i);
			const char* propName = property.name();
			if (config.contains(propName)) {
				QVariant value = config.getValue(QString::fromLocal8Bit(propName)).toVariant();
				property.writeOnGadget(&object, value);
			}
			else {
				success = false;
			}
		}
		if (ok) {
			*ok = success;
		}
		return object;
	}

	template<VI_Q_GADGET T>T JsonConfig::toMetable(const QString& jsonStr, bool* ok) {
		JsonConfig config(jsonStr);
		return toMetable<T>(config, ok);
	}

	template<VI_Q_OBJECT T>void JsonConfig::writeMetable(T* object, const JsonConfig& config, bool* ok) {
		const QMetaObject* metaObject = object->metaObject();
		bool success = true;
		for (int i = metaObject->propertyOffset(); i < metaObject->propertyCount(); i++) {
			QMetaProperty property = metaObject->property(i);
			const char* propName = property.name();
			if (config.contains(propName)) {
				QVariant value = config.getValue(QString::fromLocal8Bit(propName)).toVariant();
				property.write(object, value);
			}
			else {
				success = false;
			}
		}
		if (ok) {
			*ok = success;
		}
	}

	template<VI_Q_OBJECT T>void JsonConfig::writeMetable(T* object, const QString& jsonStr, bool* ok) {
		JsonConfig config(jsonStr);
		writeMetable(object, config, ok);
	}
}