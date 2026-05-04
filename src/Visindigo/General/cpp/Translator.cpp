#include <QtCore/qstring.h>
#include <QtCore/qfile.h>
#include <QtCore/qtextstream.h>
#include "../private/Translator_p.h"
#include "../../Utility/JsonDocument.h"
#include "../../General/Log.h"

namespace Visindigo::__Private__ {

	bool TranslatorPrivate::loadDefault() {
		return loadTranslationFile(DefaultID, true);
	}
	bool TranslatorPrivate::loadTranslationFile(Visindigo::General::Translator::LangID id, bool asDefault) {
		//vgDebug << FilePath.size();
		QString filePath = FilePath[id];
		QFile file(filePath);
		if (!file.exists()) {
			return false;
		}
		if (not file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			return false;
		}
		QTextStream ts(&file);
		ts.setEncoding(QStringConverter::Utf8);
		QString jsonConfig = ts.readAll();
		file.close();
		if (asDefault) {
			if (DefaultLang != nullptr) {
				delete DefaultLang;
			}
			DefaultLang = new Visindigo::Utility::JsonConfig();
			DefaultLang->parse(jsonConfig);
		}
		else {
			if (Lang != nullptr) {
				delete Lang;
			}
			Lang = new Visindigo::Utility::JsonConfig();
			Lang->parse(jsonConfig);
		}
		return true;
	}
	TranslatorPrivate::~TranslatorPrivate() {
		if (DefaultLang != nullptr) {
			delete DefaultLang;
		}
		if (Lang != nullptr) {
			delete Lang;
		}
	}
}
namespace Visindigo::General
{

	/*!
		\class Visindigo::General::Translator
		\inheaderfile General/Translator.h
		\since Visindigo 0.13.0
		\inmodule Visindigo
		\brief Visindigo的翻译功能类，用于替代Qt自带的翻译功能。

		Translator是Visindigo利用JSON文件实现的翻译功能类，
		它不一定比Qt自带的翻译功能具有更好的性能，但它的易用性更好。

		与Qt自带的翻译功能相比，Translator的最大特点在于其使用键名，
		而非初始语言的原文本作为翻译的索引，这意味着在代码中，
		可以极大程度地避免对文本的修改对翻译功能的影响，从而减少了代码的维护成本。

		除此之外，由于Visindigo的模块化设计，为避免不同模块间的翻译键名冲突，
		Translator使用命名空间来区分不同模块的翻译内容。

		Visindigo的键名格式如下：
		\code
			<namespace>::<key>.<subkey>...
		\endcode
		其中<namespace>为命名空间，命名空间之后则为Json节点的索引路径，例如：
		\badcode
			{
				"Part1": {
					"SubPart1": "This is SubPart1 of Part1",
					"SubPart2": "This is SubPart2 of Part1"
				},
				"Part2": "This is Part2"
			}
		\endcode
		则可以有如下的键名：
		\code
			<namespace>::Part1.SubPart1
			<namespace>::Part1.SubPart2
			<namespace>::Part2
		\endcode

		此外，在Visindigo提供的部分从配置文件生成窗口的功能中
		（例如Visindigo::Widgets::ConfigWidget、Visindigo::Widgets::QuickMenu等）
		以及明确了支持VI18N格式的参数中，如果其文本内容应该以"i18n:"开头，
		且后接键名，例如：
		\code
			i18n:<namespace>::Part1.SubPart1
		\endcode
		那么该文本就可以被自动识别并进行翻译。若不加"i18n:"前缀，则Visindigo
		会将其视为普通文本，不进行翻译。
	*/

	/*!
		\enum Visindigo::General::Translator::LangID
		\since Visindigo 0.13.0
		\value Unknown 未知语言
		\value zh_CN 简体中文
		\value zh_SC 简体中文（同zh_CN）
		\value zh_TC 繁体中文
		\value en 英文
		\value ja 日文
		\value ko 韩文
		\value ru 俄文
		\value de 德文
		\value fr 法文
		请注意，这里并没有使用ISO 639-1或ISO 639-2等国际标准的语言代码，而是使用了一套自定义的语言ID。
		这是因为目前Visindigo的翻译功能还需要同步驱动Qt Linguist的翻译功能（比如自动加载对应的qm文件），
		而Qt的qm文件未遵守ISO的要求，因此目前该枚举只能暂时与Qt的qm文件名保持一致。

		这预计会在未来的某个版本中进行调整，调整之后LangID直接遵循ISO要求，然后再提供ISO
		到qm文件名的转换函数，以保持与Qt的兼容性。
	*/
	/*!
		\since Visindigo 0.14.0
		将语言ID转换为字符串。返回值为语言ID对应的字符串，例如zh_CN、en等。
	*/
	QString Translator::langIDToString(LangID id) {
		return QMetaEnum::fromType<LangID>().valueToKey(static_cast<int>(id));
	}

