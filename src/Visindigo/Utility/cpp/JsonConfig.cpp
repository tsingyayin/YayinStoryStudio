#include "../JsonConfig.h"
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qjsonvalue.h>
#include <QtCore/qjsonarray.h>
#include "../JsonValueRef.h"
#include "../../General/Log.h"
namespace Visindigo::Utility {
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
			else {
				for (int i = 1; i < keys.size(); i++) {
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
			if (!val.toObject().contains(*(*it))) {
				if (nameList->end() != (*it) + 1) {
					QString nextkey = *((*it) + 1);
					bool ok = false;
					int index = nextkey.toInt(&ok);
					if (!val.isArray()){
						QJsonObject obj = val.toObject();
						if (ok) {
							obj.insert(*(*it), QJsonArray());
						}
						else {
							obj.insert(*(*it), QJsonValue());
						}
						val = obj;
					}else{
						QJsonArray arr = val.toArray();
						if (arr.size() <= (*it)->toInt()) {
							arr.append(QJsonValue());
						}
						val = arr;
					}
				}
			}
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
		bool remove(const QString& objName) {
			if (objName == "") { return false; }
			bool haveValue = false;
			QStringList objNameList = objName.split(".");
			QStringList::iterator it = objNameList.begin();
			QJsonValue val = remove(&objNameList, &it, JsonDoc.object(), &haveValue);
			JsonDoc.setObject(val.toObject());
			return haveValue;
		};
		QJsonValue remove(QStringList* nameList, QStringList::iterator* it, QJsonValue val, bool* haveValue) {
			if (*it == nameList->end() - 1) {
				if (val.isArray()) {
					QJsonArray arr = val.toArray();
					if (arr.size() > (*it)->toInt()) {
						arr.removeAt((*it)->toInt());
						*haveValue = true;
					}
					return arr;
				}
				else {
					QJsonObject obj = val.toObject();
					obj.remove(*(*it));
					return obj;
				}
			}
			else {
				QStringList::iterator it2 = *it + 1;
				if (val.isArray()) {
					QJsonArray arr = val.toArray();
					arr.replace((*it)->toInt(), remove(nameList, &it2, arr.at((*it)->toInt()), haveValue));
					return arr;
				}
				else {
					QJsonObject obj = val.toObject();
					obj.insert(*(*it), remove(nameList, &it2, obj.value(*(*it)), haveValue));
					return obj;
				}
			}
		}
	};

	/*!
		\class Visindigo::Utility::JsonConfig
		\inmodule Visindigo
		\brief JsonConfig定义了对Json配置的操作。
		\since Visindigo 0.10.0
		\sa Visindigo::Utility::JsonDocument QJsonObject QJsonValue

		JsonConfig提供对Json配置的读写操作，可以通过此类构建基于Json的配置。
		JsonConfig主体上提供的是一套纯动态方式的接口，允许你在运行时动态地访问和修改Json对象的内容，
		而不需要事先定义Json对象的结构。
		为此，我们规定通过句点 “.” 来访问Json对象的子对象，例如 "parent.child.key" 
		表示访问Json对象中键为 "parent" 的子对象中的键为 "child" 的子对象中的键为 "key" 的值。
		\warning 这种规定对键名有所限制，即键名不能包含句点。如果Json文档中的键名包含句点，
		则行为未定义。

		除了动态访问之外，JsonConfig也提供了一些静态函数，允许你将JsonConfig与有Q_OBJECT或
		Q_GADGET的类进行转换——只要它们的成员正确的使用Q_PROPERTY进行了标记即可。
		有关这部分内容请详见下文。

		由于JsonConfig只是在QtJson设施上进行了一层封装，因此它的性能和QtJson基本相当。
		
		如果你不以明确类型操作这个类，你可能还会需要查看QJsonValue、QJsonObject、QVariant
		等类型的文档。它们是QtJson和可变类型量的重要套件。

		JsonConfig永远不与文件直接读写挂钩，这类里所有涉及传递QString进行构造或
		初始化的函数，其语义永远是传递Json字符串本体，而不是文件路径。

		与之同理，可以使用toString()函数将Json对象转换为字符串。

		不建议使用具有QString的构造函数，是因为此构造函数不会提示Json字符串的解析错误，
		且JsonConfig没有提供中途获取解析错误的方法，因此仍然建议只用parse()函数。
		但如果你确实希望一步完成构建，建议使用静态成员函数fromJson进行构造，这允许你传递一个
		QJsonParseError指针来获取解析错误。

		请注意，在对JsonConfig进行设置之后，需要自行使用toString()将其
		保存到外部，否则设置的内容将会丢失。这同样发生在使用operator=()赋值操作符
		时。

		JsonConfig可重入，但不保证线程安全。要从多个线程同时操作一个JsonConfig对象，必须使用外部同步机制。

		\section1 概念VI_Q_OBJECT和VI_Q_GADGET
		VI_Q_OBJECT和VI_Q_GADGET是Visindigo定义的两个位于Visindigo::Utility中的
		concept，分别对应具有Q_OBJECT标记和Q_GADGET标记的类。这些类已经被Qt的
		元对象编译器添加了元信息，因此JsonConfig可以通过QMetaObject访问其中的
		Q_PROPERTY成员，并进行相应的转换。

		由于QDoc不能正确标记概念，我们在此处直接给出它们的定义：
		\code
		template <typename T> concept VI_Q_OBJECT = requires(T t) {
			{ t.metaobject() } -> std::same_as<const QMetaObject*>;
		};
		template <typename T> concept VI_Q_GADGET = requires(T t) {
			{ t.qt_check_for_QGADGET_macro() } -> std::same_as<void>;
		};
		\endcode

		值得注意的是，由于Q_OBJECT删除了类的移动和拷贝构造函数，因此JsonConfig访问
		这两种类的方式略有差别。具体来说，二者都可用fromMetable从类转换为JsonConfig。
		但对于构造，Q_GADGET用返回值的方式构造，函数为toMetable。而Q_OBJECT需要
		用户自己事先构造，然后调用writeMetable并传入指针以完成写入。

		下面以一个Q_GADGET为例说明如何使用JsonConfig进行转换：
		首先我们定义一个Q_GADGET类，并使用Q_PROPERTY标记其成员：
		\code
		class MyConfig {
			Q_GADGET
			Q_PROPERTY(int intValue MEMBER intValue)
			Q_PROPERTY(QString stringValue MEMBER stringValue)
		public:
			int intValue;
			QString stringValue;
		}
		\endcode

		然后我们假定存在这样的Json字符串：
		\code
		QString jsonStr = R"({
			"intValue": 42,
			"stringValue": "Hello, World!"
		})";
		\endcode

		我们就可以通过fromJson函数将其转换为JsonConfig对象：
		\code
		MyConfig config = JsonConfig::toMetable<MyConfig>(JsonConfig::fromJson(jsonStr));
		\endcode

		如果MyConfig非常不幸的是一个Q_OBJECT类，那么最后一步只需改为：
		\code
		MyConfig config;
		JsonConfig::fromMetable(&config, JsonConfig::fromJson(jsonStr));
		\endcode

	*/

	/*!
		\since Visindigo 0.13.0
		\a jsonStr 为Json字符串。
		\a error 为指向QJsonParseError对象的指针，用于获取解析错误信息。默认为nullptr。
		从Json字符串构造一个JsonConfig对象，并返回该对象。如果解析过程中发生错误，可以通过 \a error 参数获取错误信息。
	*/
	JsonConfig JsonConfig::fromJson(const QString& jsonStr, QJsonParseError* error)
	{
		JsonConfig config;
		QJsonParseError err;
		err = config.parse(jsonStr);
		if (error != nullptr) {
			*error = err;
		}
		return config;
	}

	/*!
		\fn template<typename T>static JsonConfig JsonConfig::fromMetable(const T& obj)
		\since Visindigo 0.13.0
		\a obj 为具有Q_GADGET或Q_OBJECT标记的类的实例。
		从具有Q_GADGET或Q_OBJECT标记的类的实例构造一个JsonConfig对象，并返回该对象。
	*/

	/*!
		\fn template<typename T>static T JsonConfig::toMetable(const JsonConfig& config, bool* ok)
		\since Visindigo 0.13.0
		\a config 为JsonConfig对象。
		\a ok 为指向bool类型的指针，用于获取转换是否成功的信息。默认为nullptr。
		将JsonConfig对象转换为具有Q_GADGET标记的类的实例，并返回该实例。如果转换过程中发生错误，ok参数将被设置为false。
	*/

	/*!
		\fn template<typename T>static T JsonConfig::toMetable(const QString& jsonStr, bool* ok)
		\since Visindigo 0.13.0
		\a jsonStr 为Json字符串。
		\a ok 为指向bool类型的指针，用于获取转换是否成功的信息。默认为nullptr。
		将Json字符串转换为具有Q_GADGET标记的类的实例，并返回该实例。如果转换过程中发生错误，ok参数将被设置为false。
	*/

	/*!
		\fn template<typename T>static void JsonConfig::writeMetable(T* obj, const JsonConfig& config, bool* ok)
		\since Visindigo 0.13.0
		\a obj 为指向具有Q_OBJECT标记的类的实例的指针。
		\a config 为JsonConfig对象。
		\a ok 为指向bool类型的指针，用于获取转换是否成功的信息。默认为nullptr。
		将JsonConfig对象写入具有Q_OBJECT标记的类的实例中。如果写入过程中发生错误，ok参数将被设置为false。
	*/

	/*!
		\fn template<typename T>static void JsonConfig::writeMetable(T* obj, const QString& jsonStr, bool* ok)
		\since Visindigo 0.13.0
		\a obj 为指向具有Q_OBJECT标记的类的实例的指针。
		\a jsonStr 为Json字符串。
		\a ok 为指向bool类型的指针，用于获取转换是否成功的信息。默认为nullptr。
		将Json字符串写入具有Q_OBJECT标记的类的实例中。如果写入过程中发生错误，ok参数将被设置为false。
	*/

	/*!
		\since Visindigo 0.13.0
		VIJsonConfig的默认构造函数
	*/
	JsonConfig::JsonConfig()
	{
		d = new JsonConfigPrivate();
	}

	/*!
		\since Visindigo 0.13.0
		\a variant 为QVariant对象。
		从QVariant构造一个JsonConfig对象。
	*/
	JsonConfig::JsonConfig(const QVariant& variant)
	{
		d = new JsonConfigPrivate();
		d->JsonDoc = QJsonDocument::fromVariant(variant);
	}

	/*!
		\since Visindigo 0.13.0
		\a jsonStr 为Json字符串。
		VIJsonConfig的构造函数
	*/
	JsonConfig::JsonConfig(const QString& jsonStr)
	{
		d = new JsonConfigPrivate();
		d->JsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8());
	}

