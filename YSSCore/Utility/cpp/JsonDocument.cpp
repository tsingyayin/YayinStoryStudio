#include "../JsonDocument.h"
#include "../JsonConfig.h"
#include "../JsonValueRef.h"
#include <QFile>

namespace YSSCore::Utility {
	class JsonDocumentPrivate
	{
		friend class JsonDocument;
		QString ConfigPath;
		QString DefaultConfigPath;
		JsonConfig Config;
		JsonConfig DefaultConfig;
	};

	/*!
		\class YSSCore::Utility::JsonDocument
		\inmodule YSSCore
		\since YSSCore 0.10.0
		\brief 此类在VIJsonConfig的基础上增加了一种允许存在默认值的配置文件的处理方式。
		\sa JsonConfig

		VIJsonDocument是VIJsonConfig的功能扩展，从本质上讲，VIJsonDocument内包含了两个VIJsonConfig对象，
		一个是默认配置，一个是“当前”配置。两个配置对应的实际Json文档都需要手动指定，在读取配置时，如果当前配置中
		没有某个配置项的值，那么会尝试从默认配置中读取，如果默认配置中也没有，那么返回一个空值。

		这也意味着，在大规模读写中，VIJsonDocument的性能会比VIJsonConfig差一些，因此在不需要默认配置的情况下，
		建议使用VIJsonConfig。对于Visindigo本身而言，VIJsonDocument只用在VIPackage中，用于提供一种
		在“用户破坏了配置文件”时的内部恢复机制。

		VIJsonDocument的配置文件路径是在构造函数中指定的，也可以在load()时现场指定，如果两者都指定了，那么load()中
		指定的路径会覆盖构造函数中的路径。请注意，调用load()函数时，除非加载失败，否则会立即覆盖当前配置。

		VIJsonDocument禁止拷贝和移动，因为设计上不希望（也建议用户不要）令一组配置文件被多个对象读写，这样会导致不可预知的结果。
	*/
	/*!
		\brief 构造一个空的VIJsonDocument对象。
		\since Visindigo 2.0.0
	*/
	JsonDocument::JsonDocument() {
		d = new JsonDocumentPrivate();
	}

	/*!
		\brief 构造一个VIJsonDocument对象。
		\a configPath 配置文件路径。
		\a defaultConfigPath 默认配置文件路径。
		\since Visindigo 2.0.0
	*/
	JsonDocument::JsonDocument(const QString& configPath, const QString& defaultConfigPath) {
		d = new JsonDocumentPrivate();
		d->ConfigPath = configPath;
		d->DefaultConfigPath = defaultConfigPath;
	}

	/*!
		\brief 析构VIJsonDocument对象。
		\since Visindigo 2.0.0
	*/
	JsonDocument::~JsonDocument() {
		delete d;
	}

