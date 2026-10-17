#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include <QtGui/qevent.h>
#include <QtWidgets/qmessagebox.h>
#include <General/Log.h>
#include <General/TranslationHost.h>
#include <General/YSSLogger.h>
#include "Editor/FileEditWidget.h"
#include "Editor/FileServerManager.h"
#include "Editor/VirtualFilePath.h"
namespace YSSCore::Editor {
	VImplClass(FileEditWidget) {
		VIAPI(FileEditWidget);
protected:
	bool fileChanged = false;
	bool autoAbandon = false;
	bool saveSilent = false;
	YSSCore::General::YSSURI Uri;
	};

	/*!
		\class YSSCore::Editor::FileEditWidget
		\brief 此类为Yayin Story Studio 提供文件编辑的基础类.
		\since YSS 0.13.0
		\inmodule YSSCore

		FileEditWidget是所有文件编辑器的基类，所有文件编辑器都需要继承此类。

		从0.18.0开始，文件身份由 \l{YSSCore::General::YSSURI} 表示，而不是路径字符串。
		URI 自带 scheme 与 authority，因此"本机文件"和"远端文件"在这里没有分支；本机文件
		只是 \c file scheme 的 URI。内容也不再由编辑器自己读取：\l{FileServerManager}
		通过 \l{YSSCore::General::YSSUniqueFileSystem} 读好二进制后，连同 URI 一起交给
		\l{onOpen()}。编辑器因此完全不关心后端是本机还是 SSH。

		\note 这类在0.15.0因引入虚拟文件概念而变更了虚函数顺序和声明，0.18.0又因引入 URI
		身份再次变更，因此ABI与此前不兼容。

		\section1 文件编辑器的职责
		\list
		\li 1. 打开文件：把外部给出的 URI 与内容显示在编辑器里。
		\li 2. 编辑文件：修改内容并能检测到修改状态。
		\li 3. 保存文件：把内容交给外部写出，自己只负责产出内容。
		\li 4. 其他操作：复制、剪切、粘贴、全选等。
		\endlist

		\section1 虚拟文件
		虚拟文件用 \l{YSSCore::General::YSSURI::Scheme::VirtualFile} 表示，其字符串形式
		仍是 \c{@ext!name?param}。虚拟文件没有二进制内容，因此 \l{onOpen()} 不会被调用，
		而是走 \l{onVirtualOpen()}。

		外部调用者无需区分两者：依然调用 \l{openFile()}，本类按 URI 的 scheme 分发。
		\section1 文件关闭的行为
		只有 \l{closeFile()} 是void类型，无法通过返回值判断是否真的关闭。关闭统一在
		\l{closeEvent()} 里处理，状态通过 \l{fileClosed()} 信号通知。
		\section1 文件备份的行为
		\l{createFileBackup()} 调用 \l{onBackup()}，返回当前文件的完整内容用于备份。
	*/

	/*!
		\since YSS 0.13.0
		构造文件编辑器。
		\a parent 父对象。
	*/
	FileEditWidget::FileEditWidget(QWidget* parent) :QFrame(parent) {
		d = new FileEditWidgetPrivate;
		d->q = this;
	}

	/*!
		\since YSS 0.13.0
		析构函数。
	*/
	FileEditWidget::~FileEditWidget() {
		delete d;
	}

	/*!
		\since YSS 0.18.0
		取当前文件的身份。
		\return 当前文件的 URI；没有打开文件时为空。
	*/
	YSSCore::General::YSSURI FileEditWidget::getUri() const {
		return d->Uri;
	}

	/*!
		\since YSS 0.13.0
		return 当前文件的路径字符串。

		真实文件返回本机绝对路径；虚拟文件返回 \c{@ext!name?param} 形式的字符串。
		这只是给界面显示和旧调用点用的便利函数，身份请用 \l{getUri()}。
	*/
	QString FileEditWidget::getFilePath() const {
		if (d->Uri.isVirtual()) {
			return d->Uri.getVirtualPath();
		}
		// 与 QFileInfo::absoluteFilePath() 保持同一形式（正斜杠）：上层的标签索引、
		// 项目文件和 QFileInfo 比较都拿这个字符串当键，形式一变就全都对不上。
		return QDir::fromNativeSeparators(d->Uri.getLocalPath());
	}