	/*!
		\since Visindigo 0.13.0
		\a json 为Json对象。
		VIJsonConfig的构造函数
	*/
	JsonConfig::JsonConfig(const QJsonDocument& json)
	{
		d = new JsonConfigPrivate();
		d->JsonDoc = json;
	}
	/*!
		\since Visindigo 0.13.0
		\a other 为VIJsonConfig的另一个实例。
		VIJsonConfig的拷贝构造函数
	*/
	JsonConfig::JsonConfig(const JsonConfig& other) noexcept
	{
		d = new JsonConfigPrivate();
		d->JsonDoc = other.d->JsonDoc;
	}

	/*!
		\since Visindigo 0.13.0
		\a other 为VIJsonConfig的另一个实例。
		VIJsonConfig的移动构造函数
	*/
	JsonConfig::JsonConfig(JsonConfig&& other) noexcept
	{
		d = other.d;
		other.d = nullptr;
	}

	/*!
		\since Visindigo 0.13.0
		VIJsonConfig的析构函数
	*/
	JsonConfig::~JsonConfig()
	{
		delete d;
	}

	/*!
		\since Visindigo 0.13.0
		\a other 为VIJsonConfig的另一个实例。
		VIJsonConfig的赋值操作符
	*/
	JsonConfig& JsonConfig::operator=(const JsonConfig& other) noexcept
	{
		d->JsonDoc = other.d->JsonDoc;
		return *this;
	}