	/*!
		\brief 从配置文件中加载配置。
		\a configPath 配置文件路径。
		\a defaultConfigPath 默认配置文件路径。
		\a whichError 指示是谁加载失败，1表示默认配置，2表示当前配置，0表示成功。
		\a configAutoCreate 是否自动创建配置文件，默认为true。当默认配置文件读入成功但当前配置文件读入失败时，
		会自动在指定位置创建当前配置文件。
		\since Visindigo 2.0.0

		此函数会从指定的配置文件中加载配置，如果加载失败，会返回一个QJsonParseError对象，whichError会指示是哪个配置文件加载失败。
		这里的加载问题主要指语法问题，如果是文件读写时发生问题，则返回值会是QJsonParseError::NoError，但whichError会指示是
		哪个文件读写失败，而非被置为0。
	*/
	QJsonParseError JsonDocument::load(const QString& configPath, const QString& defaultConfigPath, uchar* whichError
		, bool configAutoCreate) {
		if (!configPath.isEmpty()) d->ConfigPath = configPath;
		if (!defaultConfigPath.isEmpty()) d->DefaultConfigPath = defaultConfigPath;
		QFile file(d->ConfigPath);
		QFile defaultFile(d->DefaultConfigPath);
		QJsonParseError error = QJsonParseError();
		error.error = QJsonParseError::NoError;
		QTextStream stream(&file);
		QTextStream defaultStream(&defaultFile);
		stream.setEncoding(QStringConverter::Utf8);
		defaultStream.setEncoding(QStringConverter::Utf8);
		if (defaultFile.open(QIODevice::ReadOnly)) {
			error = d->DefaultConfig.parse(defaultStream.readAll());
			defaultFile.close();
			if (error.error != QJsonParseError::NoError) {
				*whichError = 1;
				return error;
			}
		}
		else {
			error.error = QJsonParseError::NoError;
			*whichError = 1;
			return error;
		}
		if (!file.exists()) {
			if (configAutoCreate) {
				if (!file.open(QIODevice::WriteOnly)) {
					error.error = QJsonParseError::NoError;
					*whichError = 2;
					return error;
				}
				file.write(d->DefaultConfig.toString().toUtf8());
				d->Config = d->DefaultConfig;
				file.close();
			}
			else {
				error.error = QJsonParseError::NoError;
				*whichError = 2;
				return error;
			}
		}
		else {
			if (file.open(QIODevice::ReadOnly)) {
				error = d->Config.parse(stream.readAll());
				file.close();
				if (error.error != QJsonParseError::NoError) {
					*whichError = 2;
					return error;
				}
			}
			else {
				error.error = QJsonParseError::NoError;
				*whichError = 2;
				return error;
			}
		}

		*whichError = 0;
		return error;
	}

	/*!
		\brief 保存配置到文件。
		\since Visindigo 2.0.0

		此函数会将当前配置保存到文件，如果文件不存在，会自动创建。考虑到读取时已经有强制文件存在的逻辑，
		因此自动创建的场景较少，但仍不排除用户故意在读取后删除文件的情况。

		此函数不保存默认配置文件中出现的改动，因为理论上用户无法从VIJsonDocument中修改默认配置。
		如果用户通过某种手段修改了默认配置，则不在本函数考虑范围内。
	*/
	void JsonDocument::save() {
		QFile file(d->ConfigPath);
		if (file.open(QIODevice::WriteOnly)) {
			QTextStream stream(&file);
			stream.setEncoding(QStringConverter::Utf8);
			stream << d->Config.toString();
			file.close();
		}
	}

	/*!
		\brief 获取配置文件中的所有键。
		\a key 键的路径。
		\since Visindigo 2.0.0

		此函数会返回指定路径下的所有键，如果路径为空，则返回根键。
	*/
	QStringList JsonDocument::keys(const QString& key) {
		return d->Config.keys(key);
	}

	/*!
		\brief 获取指定键的值。
		\a key 键的路径。
		\since Visindigo 2.0.0

		此函数会返回指定键的值，如果现有配置中键不存在，则尝试从默认配置中读取，如果默认配置中也不存在，则返回一个空值。
	*/
	JsonValueRef JsonDocument::operator[](const QString& key) {
		return JsonValueRef(&d->Config, key);
	}

	/*!
		\brief 获取指定键的值。
		\a key 键的路径。
		\since Visindigo 2.0.0

		此函数会返回指定键的值，如果现有配置中键不存在，则尝试从默认配置中读取，如果默认配置中也不存在，则返回一个空值。
	*/
	QJsonValue JsonDocument::getValue(const QString& key) {
		bool ok = false;
		QJsonValue value = d->Config.getValue(key, &ok);
		if (!ok) value = d->DefaultConfig.getValue(key);
		return value;
	}

	/*!
		\brief 获取指定键的整数值。
		\a key 键的路径。
		\since Visindigo 2.0.0

		此函数会返回指定键的整数值，如果现有配置中键不存在，则尝试从默认配置中读取，如果默认配置中也不存在，则返回0。
	*/
	qint64 JsonDocument::getInt(const QString& key) {
		return getValue(key).toInt();
	}

