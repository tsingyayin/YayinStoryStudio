#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qpushbutton.h>
#include <General/Log.h>
#include <General/TranslationHost.h>
#include <General/YSSProject.h>
#include <Utility/FileOperation.h>
#include <Utility/FileUtility.h>
#include <Utility/JsonConfig.h>
#include <Widgets/ConfigWidget.h>
#include "YSS/ASERFileTemplate.h"

namespace ASERStudio::YSS {
	class FileTemplateInitWidget_AStoryXPrivate {
		friend class FileTemplateInitWidget_AStoryX;
	protected:
		Visindigo::Widgets::ConfigWidget* ConfigWidget = nullptr;
		QLabel* WhereLabel = nullptr;
		QPushButton* CreateButton = nullptr;
		QVBoxLayout* Layout = nullptr;
		QHBoxLayout* ButtonLayout = nullptr;
		QString FileName;
		// 新文件将被创建到的目录，只用于展示最终位置。
		YSSCore::General::YSSURI TargetUri;
	};

	FileTemplateInitWidget_AStoryX::FileTemplateInitWidget_AStoryX(const YSSCore::General::YSSURI& targetUri,
		QWidget* parent)
		: YSSCore::Editor::FileTemplateInitWidget(targetUri, parent)
	{
		d = new FileTemplateInitWidget_AStoryXPrivate;
		d->TargetUri = targetUri;
		this->setMinimumWidth(800);
		this->setWindowTitle(VITRL("ASERStudio::fileProvider.window.title"));
		d->ConfigWidget = new Visindigo::Widgets::ConfigWidget(this);
		Visindigo::Utility::FileOperation::Errorable<QString> cwJsonResult = Visindigo::Utility::FileOperation::readAll(":/resource/cn.yxgeneral.aserstudio/configWidget/FTP.json");
		if (cwJsonResult) {
			d->ConfigWidget->loadCWJson(cwJsonResult.value());
		}
		else {
			vgErrorF << "Failed to read file template config widget json, error: " << Visindigo::Utility::FileOperation::errorCodeName(cwJsonResult.error());
		}
		d->Layout = new QVBoxLayout(this);
		//d->Layout->setContentsMargins(0, 0, 0, 0);
		d->Layout->addWidget(d->ConfigWidget);
		d->ButtonLayout = new QHBoxLayout();
		d->ButtonLayout->setContentsMargins(10, 0, 10, 0);
		d->Layout->addLayout(d->ButtonLayout);
		d->WhereLabel = new QLabel(this);
		d->WhereLabel->setWordWrap(true);
		d->ButtonLayout->addWidget(d->WhereLabel);
		d->CreateButton = new QPushButton(VITRL("ASERStudio::fileProvider.window.create"), this);
		d->CreateButton->setObjectName("CreateButton");
		d->CreateButton->setMinimumWidth(120);
		d->CreateButton->setMaximumWidth(160);
		d->ButtonLayout->addWidget(d->CreateButton);
		d->FileName = d->ConfigWidget->getConfig()->getString("File.Name");
		connect(d->ConfigWidget, &Visindigo::Widgets::ConfigWidget::lineEditTextChanged, this, &FileTemplateInitWidget_AStoryX::onLineEditTextChanged);
		connect(d->CreateButton, &QPushButton::clicked, this, &FileTemplateInitWidget_AStoryX::onCreateButtonClicked);
		refreshWhereLabel();
	}

	FileTemplateInitWidget_AStoryX::~FileTemplateInitWidget_AStoryX() {
		delete d;
	}

	void FileTemplateInitWidget_AStoryX::resizeEvent(QResizeEvent* event) {
		YSSCore::Editor::FileTemplateInitWidget::resizeEvent(event);
	}

	void FileTemplateInitWidget_AStoryX::onLineEditTextChanged(const QString& node, const QString& text) {
		if (node == "File.Name") {
			d->FileName = text;
		}
		refreshWhereLabel();
	}