	/*!
		\since Visindigo 0.13.0
		\a other 为VIJsonConfig的另一个实例。
		VIJsonConfig的移动操作符
	*/
	JsonConfig& JsonConfig::operator=(JsonConfig&& other) noexcept
	{
		if (this->d != nullptr) { delete d; }
		d = other.d;
		other.d = nullptr;
		return *this;
	}

	/*!
		\since Visindigo 0.13.0
		\a key 为键。\a ok 为接受是否正常工作的结果指针
		一个重载操作符，旨在以类似下标访问的方式访问Json对象，返回一个VIJsonValueRef实例。
		此实例支持直接赋值，也可以使用指针操作符访问QJsonValue。此实例还重载了到QJsonValue的
		隐式转换，可以直接将其赋值给QJsonValue。
	*/
	JsonValueRef JsonConfig::operator[](const QString& key)
	{
		return JsonValueRef(this, key);
	}
	/*!
		\since Visindigo 0.13.0
		获取指定Json节点的所有键。（不包括子对象）
	*/
	QStringList JsonConfig::keys(const QString& key) const
	{
		return d->getKeys(key);
	}

	/*!
		\since Visindigo 0.13.0
		\a key 为键。\a ok 为接受是否正常工作的结果指针
		删除Json对象的键值对。
		return 如果成功删除了键值对，返回true；如果没有找到键，返回false。
	*/
	bool JsonConfig::remove(const QString& key) {
		return d->remove(key);
	}

