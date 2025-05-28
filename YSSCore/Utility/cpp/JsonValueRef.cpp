#include "../JsonValueRef.h"
#include "../JsonConfig.h"

namespace YSSCore::Utility {
	class VIJsonValueRefPrivate {
		friend class JsonValueRef;
		JsonConfig* Config = nullptr;
		QString Key;
	};

	/*!
		\class YSSCore::Utility::JsonValueRef
		\brief 此类用于访问VIJsonConfig中的值
		\since YSSCore 0.13.0
		\inmodule YSSCore

		VIJsonValueRef是一个通过在内部保存保存VIJsonConfig和Key来访问VIJsonConfig中的值的类，
		本质上是为辅助VIJsonConfig对operator[]重载而设计的。

		此类不提供详细的文档，因为其提供与VIJsonConfig类似的接口。唯一区别是VIJsonConfig中以get开头的函数
		在VIJsonValueRef中以to开头。

		\note VIJsonValueRef一般来说只在VIJsonConfig的VIJsonConfig::operator[]重载中立即使用，不应做长时保存，
		因为对应VIJsonConfig的生命周期可能短于VIJsonValueRef，而VIJsonValueRef不会对
		其保存的ViJsonConfig做空指针检查。
	*/
	JsonValueRef::JsonValueRef(JsonConfig* config, const QString& key) {
		d = new VIJsonValueRefPrivate();
		d->Config = config;
		d->Key = key;
	}

	JsonValueRef::JsonValueRef(const JsonValueRef& other) {
		d = new VIJsonValueRefPrivate();
		d->Config = other.d->Config;
		d->Key = other.d->Key;
	}

	JsonValueRef::JsonValueRef(JsonValueRef&& other) noexcept {
		d = other.d;
		other.d = nullptr;
	}

	JsonValueRef::~JsonValueRef() {
		delete d;
	}

	void JsonValueRef::operator=(const QJsonValue& value) {
		d->Config->setValue(d->Key, value);
	}

	QJsonValue JsonValueRef::operator->() const {
		return d->Config->getValue(d->Key);
	}

	JsonValueRef::operator QJsonValue() const {
		return d->Config->getValue(d->Key);
	}

	JsonValueRef::operator bool() const {
		return d->Config->getBool(d->Key);
	}

	JsonValueRef::operator int() const {
		return d->Config->getInt(d->Key);
	}

	JsonValueRef::operator double() const {
		return d->Config->getDouble(d->Key);
	}

	JsonValueRef::operator QString() const {
		return d->Config->getString(d->Key);
	}

	QJsonValue::Type JsonValueRef::type() const {
		return d->Config->getValue(d->Key).type();
	}

	bool JsonValueRef::toBool() const {
		return d->Config->getBool(d->Key);
	}

	int JsonValueRef::toInt() const {
		return d->Config->getInt(d->Key);
	}

	double JsonValueRef::toDouble() const {
		return d->Config->getDouble(d->Key);
	}

	QString JsonValueRef::toString() const {
		return d->Config->getString(d->Key);
	}

	QList<JsonConfig> JsonValueRef::toArray() const {
		return d->Config->getArray(d->Key);
	}

	JsonConfig JsonValueRef::toObject() const {
		return d->Config->getObject(d->Key);
	}
}