	void FileTemplateInitWidget_AStoryX::onCreateButtonClicked() {
		Visindigo::Utility::JsonConfig* config = d->ConfigWidget->getConfig();
		const QString version = config->getString("File.Version");
		QStringList fileContent;
		if (version == "2.05.22.1A" || version == "uptodate") {
			fileContent = initFileV3();
		}
		else {
			QMessageBox msgBox;
			msgBox.setWindowTitle(VITRL("ASERStudio::fileProvider.version.unsupported.title"));
			msgBox.setText(VITRL("ASERStudio::fileProvider.version.unsupported.text").arg(version));
			msgBox.setStandardButtons(QMessageBox::Ok);
			int ret = msgBox.exec();
			return;
		}
		setTargetName(Visindigo::Utility::FileUtility::toLegalFileName(d->FileName) + ".astoryx");
		// 目标目录由新建文件窗口决定，这里只负责给出内容。
		QByteArray content = fileContent.join("\n").toUtf8();
		setBinaryContent(content);
		emit contentPrepared();
		close();
	}

	QStringList FileTemplateInitWidget_AStoryX::initFileV3() {
		Visindigo::Utility::FileOperation::Errorable<QStringList> contentResult = Visindigo::Utility::FileOperation::readLines(":/resource/cn.yxgeneral.aserstudio/template/3.0/newFile.astoryx");
		if (not contentResult) {
			vgErrorF << "Failed to read AStoryX template file, error: " << Visindigo::Utility::FileOperation::errorCodeName(contentResult.error());
			return QStringList();
		}
		return contentResult.value();
	}

	void FileTemplateInitWidget_AStoryX::refreshWhereLabel() {
		if (d->FileName.trimmed().isEmpty()) {
			d->WhereLabel->clear();
			d->CreateButton->setEnabled(false);
			return;
		}
		const QString targetName = Visindigo::Utility::FileUtility::toLegalFileName(d->FileName) + ".astoryx";
		// 目标目录由新建文件窗口提供，用它拼出最终位置；重名由窗口统一检查。
		const YSSCore::General::YSSURI uri = d->TargetUri.resolve(targetName);
		const QString localPath = uri.getLocalPath();
		d->WhereLabel->setText(VITRL("ASERStudio::fileProvider.window.where")
			.arg(localPath.isEmpty() ? uri.toString() : localPath));
		d->CreateButton->setEnabled(true);
	}
	FileTemplate_AStoryX::FileTemplate_AStoryX(YSSCore::Editor::EditorPlugin* plugin) :
		FileTemplateProvider("ASE AStory File Template Provider", "FileTemplate_AStoryX", plugin)
	{
		setTemplateIconPath(":/resource/cn.yxgeneral.aserstudio/icon/ASEA_Dark.png");
		setTemplateID("ASEAStoryXFile");
		setTemplateName(VITRL("ASERStudio::fileProvider.astory.name"));
		setTemplateDescription(VITRL("ASERStudio::fileProvider.astory.description"));
		setTemplateTags(QStringList({
				VITRL("ASERStudio::fileProvider.astory.tags.ase"),
				VITRL("ASERStudio::fileProvider.astory.tags.astory"),
				VITRL("ASERStudio::fileProvider.astory.tags.engine-native"),
				VITRL("ASERStudio::fileProvider.astory.tags.script"),
				VITRL("ASERStudio::fileProvider.astory.tags.arknights")
			}));
		setFileExt("astoryx");
		// Constructor implementation
	}

	FileTemplate_AStoryX::~FileTemplate_AStoryX() {
		// Destructor implementation (if needed)
	}

	YSSCore::Editor::FileTemplateInitWidget* FileTemplate_AStoryX::fileInitWidget(
		const YSSCore::General::YSSURI& targetUri) {
		return new FileTemplateInitWidget_AStoryX(targetUri);
	}
}