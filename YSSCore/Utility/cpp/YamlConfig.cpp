#include "../YamlConfig.h"
#include <QtCore/qstringlist.h>
#include <yaml-cpp/yaml.h>
namespace YSSCore::Utility {
	class YamlConfigPrivate {
		friend YamlConfig;
		YAML::Node Root;
	};
	YamlConfig::YamlConfig() {
		d = new YamlConfigPrivate();
	}
	YamlConfig::YamlConfig(const QString& yamlStr) {
		d = new YamlConfigPrivate();
		this->parse(yamlStr);
	}
	YamlConfig::YamlConfig(const YamlConfig& other) noexcept {
		d = new YamlConfigPrivate();
		d->Root = other.d->Root;
	}
	YamlConfig::YamlConfig(YamlConfig&& other) noexcept  {
		d = other.d;
		other.d = nullptr;
	}
	YamlConfig::~YamlConfig() {
		delete d;
	}
	YamlConfig& YamlConfig::operator=(const YamlConfig& other) noexcept {
		d->Root = other.d->Root;
		return *this;
	}
	YamlConfig& YamlConfig::operator=(YamlConfig&& other) noexcept {
		if (d != nullptr) { delete d; }
		d = other.d;
		other.d = nullptr;
		return *this;
	}
	YAML::Node YamlConfig::operator[](const QString& key) {
		QStringList keys = key.split(".");
		YAML::Node node = YAML::Clone(d->Root);
		for (QString key : keys) {
			if (node[key.toStdString()].IsDefined()) {
				node = YAML::Clone(node[key.toStdString()]);
			}
			else {
				return YAML::Node();
			}
		}
		return node;
	}
	QStringList YamlConfig::keys(const QString& key) {
		YAML::Node node = operator[](key);
		if (node.IsDefined()) {
			QStringList keys;
			for (auto it = node.begin(); it != node.end(); ++it) {
				keys.append(QString::fromStdString(it->first.as<std::string>()));
			}
			return keys;
		}
		return QStringList();
	}
	bool YamlConfig::parse(const QString& YamlStr) {
		try {
			d->Root = YAML::Load(YamlStr.toStdString());
			return true;
		}
		catch (const YAML::ParserException& e) {
			return false;
		}
	}
	QString YamlConfig::toString() {
		return QString::fromStdString(YAML::Dump(d->Root));
	}
	qint64 YamlConfig::getInt(const QString& key, bool* ok) {
		YAML::Node node = operator[](key);
		if (node.IsDefined()) {
			if (node.IsScalar()) {
				if (ok != nullptr) {
					*ok = true;
				}
				return node.as<qint64>();
			}
			else {
				if (ok != nullptr) {
					*ok = false;
				}
				return 0;
			}
		}
		if (ok != nullptr) {
			*ok = false;
		}
		return 0;
	}
	QString YamlConfig::getString(const QString& key, bool* ok) {
		YAML::Node node = operator[](key);
		if (node.IsDefined()) {
			if (node.IsScalar()) {
				if (ok != nullptr) {
					*ok = true;
				}
				return QString::fromStdString(node.as<std::string>());
			}
			else {
				if (ok != nullptr) {
					*ok = false;
				}
				return "";
			}
		}
		if (ok != nullptr) {
			*ok = false;
		}
		return "";
	}
	bool YamlConfig::getBool(const QString& key, bool* ok) {
		YAML::Node node = operator[](key);
		if (node.IsDefined()) {
			if (node.IsScalar()) {
				if (ok != nullptr) {
					*ok = true;
				}
				return node.as<bool>();
			}
			else {
				if (ok != nullptr) {
					*ok = false;
				}
				return false;
			}
		}
		if (ok != nullptr) {
			*ok = false;
		}
		return false;
	}
	double YamlConfig::getDouble(const QString& key, bool* ok) {
		YAML::Node node = operator[](key);
		if (node.IsDefined()) {
			if (node.IsScalar()) {
				if (ok != nullptr) {
					*ok = true;
				}
				return node.as<double>();
			}
			else {
				if (ok != nullptr) {
					*ok = false;
				}
				return 0.0;
			}
		}
		if (ok != nullptr) {
			*ok = false;
		}
		return 0.0;
	}
	YAML::Node YamlConfig::getObject(const QString& key, bool* ok) {
		YAML::Node node = operator[](key);
		if (node.IsDefined()) {
			if (node.IsMap()) {
				*ok = true;
			}
			else {
				*ok = false;
			}
			return node;
		}
		if (ok != nullptr) {
			*ok = false;
		}
		return YAML::Node();
	}
	QList<YAML::Node> YamlConfig::getArray(const QString& key, bool* ok) {
		YAML::Node node = operator[](key);
		if (node.IsDefined()) {
			if (node.IsSequence()) {
				if (ok != nullptr) {
					*ok = true;
				}
				QList<YAML::Node> list;
				for (auto it = node.begin(); it != node.end(); ++it) {
					list.append(*it);
				}
				return list;
			}
			else {
				if (ok != nullptr) {
					*ok = false;
				}
				return QList<YAML::Node>();
			}
		}
		if (ok != nullptr) {
			*ok = false;
		}
		return QList<YAML::Node>();
	}
	bool YamlConfig::contains(const QString& key) {
		YAML::Node node = operator[](key);
		return node.IsDefined();
	}
	void YamlConfig::setObject(const QString& key, const YAML::Node& value) {
		QStringList keys = key.split(".");
		QList<YAML::Node> nodes = { d->Root };
		for (int i = 0; i < keys.size() - 1; ++i) {
			if (nodes[i][keys[i].toStdString()].IsDefined()) {
				nodes[i][keys[i].toStdString()] = YAML::Node();
			}
			nodes.append(nodes[i][keys[i].toStdString()]);
		}
		nodes.last()[keys.last().toStdString()] = value;
	}
	void YamlConfig::setInt(const QString& key, qint64 value) {
		YAML::Node node = YAML::Node(value);
		setObject(key, node);
	}
	void YamlConfig::setString(const QString& key, const QString& value) {
		YAML::Node node = YAML::Node(value.toStdString());
		setObject(key, node);
	}
	void YamlConfig::setBool(const QString& key, bool value) {
		YAML::Node node = YAML::Node(value);
		setObject(key, node);
	}
	void YamlConfig::setDouble(const QString& key, double value) {
		YAML::Node node = YAML::Node(value);
		setObject(key, node);
	}
	void YamlConfig::setArray(const QString& key, const QList<YAML::Node>& value) {
		YAML::Node node = YAML::Node();
		for (const auto& item : value) {
			node.push_back(item);
		}
		setObject(key, node);
	}
	bool YamlConfig::isEmpty(const QString& key) {
		YAML::Node node = operator[](key);
		return node.IsDefined() && node.IsNull();
	}
	bool YamlConfig::isNull(const QString& key) {
		YAML::Node node = operator[](key);
		return node.IsDefined() && node.IsNull();
	}
	bool YamlConfig::isObject(const QString& key) {
		YAML::Node node = operator[](key);
		return node.IsDefined() && node.IsMap();
	}
	bool YamlConfig::isArray(const QString& key) {
		YAML::Node node = operator[](key);
		return node.IsDefined() && node.IsSequence();
	}
	bool YamlConfig::isString(const QString& key) {
		YAML::Node node = operator[](key);
		return node.IsDefined() && node.IsScalar();
	}
	bool YamlConfig::isBool(const QString& key) {
		YAML::Node node = operator[](key);
		return node.IsDefined() && node.IsScalar() && (node.as<std::string>() == "true" || node.as<std::string>() == "false");
	}
	bool YamlConfig::isInt(const QString& key) {
		YAML::Node node = operator[](key);
		return node.IsDefined() && node.IsScalar() && node.as<std::string>().find_first_not_of("0123456789") == std::string::npos;
	}
	bool YamlConfig::isDouble(const QString& key) {
		YAML::Node node = operator[](key);
		return node.IsDefined() && node.IsScalar() && node.as<std::string>().find_first_not_of("0123456789.") == std::string::npos;
	}
}