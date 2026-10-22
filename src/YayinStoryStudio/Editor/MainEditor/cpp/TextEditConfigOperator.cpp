#include <QtCore/qmetaobject.h>
#include <QtWidgets/qapplication.h>
#include <Editor/CodeDiff.h>
#include <Editor/FileServerManager.h>
#include <General/Log.h>
#include "Editor/MainEditor/TextEditConfigOperator.h"
#include "YayinStoryStudio.h"

namespace YSS::Editor {
	qint32 TextEditConfigOperator::getTabWidth() {
		bool ok = true;
		qint32 rtn = YSS::Main::getInstance()->getPluginConfig()->getInt("Settings.Editor.TextEdit.TabWidth", &ok);
		if (not ok) {
			rtn = 4;
		}
		return rtn;
	}

	void TextEditConfigOperator::setTabWidth(qint32 width, bool applyToAll) {
		YSS::Main::getInstance()->getPluginConfig()->setInt("Settings.Editor.TextEdit.TabWidth", width);
		if (applyToAll) {
			applyTabWidth();
		}
	}

	void TextEditConfigOperator::applyTabWidth() {
		qint32 width = getTabWidth();
		QList<YSSCore::Editor::FileEditWidget*> allEditors = YSSFSM->getAllFileEditWidgets();
		for (auto editor : allEditors) {
			YSSCore::Editor::TextEdit* textEdit = dynamic_cast<YSSCore::Editor::TextEdit*>(editor);
			if (textEdit) {
				textEdit->setTabWidth(width);
			}
		}
	}

	float TextEditConfigOperator::getFontScale() {
		bool ok = true;
		float rtn = YSS::Main::getInstance()->getPluginConfig()->getDouble("Settings.Editor.TextEdit.FontScale", &ok);
		if (not ok) {
			rtn = 1.0f;
		}
		return rtn;
	}

	void TextEditConfigOperator::setFontScale(float scale, bool applyToAll) {
		YSS::Main::getInstance()->getPluginConfig()->setDouble("Settings.Editor.TextEdit.FontScale", scale);
		if (applyToAll) {
			applyFontScale();
		}
	}

	void TextEditConfigOperator::applyFontScale() {
		float scale = getFontScale();
		qint32 globalFontSize = qApp->font().pointSizeF();
		qint32 targetFontSize = globalFontSize * scale;
		QList<YSSCore::Editor::FileEditWidget*> allEditors = YSSFSM->getAllFileEditWidgets();
		for (auto editor : allEditors) {
			YSSCore::Editor::TextEdit* textEdit = dynamic_cast<YSSCore::Editor::TextEdit*>(editor);
			if (textEdit) {
				textEdit->setFontSize(targetFontSize);
				continue;
			}
			YSSCore::Editor::CodeDiff* codeDiff = dynamic_cast<YSSCore::Editor::CodeDiff*>(editor);
			if (codeDiff) {
				codeDiff->setFontSize(targetFontSize);
			}
		}
	}

	YSSCore::Editor::TabCompleterItem::CompleterLevel TextEditConfigOperator::getCompleterLevel() {
		bool ok = true;
		QString level = YSS::Main::getInstance()->getPluginConfig()->getString("Settings.Editor.TextEdit.CompleterLevel", &ok);
		if (not ok) {
			level = "Some";
		}
		return (YSSCore::Editor::TabCompleterItem::CompleterLevel)
			(QMetaEnum::fromType<YSSCore::Editor::TabCompleterItem::CompleterLevel>().keyToValue(level.toUtf8().constData()));
	}

	void TextEditConfigOperator::setCompleterLevel(YSSCore::Editor::TabCompleterItem::CompleterLevel level, bool applyToAll) {
		QString levelStr = QMetaEnum::fromType<YSSCore::Editor::TabCompleterItem::CompleterLevel>().valueToKey(level);
		YSS::Main::getInstance()->getPluginConfig()->setString("Settings.Editor.TextEdit.CompleterLevel", levelStr);
		if (applyToAll) {
			applyCompleterLevel();
		}
	}