	/*!
		\since YSS 0.13.0
		return 当前文件编辑器的文件名。

		虚拟文件返回名称部分（\c{!} 之后、\c{?} 之前）。
	*/
	QString FileEditWidget::getFileName() const {
		return d->Uri.getFileName();
	}

	/*!
		\since YSS 0.13.0
		return 当前文件内容是否被修改。
	*/
	bool FileEditWidget::isFileChanged() const {
		return d->fileChanged;
	}

	/*!
		\since YSS 0.15.0
		return 当前文件是否为虚拟文件。
	*/
	bool FileEditWidget::isVirtualFile() const {
		return d->Uri.isVirtual();
	}

	/*!
		\since YSS 0.15.0
		return 当前文件的扩展名（不带点）。

		虚拟文件返回 \c{@} 与 \c{!} 之间的 ext 部分；真实文件返回文件名的后缀。
	*/
	QString FileEditWidget::getFileExt() const {
		if (d->Uri.isVirtual()) {
			return d->Uri.getHost();
		}
		return d->Uri.getFileExt();
	}

	/*!
		\since YSS 0.15.0
		return 当前虚拟文件的参数部分。

		不是虚拟文件时返回空字符串。解析方式参见 \l{VirtualFilePath}。
	*/
	QString FileEditWidget::getVirtualFileParam() const {
		if (!d->Uri.isVirtual()) {
			return QString();
		}
		return d->Uri.getQuery();
	}

	/*!
		\since YSS 0.13.0
		将当前文件内容标记为已修改状态，并触发 \l{fileChanged()} 信号。
	*/
	void FileEditWidget::setFileChanged() {
		d->fileChanged = true;
		emit fileChanged(d->Uri);
	}

	/*!
		\since YSS 0.13.0
		将当前文件内容标记为未修改状态，并触发 \l{fileChangeCanceled()} 信号。
	*/
	void FileEditWidget::cancelFileChanged() {
		d->fileChanged = false;
		emit fileChangeCanceled(d->Uri);
	}

	/*!
		\since YSS 0.15.0
		return 关闭时是否自动放弃未保存的修改。
	*/
	bool FileEditWidget::isAutoAbandon() const {
		return d->autoAbandon;
	}

	/*!
		\since YSS 0.15.0
		设置关闭时是否自动放弃未保存的修改。
		\a autoAbandon 是否自动放弃。
	*/
	void FileEditWidget::setAutoAbandon(bool autoAbandon) {
		d->autoAbandon = autoAbandon;
	}

	/*!
		\since YSS 0.18.0
		用身份与内容打开一个真实文件。
		\a uri 文件身份。
		\a data 文件内容，由 \l{FileServerManager} 从 \l{YSSUniqueFileSystem} 读好后给出。

		return 如果 \l{onOpen()} 成功则返回true。

		虚拟文件请用不带 \a data 的重载。
	*/
	bool FileEditWidget::openFile(const YSSCore::General::YSSURI& uri, const QByteArray& data) {
		if (!uri.isValid()) {
			yWarningF << "File URI is invalid.";
			return false;
		}
		d->Uri = uri;
		if (uri.isVirtual()) {
			return onVirtualOpen(uri.getHost(), uri.getFileName(), uri.getQuery());
		}
		return onOpen(uri, data);
	}

