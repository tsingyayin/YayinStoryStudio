#include <QtCore/qdatetime.h>
#include <QtCore/qdir.h>
#include <QtWidgets/qApplication.h>
#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qpushbutton.h>
#include <General/Log.h>
#include <General/TranslationHost.h>
#include <General/YSSLogger.h>
#include <General/YSSProject.h>
#include <Utility/AsyncFunction.h>
#include <Utility/FileUtility.h>
#include "Editor/ProjectPage/ProjectInfoWidget.h"
namespace YSS::ProjectPage {
	ProjectInfoWidget::ProjectInfoWidget(QWidget* parent) :Visindigo::Widgets::BorderFrame(parent) {
		this->setObjectName("ProjectInfoWidget");
		CoverLabel = new QLabel(this);
		CoverLabel->setAlignment(Qt::AlignCenter);
		CoverLabel->setObjectName("ProjectCoverLabel");
		TitleLabel = new QLabel(this);
		TitleLabel->setObjectName("ProjectTitleLabel");
		auto font = qApp->font();
		font.setPointSizeF(font.pointSizeF() * 1.5);
		//font.setBold(true);
		TitleLabel->setFont(font);
		CreateTimeLabel = new QLabel(this);
		LastModifiedTimeLabel = new QLabel(this);
		SizeLabel = new QLabel(this);
		ShowInBrowserButton = new QPushButton(this);
		ShowInBrowserButton->setToolTip(VITRL("YSS::tooltips.projectWin.showInBrowser"));
		DeleteButton = new QPushButton(this);
		DeleteButton->setToolTip(VITRL("YSS::tooltips.projectWin.delete"));
		Layout = new QGridLayout(this);
		DeleteButton->setObjectName("DeleteButton");
		this->setLayout(Layout);
		Layout->addWidget(CoverLabel, 0, 0, 4, 1);
		Layout->addWidget(TitleLabel, 0, 1, 1, 3);
		Layout->addWidget(SizeLabel, 1, 1, 1, 1);
		Layout->addWidget(CreateTimeLabel, 2, 1, 1, 1);
		Layout->addWidget(LastModifiedTimeLabel, 3, 1, 1, 1);
		Layout->addWidget(ShowInBrowserButton, 2, 3, 1, 1);
		Layout->addWidget(DeleteButton, 3, 3, 1, 1);

		connect(DeleteButton, &QPushButton::clicked, this, &ProjectInfoWidget::onDeleteButtonClicked);
		connect(ShowInBrowserButton, &QPushButton::clicked, this, &ProjectInfoWidget::onShowInBrowserButtonClicked);
		initWidget();
		setColorfulEnable(true);
		onThemeChanged();
	}

	void ProjectInfoWidget::initWidget() {
		SizeRequestId++;
		Project = nullptr;
		CurrentUri = YSSCore::General::YSSURI();
		CoverLabel->setText(VITRL("YSS::project.noCover"));
		CoverLabel->setStyleSheet("QLabel{border-image:url()}");
		TitleLabel->setText(VITRL("YSS::project.selectToShow"));
		CreateTimeLabel->setText(VITRL("YSS::project.createTime") + ": ");
		LastModifiedTimeLabel->setText(VITRL("YSS::project.lastModifiedTime") + ": ");
		SizeLabel->setText(VITRL("YSS::project.size") + ": ");
		ShowInBrowserButton->setText(VITRL("YSS::project.showInBrowser"));
		ShowInBrowserButton->setEnabled(false);
		DeleteButton->setText(VITRL("YSS::project.delete"));
		DeleteButton->setEnabled(false);
	}

	void ProjectInfoWidget::showUri(const YSSCore::General::YSSURI& uri, const QString& name,
		const QDateTime& createTime, const QDateTime& lastModified) {
		initWidget();
		if (not uri.isValid()) {
			return;
		}
		CurrentUri = uri;
		const QString unavailable = VITRL("YSS::project.remoteInfoUnavailable");
		TitleLabel->setText(name.isEmpty() ? unavailable : name);
		// 项目没有真正加载，体积无从得知，也不该显示成“计算中”。
		SizeLabel->setText(VITRL("YSS::project.size") + ": " + unavailable);
		CreateTimeLabel->setText(VITRL("YSS::project.createTime") + ": "
			+ (createTime.isValid() ? createTime.toString("yyyy-MM-dd hh:mm:ss") : unavailable));
		LastModifiedTimeLabel->setText(VITRL("YSS::project.lastModifiedTime") + ": "
			+ (lastModified.isValid() ? lastModified.toString("yyyy-MM-dd hh:mm:ss") : unavailable));
		CoverLabel->setText(VITRL("YSS::project.noCover"));
		// 远端目录没法在资源管理器里打开；移除历史记录仍然可用。
		ShowInBrowserButton->setEnabled(uri.isNotRemote());
		DeleteButton->setEnabled(true);
	}

