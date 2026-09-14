#ifndef YayinStoryStudio_Editor_AgentPage_AgentWin_h
#define YayinStoryStudio_Editor_AgentPage_AgentWin_h
#include <QtCore/qstring.h>
#include <QtGui/qcolor.h>
#include <QtWidgets/qframe.h>
#include <QtWidgets/qtextedit.h>
#include <Widgets/ThemeManager.h>
class QKeyEvent;
class QLabel;
class QPushButton;
class QTextBrowser;
class QTextCursor;
class QTimer;
namespace Visindigo::Widgets {
	class BorderLabel;
}
namespace Visindigo::Agent {
	class Dialog;
	class Message;
}
namespace YSS::AgentPage {
	class ChatInput : public QTextEdit {
		Q_OBJECT;
	public:
		ChatInput(QWidget* parent = nullptr);
	signals:
		void submitted();
	protected:
		virtual void keyPressEvent(QKeyEvent* event) override;
	};

	class AgentWin : public QFrame, public Visindigo::Widgets::ColorfulWidget {
		Q_OBJECT;
	private:
		Visindigo::Widgets::BorderLabel* TitleLabel;
		QTextBrowser* Transcript;
		ChatInput* Input;
		QPushButton* SendButton;
		QLabel* StatusLabel;
		Visindigo::Agent::Dialog* Chat;
		QString ModelName;
		QColor TextColor;
		QTimer* RefreshTimer;
		qint32 RenderedCount;
		int TailBlock;
		bool Ready;
	public:
		AgentWin();
		virtual ~AgentWin();
		virtual void onThemeChanged() override;
	public slots:
		void onSendClicked();
		void onStreamBegan();
		void onStreamDelta(const QString& delta);
		void onStreamReasoning(const QString& delta);
		void onStreamEnded(const QString& fullContent);
		void onRunFailed(const QString& message);
	private:
		void setupCenter();
		void refreshTranscript();
		void scheduleRefresh();
		void writeMessage(QTextCursor& cursor, const Visindigo::Agent::Message& message);
	};
}
#endif // YayinStoryStudio_Editor_AgentPage_AgentWin_h
