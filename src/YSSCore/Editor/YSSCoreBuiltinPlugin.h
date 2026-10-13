#ifndef YSSCore_Editor_YSSCoreBuiltinPlugin_h
#define YSSCore_Editor_YSSCoreBuiltinPlugin_h
#include "Editor/EditorPlugin.h"
// Main
namespace YSSCore::Editor {
	class YSSCoreAPI YSSCoreBuiltinPlugin :public EditorPlugin {
		Q_OBJECT;
	public:
		YSSCoreBuiltinPlugin();
		virtual ~YSSCoreBuiltinPlugin();
	public:
		static YSSCoreBuiltinPlugin* getInstance();
	public:
		virtual void onPluginEnable() override;
	private:
		static YSSCoreBuiltinPlugin* Instance;
	};
}
#endif // YSSCore_Editor_YSSCoreBuiltinPlugin_h