	void TextEditConfigOperator::applyCompleterLevel() {
		YSSCore::Editor::TabCompleterItem::CompleterLevel level = getCompleterLevel();
		QList<YSSCore::Editor::FileEditWidget*> allEditors = YSSFSM->getAllFileEditWidgets();
		for (auto editor : allEditors) {
			YSSCore::Editor::TextEdit* textEdit = dynamic_cast<YSSCore::Editor::TextEdit*>(editor);
			if (textEdit) {
				textEdit->setCompleterLevel(level);
			}
		}
	}

	QFont TextEditConfigOperator::getTextFont() {
		bool ok = true;
		QString fontFamily = YSS::Main::getInstance()->getPluginConfig()->getString("Settings.Editor.TextEdit.FontFamily", &ok);
		if (not ok) {
			fontFamily = "Microsoft YaHei";
		}
		return QFont(fontFamily);
	}

	void TextEditConfigOperator::setTextFont(const QFont& font, bool applyToAll) {
		YSS::Main::getInstance()->getPluginConfig()->setString("Settings.Editor.TextEdit.FontFamily", font.family());
		if (applyToAll) {
			applyTextFont();
		}
	}

	void TextEditConfigOperator::applyTextFont() {
		QFont font = getTextFont();
		QList<YSSCore::Editor::FileEditWidget*> allEditors = YSSFSM->getAllFileEditWidgets();
		for (auto editor : allEditors) {
			YSSCore::Editor::TextEdit* textEdit = dynamic_cast<YSSCore::Editor::TextEdit*>(editor);
			if (textEdit) {
				textEdit->setFont(font);
				continue;
			}
			YSSCore::Editor::CodeDiff* codeDiff = dynamic_cast<YSSCore::Editor::CodeDiff*>(editor);
			if (codeDiff) {
				codeDiff->setTextFont(font);
			}
		}
	}

	bool TextEditConfigOperator::getInlayHintVisible() {
		bool ok = true;
		bool rtn = YSS::Main::getInstance()->getPluginConfig()->getBool("Settings.Editor.TextEdit.InlayHintVisible", &ok);
		if (not ok) {
			rtn = true;
		}
		return rtn;
	}

	void TextEditConfigOperator::setInlayHintVisible(bool visible, bool applyToAll) {
		YSS::Main::getInstance()->getPluginConfig()->setBool("Settings.Editor.TextEdit.InlayHintVisible", visible);
		if (applyToAll) {
			applyInlayHintVisible();
		}
	}

	void TextEditConfigOperator::applyInlayHintVisible() {
		bool visible = getInlayHintVisible();
		QList<YSSCore::Editor::FileEditWidget*> allEditors = YSSFSM->getAllFileEditWidgets();
		for (auto editor : allEditors) {
			YSSCore::Editor::TextEdit* textEdit = dynamic_cast<YSSCore::Editor::TextEdit*>(editor);
			if (textEdit) {
				textEdit->setInlayHintVisible(visible);
			}
		}
	}

	void TextEditConfigOperator::applyAll() {
		applyTextFont();
		applyTabWidth();
		applyFontScale();
		applyCompleterLevel();
		applyInlayHintVisible();
	}

	void TextEditConfigOperator::applyTo(YSSCore::Editor::FileEditWidget* target) {
		if (not target) {
			return;
		}
		QFont textFont = getTextFont();
		float fontScale = getFontScale();
		qint32 globalFontSize = qApp->font().pointSizeF();
		qint32 targetFontSize = globalFontSize * fontScale;

		YSSCore::Editor::CodeDiff* codeDiff = dynamic_cast<YSSCore::Editor::CodeDiff*>(target);
		if (codeDiff) {
			codeDiff->setTextFont(textFont);
			codeDiff->setFontSize(targetFontSize);
			vgDebugF << "Applied code diff config to target code diff: " << target;
			return;
		}

		YSSCore::Editor::TextEdit* textEdit = dynamic_cast<YSSCore::Editor::TextEdit*>(target);
		if (not textEdit) {
			return;
		}
		textEdit->setFont(textFont);

		qint32 tabWidth = getTabWidth();
		textEdit->setTabWidth(tabWidth);

		textEdit->setFontSize(targetFontSize);

		YSSCore::Editor::TabCompleterItem::CompleterLevel completerLevel = getCompleterLevel();
		textEdit->setCompleterLevel(completerLevel);

		bool inlayHintVisible = getInlayHintVisible();
		textEdit->setInlayHintVisible(inlayHintVisible);

		vgDebugF << "Applied text edit config to target text edit: " << target;
	}


}