	/*!
		\fn QJsonParseError JsonConfig::parse(const QString& jsonStr)
		\since Visindigo 0.13.0
		\a json 为Json对象。
		解析Json字符串。如果解析成功，返回的QJsonParseError的error属性为QJsonParseError::NoError。
		并立即覆盖全部原有内容。如果解析失败，返回的QJsonParseError的error属性为相应的错误类型。
	*/
	QJsonParseError JsonConfig::parse(const QString& jsonStr)
	{
		if (jsonStr.startsWith(":") || jsonStr.startsWith(".") || jsonStr.startsWith("/")) {
			vgWarningF << "May be input is file path, got " << jsonStr << ". This function only accepts Json string.";
		}
		QJsonParseError error;
		d->JsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);
		return error;
	}

	/*!
		\since Visindigo 0.13.0
		获取指定Json节点的字符串。
	*/
	QString JsonConfig::toString(QJsonDocument::JsonFormat format)
	{
		return d->JsonDoc.toJson(format);
	}

	/*!
		\since Visindigo 0.13.0
		\a key 为键。\a ok 为接受是否正常工作的结果指针
		获取指定Json节点的值。
	*/
	QJsonValue JsonConfig::getValue(const QString& key, bool* ok) const
	{
		if (ok != nullptr) {
			*ok = d->contains(key);
		}
		return d->getValue(key);
	}

	/*!
		\since Visindigo 0.13.0
		\a key 为键。\a ok 为接受是否正常工作的结果指针
		获取指定Json节点的整数值。
	*/
	qint64 JsonConfig::getInt(const QString& key, bool* ok) const
	{
		if (ok != nullptr) {
			*ok = d->contains(key);
		}
		return d->getValue(key).toInt();
	}

	/*!
		\since Visindigo 0.13.0
		\a key 为键。\a ok 为接受是否正常工作的结果指针
		获取指定Json节点的字符串值。
	*/
	QString JsonConfig::getString(const QString& key, bool* ok) const
	{
		if (ok != nullptr) {
			*ok = d->contains(key);
		}
		return d->getValue(key).toString();
	}
	/*!
		\since Visindigo 0.13.0
		\a key 为键。\a ok 为接受是否正常工作的结果指针
		获取指定Json节点的布尔值。
	*/
	bool JsonConfig::getBool(const QString& key, bool* ok) const
	{
		if (ok != nullptr) {
			*ok = d->contains(key);
		}
		return d->getValue(key).toBool();
	}

	/*!
		\since Visindigo 0.13.0
		\a key 为键。\a ok 为接受是否正常工作的结果指针
		获取指定Json节点的浮点数值。
	*/
	double JsonConfig::getDouble(const QString& key, bool* ok) const
	{
		if (ok != nullptr) {
			*ok = d->contains(key);
		}
		return d->getValue(key).toDouble();
	}

	/*!
		\since Visindigo 0.13.0
		\a key 为键。\a ok 为接受是否正常工作的结果指针
		获取指定Json节点的子对象。
	*/
	JsonConfig JsonConfig::getObject(const QString& key, bool* ok) const
	{
		if (ok != nullptr) {
			*ok = d->contains(key);
		}
		JsonConfig config;
		config.d->JsonDoc = QJsonDocument(d->getValue(key).toObject());
		return config;
	}

	/*!
		\since Visindigo 0.13.0
		\a key 为键。\a ok 为接受是否正常工作的结果指针
		获取指定Json节点的数组。
	*/
	QList<JsonConfig> JsonConfig::getArray(const QString& key, bool* ok) const
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
		\since Visindigo 0.13.0
		\a key 为键。\a ok 为接受是否正常工作的结果指针
		获取指定Json节点的字符串列表。
	*/
	QStringList JsonConfig::getStringList(const QString& key, bool* ok) const
	{
		if (ok != nullptr) {
			*ok = d->contains(key);
		}
		QStringList list;
		QJsonArray array = d->getValue(key).toArray();
		for (const QJsonValue& value : array) {
			list.append(value.toString());
		}
		return list;
	}

	/*!
		\since Visindigo 0.14.0
		\a key 为键。\a ok 为接受是否正常工作的结果指针
		获取指定Json节点的qint64列表。
	*/
	QList<qint64> JsonConfig::getIntArray(const QString& key, bool* ok) const
	{
		if (ok != nullptr) {
			*ok = d->contains(key);
		}
		QList<qint64> list;
		QJsonArray array = d->getValue(key).toArray();
		for (const QJsonValue& value : array) {
			list.append(value.toInt());
		}
		return list;
	}

	/*!
		\since Visindigo 0.14.0
		\a key 为键。\a ok 为接受是否正常工作的结果指针
		获取指定Json节点的double列表。
	*/
	QList<double> JsonConfig::getDoubleArray(const QString& key, bool* ok) const
	{
		if (ok != nullptr) {
			*ok = d->contains(key);
		}
		QList<double> list;
		QJsonArray array = d->getValue(key).toArray();
		for (const QJsonValue& value : array) {
			list.append(value.toDouble());
		}
		return list;
	}

	/*!
		\since Visindigo 0.13.0
		\a key 为键。\a ok 为接受是否正常工作的结果指针
		判断指定Json节点是否为是否包含指定的键。
	*/
	bool JsonConfig::contains(const QString& key) const
	{
		return d->contains(key);
	}

	/*!
		\since Visindigo 0.13.0
		清空Json对象的内容。
	*/
	void JsonConfig::clear()
	{
		d->JsonDoc.setObject(QJsonObject());
	}

	/*!
		\since Visindigo 0.13.0
		\a key 为键。\a ok 为接受是否正常工作的结果指针
		\a value 为值。
		设置指定Json节点为值。
	*/
	void JsonConfig::setValue(const QString& key, const QJsonValue& value)
	{
		d->setValue(key, const_cast<QJsonValue&>(value));
	}

	/*!
		\since Visindigo 0.13.0
		\a key 为键。\a ok 为接受是否正常工作的结果指针
		\a value 为整数值。
		设置指定Json节点为整数值。
	*/
	void JsonConfig::setInt(const QString& key, qint64 value)
	{
		QJsonValue v(value);
		d->setValue(key, v);
	}

	/*!
		\since Visindigo 0.13.0
		\a key 为键。\a ok 为接受是否正常工作的结果指针
		\a value 为字符串值。
		设置指定Json节点为字符串值。
	*/
	void JsonConfig::setString(const QString& key, const QString& value)
	{
		QJsonValue v(value);
		d->setValue(key, v);
	}

	/*!
		\since Visindigo 0.13.0
		\a key 为键。\a ok 为接受是否正常工作的结果指针
		\a value 为布尔值。
		设置指定Json节点为布尔值。
	*/
	void JsonConfig::setBool(const QString& key, bool value)
	{
		QJsonValue v(value);
		d->setValue(key, v);
	}

	/*!
		\since Visindigo 0.13.0
		\a key 为键。\a ok 为接受是否正常工作的结果指针
		\a value 为浮点数值。
		设置指定Json节点为浮点数值。
	*/
	void JsonConfig::setDouble(const QString& key, double value)
	{
		QJsonValue v(value);
		d->setValue(key, v);
	}

	/*!
		\since Visindigo 0.13.0
		\a key 为键。\a ok 为接受是否正常工作的结果指针
		\a value 为子对象。
		设置指定Json节点为子对象。
	*/
	void JsonConfig::setObject(const QString& key, const JsonConfig& value)
	{
		QJsonValue v(value.d->JsonDoc.object());
		d->setValue(key, v);
	}

	/*!
		\since Visindigo 0.13.0
		\a key 为键。\a ok 为接受是否正常工作的结果指针
		\a value 为数组。
		设置指定Json节点为数组。
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
		\since Visindigo 0.13.0
		\a key 为键。\a ok 为接受是否正常工作的结果指针
		\a value 为字符串列表。
		设置指定Json节点为字符串列表。
	*/
	void JsonConfig::setArray(const QString& key, const QStringList& value)
	{
		QJsonArray array;
		for (const QString& str : value) {
			array.append(QJsonValue(str));
		}
		QJsonValue v(array);
		d->setValue(key, v);
	}

	/*!
		\since Visindigo 0.13.0
		\a key 为键。\a ok 为接受是否正常工作的结果指针
		\a value 为整数列表。
		设置指定Json节点为字符串列表。
	*/
	void JsonConfig::setStringList(const QString& key, const QStringList& value)
	{
		setArray(key, value);
	}

	/*!
		\since Visindigo 0.14.0
		\a key 为键。\a ok 为接受是否正常工作的结果指针
		\a value 为整数列表。
		设置指定Json节点为整数列表。
	*/
	void JsonConfig::setIntArray(const QString& key, const QList<qint64>& value)
	{
		QJsonArray array;
		for (qint64 num : value) {
			array.append(QJsonValue(num));
		}
		QJsonValue v(array);
		d->setValue(key, v);
	}

	/*!
		\since Visindigo 0.14.0
		\a key 为键。\a ok 为接受是否正常工作的结果指针
		\a value 为浮点数列表。
		设置指定Json节点为浮点数列表。
	*/
	void JsonConfig::setDoubleArray(const QString& key, const QList<double>& value)
	{
		QJsonArray array;
		for (double num : value) {
			array.append(QJsonValue(num));
		}
		QJsonValue v(array);
		d->setValue(key, v);
	}

	/*!
		\since Visindigo 0.13.0
		\a key 为键。\a ok 为接受是否正常工作的结果指针
		判断指定Json节点是否为是否为空。
	*/
	bool JsonConfig::isEmpty(const QString& key) const
	{
		return d->getValue(key).isObject() && d->getValue(key).toObject().isEmpty();
	}

	/*!
		\since Visindigo 0.13.0
		\a key 为键。\a ok 为接受是否正常工作的结果指针
		判断指定Json节点是否为是否为空。
	*/

	bool JsonConfig::isNull(const QString& key) const
	{
		return d->getValue(key).isNull();
	}

	/*!
		\since Visindigo 0.13.0
		\a key 为键。\a ok 为接受是否正常工作的结果指针
		判断指定Json节点是否为是否为对象。
	*/
	bool JsonConfig::isObject(const QString& key) const
	{
		return d->getValue(key).isObject();
	}

	/*!
		\since Visindigo 0.13.0
		\a key 为键。\a ok 为接受是否正常工作的结果指针
		判断指定Json节点是否为是否为数组。
	*/
	bool JsonConfig::isArray(const QString& key) const
	{
		return d->getValue(key).isArray();
	}

	/*!
		\since Visindigo 0.13.0
		\a key 为键。\a ok 为接受是否正常工作的结果指针
		判断指定Json节点是否为是否为字符串。
	*/

	bool JsonConfig::isString(const QString& key) const
	{
		return d->getValue(key).isString();
	}

	/*!
		\since Visindigo 0.13.0
		\a key 为键。\a ok 为接受是否正常工作的结果指针
		判断指定Json节点是否为是否为布尔值。
	*/
	bool JsonConfig::isBool(const QString& key) const
	{
		return d->getValue(key).isBool();
	}

	/*!
		\since Visindigo 0.13.0
		\a key 为键。\a ok 为接受是否正常工作的结果指针
		判断指定Json节点是否为是否为整数。
	*/
	bool JsonConfig::isInt(const QString& key) const
	{
		return d->getValue(key).isDouble();
	}

	/*!
		\since Visindigo 0.13.0
		\a key 为键。\a ok 为接受是否正常工作的结果指针
		判断指定Json节点是否为是否为浮点数。
	*/
	bool JsonConfig::isDouble(const QString& key) const
	{
		return d->getValue(key).isDouble();
	}
}