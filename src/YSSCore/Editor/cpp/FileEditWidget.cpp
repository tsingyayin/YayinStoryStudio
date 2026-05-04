#include <QtGui/qevent.h>
#include <QtCore/qfileinfo.h>
#include "../FileEditWidget.h"
#include <General/Log.h>
#include "General/YSSLogger.h"
#include <Utility/FileUtility.h>
#include <QtCore/qregularexpression.h>
namespace YSSCore::Editor {
	VImplClass(FileEditWidget) {
		VIAPI(FileEditWidget);
	protected:
		bool fileChanged = false;
		bool isVirtualFile = false;
		QString filePath;
	};
	/*!
		\class YSSCore::Editor::FileEditWidget
		\brief 此类为Yayin Story Studio 提供文件编辑的基类。
		\since Visindigo 0.13.0
		\inmodule YSSCore

		FileEditWidget是所有文件编辑器的基类，所有文件编辑器都需要继承此类。

		\note 这类在0.15.0因引入虚拟文件概念而变更了虚函数顺序和声明，因此ABI与此前不同。

		\section1 文件编辑器的职责
		文件编辑器的职责是提供一个界面来编辑文件内容，并提供一些基本的文件操作功能，
		如保存、撤销、重做等。具体来说，文件编辑器需要实现以下功能：
		\list
		\li 1. 打开文件：能够打开一个指定路径的文件，并将其内容显示在编辑器中。
		\li 2. 编辑文件：能够修改文件内容，并能够检测到文件内容的修改状态。
		\li 3. 保存文件：能够将修改后的文件内容保存到指定路径的文件中。
		\li 4. 其他操作：能够提供一些常用的编辑操作，如复制、剪切、粘贴、全选等。
		\endlist
		上述未提及的功能，如重载、光标定位等都是可选功能。重载默认按照打开文件的方式重新加载文件内容，光标定位默认不实现。

		\section1 文件编辑器的使用接口和实现接口
		文件编辑器提供了一套使用接口和实现接口。使用接口是指外部代码调用该
		文件编辑器时使用的接口，比如openFile、saveFile等；
		实现接口是指文件编辑器的子类需要重载的接口，如onOpen、onSave等。

		目前，所有实现接口都以on开头，且设为protected，理应不被外部调用，
		实际上也不应被继承类调用，除非你明确要复用基类的实现逻辑。

		\section1 虚拟文件
		YSSCore提供了一套被称之为虚拟文件的概念。有关该概念的详细阐述，
		以及虚拟文件的虚拟文件路径的格式，请参考YSSCore::Editor::FileServer的相关文档。
		这里我们只说明一下FileEditWidget在虚拟文件模式下的一些特殊行为。

		当openFile函数被调用时，如果传入的路径符合虚拟文件路径的格式（即以@开头，且包含!和?），
		则FileEditWidget会自动将其识别为虚拟文件，并将isVirtualFile属性设为true。
		此时，文件路径、文件名、扩展名等相关信息会按照虚拟文件路径的格式进行解析和返回。

		在虚拟文件模式下，有关文件读写操作的四函数从onOpen、onClose、onSave、onReload
		变更为onVirtualOpen、onVirtualClose、onVirtualSave、onVirtualReload，且默认实现为返回false。
		但对应的外部使用接口不变，仍然为openFile、close、saveFile、reloadFile等。
		
		也就是说，外部调用者无需关心当前文件是否为虚拟文件，仍然按照正常的方式调用这些接口即可，
		FileEditWidget会根据当前文件的类型自动调用相应的实现接口。不过值得一提的是，
		在虚拟文件模式下，saveFile传入的两个参数均会被忽略。

		\section1 文件关闭的行为
		在四大读写操作函数中，只有closeFile()是void类型，这意味着无法通过返回值来判断文件是否成功关闭了。
		因为文件的关闭可能并非是从手动调用触发的，也可能是从用户点击关闭按钮触发的，而且也可能因
		用户的要求中途取消了关闭操作。因此FileEditWidget通过重载closeEvent函数来统一处理所有
		来源的关闭，且不可能从closeFile知道是否成功关闭。要获得文件的关闭状态，应该监听fileClosed信号。

		此外，根据目前FileServerManager的实现，fileClosed信号发出之后，在下一个事件循环该
		FileEditWidget对象就会被销毁，因此任何连接到fileClosed信号的槽函数都只应做一些立即操作，
		并且不得缓存该对象的指针以备后用，因为该对象可能已经被销毁了。
	*/

