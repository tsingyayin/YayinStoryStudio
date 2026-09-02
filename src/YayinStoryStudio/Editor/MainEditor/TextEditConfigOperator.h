#ifndef YayinStoryStudio_Editor_MainEditor_TextEditConfigOperator_h
#define YayinStoryStudio_Editor_MainEditor_TextEditConfigOperator_h
#include <Utility/JsonConfig.h>
#include <Editor/TextEdit.h>
#include <Editor/TabCompleterProvider.h>
namespace YSS::Editor {
	class TextEditConfigOperator {
	public:
		TextEditConfigOperator() = delete;

		static void applyAll();
		static void applyTo(YSSCore::Editor::TextEdit* target);

		static qint32 getTabWidth();
		static void setTabWidth(qint32 width, bool applyToAll = false);
		static void applyTabWidth();

		static float getFontScale();
		static void setFontScale(float scale, bool applyToAll = false);
		static void applyFontScale();

		static YSSCore::Editor::TabCompleterItem::CompleterLevel getCompleterLevel();
		static void setCompleterLevel(YSSCore::Editor::TabCompleterItem::CompleterLevel level, bool applyToAll = false);
		static void applyCompleterLevel();

		static QFont getTextFont();
		static void setTextFont(const QFont& font, bool applyToAll = false);
		static void applyTextFont();
	};
}
#endif // YayinStoryStudio_Editor_MainEditor_TextEditConfigOperator_h
