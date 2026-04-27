#include "Editor/private/TextEdit_p.h"
#include "Editor/TextEdit.h"
#include <QtWidgets/qcheckbox.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/qlineedit.h>
#include <General/TranslationHost.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qmessagebox.h>
namespace YSSCore::__Private__ {
	TextEditFindAndReplace::TextEditFindAndReplace(YSSCore::Editor::TextEdit* parent) : Visindigo::Widgets::BorderFrame(parent) {
		this->parent = parent;
		layout = new QGridLayout(this);
		titleLabel = new QLabel(VITR("YSS::editor.far.title"), this);
		closeButton = new QPushButton(VITR("Visindigo::general.close"), this);
		sourceAsReCheckBox = new QCheckBox(VITR("YSS::editor.far.useRegExp"), this);
		caseSensitiveCheckBox = new QCheckBox(VITR("YSS::editor.far.matchCase"), this);
		wholeWordCheckBox = new QCheckBox(VITR("YSS::editor.far.wholeWord"), this);

		rawInputLabel = new QLabel(VITR("YSS::editor.far.find"), this);
		rawInput = new QLineEdit(this);
		replaceInputLabel = new QLabel(VITR("YSS::editor.far.replace"), this);
		replaceInput = new QLineEdit(this);

		matchCountLabel = new QLabel(this);
		findNextButton = new QPushButton(VITR("YSS::editor.far.findNext"), this);
		replaceNextButton = new QPushButton(VITR("YSS::editor.far.replaceNext"), this);
		replaceAllButton = new QPushButton(VITR("YSS::editor.far.replaceAll"), this);
		
		layout->addWidget(titleLabel, 0, 0, 1, 3);
		layout->addWidget(closeButton, 0, 3);
		layout->addWidget(sourceAsReCheckBox, 1, 1);
		layout->addWidget(caseSensitiveCheckBox, 1, 2);
		layout->addWidget(wholeWordCheckBox, 1, 3);
		layout->addWidget(rawInputLabel, 2, 0);
		layout->addWidget(rawInput, 2, 1, 1, 3);
		layout->addWidget(replaceInputLabel, 3, 0);
		layout->addWidget(replaceInput, 3, 1, 1, 3);
		layout->addWidget(matchCountLabel, 4, 0);
		layout->addWidget(findNextButton, 4, 1);
		layout->addWidget(replaceNextButton, 4, 2);
		layout->addWidget(replaceAllButton, 4, 3);

		connect(closeButton, &QPushButton::clicked, this, [this]() {
			this->parent->clearFindAllSelection();
			this->hide();
			});

		connect(caseSensitiveCheckBox, &QCheckBox::checkStateChanged, this, [this]() {
			this->findAll();
			});

		connect(wholeWordCheckBox, &QCheckBox::checkStateChanged, this, [this]() {
			this->findAll();
			});

		connect(sourceAsReCheckBox, &QCheckBox::checkStateChanged, this, [this]() {
			this->findAll();
			});
		
		connect(rawInput, &QLineEdit::returnPressed, this, [this]() {
			this->findAll();
			});
		connect(findNextButton, &QPushButton::clicked, this, [this]() {
			auto rtn = this->parent->findNext(rawInput->text(), sourceAsReCheckBox->isChecked(), -1, 
				(caseSensitiveCheckBox->isChecked() ? QTextDocument::FindCaseSensitively : QTextDocument::FindFlags()) | 
				(wholeWordCheckBox->isChecked() ? QTextDocument::FindWholeWords : QTextDocument::FindFlags()), true);
			if (rtn.isNull()) {
				QMessageBox::information(this, VITR("YSS::editor.far.notFound"), VITR("YSS::editor.far.notFoundDesc").arg(rawInput->text()));
			}
			});

		connect(replaceNextButton, &QPushButton::clicked, this, [this]() {
			bool rtn = this->parent->replaceNext(rawInput->text(), replaceInput->text(), sourceAsReCheckBox->isChecked(), -1,
				(caseSensitiveCheckBox->isChecked() ? QTextDocument::FindCaseSensitively : QTextDocument::FindFlags()) |
				(wholeWordCheckBox->isChecked() ? QTextDocument::FindWholeWords : QTextDocument::FindFlags()), true);
			if (not rtn) {
				QMessageBox::information(this, VITR("YSS::editor.far.notFound"), VITR("YSS::editor.far.notFoundDesc").arg(rawInput->text()));
			}
			});

		connect(replaceAllButton, &QPushButton::clicked, this, [this]() {
			qint32 rtn = this->parent->replaceAll(rawInput->text(), replaceInput->text(), sourceAsReCheckBox->isChecked(),
				(caseSensitiveCheckBox->isChecked() ? QTextDocument::FindCaseSensitively : QTextDocument::FindFlags()) |
				(wholeWordCheckBox->isChecked() ? QTextDocument::FindWholeWords : QTextDocument::FindFlags()));
			QMessageBox::information(this, VITR("YSS::editor.far.replaced"), VITR("YSS::editor.far.replacedDesc").arg(rtn));
			});
	}

	TextEditFindAndReplace::~TextEditFindAndReplace() {
	}

	void TextEditFindAndReplace::findAll() {
		auto cursors = this->parent->findAll(rawInput->text(), sourceAsReCheckBox->isChecked(),
			(caseSensitiveCheckBox->isChecked() ? QTextDocument::FindCaseSensitively : QTextDocument::FindFlags()) |
			(wholeWordCheckBox->isChecked() ? QTextDocument::FindWholeWords : QTextDocument::FindFlags()), true);
		matchCountLabel->setText(VITR("YSS::editor.far.matchCount").arg(cursors.size()));
	}

	void TextEditFindAndReplace::setFindText(const QString& text) {
		rawInput->setText(text);
		findAll();
	}

	TextEditCursorInfo::TextEditCursorInfo(YSSCore::Editor::TextEdit* parent) : Visindigo::Widgets::BorderFrame(parent) {
		InfoLabel = new QLabel(this);
		TotalLinesLabel = new QLabel(this);
		Layout = new QHBoxLayout(this);
		Layout->setContentsMargins(10, 0, 10, 0);
		Layout->setSpacing(10);
		this->setLayout(Layout);
		Layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
		Layout->addWidget(InfoLabel);
		Layout->addWidget(TotalLinesLabel);
	}

	TextEditCursorInfo::~TextEditCursorInfo() {
	}

	void TextEditCursorInfo::setCursorInfo(qint32 line, qint32 column, qint32 selection) {
		if (selection > 0) {
			InfoLabel->setText(VITR("YSS::editor.textEdit.cursorInfo_s").arg(line).arg(column).arg(selection));
		}
		else {
			InfoLabel->setText(VITR("YSS::editor.textEdit.cursorInfo").arg(line).arg(column));
		}
	}

	void TextEditCursorInfo::setTotalLines(qint32 total) {
		TotalLinesLabel->setText(VITR("YSS::editor.textEdit.totalLines").arg(total));
	}

}