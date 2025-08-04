#include "../FS_ASRule.h"
#include <QFileInfo>
#include <QLabel>

ASRuleFileServer::ASRuleFileServer(Visindigo::Editor::EditorPlugin* plugin) :
	FileServer("ASE ASRule File Server", "ASEASRuleFS", plugin)
{
	setEditorType(Visindigo::Editor::FileServer::BuiltInEditor);
	setSupportedFileExts({ "asrule" });
}

ASRuleFileServer::~ASRuleFileServer() {
	FileServer::~FileServer();
}

Visindigo::Editor::FileEditWidget* ASRuleFileServer::onCreateFileEditWidget() {
	return new ASRuleFileEditWidget();
}

ASRuleFileEditWidget::ASRuleFileEditWidget(QWidget* parent) : Visindigo::Editor::FileEditWidget(parent) {
	Layout = new QVBoxLayout(this);
}

ASRuleFileEditWidget::~ASRuleFileEditWidget() {
	// Cleanup if needed
}

bool ASRuleFileEditWidget::onOpen(const QString& path) {
	if (path.isEmpty()) {
		qDebug() << "File path is empty.";
		return false;
	}
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qDebug() << "Failed to open file:" << path;
		return false;
	}
	QTextStream in(&file);
	in.setEncoding(QStringConverter::Utf8);
	generateContent(&in);
	file.close();
	return true;
}

bool ASRuleFileEditWidget::onClose() {
	// Handle close event if needed
	return true;
}

bool ASRuleFileEditWidget::onSave(const QString& path) {
	return true;
}
bool ASRuleFileEditWidget::onReload() {
	return true;
}
void ASRuleFileEditWidget::generateContent(QTextStream* ts) {
	while (!ts->atEnd()) {
		QString line = ts->readLine();
		QLabel* label = new QLabel(line, this);
		label->setWordWrap(true);
		Layout->addWidget(label);
	}
}