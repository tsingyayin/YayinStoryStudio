#include "../JsonConfig.h"

namespace YSS::Utility {
	class JsonConfigPrivate
	{
		friend class JsonConfig;
		QJsonDocument JsonDoc;
		QStringList getKeys(const QString& key) {
			QJsonObject Json = JsonDoc.object();
			if (key.isEmpty()) {
				return Json.keys();
			}
			QStringList keys = key.split(".");
			QJsonValue value = Json[keys[0]];
			if (keys.size() == 1) {
				return value.toObject().keys();
			}
			else {
				for (int i = 1; i < keys.size() - 1; i++) {
					if (value.isObject()) {
						value = value.toObject()[keys[i]];
					}
					else if (value.isArray()) {
						bool ok;
						int index = keys[i].toInt(&ok);
						if (ok) {
							value = value.toArray()[index];
						}
						else {
							return QStringList();
						}
					}
					else {
						return QStringList();
					}
				}
				if (value.isObject()) {
					return value.toObject().keys();
				}
				else if (value.isArray()) {
					QStringList rtn;
					for (int i = 0; i < value.toArray().size(); i++) {
						rtn.append(QString::number(i));
					}
					return rtn;
				}
				else {
					return QStringList();
				}
			}
		}
		bool contains(const QString& key) {
			QStringList keys = key.split(".");
			if (keys.size() == 1) {
				return JsonDoc.object().contains(keys[0]);
			}
			else {
				QString parentKey = key.section(".", 0, -2);
				return getKeys(parentKey).contains(keys[keys.size() - 1]);
			}
		}
		QJsonValue getValue(const QString& key) {
			QJsonObject Json = JsonDoc.object();
			if (key.isEmpty()) {
				return Json;
			}
			QStringList keys = key.split(".");
			QJsonValue value = Json[keys[0]];
			if (keys.size() == 1) {
				return value;
			}
			else {
				for (int i = 1; i < keys.size(); i++) {
					qDebug() << value.type();
					qDebug() << keys[i];
					if (value.isObject()) {
						value = value.toObject()[keys[i]];
					}
					else if (value.isArray()) {
						bool ok;
						int index = keys[i].toInt(&ok);
						if (ok) {
							value = value.toArray()[index];
						}
						else {
							return QJsonValue();
						}
					}
					else {
						return QJsonValue();
					}
				}
				return value;
			}
		}
		QJsonValue setValue(QStringList* nameList, QStringList::iterator* it, QJsonValue val, const QJsonValue& var) {
			if (*it == nameList->end() - 1) {
				if (val.isArray()) {
					QJsonArray arr = val.toArray();
					if (arr.size() <= (*it)->toInt()) {
						arr.append(var);
					}
					else {
						arr.replace((*it)->toInt(), var);
					}
					return arr;
				}
				else {
					QJsonObject obj = val.toObject();
					obj.insert(*(*it), var);
					return obj;
				}
			}
			else {
				QStringList::iterator it2 = *it + 1;
				if (val.isArray()) {
					QJsonArray arr = val.toArray();
					arr.replace((*it)->toInt(), setValue(nameList, &it2, arr.at((*it)->toInt()), var));
					return arr;
				}
				else {
					QJsonObject obj = val.toObject();
					obj.insert(*(*it), setValue(nameList, &it2, obj.value(*(*it)), var));
					return obj;
				}
			}
		}
		void setValue(const QString& objName, const QJsonValue& value) {
			if (objName == "") { return; }
			bool haveValue = false;
			QStringList objNameList = objName.split(".");
			QStringList::iterator it = objNameList.begin();
			QJsonValue val = setValue(&objNameList, &it, JsonDoc.object(), value);
			JsonDoc.setObject(val.toObject());
		}
	};

	/*!
		\class JsonConfig
		\inmodule VIMiCore
		\brief VIJsonConfig定义了对Json配置的操作。
		\since Visindigo 2.0.0
		\sa JsonDocument QJsonObject QJsonValue

		VIJsonConfig提供对Json配置的读写操作，可以通过此类构建基于Json的配置。一般来说
		不需要直接使用此类，请改用VIJsonDocument。VIJsonDocument提供从文件直接读写Json，
		以及允许存在默认文档的配置读写功能。VIJsonConfig使用大量QtJson支持的对象，可参见
		QJsonObject，QJsonValue等内容。

		VIJsonConfig不与文件直接读写挂钩，使用VIJsonConfig时，可以使用重载为
		QString的构造函数\b{（不建议）}，或者使用parse()函数将Json字符串加载到此类中。

		与之同理，可以使用toString()函数将Json对象转换为字符串。

		不建议使用具有QString的构造函数，是因为此构造函数不会提示Json字符串的解析错误，
		且VIJsonConfig没有提供中途获取解析错误的方法，因此仍然建议只用parse()函数。

		请注意，在对VIJsonConfig进行设置之后，需要自行使用toString()将其
		保存到外部，否则设置的内容将会丢失。这同样发生在使用operator=()赋值操作符
		时。


	*/

