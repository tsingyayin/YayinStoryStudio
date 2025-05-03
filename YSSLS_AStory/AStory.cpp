#include "AStory.h"

AStorySyntaxHighlighter::AStorySyntaxHighlighter(QTextDocument* parent) : QSyntaxHighlighter(parent) {
	Format_C_BGP.setForeground(QColor("#00A2E8"));
	Format_C_Speak.setForeground(QColor("#A349A4"));
	Format_T_String.setForeground(QColor("#FF7F27"));
	Format_T_Number.setForeground(QColor("#B5E61D"));
	Format_T_Position.setForeground(QColor("#22B14C"));
	Format_T_Keyword.setForeground(QColor("#3F48CC"));
}
void AStorySyntaxHighlighter::highlightBlock(const QString& text) {
	if (text.startsWith("背景：")) {
		controller_BGP(text);
	}
	else {
		controller_Speak(text);
	}
}
void AStorySyntaxHighlighter::controller_BGP(const QString& text) {
	setFormat(0, 3, Format_C_BGP);
	QString para = text.mid(3);
	QStringList paras = para.split("/");
	for (int i = paras.length(); i < 7; i++) {
		paras.append("");
	}
	QList<ParaType> BGPParaTypes = {
		String, Number, Position, Position, Number, Keyword, Number
	};
	int start = 3;
	for (int i = 0; i < paras.length(); i++) {
		if (paras[i].length() == 0) {
			start += 1;
			continue;
		}
		int length = paras[i].length();
		setFormatWithParaType(start, length, BGPParaTypes[i]);
		start += length + 1;
	}
	setCurrentBlockState(ControllerType::BGP);
}
void AStorySyntaxHighlighter::controller_Speak(const QString& text) {
	int tabIndex = text.indexOf("	");
	setFormat(0, tabIndex, Format_C_Speak);
	setFormat(tabIndex, text.length() - tabIndex, Format_T_String);
	setCurrentBlockState(ControllerType::Speak);
}
void AStorySyntaxHighlighter::setFormatWithParaType(int start, int count, ParaType type) {
	switch (type) {
	case ParaType::String:
		setFormat(start, count, Format_T_String);
		break;
	case ParaType::Number:
		setFormat(start, count, Format_T_Number);
		break;
	case ParaType::Position:
		setFormat(start, count, Format_T_Position);
		break;
	case ParaType::Keyword:
		setFormat(start, count, Format_T_Keyword);
		break;
	default:
		break;
	}
}

AStoryLanguageServer::AStoryLanguageServer():
	YSSCore::Editor::LangServer("AStory", {"astory"})
{
	
}
void AStoryLanguageServer::serverEnable() {
	Highlighter = new AStorySyntaxHighlighter();
	setHighlighter(Highlighter);
	qDebug() << getPluginFolder();
}
void AStoryLanguageServer::serverDisable()
{
	if (Highlighter != nullptr) {
		delete Highlighter;
	}
}