	void ProjectInfoWidget::showProject(YSSCore::General::YSSProject* project) {
		if (project == nullptr) {
			initWidget();
			return;
		}
		initWidget();
		Project = project;
		CurrentUri = project->getProjectPath();
		DeleteButton->setEnabled(true);
		ShowInBrowserButton->setEnabled(project->getProjectFolder().isNotRemote());
		QString path = project->getProjectFolder().getLocalPath();
		int requestId = ++SizeRequestId;
		if (path.isEmpty()) {
			SizeLabel->setText(VITRL("YSS::project.size") + ": " + VITRL("YSS::project.calculating"));
		}
		else {
			YSSAsync<qint64, QString>(
				{ QString(path) },
				[](QString path)->qint64 {
					return Visindigo::Utility::FileUtility::sizeBytes(path);
				},
				// 传入this作为上下文：本控件在求值完成前被销毁时，这段回调不会再执行。
				[this, requestId](qint64 size) {
					if (requestId != SizeRequestId) {
						// 用户已经切到别的项目，这个结果是过期的。
						return;
					}
					QString readable = Visindigo::Utility::FileUtility::readableSize(size);
					SizeLabel->setText(VITRL("YSS::project.size") + ": " + readable);
				},
				this
			);
			SizeLabel->setText(VITRL("YSS::project.size") + ": " + VITRL("YSS::project.calculating"));
		}
		CoverLabel->setText("");
		const YSSCore::General::YSSURI coverUri = project->getProjectIconPath();
		const QString coverPath = coverUri.getLocalPath();
		if (coverPath.isEmpty()) {
			CoverLabel->setStyleSheet("");
			CoverLabel->setText(VITRL("YSS::project.noCover"));
		}
		else {
			CoverLabel->setStyleSheet(QStringLiteral("QLabel{border-image:url(\"%1\")}")
				.arg(QDir::fromNativeSeparators(coverPath)));
		}
		TitleLabel->setText(project->getProjectName());
		CreateTimeLabel->setText(VITRL("YSS::project.createTime") + ": " + project->getProjectCreateTime().toString("yyyy-MM-dd hh:mm:ss"));
		LastModifiedTimeLabel->setText(VITRL("YSS::project.lastModifiedTime") + ": " + project->getProjectLastModifyTime().toString("yyyy-MM-dd hh:mm:ss"));
	}

	void ProjectInfoWidget::onDeleteButtonClicked() {
		if (not CurrentUri.isValid()) {
			return;
		}
		QMessageBox msgBox;
		msgBox.setWindowTitle(VITRL("YSS::project.delete"));
		msgBox.setText(VITRL("YSS::project.deleteConfirm"));
		msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
		msgBox.setDefaultButton(QMessageBox::Cancel);
		int ret = msgBox.exec();
		switch (ret) {
		case QMessageBox::Yes:
			emit removeConfirmed(CurrentUri);
			return;
		case QMessageBox::Cancel:
			return;
		}
	}

	void ProjectInfoWidget::onRemoveFromListButtonClicked() {
		if (not CurrentUri.isValid()) {
			return;
		}
		QMessageBox msgBox;
		msgBox.setWindowTitle(VITRL("YSS::project.delete"));
		msgBox.setText(VITRL("YSS::project.deleteConfirm"));
		msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
		msgBox.setDefaultButton(QMessageBox::Cancel);
		int ret = msgBox.exec();
		switch (ret) {
		case QMessageBox::Yes:
			emit removeConfirmed(CurrentUri);
			return;
		case QMessageBox::Cancel:
			return;
		}
	}

	void ProjectInfoWidget::onShowInBrowserButtonClicked() {
		if (Project != nullptr) {
			QString dir = Project->getProjectFolder().getLocalPath();
			//yDebugF << dir;
			Visindigo::Utility::FileUtility::openExplorer(dir);
		}
	}

	void ProjectInfoWidget::resizeEvent(QResizeEvent* event) {
		QFrame::resizeEvent(event);
		//this->CoverLabel->setFixedHeight((float)this->CoverLabel->width() / 16 * 9); // 16:9 aspect ratio
		this->CoverLabel->setFixedWidth(this->CoverLabel->height() / 9 * 16);
	}

	void ProjectInfoWidget::onThemeChanged() {
		//vgDebugF << "ProjectInfo";
		//this->applyVIStyleTemplate("YSS::ProjectInfo");
	}
}