	/*!
		\since YSS 0.18.0
		用身份打开一个文件。只适用于虚拟文件。

		\a uri 文件身份。

		return 如果 \l{onVirtualOpen()} 成功则返回true。

		真实文件必须在 \l{FileServerManager} 里读好内容后调用带 \a data 的重载，
		编辑器不再自己读文件。
	*/
	bool FileEditWidget::openFile(const YSSCore::General::YSSURI& uri) {
		if (!uri.isValid()) {
			yWarningF << "File URI is invalid.";
			return false;
		}
		if (!uri.isVirtual()) {
			yWarningF << "Real files must be opened through FileServerManager so that the content is provided.";
			return false;
		}
		d->Uri = uri;
		return onVirtualOpen(uri.getHost(), uri.getFileName(), uri.getQuery());
	}

	/*!
		\since YSS 0.13.0
		保存当前文件编辑器的内容。
		\a target 保存到的目标 URI。为空时保存到当前 URI。
		\a deleteWhenSaveAs 另存为时是否删除原文件。
		\a silent 静默保存。为真时 \l{onSave()}/\l{onVirtualSave()} 不应弹出任何界面。

		return 如果 \l{onSave()} 产出内容且写出成功，返回true。

		派生类只负责在 \l{onSave()} 里把内容返回，写盘由本类通过
		\l{FileServerManager} 完成。保存成功后依次触发 \l{fileChangeCanceled()}、
		\l{fileSaved()}，路径变化时再触发 \l{fileRenamed()}。

		虚拟文件会直接调用 \l{onVirtualSave()}，\a target 与 \a deleteWhenSaveAs 均无效。
	*/
	bool FileEditWidget::saveFile(const YSSCore::General::YSSURI& target, bool deleteWhenSaveAs, bool silent) {
		if (d->Uri.isVirtual()) {
			d->saveSilent = silent;
			bool ok = onVirtualSave();
			d->saveSilent = false;
			return ok;
		}
		const YSSCore::General::YSSURI targetUri = target.isValid() ? target : d->Uri;
		if (!targetUri.isValid() || targetUri.isVirtual()) {
			return false;
		}
		QByteArray data;
		d->saveSilent = silent;
		std::expected<QByteArray, bool> produced = onSave(targetUri);
		d->saveSilent = false;
		if (!produced.has_value()) {
			if (produced.error()) {
				yWarningF << "Failed to produce content for" << targetUri.toString();
			}
			return false;
		}
		if (!FileServerManager::getInstance()->writeFile(targetUri, produced.value())) {
			yWarningF << "Failed to write file" << targetUri.toString();
			return false;
		}
		const YSSCore::General::YSSURI oldUri = d->Uri;
		d->Uri = targetUri;
		d->fileChanged = false;
		if (oldUri != targetUri) {
			if (deleteWhenSaveAs) {
				FileServerManager::getInstance()->removeFile(oldUri);
			}
			emit fileChangeCanceled(oldUri);
			emit fileSaved(targetUri);
			emit fileRenamed(oldUri, targetUri);
		}
		else {
			emit fileChangeCanceled(targetUri);
			emit fileSaved(targetUri);
		}
		return true;
	}

	/*!
		\since YSS 0.17.0
		return 当前这次保存是否为静默保存。

		只在 \l{onSave()}/\l{onVirtualSave()} 执行期间为真。派生类在这两个函数里需要
		弹窗询问或提示时，应当先查这个值。
	*/
	bool FileEditWidget::isSaveSilent() const {
		return d->saveSilent;
	}

	/*!
		\since YSS 0.13.0
		重新加载当前文件编辑器的内容。

		return 如果文件成功重新加载，返回true。

		虚拟文件路由到 \l{onVirtualReload()}；真实文件由本类通过 \l{FileServerManager}
		重新读取内容后交给 \l{onReload()}。
	*/
	bool FileEditWidget::reloadFile() {
		if (d->Uri.isVirtual()) {
			return onVirtualReload();
		}
		QByteArray data;
		if (!FileServerManager::getInstance()->readFile(d->Uri, &data)) {
			return false;
		}
		return onReload(data);
	}

	/*!
		\since YSS 0.13.0
		关闭当前文件编辑器。\a autoAbandon 是否自动放弃未保存的修改。
	*/
	void FileEditWidget::closeFile(bool autoAbandon) {
		d->autoAbandon = autoAbandon;
		close();
	}

