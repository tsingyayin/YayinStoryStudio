#include "FileServer/YSSPEditor.h"
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qframe.h>
#include <QtWidgets/qtextedit.h>
#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qlineedit.h>
#include <Editor/DebugServerManager.h>
#include <Utility/JsonConfig.h>
#include <Utility/FileUtility.h>
#include <General/YSSProject.h>
#include <General/TranslationHost.h>
namespace YSSFileExt {
	class YSSPEditorPrivate {
		friend class YSSPEditor;
	protected:
		QLabel* CoverLabel;
		QLabel* ProjectNameLabel;
		QLabel* AuthorLabel;
		QLineEdit* AuthorLineEdit;
		QGridLayout* Layout;
		QLabel* DescriptionLabel;
		QTextEdit* DescriptionEdit;
		QLabel* DebugServerLabel;
		QComboBox* DebugServerComboBox;
	};

	YSSPEditor::YSSPEditor(QWidget* parent) : YSSCore::Editor::FileEditWidget(parent) {
		d = new YSSPEditorPrivate;
		d->Layout = new QGridLayout(this);
		d->CoverLabel = new QLabel(this);
		d->CoverLabel->setFixedSize(320, 180);
		d->ProjectNameLabel = new QLabel(this);
		d->AuthorLabel = new QLabel(this);
		d->AuthorLabel->setText(VITR("YSS::project.author"));
		d->AuthorLineEdit = new QLineEdit(this);
		d->DescriptionLabel = new QLabel(this);
		d->DescriptionLabel->setText(VITR("YSS::project.description"));
		d->DescriptionEdit = new QTextEdit(this);
		d->DebugServerLabel = new QLabel(this);
		d->DebugServerLabel->setText(VITR("YSS::project.debugServer"));
		d->DebugServerComboBox = new QComboBox(this);
		d->Layout->addWidget(d->CoverLabel, 0, 0, 3, 1);
		d->Layout->addWidget(d->ProjectNameLabel, 0, 1, 2, 1);
		d->Layout->addWidget(d->AuthorLabel, 2, 1);
		d->Layout->addWidget(d->AuthorLineEdit, 2, 2);
		d->Layout->addWidget(d->DescriptionLabel, 3, 0);
		d->Layout->addWidget(d->DescriptionEdit, 4, 0);
		d->Layout->addWidget(d->DebugServerLabel, 5, 0);
		d->Layout->addWidget(d->DebugServerComboBox, 6, 0);

		QStringList debuggerList = YSSDSM->getDebugServerIDs();
		d->DebugServerComboBox->addItems(debuggerList);
		connect(d->AuthorLineEdit, &QLineEdit::textChanged, this, &YSSPEditor::setFileChanged);
		connect(d->DescriptionEdit, &QTextEdit::textChanged, this, &YSSPEditor::setFileChanged);
		connect(d->DebugServerComboBox, &QComboBox::currentTextChanged, this, &YSSPEditor::setFileChanged);
	}
	YSSPEditor::~YSSPEditor() {
		delete d;
	}
	bool YSSPEditor::onOpen(const QString& path) {
		auto project = YSSCore::General::YSSProject::getCurrentProject();
		QString iconPath = project->getProjectIconPath();
		iconPath = project->getProjectFolder() + "/" + iconPath;
		if (not iconPath.isEmpty()) {
			QPixmap pixmap(iconPath);
			d->CoverLabel->setPixmap(pixmap.scaled(d->CoverLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
		}
		else {
			d->CoverLabel->setText(VITR("YSS::project.noCover"));
		}
		d->ProjectNameLabel->setText(project->getProjectName());
		d->AuthorLineEdit->setText(project->getProjectAuthor());
		d->DescriptionEdit->setText(project->getProjectDescription());
		QString debuggerID = project->getProjectDebugServerID();
		int index = d->DebugServerComboBox->findText(debuggerID);
		if (index != -1) {
			d->DebugServerComboBox->setCurrentIndex(index);
		}
		return true;
	}
	bool YSSPEditor::onClose() {
		return true;
	}
	bool YSSPEditor::onSave(const QString& path) {
		auto project = YSSCore::General::YSSProject::getCurrentProject();
		project->setProjectAuthor(d->AuthorLineEdit->text());
		project->setProjectDescription(d->DescriptionEdit->toPlainText());
		project->setProjectName(d->ProjectNameLabel->text());
		project->setProjectDebugServerID(d->DebugServerComboBox->currentText());
		return true;
	}
	YSSPFileServer::YSSPFileServer(YSSCore::Editor::EditorPlugin* plugin) :
		YSSCore::Editor::FileServer("YSSP File Server", "YSSFileExt.FileServer.YSSP", plugin) {
		setEditorType(EditorType::BuiltInEditor);
		setSupportedFileExts({ "yssp" });
	}
	YSSPFileServer::~YSSPFileServer() {
	}
	YSSCore::Editor::FileEditWidget* YSSPFileServer::onCreateFileEditWidget() {
		return new YSSPEditor();
	}
}