	/*!
		\since Visindigo 2.0.0
		VIJsonConfig的默认构造函数
	*/
	JsonConfig::JsonConfig()
	{
		d = new JsonConfigPrivate();
	}
	/*!
		\since Visindigo 2.0.0
		\a jsonStr 为Json字符串。
		VIJsonConfig的构造函数
	*/
	JsonConfig::JsonConfig(const QString& jsonStr)
	{
		d = new JsonConfigPrivate();
		d->JsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8());
	}

	/*!
		\since Visindigo 2.0.0
		\a json 为Json对象。
		VIJsonConfig的构造函数
	*/
	JsonConfig::JsonConfig(const QJsonDocument& json)
	{
		d = new JsonConfigPrivate();
		d->JsonDoc = json;
	}
	/*!
		\since Visindigo 2.0.0
		\a other 为VIJsonConfig的另一个实例。
		VIJsonConfig的拷贝构造函数
	*/
	JsonConfig::JsonConfig(const JsonConfig& other) noexcept
	{
		d = new JsonConfigPrivate();
		d->JsonDoc = other.d->JsonDoc;
	}

	/*!
		\since Visindigo 2.0.0
		\a other 为VIJsonConfig的另一个实例。
		VIJsonConfig的移动构造函数
	*/
	JsonConfig::JsonConfig(JsonConfig&& other) noexcept
	{
		d = other.d;
		other.d = nullptr;
	}

	/*!
		\since Visindigo 2.0.0
		VIJsonConfig的析构函数
	*/
	JsonConfig::~JsonConfig()
	{
		delete d;
	}

	/*!
		\since Visindigo 2.0.0
		\a other 为VIJsonConfig的另一个实例。
		VIJsonConfig的赋值操作符
	*/
	JsonConfig& JsonConfig::operator=(const JsonConfig& other) noexcept
	{
		d->JsonDoc = other.d->JsonDoc;
		return *this;
	}

	/*!
		\since Visindigo 2.0.0
		\a other 为VIJsonConfig的另一个实例。
		VIJsonConfig的移动操作符
	*/
	JsonConfig& JsonConfig::operator=(JsonConfig&& other) noexcept
	{
		d = other.d;
		other.d = nullptr;
		return *this;
	}

	/*!
		\since Visindigo 2.0.0
		\a key 为键。
		一个重载操作符，旨在以类似下标访问的方式访问Json对象，返回一个VIJsonValueRef实例。
		此实例支持直接赋值，也可以使用指针操作符访问QJsonValue。此实例还重载了到QJsonValue的
		隐式转换，可以直接将其赋值给QJsonValue。
	*/
	JsonValueRef JsonConfig::operator[](const QString& key)
	{
		return JsonValueRef(this, key);
	}
	/*!
		\since Visindigo 2.0.0
		获取Json对象的所有键。（不包括子对象）
	*/
	QStringList JsonConfig::keys(const QString& key)
	{
		return d->getKeys(key);
	}

	/*!
		\fn QJsonParseError JsonConfig::parse(const QString& jsonStr)
		\since Visindigo 2.0.0
		\a json 为Json对象。
		解析Json字符串。
	*/
	QJsonParseError JsonConfig::parse(const QString& jsonStr)
	{
		QJsonParseError error;
		d->JsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);
		return error;
	}

	/*!
		\since Visindigo 2.0.0
		获取Json对象的字符串。
	*/
	QString JsonConfig::toString(QJsonDocument::JsonFormat format)
	{
		return d->JsonDoc.toJson(format);
	}

	/*!
		\since Visindigo 2.0.0
		\a key 为键。
		获取Json对象的值。
	*/
	QJsonValue JsonConfig::getValue(const QString& key, bool* ok)
	{
		if (ok != nullptr) {
			*ok = d->contains(key);
		}
		return d->getValue(key);
	}

	/*!
		\since Visindigo 2.0.0
		\a key 为键。
		获取Json对象的整数值。
	*/
	qint64 JsonConfig::getInt(const QString& key, bool* ok)
	{
		if (ok != nullptr) {
			*ok = d->contains(key);
		}
		return d->getValue(key).toInt();
	}

	/*!
		\since Visindigo 2.0.0
		\a key 为键。
		获取Json对象的字符串值。
	*/
	QString JsonConfig::getString(const QString& key, bool* ok)
	{
		if (ok != nullptr) {
			*ok = d->contains(key);
		}
		return d->getValue(key).toString();
	}
	/*!
		\since Visindigo 2.0.0
		\a key 为键。
		获取Json对象的布尔值。
	*/
	bool JsonConfig::getBool(const QString& key, bool* ok)
	{
		if (ok != nullptr) {
			*ok = d->contains(key);
		}
		return d->getValue(key).toBool();
	}

	/*!
		\since Visindigo 2.0.0
		\a key 为键。
		获取Json对象的浮点数值。
	*/
	double JsonConfig::getDouble(const QString& key, bool* ok)
	{
		if (ok != nullptr) {
			*ok = d->contains(key);
		}
		return d->getValue(key).toDouble();
	}

	/*!
		\since Visindigo 2.0.0
		\a key 为键。
		获取Json对象的子对象。
	*/
	JsonConfig JsonConfig::getObject(const QString& key, bool* ok)
	{
		if (ok != nullptr) {
			*ok = d->contains(key);
		}
		JsonConfig config;
		config.d->JsonDoc = QJsonDocument(d->getValue(key).toObject());
		return config;
	}

	/*!
		\since Visindigo 2.0.0
		\a key 为键。
		获取Json对象的数组。
	*/
	QList<JsonConfig> JsonConfig::getArray(const QString& key, bool* ok)
	{
		if (ok != nullptr) {
			*ok = d->contains(key);
		}
		QList<JsonConfig> list;
		QJsonArray array = d->getValue(key).toArray();
		for (int i = 0; i < array.size(); i++) {
			JsonConfig config;
			config.d->JsonDoc = QJsonDocument(array[i].toObject());
			list.append(config);
		}
		return list;
	}

	/*!
		\since Visindigo 2.0.0
		\a key 为键。
		判断Json对象是否包含指定的键。
	*/
	bool JsonConfig::contains(const QString& key)
	{
		return d->contains(key);
	}

	/*!
		\since Visindigo 2.0.0
		\a key 为键。
		\a value 为值。
		设置Json对象的值。
	*/
	void JsonConfig::setValue(const QString& key, const QJsonValue& value)
	{
		d->setValue(key, const_cast<QJsonValue&>(value));
	}

	/*!
		\since Visindigo 2.0.0
		\a key 为键。
		\a value 为整数值。
		设置Json对象的整数值。
	*/
	void JsonConfig::setInt(const QString& key, qint64 value)
	{
		QJsonValue v(value);
		d->setValue(key, v);
	}

	/*!
		\since Visindigo 2.0.0
		\a key 为键。
		\a value 为字符串值。
		设置Json对象的字符串值。
	*/
	void JsonConfig::setString(const QString& key, const QString& value)
	{
		QJsonValue v(value);
		d->setValue(key, v);
	}

	/*!
		\since Visindigo 2.0.0
		\a key 为键。
		\a value 为布尔值。
		设置Json对象的布尔值。
	*/
	void JsonConfig::setBool(const QString& key, bool value)
	{
		QJsonValue v(value);
		d->setValue(key, v);
	}

	/*!
		\since Visindigo 2.0.0
		\a key 为键。
		\a value 为浮点数值。
		设置Json对象的浮点数值。
	*/
	void JsonConfig::setDouble(const QString& key, double value)
	{
		QJsonValue v(value);
		d->setValue(key, v);
	}

	/*!
		\since Visindigo 2.0.0
		\a key 为键。
		\a value 为子对象。
		设置Json对象的子对象。
	*/
	void JsonConfig::setObject(const QString& key, const JsonConfig& value)
	{
		QJsonValue v(value.d->JsonDoc.object());
		d->setValue(key, v);
	}

	/*!
		\since Visindigo 2.0.0
		\a key 为键。
		\a value 为数组。
		设置Json对象的数组。
	*/
	void JsonConfig::setArray(const QString& key, const QList<JsonConfig>& value)
	{
		QJsonArray array;
		for (int i = 0; i < value.size(); i++) {
			array.append(value[i].d->JsonDoc.object());
		}
		QJsonValue v(array);
		d->setValue(key, v);
	}

	/*!
		\since Visindigo 2.0.0
		\a key 为键。
		判断Json对象是否为空。
	*/
	bool JsonConfig::isEmpty(const QString& key)
	{
		return d->getValue(key).isObject() && d->getValue(key).toObject().isEmpty();
	}

	/*!
		\since Visindigo 2.0.0
		\a key 为键。
		判断Json对象是否为空。
	*/

	bool JsonConfig::isNull(const QString& key)
	{
		return d->getValue(key).isNull();
	}

	/*!
		\since Visindigo 2.0.0
		\a key 为键。
		判断Json对象是否为对象。
	*/
	bool JsonConfig::isObject(const QString& key)
	{
		return d->getValue(key).isObject();
	}

	/*!
		\since Visindigo 2.0.0
		\a key 为键。
		判断Json对象是否为数组。
	*/
	bool JsonConfig::isArray(const QString& key)
	{
		return d->getValue(key).isArray();
	}

	/*!
		\since Visindigo 2.0.0
		\a key 为键。
		判断Json对象是否为字符串。
	*/

	bool JsonConfig::isString(const QString& key)
	{
		return d->getValue(key).isString();
	}

	/*!
		\since Visindigo 2.0.0
		\a key 为键。
		判断Json对象是否为布尔值。
	*/
	bool JsonConfig::isBool(const QString& key)
	{
		return d->getValue(key).isBool();
	}

	/*!
		\since Visindigo 2.0.0
		\a key 为键。
		判断Json对象是否为整数。
	*/
	bool JsonConfig::isInt(const QString& key)
	{
		return d->getValue(key).isDouble();
	}

	/*!
		\since Visindigo 2.0.0
		\a key 为键。
		判断Json对象是否为浮点数。
	*/
	bool JsonConfig::isDouble(const QString& key)
	{
		return d->getValue(key).isDouble();
	}

}