	/*!
		\brief 获取指定键的字符串值。
		\a key 键的路径。
		\since Visindigo 2.0.0

		此函数会返回指定键的字符串值，如果现有配置中键不存在，则尝试从默认配置中读取，如果默认配置中也不存在，则返回一个空字符串。
	*/
	QString JsonDocument::getString(const QString& key) {
		return getValue(key).toString();
	}

	/*!
		\brief 获取指定键的布尔值。
		\a key 键的路径。
		\since Visindigo 2.0.0

		此函数会返回指定键的布尔值，如果现有配置中键不存在，则尝试从默认配置中读取，如果默认配置中也不存在，则返回false。
	*/
	bool JsonDocument::getBool(const QString& key) {
		return getValue(key).toBool();
	}

	/*!
		\brief 获取指定键的浮点数值。
		\a key 键的路径。
		\since Visindigo 2.0.0

		此函数会返回指定键的浮点数值，如果现有配置中键不存在，则尝试从默认配置中读取，如果默认配置中也不存在，则返回0.0。
	*/
	double JsonDocument::getDouble(const QString& key) {
		return getValue(key).toDouble();
	}

	/*!
		\brief 获取指定键的对象。
		\a key 键的路径。
		\since Visindigo 2.0.0

		此函数会返回指定键的对象，如果现有配置中键不存在，则尝试从默认配置中读取，如果默认配置中也不存在，则返回一个空对象。
	*/
	JsonConfig JsonDocument::getObject(const QString& key) {
		bool ok = false;
		JsonConfig config = d->Config.getObject(key, &ok);
		if (!ok) config = d->DefaultConfig.getObject(key);
		return config;
	}

	/*!
		\brief 获取指定键的数组。
		\a key 键的路径。
		\since Visindigo 2.0.0

		此函数会返回指定键的数组，如果现有配置中键不存在，则尝试从默认配置中读取，如果默认配置中也不存在，则返回一个空数组。
	*/
	QList<JsonConfig> JsonDocument::getArray(const QString& key) {
		bool ok = false;
		QList<JsonConfig> list = d->Config.getArray(key, &ok);
		if (!ok) list = d->DefaultConfig.getArray(key);
		return list;
	}

	/*!
		\brief 判断指定键是否存在。
		\a key 键的路径。
		\since Visindigo 2.0.0

		此函数会判断指定键是否存在，如果现有配置中键不存在，则尝试从默认配置中读取，如果默认配置中也不存在，则返回false。
	*/
	bool JsonDocument::contains(const QString& key) {
		return d->Config.contains(key) || d->DefaultConfig.contains(key);
	}

	/*!
		\brief 设置指定键的值。
		\a key 键的路径。
		\a value 值。
		\since Visindigo 2.0.0

		此函数会设置指定键的值，如果键不存在，则会自动创建。
	*/
	void JsonDocument::setValue(const QString& key, const QJsonValue& value) {
		d->Config.setValue(key, value);
	}

	/*!
		\brief 设置指定键的整数值。
		\a key 键的路径。
		\a value 值。
		\since Visindigo 2.0.0

		此函数会设置指定键的整数值，如果键不存在，则会自动创建。
	*/
	void JsonDocument::setInt(const QString& key, qint64 value) {
		setValue(key, QJsonValue(value));
	}

	/*!
		\brief 设置指定键的字符串值。
		\a key 键的路径。
		\a value 值。
		\since Visindigo 2.0.0

		此函数会设置指定键的字符串值，如果键不存在，则会自动创建。
	*/
	void JsonDocument::setString(const QString& key, const QString& value) {
		setValue(key, QJsonValue(value));
	}

	/*!
		\brief 设置指定键的布尔值。
		\a key 键的路径。
		\a value 值。
		\since Visindigo 2.0.0

		此函数会设置指定键的布尔值，如果键不存在，则会自动创建。
	*/
	void JsonDocument::setBool(const QString& key, bool value) {
		setValue(key, QJsonValue(value));
	}

