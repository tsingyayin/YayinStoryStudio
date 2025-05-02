#pragma once
#include <QtGui>
#include "../../Utility/JsonConfig.h"
namespace YSS::Editor {
	class AStorySyntaxHighlighter : public QSyntaxHighlighter {
		Q_OBJECT;
	public:
		enum ControllerType {
			UnknownControllerType = 0,
			Speak = 1,
			BGP = 2
		};
		enum ParaType {
			UnknownParaType = 0,
			String = 1,
			Number = 2,
			Position = 3,
			Keyword = 4
		};
	private:
		QTextCharFormat Format_C_BGP;
		QTextCharFormat Format_C_Speak;
		QTextCharFormat Format_T_String;
		QTextCharFormat Format_T_Number;
		QTextCharFormat Format_T_Position;
		QTextCharFormat Format_T_Keyword;
		Utility::JsonConfig* Config = nullptr;
	public:
		AStorySyntaxHighlighter(QTextDocument* parent = nullptr);
		virtual void highlightBlock(const QString& text) override;
	private:
		void controller_BGP(const QString& text);
		void controller_Speak(const QString& text);
		void setFormatWithParaType(int start, int count, ParaType type);
	};
}