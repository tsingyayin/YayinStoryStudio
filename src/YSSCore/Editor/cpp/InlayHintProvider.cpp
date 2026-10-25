#include "Editor/EditorPlugin.h"
#include "Editor/InlayHintHoverWidget.h"
#include "Editor/InlayHintProvider.h"
namespace YSSCore::Editor {
	class InlayHintProviderPrivate {
		friend class InlayHintProvider;
	};

	/*!
		\class YSSCore::Editor::InlayHintProvider
		\brief InlayHintProvider为图片内联提示提供绘制与悬停交互。
		\since YSS 0.18.0
		\inmodule YSSCore

		InlayHintProvider是一个插件模块，通过 InlayHintManager 按模块ID注册。
		当内联提示的 Multi 为 \c{@providerId!name?param} 且 providerId 与已注册的
		Provider模块ID相同时，该提示由对应 Provider 绘制。

		Provider只负责绘制内联区域的内容，不持有广泛的上下文：drawInlayHint 只会收到
		可用的绘制尺寸、当前文件URI、Multi 中的 name 以及解析后的JSON参数。
		需要更多交互时，由 createHoverWidget 返回一个 InlayHintHoverWidget，
		由 TextEdit 在合适的时机调用其 onHoverRequest()。

		\note Provider的模块ID会直接出现在 Multi 中，因此它必须在整个应用内唯一，
		而不能只保证在所属插件内唯一。
	*/
	InlayHintProvider::InlayHintProvider(const QString& name, const QString& id, EditorPlugin* plugin) :
		Visindigo::General::PluginModule((Visindigo::General::Plugin*)plugin, id, YSSPluginModule_InlayHintProvider, name) {
		d = new InlayHintProviderPrivate();
	}

	InlayHintProvider::~InlayHintProvider() {
		delete d;
	}

	/*!
		\since YSS 0.18.0
		\a w 内联区域宽度（像素）。
		\a h 内联区域高度（像素）。
		\a file 当前文本编辑器的文件URI。
		\a name Multi 中 \c{!} 与 \c{?} 之间的部分。
		\a para Multi 中 \c{?} 后面的JSON参数。

		\return 要绘制到内联区域的图片。返回的引用必须在本次调用期间保持有效；
		调用方会在绘制前立即使用它。

		\note 该函数会在每次重绘提示时调用，Provider应自行缓存耗时的绘制结果。
	*/
	QImage& InlayHintProvider::drawInlayHint(qint32 w, qint32 h, const YSSCore::General::YSSURI& file,
		const QString& name, Visindigo::Utility::JsonConfig& para) {
		Q_UNUSED(w);
		Q_UNUSED(h);
		Q_UNUSED(file);
		Q_UNUSED(name);
		Q_UNUSED(para);
		static QImage Empty;
		return Empty;
	}

	/*!
		\since YSS 0.18.0
		\a targetFile 当前文本编辑器的文件URI。

		\return 新建的悬停浮窗。返回nullptr表示该 Provider 不需要悬停交互。
		返回的实例所有权归调用它的 TextEdit，Provider 不应再管理它。
	*/
	InlayHintHoverWidget* InlayHintProvider::createHoverWidget(const YSSCore::General::YSSURI& targetFile) {
		Q_UNUSED(targetFile);
		return nullptr;
	}
}