	/*!
		\since YSS 0.16.0
		创建当前文件的备份。
	*/
	QByteArray FileEditWidget::createFileBackup() {
		return onBackup();
	}

	/*!
		\since YSS 0.13.0
		复制当前选中的内容到剪贴板。
	*/
	bool FileEditWidget::copy() {
		return onCopy();
	}

	/*!
		\since YSS 0.13.0
		剪切当前选中的内容到剪贴板。
	*/
	bool FileEditWidget::cut() {
		return onCut();
	}

	/*!
		\since YSS 0.13.0
		从剪贴板粘贴内容到当前光标位置。
	*/
	bool FileEditWidget::paste() {
		return onPaste();
	}

	/*!
		\since YSS 0.13.0
		撤销上一次编辑操作。
	*/
	bool FileEditWidget::undo() {
		return onUndo();
	}

	/*!
		\since YSS 0.13.0
		重做上一次被撤销的编辑操作。
	*/
	bool FileEditWidget::redo() {
		return onRedo();
	}

	/*!
		\since YSS 0.13.0
		全选当前编辑器的内容。
	*/
	bool FileEditWidget::selectAll() {
		return onSelectAll();
	}

	/*!
		\since YSS 0.15.0
		把光标移动到指定行列。
	*/
	bool FileEditWidget::cursorToPosition(qint32 lineNumber, qint32 column) {
		return onCursorToPosition(lineNumber, column);
	}

	/*!
		\since YSS 0.18.0
		派生类必须实现此函数以处理文件打开逻辑。
		\a uri 文件身份。
		\a data 文件内容。

		return 如果文件成功打开并加载，返回true；否则返回false。

		\warning 从0.15开始，这个函数可能因为onReload而重复调用，不保证
		在整个生命周期内只调用一次。因此如果你在这个函数内尝试懒加载你的
		FileEditWidget，应当首先检查资源是否已经初始化，避免出现内存泄漏。
	*/
	bool FileEditWidget::onOpen(const YSSCore::General::YSSURI& uri, const QByteArray& data) {
		return false;
	}

	/*!
		\since YSS 0.13.0
		派生类可以实现此虚函数以处理文件关闭逻辑。

		return 如果文件编辑器可以关闭，返回true；否则返回false。

		默认会根据 \l{isFileChanged()} 判断并询问是否保存。若 \l{isAutoAbandon()}
		返回true，则直接允许关闭。

		\warning 不要在这个函数里调用close或deleteLater，会造成递归爆栈。
	*/
	bool FileEditWidget::onClose() {
		if (isFileChanged() && not isAutoAbandon()) {
			int ret = QMessageBox::warning(this, VITRL("YSS::editor.saveWarning.title"),
				VITRL("YSS::editor.saveWarning.message").arg(getFileName()),
				QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
				QMessageBox::Save);
			switch (ret) {
			case QMessageBox::Save:
				return saveFile();
			case QMessageBox::Discard:
				return true;
			case QMessageBox::Cancel:
				return false;
			default:
				return false;
			}
		}
		else {
			return true;
		}
	}

	/*!
		\since YSS 0.18.0
		派生类必须实现此函数以产出要保存的内容。
		\a uri 要保存到的目标身份。

		return 成功时返回完整文件内容；失败时返回 \c{std::unexpected(flag)}，其中
		\c{flag} 表示这次失败是否需要让用户知道：\c{true} 是真失败（编码不支持、
		内容生成出错等），\c{false} 是本来就不打算保存（只读视图之类），应当静默跳过。
		失败时不会写盘，也不会发出任何保存信号。

		这里只产出内容，不要自己写文件——写盘由 \l{saveFile()} 交给
		\l{FileServerManager} 完成，这样 SSH 上的文件也能保存。
	*/
	std::expected<QByteArray, bool> FileEditWidget::onSave(const YSSCore::General::YSSURI& uri) {
		Q_UNUSED(uri);
		return std::unexpected(false);
	}

