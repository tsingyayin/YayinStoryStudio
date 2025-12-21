#include "../CodeDiff.h"
#include <QtCore/qstringlist.h>
#include "../Console.h"

namespace Visindigo::Utility {
	class CodeLinePrivate {
		friend class CodeLine;
	protected:
		QString line;
	};

	/*!
		\class Visindigo::Utility::CodeLine
		\brief 此类用于表示任何代码行。
		\since Visindigo 0.13.0
		\inmodule Visindigo

		CodeLine类用于表示代码行，继承自DiffUnit类。
	*/

	/*!
		\since Visindigo 0.13.0
		\a str 为代码行字符串。
		CodeLine的构造函数
	*/
	CodeLine::CodeLine(const QString& str) :DiffUnit() {
		d = new CodeLinePrivate();
		d->line = str;
	}

	/*!
		\since Visindigo 0.13.0
		CodeLine的析构函数
	*/
	CodeLine::~CodeLine() {
		delete d;
	}

	/*!
		\since Visindigo 0.13.0
		\a other 为DiffUnit的另一个实例。
		CodeLine的比较函数。在当前版本的实现中，只检查双方字符串是否相同。
	*/
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

	/*!
		\since Visindigo 0.13.0
		获取当前代码行的字符串。
	*/
	QString CodeLine::getLine() {
		return d->line;
	}
	class CodeDiffPrivate {
		friend class CodeDiff;
	protected:
		DiffSequence* oldCode;
		DiffSequence* newCode;
	};

	/*!
		\class Visindigo::Utility::CodeDiff
		\brief 此类用于表示代码差异。
		\since Visindigo 0.13.0
		\inmodule Visindigo

		CodeDiff类用于表示代码差异，包含旧代码和新代码的比较。
	*/
	CodeDiff::CodeDiff() {
		d = new CodeDiffPrivate();
		d->oldCode = new DiffSequence();
		d->newCode = new DiffSequence();
	}

	/*!
		\since Visindigo 0.13.0
		CodeDiff的析构函数
	*/
	CodeDiff::~CodeDiff() {
		delete d->oldCode;
		delete d->newCode;
		delete d;
	}

	/*!
		\since Visindigo 0.13.0
		\a oldCode 为旧代码行列表。
		设置旧代码行列表。
	*/
	void CodeDiff::setOldCode(QStringList oldCode) {
		d->oldCode->clear();
		for (const QString& line : oldCode) {
			CodeLine* codeLine = new CodeLine(line);
			d->oldCode->addDiffUnit(codeLine);
		}
	}

	/*!
		\since Visindigo 0.13.0
		\a newCode 为新代码行列表。
		设置新代码行列表。
	*/
	void CodeDiff::setNewCode(QStringList newCode) {
		d->newCode->clear();
		for (const QString& line : newCode) {
			CodeLine* codeLine = new CodeLine(line);
			d->newCode->addDiffUnit(codeLine);
		}
	}

	/*!
		\since Visindigo 0.13.0
		获取旧代码行列表。
	*/
	DiffSequence* CodeDiff::getOldCode() {
		return d->oldCode;
	}

	/*!
		\since Visindigo 0.13.0
		获取新代码行列表。
	*/
	DiffSequence* CodeDiff::getNewCode() {
		return d->newCode;
	}

	/*!
		\since Visindigo 0.13.0
		比较旧代码和新代码。
	*/
	void CodeDiff::compare() {
		d->oldCode->compareAsOld(d->newCode);
	}

	/*!
		\since Visindigo 0.13.0
		\a str 为字符串
		\a length 为长度。
		将字符串格式化为指定长度的字符串。（这是一个应该被隐藏的内部函数）
		return 格式化后的字符串。
	*/
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

	/*!
		\since Visindigo 0.13.0
		调试打印代码差异。

		这个函数的打印行为类似Visual Studio Code中的代码差异显示。
		\note 尽管这是个调试函数，但它真的很好用，因此不会在非调试版本中被删除。
	*/
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
			switch (d->oldCode->getDiffUnit(oIndex)->getDiffType()) {
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
						Console::print(QString("%1|%2%3|").arg(" ", 40).arg(" ", 8).arg(QString::number(nIndex), 8) +
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