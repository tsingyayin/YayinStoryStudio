#pragma once
#include <QtCore/qstring.h>
#include <Utility/JsonConfig.h>
#include <QtGui/qsyntaxhighlighter.h>
#include "ASEParameter.h"

class ASRuleAdaptor;
class AStoryController;
class AStoryControllerParseData;

class AStorySyntaxHighlighter : public QSyntaxHighlighter {
	Q_OBJECT;
private:
	YSSCore::Utility::JsonConfig* Config = nullptr;
	ASRuleAdaptor* RuleAdaptor = nullptr;
public:
	AStorySyntaxHighlighter(QTextDocument* parent = nullptr);
	virtual void highlightBlock(const QString& text) override;
private:
	void setFormatWithParaType(int start, int length, QString type_T, QString type_B = "");
};