#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qframe.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qtextedit.h>
#include <Editor/DebugServerManager.h>
#include <General/TranslationHost.h>
#include <General/YSSProject.h>
#include <Utility/FileUtility.h>
#include <Utility/JsonConfig.h>
#include "FileServer/YSSPEditor.h"
namespace YSSFileExt {
	class YSSPEditorPrivate {
		friend class YSSPEditor;
	protected:
		QWidget* Content;
		QLabel* CoverLabel;
		QLabel* ProjectNameLabel;
		QLabel* AuthorLabel;
		QLineEdit* AuthorLineEdit;
		QGridLayout* Layout;
		QLabel* DescriptionLabel;
		QTextEdit* DescriptionEdit;
		QLabel* DebugServerLabel;
		QComboBox* DebugServerComboBox;
		QHBoxLayout* ContentLayout;
		QPushButton* SaveButton;
	};

	YSSPEditor::YSSPEditor(QWidget* parent) : YSSCore::Editor::FileEditWidget(parent) {
		d = new YSSPEditorPrivate;
		d->Content = new QWidget(this);
		d->Layout = new QGridLayout(d->Content);
		d->CoverLabel = new QLabel(d->Content);
		d->CoverLabel->setFixedSize(160, 90);
		d->ProjectNameLabel = new QLabel(d->Content);
		d->AuthorLabel = new QLabel(d->Content);
		d->AuthorLabel->setText(VITRL("YSS::project.author"));
		d->AuthorLineEdit = new QLineEdit(d->Content);
		d->DescriptionLabel = new QLabel(d->Content);
		d->DescriptionLabel->setText(VITRL("YSS::project.description"));
		d->DescriptionEdit = new QTextEdit(d->Content);
		d->DebugServerLabel = new QLabel(d->Content);
		d->DebugServerLabel->setText(VITRL("YSS::project.debugServer"));
		d->DebugServerComboBox = new QComboBox(d->Content);
		d->SaveButton = new QPushButton(d->Content);
		d->SaveButton->setText(VITRL("Visindigo::general.save"));
		d->Layout->addWidget(d->CoverLabel, 0, 0, 3, 1);
		d->Layout->addWidget(d->ProjectNameLabel, 0, 1, 2, 2);
		d->Layout->addWidget(d->AuthorLabel, 2, 1);
		d->Layout->addWidget(d->AuthorLineEdit, 2, 2);
		d->Layout->addWidget(d->DescriptionLabel, 3, 0, 1, 3);
		d->Layout->addWidget(d->DescriptionEdit, 4, 0, 1, 3);
		d->Layout->addWidget(d->DebugServerLabel, 5, 0, 1, 3);
		d->Layout->addWidget(d->DebugServerComboBox, 6, 0, 1, 3);
		d->Layout->addWidget(d->SaveButton, 7, 0, 1, 3);
		d->ContentLayout = new QHBoxLayout(this);
		d->ContentLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
		d->ContentLayout->addWidget(d->Content);
		d->ContentLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
		QStringList debuggerList = YSSDSM->getDebugServerIDs();
		d->DebugServerComboBox->addItems(debuggerList);
	}
	YSSPEditor::~YSSPEditor() {
		delete d;
	}
	bool YSSPEditor::onOpen(const YSSCore::General::YSSURI& uri, const QByteArray& data) {
		Q_UNUSED(uri);
		Q_UNUSED(data);
		auto project = YSSCore::General::YSSProject::getCurrentProject();
		QString iconPath = project->getProjectIconPath().getLocalPath();
		if (not iconPath.isEmpty()) {
			QPixmap pixmap(iconPath);
			d->CoverLabel->setPixmap(pixmap.scaled(d->CoverLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
		}
		else {
			d->CoverLabel->setText(VITRL("YSS::project.noCover"));
		}
		d->ProjectNameLabel->setText(project->getProjectName());
		d->AuthorLineEdit->setText(project->getProjectAuthor());
		d->DescriptionEdit->setText(project->getProjectDescription());
		QString debuggerID = project->getProjectDebugServerID();
		int index = d->DebugServerComboBox->findText(debuggerID);
		if (index != -1) {
			d->DebugServerComboBox->setCurrentIndex(index);
		}
		connect(d->AuthorLineEdit, &QLineEdit::textChanged, this, &YSSPEditor::setFileChanged);
		connect(d->DescriptionEdit, &QTextEdit::textChanged, this, &YSSPEditor::setFileChanged);
		connect(d->DebugServerComboBox, &QComboBox::currentTextChanged, this, &YSSPEditor::setFileChanged);
		connect(d->SaveButton, &QPushButton::clicked, [this]() {
			saveFile();
			});
		return true;
	}
	std::expected<QByteArray, bool> YSSPEditor::onSave(const YSSCore::General::YSSURI& uri) {
		Q_UNUSED(uri);
		auto project = YSSCore::General::YSSProject::getCurrentProject();
		project->setProjectAuthor(d->AuthorLineEdit->text());
		project->setProjectDescription(d->DescriptionEdit->toPlainText());
		project->setProjectName(d->ProjectNameLabel->text());
		project->setProjectDebugServerID(d->DebugServerComboBox->currentText());
		// 项目信息写进 yssp，本身没有独立的文件内容可写。
		return std::unexpected(false);
	}
	YSSPFileServer::YSSPFileServer(YSSCore::Editor::EditorPlugin* plugin) :
		YSSCore::Editor::FileServer("YSSP File Server", "YSSFileExt.FileServer.YSSP", plugin) {
		setEditorType(EditorType::BuiltInEditor);
		setSupportedFileExts({ "yssp" });
	}
	YSSPFileServer::~YSSPFileServer() {}
	YSSCore::Editor::FileEditWidget* YSSPFileServer::onCreateFileEditWidget() {
		return new YSSPEditor();
	}
}