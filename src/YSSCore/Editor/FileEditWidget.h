#ifndef YSSCore_Editor_FileEditWidget_h
#define YSSCore_Editor_FileEditWidget_h
#include <QtWidgets/qframe.h>
#include <General/YSSURI.h>
#include "YSSCoreCompileMacro.h"
#include <expected>
// Forward declarations
class QWidget;
class QString;
class QByteArray;
class QCloseEvent;
namespace YSSCore::Editor {
	class FileEditWidgetPrivate;
}
// Main
namespace YSSCore::Editor {
	class YSSCoreAPI FileEditWidget :public QFrame {
		Q_OBJECT;
		VImpl(FileEditWidget);
	signals:
		void fileChanged(const YSSCore::General::YSSURI& uri);
		void fileChangeCanceled(const YSSCore::General::YSSURI& uri);
		void fileSaved(const YSSCore::General::YSSURI& uri);
		void fileRenamed(const YSSCore::General::YSSURI& rawUri, const YSSCore::General::YSSURI& changedUri);
		void fileClosed(const YSSCore::General::YSSURI& uri);
		// 异步写入失败。写入是异步的，失败不会体现在 saveFile() 的返回值里。
		void fileSaveFailed(const YSSCore::General::YSSURI& uri);
	public:
		FileEditWidget(QWidget* parent = nullptr);
		virtual ~FileEditWidget();
		YSSCore::General::YSSURI getUri() const;
		QString getFilePath() const;
		QString getFileName() const;
		bool isFileChanged() const;
		bool isVirtualFile() const;
		QString getFileExt() const;
		QString getVirtualFileParam() const;
		void setFileChanged();
		void cancelFileChanged();
		bool isAutoAbandon() const;
		void setAutoAbandon(bool autoAbandon);
	public:
		bool openFile(const YSSCore::General::YSSURI& uri, const QByteArray& data);
		bool openFile(const YSSCore::General::YSSURI& uri);
		bool saveFile(const YSSCore::General::YSSURI& target = YSSCore::General::YSSURI(),
			bool deleteWhenSaveAs = false, bool silent = false);
		bool isSaveSilent() const;
		// 是否还有已提交但尚未完成的异步写入。
		bool isSaving() const;
		bool reloadFile();
		void closeFile(bool autoAbandon = false);
		QByteArray createFileBackup();
	public:
		bool copy();
		bool cut();
		bool paste();
		bool undo();
		bool redo();
		bool selectAll();
		bool cursorToPosition(qint32 lineNumber, qint32 column);
	protected:
		virtual bool onOpen(const YSSCore::General::YSSURI& uri, const QByteArray& data);
		virtual bool onClose();
		virtual std::expected<QByteArray, bool> onSave(const YSSCore::General::YSSURI& uri);
		virtual bool onReload(const QByteArray& data);
	protected:
		virtual bool onCopy();
		virtual bool onCut();
		virtual bool onPaste();
		virtual bool onUndo();
		virtual bool onRedo();
		virtual bool onSelectAll();
		virtual bool onCursorToPosition(qint32 lineNumber, qint32 column);
	protected:
		virtual bool onVirtualOpen(const QString& ext, const QString& fileName, const QString& param);
		virtual bool onVirtualClose();
		virtual bool onVirtualSave();
		virtual bool onVirtualReload();
	public:
		virtual void closeEvent(QCloseEvent* event) override;
	protected:
		virtual QByteArray onBackup();
	private:
		// 保存成功且本次关闭正处于等待状态时，排队触发真正的关闭。
		void finishClose();
	};
}

#endif // YSSCore_Editor_FileEditWidget_h