	/*!
		\since YSS 0.18.0
		派生类实现此函数以处理文件重新加载逻辑。
		\a data 重新读取到的文件内容。

		return 如果文件成功重新加载，返回true。

		默认实现用当前 URI 与给定内容重新调用 \l{onOpen()}。
	*/
	bool FileEditWidget::onReload(const QByteArray& data) {
		if (!d->Uri.isValid()) {
			return false;
		}
		return onOpen(d->Uri, data);
	}

	/*!
		\since YSS 0.13.0
		派生类实现此虚函数以处理复制逻辑。
	*/
	bool FileEditWidget::onCopy() {
		return false;
	}

	/*!
		\since YSS 0.13.0
		派生类实现此虚函数以处理剪切逻辑。
	*/
	bool FileEditWidget::onCut() {
		return false;
	}

	/*!
		\since YSS 0.13.0
		派生类实现此虚函数以处理粘贴逻辑。
	*/
	bool FileEditWidget::onPaste() {
		return false;
	}

	/*!
		\since YSS 0.13.0
		派生类实现此虚函数以处理撤销逻辑。
	*/
	bool FileEditWidget::onUndo() {
		return false;
	}

	/*!
		\since YSS 0.13.0
		派生类实现此虚函数以处理重做逻辑。
	*/
	bool FileEditWidget::onRedo() {
		return false;
	}

	/*!
		\since YSS 0.13.0
		派生类实现此虚函数以处理全选逻辑。
	*/
	bool FileEditWidget::onSelectAll() {
		return false;
	}

	/*!
		\since YSS 0.15.0
		派生类实现此虚函数以处理光标定位逻辑。
		\a lineNumber 行号。
		\a column 列号。
	*/
	bool FileEditWidget::onCursorToPosition(qint32 lineNumber, qint32 column) {
		return false;
	}

	/*!
		\since YSS 0.15.0
		处理虚拟文件的打开逻辑。
		\a ext 虚拟文件的扩展名。
		\a fileName 虚拟文件的文件名。
		\a param 其他参数字符串。

		return 如果虚拟文件成功打开，返回true。默认返回false。
	*/
	bool FileEditWidget::onVirtualOpen(const QString& ext, const QString& fileName, const QString& param) {
		return false;
	}

	/*!
		\since YSS 0.15.0
		处理虚拟文件的关闭逻辑。
		return 如果虚拟文件编辑器可以关闭，返回true。默认返回true。
	*/
	bool FileEditWidget::onVirtualClose() {
		return true;
	}

	/*!
		\since YSS 0.15.0
		处理虚拟文件的保存逻辑。
		return 如果虚拟文件成功保存，返回true。默认返回false。
	*/
	bool FileEditWidget::onVirtualSave() {
		return false;
	}

	/*!
		\since YSS 0.15.0
		处理虚拟文件的重新加载逻辑。默认重新调用 \l{onVirtualOpen()}。
	*/
	bool FileEditWidget::onVirtualReload() {
		return onVirtualOpen(getFileExt(), getFileName(), getVirtualFileParam());
	}

	/*!
		\since YSS 0.13.0
		\a event 关闭事件对象。

		处理文件编辑器关闭事件：调用 \l{onVirtualClose()} 或 \l{onClose()} 决定是否允许关闭，
		允许时发出 \l{fileClosed()}。
	*/
	void FileEditWidget::closeEvent(QCloseEvent* event) {
		const bool allowed = d->Uri.isVirtual() ? onVirtualClose() : onClose();
		if (allowed) {
			event->accept();
			emit fileClosed(d->Uri);
		}
		else {
			event->ignore();
		}
	}

	/*!
		\since YSS 0.16.0
		派生类可以实现此虚函数以提供文件备份数据。

		默认返回空的QByteArray。返回空时YSS不会为该文件创建备份文件。
	*/
	QByteArray FileEditWidget::onBackup() {
		return QByteArray();
	}
}