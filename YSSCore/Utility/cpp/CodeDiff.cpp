#include "../CodeDiff.h"
#include <QtCore/qstringlist.h>
#include "../Console.h"
#include <QtCore/qdebug.h>
namespace YSSCore::Utility {
	class CodeLinePrivate {
		friend class CodeLine;
	protected:
		QString line;
	};

	CodeLine::CodeLine(const QString& str):DiffUnit() {
		d = new CodeLinePrivate();
		d->line = str;
	}

	CodeLine::~CodeLine() {
		delete d;
	}

	DiffUnit::CompareType CodeLine::onCompare(const DiffUnit* other) {
		//not consider modified, maybe add later
		if (other == nullptr) {
			return DiffUnit::CompareType::NotEqual;
		}
		const CodeLine* otherLine = dynamic_cast<const CodeLine*>(other);
		if (otherLine == nullptr) {
			return DiffUnit::CompareType::NotEqual;
		}
		if (d->line == otherLine->d->line) {
			return DiffUnit::CompareType::Equal;
		}
		else {
			return DiffUnit::CompareType::NotEqual;
		}
	}

	QString CodeLine::getLine() {
		return d->line;
	}
	class CodeDiffPrivate {
		friend class CodeDiff;
	protected:
		DiffSequence* oldCode;
		DiffSequence* newCode;
	};
	CodeDiff::CodeDiff() {
		d = new CodeDiffPrivate();
		d->oldCode = new DiffSequence();
		d->newCode = new DiffSequence();
	}
	CodeDiff::~CodeDiff() {
		delete d->oldCode;
		delete d->newCode;
		delete d;
	}
	void CodeDiff::setOldCode(QStringList oldCode) {
		d->oldCode->clear();
		for (const QString& line : oldCode) {
			CodeLine* codeLine = new CodeLine(line);
			d->oldCode->addDiffUnit(codeLine);
		}
	}
	void CodeDiff::setNewCode(QStringList newCode) {
		d->newCode->clear();
		for (const QString& line : newCode) {
			CodeLine* codeLine = new CodeLine(line);
			d->newCode->addDiffUnit(codeLine);
		}
	}
	DiffSequence* CodeDiff::getOldCode() {
		return d->oldCode;
	}
	DiffSequence* CodeDiff::getNewCode() {
		return d->newCode;
	}
	void CodeDiff::compare() {
		d->oldCode->compareAsOld(d->newCode);
	}
	static QString fixedLength(QString str, int length = 40) {
		str = str.replace("\t", "    ");
		if (str.length() > length) {
			str = str.left(length - 3) + "...";
		}
		else {
			str = str.leftJustified(length, ' ');
		}
		return str;
	}
	void CodeDiff::debugPrint() {
		int oIndex = 0;
		int nIndex = 0;
		int oLength = d->oldCode->size();
		int nLength = d->newCode->size();
		Console::print("CodeDiff Debug Print");
		Console::print(QString("%1|%2|%3").arg("Old Code", 40).arg("Line", 16).arg("New Code", 40));
		Console::print(QString("%1").arg(" ", 92, '='));
		bool printAdd = false;
		while (oIndex < oLength && nIndex < nLength) {
			switch(d->oldCode->getDiffUnit(oIndex)->getDiffType()) {
			case DiffUnit::DiffType::Removed:
				Console::print(Console::inErrorStyle(fixedLength(dynamic_cast<CodeLine*>(d->oldCode->getDiffUnit(oIndex))->getLine()))
					+ QString("|%1%2|%3").arg(QString::number(oIndex), 8).arg(" ", 8).arg(" ", 40));
				printAdd = true;
				break;
			case DiffUnit::DiffType::None:
			case DiffUnit::DiffType::Modified:
				if (d->newCode->getDiffUnit(nIndex)->getDiffType() == DiffUnit::DiffType::Added) {
					printAdd = true;
				}
				if (printAdd) {
					while (d->newCode->getDiffUnit(nIndex)->getDiffType() == DiffUnit::DiffType::Added) {
						Console::print(QString("%1|%2%3|").arg(" ", 40).arg(" ", 8).arg(QString::number(nIndex), 8)+
							Console::inSuccessStyle(fixedLength(dynamic_cast<CodeLine*>(d->newCode->getDiffUnit(nIndex))->getLine()))
						);
						nIndex++;
					}
					printAdd = false;
				}
				Console::print(
					QString("%1|%2%3|%4").arg(fixedLength(dynamic_cast<CodeLine*>(d->oldCode->getDiffUnit(oIndex))->getLine()))
					.arg(QString::number(oIndex), 8).arg(QString::number(nIndex), 8)
					.arg(fixedLength(dynamic_cast<CodeLine*>(d->newCode->getDiffUnit(nIndex))->getLine())));
				nIndex++;
				
				break;
			}
			oIndex++;
		}
		while (nIndex < nLength) {
			Console::print(QString("%1|%2%3|").arg(" ", 40).arg(" ", 8).arg(QString::number(nIndex), 8) +
				Console::inSuccessStyle(fixedLength(dynamic_cast<CodeLine*>(d->newCode->getDiffUnit(nIndex))->getLine()))
			);
			nIndex++;
		}
	}
	
}