	/*!
		\since Visindigo 0.14.0
		将字符串转换为语言ID。参数str为要转换的字符串，例如zh_CN、en等。
		返回值为对应的语言ID，如果转换失败，则返回LangID::Unknown。
	*/
	Translator::LangID Translator::stringToLangID(const QString& str) {
		bool ok;
		int value = QMetaEnum::fromType<LangID>().keyToValue(str.toStdString().c_str(), &ok);
		if (ok) {
			return static_cast<LangID>(value);
		}
		else {
			return LangID::Unknown;
		}
	}
	/*!
		\since Visindigo 0.13.0
		构造Translator对象，并设置命名空间。
	*/
	Translator::Translator(Plugin* parent, const QString& nameSpace) 
		: PluginModule(parent, "translator", VIModuleType_Translator, nameSpace)
	{
		d = new Visindigo::__Private__::TranslatorPrivate;
		setNamespace(nameSpace);
	}

	/*!
		\since Visindigo 0.13.0
		析构Translator对象。
	*/
	Translator::~Translator() {
		delete d;
	}

	/*!
		\since Visindigo 0.13.0
		设置命名空间。
	*/
	void Translator::setNamespace(const QString& nameSpace) {
		d->ID = nameSpace;
	}

	/*!
		\since Visindigo 0.13.0
		获取当前命名空间。
	*/
	QString Translator::getNamespace() {
		return d->ID;
	}

	/*!
		\since Visindigo 0.13.0
		设置默认语言。默认语言用于在当前语言缺少某个键名的翻译时进行回退。
	*/
	void Translator::setDefaultLang(LangID lang) {
		d->DefaultID = lang;
	}

	/*!
		\since Visindigo 0.13.0
		设置语言文件路径。参数langFilePath为一个QMap，其键为语言ID，值为对应的语言文件路径。

		语言文件不会在设置后立即加载，只有当Translator被挂载到TranslationHost后，才会按需加载对应的语言文件。

		\warning 不推荐使用这个函数。由于未知原因，此函数在某些情况下会创建
		病态的QMap对象，从而导致程序崩溃。建议使用addLangFilePath()函数逐个添加语言文件路径。

		\sa TranslationHost
	*/
	void Translator::setLangFilePath(const QMap<LangID, QString>& langFilePath) {
		d->FilePath = langFilePath;
	}

	/*!
		\since Visindigo 0.13.0
		添加语言文件路径。参数id为语言ID，path为对应的语言文件路径。
	*/
	void Translator::addLangFilePath(LangID id, QString path) {
		d->FilePath.insert(id, path);
	}

	/*!
		\since Visindigo 0.13.0
		翻译键名为\a key 的文本，并返回翻译结果。如果当前语言和默认语言均缺少该键名的翻译，则返回原键名。
	*/
	QString Translator::tr(const QString& key) {
		if (d->Lang == nullptr && d->DefaultLang == nullptr) [[unlikely]] {
			return key;
		}
		if (d->Lang != nullptr) [[likely]] {
			QString tr = d->Lang->getString(key);
			if (tr.isEmpty()) [[unlikely]] {
				if (d->DefaultLang != nullptr) {
					tr = d->DefaultLang->getString(key);
					if (tr.isEmpty()) {
						return key;
					}
					else {
						return tr;
					}
				}
				else {
					return key;
				}
			}
			else [[likely]] {
				return tr;
			}
		}
		else {
			if (d->DefaultLang != nullptr) {
				QString tr = d->DefaultLang->getString(key);
				if (tr.isEmpty()) {
					return key;
				}
				else {
					return tr;
				}
			}
			else {
				return key;
			}
		}
	}

	
}