	/*!
		\brief 设置指定键的浮点数值。
		\a key 键的路径。
		\a value 值。
		\since Visindigo 2.0.0

		此函数会设置指定键的浮点数值，如果键不存在，则会自动创建。
	*/
	void JsonDocument::setDouble(const QString& key, double value) {
		setValue(key, QJsonValue(value));
	}

	/*!
		\brief 设置指定键的对象。
		\a key 键的路径。
		\a value 值。
		\since Visindigo 2.0.0

		此函数会设置指定键的对象，如果键不存在，则会自动创建。
	*/
	void JsonDocument::setObject(const QString& key, const JsonConfig& value) {
		d->Config.setObject(key, value);
	}

	/*!
		\brief 设置指定键的数组。
		\a key 键的路径。
		\a value 值。
		\since Visindigo 2.0.0

		此函数会设置指定键的数组，如果键不存在，则会自动创建。
	*/
	void JsonDocument::setArray(const QString& key, const QList<JsonConfig>& value) {
		d->Config.setArray(key, value);
	}

	/*!
		\brief 判断指定键的值是否为空。
		\a key 键的路径。
		\since Visindigo 2.0.0

		此函数会判断指定键的值是否为空，如果现有配置中键不存在，则尝试从默认配置中读取，如果默认配置中也不存在，则返回true。
	*/
	bool JsonDocument::isEmpty(const QString& key) {
		return getValue(key).isNull();
	}

	/*!
		\brief 判断指定键的值是否为null。
		\a key 键的路径。
		\since Visindigo 2.0.0

		此函数会判断指定键的值是否为null，如果现有配置中键不存在，则尝试从默认配置中读取，如果默认配置中也不存在，则返回true。
	*/
	bool JsonDocument::isNull(const QString& key) {
		return getValue(key).isNull();
	}

	/*!
		\brief 判断指定键的值是否为对象。
		\a key 键的路径。
		\since Visindigo 2.0.0

		此函数会判断指定键的值是否为对象，如果现有配置中键不存在，则尝试从默认配置中读取，如果默认配置中也不存在，则返回false。
	*/
	bool JsonDocument::isObject(const QString& key) {
		return getValue(key).isObject();
	}

	/*!
		\brief 判断指定键的值是否为数组。
		\a key 键的路径。
		\since Visindigo 2.0.0

		此函数会判断指定键的值是否为数组，如果现有配置中键不存在，则尝试从默认配置中读取，如果默认配置中也不存在，则返回false。
	*/
	bool JsonDocument::isArray(const QString& key) {
		return getValue(key).isArray();
	}

	/*!
		\brief 判断指定键的值是否为字符串。
		\a key 键的路径。
		\since Visindigo 2.0.0

		此函数会判断指定键的值是否为字符串，如果现有配置中键不存在，则尝试从默认配置中读取，如果默认配置中也不存在，则返回false。
	*/

	bool JsonDocument::isString(const QString& key) {
		return getValue(key).isString();
	}

	/*!
		\brief 判断指定键的值是否为布尔值。
		\a key 键的路径。
		\since Visindigo 2.0.0

		此函数会判断指定键的值是否为布尔值，如果现有配置中键不存在，则尝试从默认配置中读取，如果默认配置中也不存在，则返回false。
	*/
	bool JsonDocument::isBool(const QString& key) {
		return getValue(key).isBool();
	}

	/*!
		\brief 判断指定键的值是否为整数。
		\a key 键的路径。
		\since Visindigo 2.0.0

		此函数会判断指定键的值是否为整数，如果现有配置中键不存在，则尝试从默认配置中读取，如果默认配置中也不存在，则返回false。
	*/
	bool JsonDocument::isInt(const QString& key) {
		return getValue(key).isDouble();
	}

	/*!
		\brief 判断指定键的值是否为浮点数。
		\a key 键的路径。
		\since Visindigo 2.0.0

		此函数会判断指定键的值是否为浮点数，如果现有配置中键不存在，则尝试从默认配置中读取，如果默认配置中也不存在，则返回false。
	*/
	bool JsonDocument::isDouble(const QString& key) {
		return getValue(key).isDouble();
	}
}