	/*!
		\fn YSSCore::Editor::FileEditWidget::fileChanged(const QString& filePath)
		\since YSS 0.13.0
		当文件内容被修改时，应触发此信号。
	*/

	/*!
		\fn YSSCore::Editor::FileEditWidget::fileChangeCanceled(const QString& filePath)
		\since YSS 0.14.0
		当文件内容修改被取消时，应触发此信号。
	*/

	/*!
		\fn YSSCore::Editor::FileEditWidget::fileSaved(const QString& filePath)
		\since YSS 0.13.0
		当文件被成功保存时，自动触发此信号。
	*/

	/*!
		\fn YSSCore::Editor::FileEditWidget::fileRenamed(const QString& rawPath, const QString& changedPath)
		\since YSS 0.15.0
		当文件被成功保存，且路径与此前不一致时，自动触发此信号。
	*/

	/*!
		\fn YSSCore::Editor::FileEditWidget::closed(const QString& filePath)
		\since YSS 0.15.0
		当文件最终被关闭时，自动触发此信号。
	*/

	/*!
		\since Visindigo 0.13.0
		\a parent 为父对象。
		构造FileEditWidget对象。
	*/
	FileEditWidget::FileEditWidget(QWidget* parent) : QFrame(parent) {
		d = new FileEditWidgetPrivate;
	}

	/*!
		\since Visindigo 0.13.0
		析构FileEditWidget对象。
	*/
	FileEditWidget::~FileEditWidget() {
		delete d;
	}

	/*!
		\since Visindigo 0.13.0
		获取当前文件编辑器的文件路径。
		return 返回当前文件编辑器的文件路径。如果当前没有打开任何文件，则返回空字符串。

		注意，这可能是个虚拟文件路径，如果当前文件是通过FileServer打开的虚拟文件，
		则返回的路径格式为@ext!fileName?param，其中ext是文件扩展名，fileName是文件名，param是额外参数。
		对于虚拟文件路径，getFileName()函数会正确解析并返回文件名部分。
	*/
	QString FileEditWidget::getFilePath() const {
		return d->filePath;
	}

	/*!
		\since Visindigo 0.13.0
		获取当前文件编辑器的文件名。
		return 返回当前文件编辑器的文件名。如果当前没有打开任何文件，则返回空字符串。
	*/
	QString FileEditWidget::getFileName() const {
		if (d->filePath.isEmpty()) {
			static auto re = QRegularExpression(R"(^@([^!]+)!([^?]+)\?(.*)$)");
			auto match = re.match(d->filePath);
			if (match.hasMatch()) {
				return match.captured(2);
			}
		}
		return QFileInfo(d->filePath).fileName();
	}

	/*!
		\since Visindigo 0.13.0
		判断当前文件内容是否被修改。
		return 如果文件内容被修改，返回true；否则返回false。
	*/
	bool FileEditWidget::isFileChanged() const {
		return d->fileChanged;
	}

	/*!
		\since Visindigo 0.15.0
		判断当前文件是否为虚拟文件。
		return 如果当前文件为虚拟文件，返回true；否则返回false。

		虚拟文件是指不对应实际磁盘文件的文件，通常由程序动态生成或从其他来源加载。对于虚拟文件，某些操作可能会有所不同，例如保存时可能需要特殊处理。
	*/
	bool FileEditWidget::isVirtualFile() const {
		return d->isVirtualFile;
	}

	/*!
		\since Visindigo 0.15.0
		获取当前文件的扩展名。
		return 返回当前文件的扩展名（不带点）。如果当前没有打开任何文件，则返回空字符串。

		对于虚拟文件路径，扩展名是路径中@和!之间的部分。
	*/
	QString FileEditWidget::getFileExt() const {
		if (d->isVirtualFile){
			static auto re = QRegularExpression(R"(^@([^!]+)!([^?]+)\?(.*)$)");
			auto match = re.match(d->filePath);
			if (match.hasMatch()) {
				return match.captured(1);
			}
		}
		return QFileInfo(d->filePath).suffix();
	}

	/*!
		\since Visindigo 0.15.0
		获取当前虚拟文件的参数部分。
		return 返回当前虚拟文件的参数部分。如果当前文件不是虚拟文件或没有参数，则返回空字符串。

		对于虚拟文件路径，参数部分是路径中?之后的部分。
	*/
	QString FileEditWidget::getVirtualFileParam() const {
		if (d->isVirtualFile) {
			static auto re = QRegularExpression(R"(^@([^!]+)!([^?]+)\?(.*)$)");
			auto match = re.match(d->filePath);
			if (match.hasMatch()) {
				return match.captured(3);
			}
		}
		return QString();
	}

