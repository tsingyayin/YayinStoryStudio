#include "Editor/MainEditor/TextEditConfigOperator.h"
#include "YayinStoryStudio.h"
#include <Editor/FileServerManager.h>
#include <QtWidgets/qapplication.h>
#include <QtCore/qmetaobject.h>
#include <General/Log.h>

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
			}
		}
	}

	void TextEditConfigOperator::applyAll() {
		applyTextFont();
		applyTabWidth();
		applyFontScale();
		applyCompleterLevel();
	}

	void TextEditConfigOperator::applyTo(YSSCore::Editor::TextEdit* target) {
		if (not target) {
			return;
		}
		QFont textFont = getTextFont();
		target->setFont(textFont);

		qint32 tabWidth = getTabWidth();
		target->setTabWidth(tabWidth);

		float fontScale = getFontScale();
		qint32 globalFontSize = qApp->font().pointSizeF();
		qint32 targetFontSize = globalFontSize * fontScale;
		target->setFontSize(targetFontSize);

		YSSCore::Editor::TabCompleterItem::CompleterLevel completerLevel = getCompleterLevel();
		target->setCompleterLevel(completerLevel);

		vgDebugF << "Applied text edit config to target text edit: " << target;
	}


}