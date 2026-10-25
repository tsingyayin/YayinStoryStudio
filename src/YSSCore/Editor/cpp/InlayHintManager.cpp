#include <QtCore/qmap.h>
#include <QtCore/qobject.h>
#include "Editor/InlayHintManager.h"
#include "Editor/InlayHintProvider.h"
namespace YSSCore::Editor {
	class InlayHintManagerPrivate :public QObject {
		friend class InlayHintManager;
	protected:
		QMap<QString, InlayHintProvider*> ProviderMap;
		static InlayHintManager* Instance;
	};
	InlayHintManager* InlayHintManagerPrivate::Instance = nullptr;

	/*!
		\macro YSSIHM
		\since YSS 0.18.0
		\relates YSSCore::Editor::InlayHintManager
		YSSCore::Editor::InlayHintManager::getInstance() 的简化写法。
	*/

	/*!
		\class YSSCore::Editor::InlayHintManager
		\brief InlayHintManager保存图片内联提示Provider的实例。
		\since YSS 0.18.0
		\inmodule YSSCore

		InlayHintManager是一个单例类，按 Provider 的模块ID保存 InlayHintProvider。
		它只负责保存：removeProvider 不会销毁 Provider，Provider 的生命周期仍由注册它的插件管理。

		\note Provider的模块ID会直接出现在内联提示的 Multi 中，因此必须在整个应用内唯一，
		而不能只保证在所属插件内唯一。
	*/
	InlayHintManager::InlayHintManager() {
		d = new InlayHintManagerPrivate();
	}

	InlayHintManager::~InlayHintManager() {
		delete d;
	}

	/*!
		\since YSS 0.18.0
		\return 单例实例。
	*/
	InlayHintManager* InlayHintManager::getInstance() {
		if (InlayHintManagerPrivate::Instance == nullptr) {
			InlayHintManagerPrivate::Instance = new InlayHintManager();
		}
		return InlayHintManagerPrivate::Instance;
	}

	/*!
		\since YSS 0.18.0
		\return 当前注册的全部 Provider。
	*/
	QList<InlayHintProvider*> InlayHintManager::getProviders() {
		return d->ProviderMap.values();
	}

	/*!
		\since YSS 0.18.0
		\a id Provider 的模块ID。

		\return 对应 Provider；没有注册时返回nullptr。
	*/
	InlayHintProvider* InlayHintManager::getProvider(const QString& id) {
		return d->ProviderMap.value(id, nullptr);
	}

	/*!
		\since YSS 0.18.0
		\a provider 要注册的 Provider。

		按 Provider 的模块ID注册。同一ID只保留最先注册的实例。
	*/
	void InlayHintManager::addProvider(InlayHintProvider* provider) {
		if (provider == nullptr) {
			return;
		}
		const QString id = provider->getModuleID();
		if (d->ProviderMap.contains(id)) {
			return;
		}
		d->ProviderMap.insert(id, provider);
		// Provider 由插件管理，可能在未调用 removeProvider 的情况下被销毁。
		QObject::connect(provider, &QObject::destroyed, d, [this, id, provider]() {
			if (d->ProviderMap.value(id, nullptr) == provider) {
				d->ProviderMap.remove(id);
			}
			});
	}

	/*!
		\since YSS 0.18.0
		\a provider 要移除的 Provider。

		移除 Provider，但不会销毁它。
	*/
	void InlayHintManager::removeProvider(InlayHintProvider* provider) {
		if (provider == nullptr) {
			return;
		}
		const QString id = provider->getModuleID();
		if (d->ProviderMap.value(id, nullptr) == provider) {
			d->ProviderMap.remove(id);
		}
	}
}