	/*!
		\since Visindigo 0.13.0
		将当前文件内容标记为已修改状态。
		此函数通常在派生类中被调用，当用户修改了文件内容时，应调用此函数以更新文件状态。
		之后，它会自动触发fileChanged信号，通知外部文件内容已被修改，并传递当前文件路径作为参数。
	*/
	void FileEditWidget::setFileChanged() {
		d->fileChanged = true;
		emit fileChanged(d->filePath);
	}

	/*!
		\since Visindigo 0.13.0
		将当前文件内容标记为未修改状态。
		此函数通常在派生类中被调用，当文件内容被保存或重新加载后，应调用此函数以更新文件状态。
		之后，它会自动触发fileChangeCanceled信号，通知外部文件内容修改已被取消，并传递当前文件路径作为参数。
	*/
	void FileEditWidget::cancelFileChanged() {
		d->fileChanged = false;
		emit fileChangeCanceled(d->filePath);
	}

	/*!
		\since Visindigo 0.13.0
		打开指定路径的文件，并加载其内容。
		\a path 要打开的文件路径。
		return 如果文件成功打开并加载，返回true；否则返回false。

		此类在判定文件路径是否为空后，调用派生类实现的onOpen()函数以实际打开文件。

		从0.15.0开始，这个函数会事先判定文件路径是否满足虚拟文件路径，如果是的话，则调用
		onVirtualOpen()函数打开。有关虚拟文件路径的概念，请参考FileServer类的说明。
	*/
	bool FileEditWidget::openFile(const QString& path) {
		static auto re = QRegularExpression(R"(^@([^!]+)!([^?]+)\?(.*)$)");
		auto match = re.match(path);
		if (match.hasMatch()) {
			d->isVirtualFile = true;
			QString ext = match.captured(1);
			QString fileName = match.captured(2);
			QString param = match.captured(3);
			d->filePath = path;
			return onVirtualOpen(ext, fileName, param);
		}
		if (path.isEmpty()) {
			yWarningF << "File path is empty.";
			return false;
		}
		d->filePath = path;
		return onOpen(path);
	}

	/*!
		\since YSS 0.15.0
		保存当前文件编辑器的内容到指定路径。
		\a path 要保存的文件路径。如果为空字符串，则使用当前文件路径。非空字符串在保存成功后会更新当前文件路径。
		\a deleteWhenSaveAs 当执行另存为操作时，是否删除原文件。默认为false，即不删除原文件。如果设置为true，
		可以视作重命名文件，保存成功后原文件将被删除。请谨慎使用此选项，以免误删重要文件。
		return 如果文件成功保存，返回true；否则返回false。
		此类在判定文件路径是否为空后，调用派生类实现的onSave()函数以实际保存文件。
		一旦文件成功保存，fileChanged状态会被重置为false，并触发fileSaved信号。

		\note 这个函数最初从0.13.0引入，在0.15.0中增加了deleteWhenSaveAs参数以支持在另存为操作时删除原文件的功能。
		此外，从0.15.0开始，这个函数会在调用虚函数onSave之后，额外检查文件是否确实存在，以确保保存操作的成功性。
		这是因为某些派生类的onSave实现可能会在保存失败时没有正确返回false，因此增加了文件存在性的检查以提高函数的可靠性。

		在虚拟文件模式下，它会直接调用onVirtualSave()函数进行保存。虚拟保存时，\a path 和 \a deleteWhenSaveAs 均无效。
	*/
	bool FileEditWidget::saveFile(const QString& path, bool deleteWhenSaveAs) {
		if (d->isVirtualFile) {
			return onVirtualSave();
		}
		QString pathCopy = path;
		if (path.isEmpty()) {
			if (d->filePath.isEmpty()) {
				return false;
			}
			pathCopy = d->filePath;
		}
		bool ok = onSave(pathCopy);
		if (not Visindigo::Utility::FileUtility::isFileExist(pathCopy)) {
			return false;
		}
		if (ok) {
			cancelFileChanged();
			if (not path.isEmpty() && path != d->filePath) {
				if (deleteWhenSaveAs) {
					QFile::remove(d->filePath);
				}
				QString oldPath = d->filePath;
				d->filePath = pathCopy;
				emit fileSaved(d->filePath);
				emit fileRenamed(oldPath, pathCopy);
			}
			else {
				d->filePath = pathCopy;
				emit fileSaved(d->filePath);
			}
		}
		return ok;
	}

