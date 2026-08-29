#ifndef YSS_MainEditor_MultiTerminal_h
#define YSS_MainEditor_MultiTerminal_h
#include <QtCore/qdir.h>
#include <Editor/FileEditWidget.h>
#include <Editor/FileServer.h>

namespace YSS::Editor {
	class MultiTerminalPrivate;
	class MultiTerminalVFS :public YSSCore::Editor::FileServer {
		Q_OBJECT;
	public:
		MultiTerminalVFS(YSSCore::Editor::EditorPlugin* plugin);
		virtual YSSCore::Editor::FileEditWidget* onCreateFileEditWidget() override;
	};

	class MultiTerminal :public YSSCore::Editor::FileEditWidget {
		Q_OBJECT;
	signals:
		void currentTerminalChanged(const QString& name);
	public:
		MultiTerminal(QWidget* parent = nullptr);
		~MultiTerminal();
		static MultiTerminal* getInstance();
		void addTerminal(const QString& name, const QString& command, const QDir& workingDir = QDir::homePath());
		void closeTerminal(const QString& name);
		void closeAll();
		bool containsTerminal(const QString& name) const;
		QStringList getTerminalNames() const;
		QString getCurrentTerminalName() const;
		void setCurrentTerminal(const QString& name);
	public:
		virtual bool onVirtualOpen(const QString& ext, const QString& fileName, const QString& param) override;
	private:
		MultiTerminalPrivate* d;
	};
}
#endif // YSS_MainEditor_MultiTerminal_h