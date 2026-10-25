#ifndef YSSCore_Editor_InlayHintProvider_h
#define YSSCore_Editor_InlayHintProvider_h
#include "YSSCoreCompileMacro.h"
#include <QtGui/qimage.h>
#include <General/PluginModule.h>
#include <General/YSSURI.h>
#include <Utility/JsonConfig.h>
namespace YSSCore::Editor {
	class EditorPlugin;
	class InlayHintHoverWidget;
	class InlayHintProviderPrivate;
	class YSSCoreAPI InlayHintProvider :public Visindigo::General::PluginModule {
		Q_OBJECT;
		friend class InlayHintProviderPrivate;
	public:
		InlayHintProvider(const QString& name, const QString& id, EditorPlugin* plugin);
		virtual ~InlayHintProvider();
	public:
		virtual QImage& drawInlayHint(qint32 w, qint32 h, const YSSCore::General::YSSURI& file,
			const QString& name, Visindigo::Utility::JsonConfig& para) = 0;
		virtual InlayHintHoverWidget* createHoverWidget(const YSSCore::General::YSSURI& targetFile);
	private:
		InlayHintProviderPrivate* d;
	};
}
#endif // YSSCore_Editor_InlayHintProvider_h