	/*!
		\since Visindigo 0.13.0
		重新加载当前文件编辑器的内容。
		return 如果文件成功重新加载，返回true；否则返回false。
		此类在判定当前文件路径是否为空后，调用派生类实现的onReload()函数以实际重新加载文件。
	*/
	bool FileEditWidget::reloadFile() {
		if (d->filePath.isEmpty()) {
			yWarningF << "File path is empty.";
			return false;
		}
		if (d->isVirtualFile) {
			return onVirtualReload();
		}
		return onReload();
	}

	/*!
		\since Visindigo 0.13.0
		关闭当前文件编辑器。
		
		这个函数直接调用QWidget的close()函数，触发关闭事件。
		
		关闭事件会调用派生类实现的onClose()函数以决定是否允许关闭。
	*/
	void FileEditWidget::closeFile() {
		close();
	}

	/*!
		\since Visindigo 0.13.0
		复制当前选中的内容到剪贴板。
		return 如果复制操作成功，返回true；否则返回false。
		此类调用派生类实现的onCopy()函数以实际处理复制逻辑。
	*/
	bool FileEditWidget::copy() {
		return onCopy();
	}

	/*!
		\since Visindigo 0.13.0
		剪切当前选中的内容到剪贴板。
		return 如果剪切操作成功，返回true；否则返回false。
		此类调用派生类实现的onCut()函数以实际处理剪切逻辑。
	*/
	bool FileEditWidget::cut() {
		return onCut();
	}

	/*!
		\since Visindigo 0.13.0
		从剪贴板粘贴内容到当前光标位置。
		return 如果粘贴操作成功，返回true；否则返回false。
		此类调用派生类实现的onPaste()函数以实际处理粘贴逻辑。
	*/
	bool FileEditWidget::paste() {
		return onPaste();
	}

	/*!
		\since Visindigo 0.13.0
		撤销上一次编辑操作。
		return 如果撤销操作成功，返回true；否则返回false。
		此类调用派生类实现的onUndo()函数以实际处理撤销逻辑。
	*/
	bool FileEditWidget::undo() {
		return onUndo();
	}

	/*!
		\since Visindigo 0.13.0
		重做上一次被撤销的编辑操作。
		return 如果重做操作成功，返回true；否则返回false。
		此类调用派生类实现的onRedo()函数以实际处理重做逻辑。
	*/
	bool FileEditWidget::redo() {
		return onRedo();
	}

	/*!
		\since Visindigo 0.13.0
		选中当前文件编辑器中的所有内容。
		return 如果全选操作成功，返回true；否则返回false。
		此类调用派生类实现的onSelectAll()函数以实际处理全选逻辑。
	*/
	bool FileEditWidget::selectAll() {
		return onSelectAll();
	}

	/*!
		\since YSSCore 0.13.0
		\a lineNumber 行号，从1开始。
		\a column 列号，从1开始。
		将光标移动到指定的行号和列号位置。
	*/
	bool FileEditWidget::cursorToPosition(qint32 lineNumber, qint32 column) {
		return onCursorToPosition(lineNumber, column);
	}


	/*!
		\since YSS 0.13.0
		派生类必须实现此纯虚函数以处理文件打开逻辑。
		\a path 要打开的文件路径。

		return 如果文件成功打开并加载，返回true；否则返回false。

		\warning 从0.15开始，这个函数可能因为onReload而重复调用，不保证
		在整个生命周期内只调用一次。因此如果你在这个函数内尝试懒加载你的
		FileEditWidget，应当首先检查资源是否已经初始化，避免出现内存泄漏。
	*/
	bool FileEditWidget::onOpen(const QString& path) {
		return false;
	}

	/*!
		\since YSS 0.13.0

		派生类必须实现此纯虚函数以处理文件关闭逻辑。
		return 如果文件编辑器可以关闭，返回true；否则返回false。

		\warning 不要在这个函数里调用close或deleteLater等任何实质上真的
		会关闭这个Widget的函数，会造成递归爆栈。这个函数的唯一作用是通过
		返回值告知YSS是否真的应该关闭它。因此你可以在里面做一些保存、警告等操作
	*/
	bool FileEditWidget::onClose() {
		return true;
	}

	/*!
		\since YSS 0.13.0

		派生类必须实现此纯虚函数以处理文件保存逻辑。
		\a path 要保存的文件路径。如果为空字符串，则使用当前文件路径。非空字符串在保存成功后会更新当前文件路径。
		return 如果文件成功保存，返回true；否则返回false。
	*/
	bool FileEditWidget::onSave(const QString& path) {
		return false;
	}

