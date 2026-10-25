#ifndef YSSCore_Editor_InlayHintManager_h
#define YSSCore_Editor_InlayHintManager_h
#include "YSSCoreCompileMacro.h"
#include <QtCore/qlist.h>
#include <QtCore/qstring.h>
namespace YSSCore::Editor {
	class InlayHintProvider;
	class InlayHintManagerPrivate;
	class YSSCoreAPI InlayHintManager {
	private:
		InlayHintManager();
	public:
		~InlayHintManager();
		static InlayHintManager* getInstance();
		QList<InlayHintProvider*> getProviders();
		InlayHintProvider* getProvider(const QString& id);
		void addProvider(InlayHintProvider* provider);
		void removeProvider(InlayHintProvider* provider);
	private:
		InlayHintManagerPrivate* d;
	};
}

#define YSSIHM YSSCore::Editor::InlayHintManager::getInstance()
#endif // YSSCore_Editor_InlayHintManager_h
