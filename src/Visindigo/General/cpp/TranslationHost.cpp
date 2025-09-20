#include "../TranslationHost.h"
#include "../private/Translator_p.h"
#include "../Log.h"
namespace Visindigo::General {
	class TranslationHostPrivate {
		friend class TranslationHost;
		static TranslationHost* Instance;
		QMap<QString, Translator*> Translators;
		Translator::LangID GlobalID = Translator::LangID::zh_CN;
		void refreshLang(Translator::LangID langID) {
			for (QString id : Translators.keys()) {
				Translator* trans = Translators[id];
				trans->d->loadTranslationFile(langID);
			}
		}
	};
	TranslationHost* TranslationHostPrivate::Instance = nullptr;

	/*!
		\class Visindigo::General::TranslationHost
		\since Visindigo 0.13.0
		\ingroup Visindigo
		\brief 此类为YayinStoryStudio提供翻译管理器。

		TranslationHost负责管理所有Translator，并提供统一的翻译接口。
		
		TranslationHost可以中途切换语言，切换语言后，所有已注册的Translator都会重新加载对应语言的翻译文件。
		在Translator全部重载完毕后，TranslationHost会发出langChanged 信号，通知应用程序语言已更改。
		\sa Translator
	*/

	/*!
		\fn Visindigo::General::TranslationHost::langChanged(Translator::LangID id)
		当语言被更改时发出该信号。
	*/

	/*!
		\since Visindigo 0.13.0
		构造TranslationHost对象。
		
		TranslationHost是一个单例类，可以通过getInstance()函数获取实例。
	*/
	TranslationHost::TranslationHost():QObject() {
		d = new TranslationHostPrivate;
		TranslationHostPrivate::Instance = this;
		ySuccessF << "Success!";
		Visindigo::General::VisindigoTranslator* coreTranslator = new Visindigo::General::VisindigoTranslator();
		active(coreTranslator);
	}

	/*!
		\since Visindigo 0.13.0
		析构TranslationHost对象。一般来说，没有任何情况需要手动析构此对象。TranslationHost应该与使用它的应用程序有一致的生命周期。
	*/
	TranslationHost::~TranslationHost() {
		delete d;
	}

	/*!
		\since Visindigo 0.13.0
		获取TranslationHost单例对象的指针。
	*/
	TranslationHost* TranslationHost::getInstance() {
		return TranslationHostPrivate::Instance;
	}

	/*!
		\since Visindigo 0.13.0
		设置全局语言ID为 \a id 。
		如果当前语言ID与 \a id 不同，则会重新加载所有已注册的Translator的翻译文件，并发出langChanged 信号。
	*/
	void TranslationHost::setLangID(Translator::LangID id) {
		if (id != d->GlobalID) {
			d->refreshLang(id);
		}
		d->GlobalID = id;
	}

	/*!
		\since Visindigo 0.13.0
		注册一个Translator对象 \a translator 到TranslationHost中。
		注册后，TranslationHost会自动加载该Translator的默认语言文件和当前全局语言文件。
		如果同一个Translator对象被重复注册，则不会有任何效果。
	*/
	void TranslationHost::active(Translator* translator) {
		if (d->Translators.contains(translator->getNamespace())) {
			return;
		}
		d->Translators.insert(translator->getNamespace(), translator);
		translator->d->loadDefault();
		translator->d->loadTranslationFile(d->GlobalID);
		yNotice << "Translator:" << translator->getNamespace() << "actived.";
	}


	/*!
		\since Visindigo 0.13.0
		翻译函数。根据 \a nameSpace 和 \a key 获取对应的翻译字符串。
		如果找不到对应的Translator或翻译字符串，则返回 "nameSpace::key" 形式的字符串。
		
		\a nameSpace 翻译命名空间，对应Translator的getNamespace()返回值。
		\a key 翻译键，对应Translator中定义的翻译键。
		\return 返回翻译后的字符串，或 "nameSpace::key" 形式的字符串。
		
		\sa tr(const QString& key)
	*/
	QString TranslationHost::tra(const QString& nameSpace, const QString& key) {
		Translator* trans = d->Translators[nameSpace];
		if (trans != nullptr) [[likely]] {
			return trans->tr(key);
		}
		else {
			yErrorF << "No such namespace" << nameSpace << "for key" << key;
			return nameSpace + "::" + key;
		}
	}

	/*!
		\since Visindigo 0.13.0
		翻译函数。根据 \a key 获取对应的翻译字符串。
		\a key 应为 "nameSpace::key" 形式的字符串，其中 nameSpace 对应Translator的getNamespace()返回值，key对应Translator中定义的翻译键。
		如果找不到对应的Translator或翻译字符串，则返回 \a key 本身。
		
		\a key 翻译键，必须为 "nameSpace::key" 形式的字符串。
		\return 返回翻译后的字符串，或 \a key 本身。
		
		\sa tr(const QString& nameSpace, const QString& key)
	*/
	QString TranslationHost::tra(const QString& key) {
		QStringList keys = key.split("::");
		if (keys.length() == 2) {
			return tra(keys[0], keys[1]);
		}
		yErrorF << "Key" << key << "dose not use namespace syntax.";
		return key;
	}

	/*!
		\since Visindigo 0.13.0
		国际化翻译函数。如果 \a raw 以 "i18n:" 开头，则将其视为需要翻译的字符串，格式为 "i18n:nameSpace::key"。
		函数会提取 nameSpace 和 key，并调用 tr(nameSpace, key) 进行翻译。
		如果 \a raw 不以 "i18n:" 开头，则直接返回 \a raw 本身。
		
		\a raw 可能需要翻译的原始字符串。
		\return 返回翻译后的字符串，或 \a raw 本身。
		
		\sa tr(const QString& nameSpace, const QString& key)
	*/
	QString TranslationHost::i18n(const QString& raw) {
		if (raw.startsWith("i18n:")) {
			QStringList keys = raw.mid(5).split("::");
			if (keys.length() != 2) {
				return raw;
			}
			return tra(keys[0], keys[1]);
		}
		return raw;
	}
}