	/*!
		\since YSS 0.13.0
		派生类实现此纯虚函数以处理文件重新加载逻辑。
		return 如果文件成功重新加载，返回true；否则返回false。

		在0.15之前的版本，这个函数默认返回false
		从0.15开始，这个函数默认使用现在的文件路径重新调用onOpen()
	*/
	bool FileEditWidget::onReload() {
		if (d->filePath.isEmpty()) {
			return false;
		}
		return onOpen(d->filePath);
	}

	/*!
		\since YSS 0.13.0
		派生类实现此纯虚函数以处理复制逻辑。
		return 如果复制操作成功，返回true；否则返回false。
	*/
	bool FileEditWidget::onCopy() {
		return false;
	}
	/*!
		\since YSS 0.13.0
		派生类实现此纯虚函数以处理剪切逻辑。
		return 如果剪切操作成功，返回true；否则返回false。
	*/
	bool FileEditWidget::onCut() {
		return false;
	}

	/*!
		\since YSS 0.13.0
		派生类实现此纯虚函数以处理粘贴逻辑。
		return 如果粘贴操作成功，返回true；否则返回false。
	*/
	bool FileEditWidget::onPaste() {
		return false;
	}

	/*!
		\since YSS 0.13.0
		派生类实现此纯虚函数以处理撤销逻辑。
		return 如果撤销操作成功，返回true；否则返回false。
	*/
	bool FileEditWidget::onUndo() {
		return false;
	}

	/*!
		\since YSS 0.13.0
		派生类实现此纯虚函数以处理重做逻辑。
		return 如果重做操作成功，返回true；否则返回false。
	*/
	bool FileEditWidget::onRedo() {
		return false;
	}

	/*!
		\since YSS 0.13.0
		派生类实现此纯虚函数以处理全选逻辑。
		return 如果全选操作成功，返回true；否则返回false。
	*/
	bool FileEditWidget::onSelectAll() {
		return false;
	}

	/*!
		\since YSSCore 0.13.0
		\a lineNumber 行号，从1开始。
		\a column 列号，从1开始。
		由派生类实现的函数，用于将光标移动到指定的行号和列号位置。
		考虑到FileEditWidget并不总一定是代码编辑器，因此可按需实现它。
	*/
	bool FileEditWidget::onCursorToPosition(qint32 lineNumber, qint32 column) {
		return false;
	}

	/*!
		\since YSSCore 0.15.0
		\a ext 虚拟文件的扩展名。
		\a fileName 虚拟文件的文件名。
		\a param 其他参数字符串，可以包含任意内容，由调用者和实现者自行约定格式和含义。

		处理虚拟文件的打开逻辑。

		默认返回false。
	*/
	bool FileEditWidget::onVirtualOpen(const QString& ext, const QString& fileName, const QString& param) {
		return false;
	}

	/*!
		\since YSSCore 0.15.0
		处理虚拟文件的关闭逻辑。
		return 如果虚拟文件编辑器可以关闭，返回true；否则返回false。

		默认返回true。
	*/
	bool FileEditWidget::onVirtualClose() {
		return true;
	}

	/*!
		\since YSSCore 0.15.0
		处理虚拟文件的保存逻辑。
		return 如果虚拟文件成功保存，返回true；否则返回false。

		默认实现为返回false，表示虚拟文件不支持保存操作。派生类可以根据需要重载此函数以提供虚拟文件的保存功能。
	*/
	bool FileEditWidget::onVirtualSave() {
		return false;
	}

	/*!
		\since YSSCore 0.15.0
		处理虚拟文件的重新加载逻辑。
		return 如果虚拟文件成功重新加载，返回true；否则返回false。

		这个实现默认重新调用onVirtualOpen。
	*/
	bool FileEditWidget::onVirtualReload() {
		return onVirtualOpen(getFileExt(), getFileName(), getVirtualFileParam());
	}

	

	/*!
		\since YSSCore 0.13.0
		处理文件编辑器关闭事件。
		\a event 关闭事件对象。
		此函数在文件编辑器尝试关闭时被调用，调用派生类实现的onClose()函数以决定是否允许关闭。
		如果onClose()返回true，则接受关闭事件；否则忽略关闭事件，文件编辑器保持打开状态。
		\warning 派生类若重载了closeEvent()，应确保调用基类的实现以保持正确的关闭行为。
	*/
	void FileEditWidget::closeEvent(QCloseEvent* event) {
		if (d->isVirtualFile) {
			if (onVirtualClose()) {
				event->accept();
				emit fileClosed(d->filePath);
			}
			else {
				event->ignore();
			}
			return;
		}
		if (onClose()) {
			event->accept();
			emit fileClosed(d->filePath);
		}
		else {
			event->ignore();
		}
	}
}