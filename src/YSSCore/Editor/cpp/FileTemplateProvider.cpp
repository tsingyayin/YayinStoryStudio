#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qpushbutton.h>
#include <General/Plugin.h>
#include <General/TranslationHost.h>
#include <Utility/FileUtility.h>
#include "Editor/EditorPlugin.h"
#include "Editor/FileTemplateProvider.h"
namespace YSSCore::Editor {
	class FileTemplateInitWidgetPrivate
	{
		friend class FileTemplateInitWidget;
	protected:
		// 新文件将被创建到的目录。
		YSSCore::General::YSSURI TargetUri;
		// 由模板自行生成的待写入内容。
		QByteArray BinaryContent;
		// 目标文件名（含扩展名，不含目录）。为空时编辑器不会创建文件。
		QString TargetName;
	};

	/*!
		\class YSSCore::Editor::FileTemplateInitWidget
		\brief FileTemplateInitWidget是一个QFrame，作为FileTemplateProvider的初始化界面.
		\since YSS 0.13.0
		\inmodule YSSCore
		\ingroup CreateService

		FileTemplateInitWidget是一个QFrame，作为FileTemplateProvider的初始化界面。
		当用户选择某个文件模板来创建文件时，编辑器会调用该模板的fileInitWidget函数，
		并将其显示在界面上。用户可以在这个界面上进行
		必要的配置（如内容相关的选项），然后点击“创建”按钮来完成内容的准备工作。

		\warning 请注意，为了不对实现造成过多压力，这个类不要求自己直接把文件写入目标位置。
		因此，虽然此类提供YSSURI作为目标路径，但它仅用于展示和校验，不要自己写盘。

		只要在这个页面上收集到其他必须的信息，然后用 \l{setTargetName()} 给出目标文件名、
		用 \l{setBinaryContent()} 给出文件内容，然后发出 \l{contentPrepared()} 信号即可。
		
		YSS会在收到这个信号后将文件内容写入到指定的位置。并在写入成功后再用标准的打开流程打开这个新文件。
		这样无论目标是本机还是远端，写盘都由统一文件系统完成，模板本身不需要关心路径与传输细节。

		至于此类的生命周期，YSS会在显示它之前通过QWidget::setAttribute将其设置为关闭时自动销毁。
		因此，不要以任何方式额外处理此类的生命周期。

		如果你认为根据用户的输入无法创建文件， 那么只需要不发出任何信号即可。用户直接关闭
		此页面会被YSS视为取消创建操作，不会有任何后续操作发生。

		\warning 这类在0.14.0调整了虚函数，因此ABI与此前不兼容。
		\warning 这类在0.18.0改为只提供内容，写盘交给编辑器，ABI与此前不兼容。
	*/

	/*!
		\fn void FileTemplateInitWidget::contentPrepared();
		\since YSS 0.18.0

		内容准备就绪时（如用户点击了“创建”按钮）发出这个信号。发出之前应当先用
		\l{setTargetName()} 与 \l{setBinaryContent()} 把结果填好。
	*/

	/*!
		\since YSS 0.18.0
		\a targetUri 新文件将被创建到的目录。仅用于展示与校验，写盘始终由编辑器完成。
		\a parent 父对象

		构造函数
	*/
	FileTemplateInitWidget::FileTemplateInitWidget(const YSSCore::General::YSSURI& targetUri,
		QWidget* parent)
		: QFrame(parent) {
		d = new FileTemplateInitWidgetPrivate;
		d->TargetUri = targetUri;
	}

	/*!
		\since YSS 0.13.0

		析构函数
	*/
	FileTemplateInitWidget::~FileTemplateInitWidget() {
		delete d;
	}

	/*!
		\since YSS 0.18.0

		return 新文件将被创建到的目录。你可以用它给用户展示最终位置，但不要自己写盘。
	*/
	YSSCore::General::YSSURI FileTemplateInitWidget::getTargetUri() const {
		return d->TargetUri;
	}

	/*!
		\since YSS 0.18.0

		return 待写入文件的原始内容。
	*/
	const QByteArray& FileTemplateInitWidget::getBinaryContent() const {
		return d->BinaryContent;
	}

	/*!
		\since YSS 0.18.0
		\a content 待写入文件的原始内容

		设置待写入文件的内容。编辑器会在收到 \l{contentPrepared()} 后把它写到目标 URI。
	*/
	void FileTemplateInitWidget::setBinaryContent(QByteArray& content) {
		d->BinaryContent = content;
	}

	/*!
		\since YSS 0.18.0

		return 目标文件名（含扩展名，不含目录）。
	*/
	QString FileTemplateInitWidget::getTargetName() const {
		return d->TargetName;
	}

	/*!
		\since YSS 0.18.0
		\a name 目标文件名（含扩展名，不含目录）

		设置目标文件名。编辑器用它与基础目录拼出完整的文件 URI，因此这里不要带目录分隔符。
	*/
	void FileTemplateInitWidget::setTargetName(const QString& name) {
		d->TargetName = name;
	}

	class FileTemplateProviderPrivate {
		friend class FileTemplateProvider;
	protected:
		QString TemplateIconPath;
		QString TemplateID;
		QString TemplateName;
		QString TemplateDescription;
		QStringList TemplateTags;
		QString FileExt;
	};

	// 默认的模板初始化界面：只问文件名，内容是空的。
	//
	// 这里不写 Q_OBJECT：它不需要自己的信号与槽，连接都用 lambda 完成，
	// 因此不必为它单独生成 moc 代码。
	class DefaultFileTemplateInitWidget : public FileTemplateInitWidget {
	public:
		DefaultFileTemplateInitWidget(const YSSCore::General::YSSURI& targetUri,
			const QString& extension, const QString& defaultMainName,
			const QString& title, QWidget* parent)
			: FileTemplateInitWidget(targetUri, parent), Extension(extension) {
			setMinimumWidth(480);
			setWindowTitle(title);
			auto* layout = new QVBoxLayout(this);
			layout->setContentsMargins(12, 12, 12, 12);
			layout->setSpacing(8);

			auto* nameTitleLabel = new QLabel(VITRL("Visindigo::general.fileName"), this);
			NameEdit = new QLineEdit(this);
			NameEdit->setText(defaultMainName);
			NameEdit->selectAll();
			layout->addWidget(nameTitleLabel);
			layout->addWidget(NameEdit);

			WhereLabel = new QLabel(this);
			WhereLabel->setWordWrap(true);
			layout->addWidget(WhereLabel);

			auto* buttonLayout = new QHBoxLayout();
			buttonLayout->addStretch(1);
			CreateButton = new QPushButton(VITRL("Visindigo::general.new"), this);
			CreateButton->setMinimumWidth(120);
			buttonLayout->addWidget(CreateButton);
			layout->addLayout(buttonLayout);

			connect(NameEdit, &QLineEdit::textChanged, this, [this]() {
				const QString name = this->targetNameOf(this->NameEdit->text());
				this->refreshWhereLabel(name);
				this->CreateButton->setEnabled(!name.isEmpty());
				});
			connect(CreateButton, &QPushButton::clicked, this, [this]() {
				const QString name = this->targetNameOf(this->NameEdit->text());
				if (name.isEmpty()) {
					return;
				}
				setTargetName(name);
				QByteArray emptyContent;
				setBinaryContent(emptyContent);
				emit contentPrepared();
				close();
				});
			const QString name = targetNameOf(NameEdit->text());
			refreshWhereLabel(name);
			CreateButton->setEnabled(!name.isEmpty());
		}
	private:
		QString targetNameOf(const QString& rawName) const {
			const QString legal = Visindigo::Utility::FileUtility::toLegalFileName(rawName).trimmed();
			if (legal.isEmpty()) {
				return QString();
			}
			if (Extension.isEmpty()) {
				return legal;
			}
			return legal + QLatin1Char('.') + Extension;
		}
		// 用目标目录拼出最终位置给用户看；目标目录可能是远端，因此按显示用的文本处理。
		void refreshWhereLabel(const QString& targetName) {
			if (targetName.isEmpty()) {
				WhereLabel->clear();
				return;
			}
			const YSSCore::General::YSSURI uri = getTargetUri().resolve(targetName);
			const QString localPath = uri.getLocalPath();
			WhereLabel->setText(localPath.isEmpty() ? uri.toString() : localPath);
		}
	private:
		QString Extension;
		QLineEdit* NameEdit = nullptr;
		QLabel* WhereLabel = nullptr;
		QPushButton* CreateButton = nullptr;
	};

	/*!
		\class YSSCore::Editor::FileTemplateProvider
		\brief FileTemplateProvider向编辑器提供一个文件模板.
		\since YSS 0.13.0
		\inmodule YSSCore
		\ingroup CreateService

		FileTemplateProvider是一个插件模块，向编辑器提供一个文件模板。编辑器会在新建文件界面展示所有注册的FileTemplateProvider，
		用户可以选择一个模板来创建文件。

		值得指出的是，YSS假定了所有创建操作都需要通过一个UI来解决，所以这个Provider类本身主要用于记录
		文件的相关信息（如图标、ID、名称、描述、标签、扩展名等），以及提供一个UI界面来引导用户完成创建过程。真正的创建逻辑应该在FileTemplateInitWidget中实现。

		如果模板不需要任何自定义输入，可以什么都不做：\l{fileInitWidget()} 的默认实现会返回一个
		只输入文件名的界面，扩展名取自 \l{setFileExt()} 设置的值。

		\warning 这类在0.14.0完善了const语义，因此ABI与此前不兼容。
	*/

	/*!
		\since YSS 0.13.0
		\a name 模板名称
		\a id 模板ID，必须唯一
		\a plugin 提供该模板的插件实例

		构造函数
	*/
	FileTemplateProvider::FileTemplateProvider(const QString& name, const QString& id, EditorPlugin* plugin) :
		Visindigo::General::PluginModule((Visindigo::General::Plugin*)plugin, id, YSSPluginModule_FileTemplateProvider, name) {
		d = new FileTemplateProviderPrivate;
	}

	/*!
		\since YSS 0.13.0

		析构函数
	*/
	FileTemplateProvider::~FileTemplateProvider() {
		delete d;
	}

	/*!
		\since YSS 0.13.0

		return 模板图标路径
	*/
	QString FileTemplateProvider::getTemplateIconPath() const {
		return d->TemplateIconPath;
	}

	/*!
		\since YSS 0.13.0
		\a iconPath 模板图标路径

		设置模板图标路径。这个图标会在新建文件界面展示，帮助用户识别不同的模板。建议使用相对于插件资源目录的相对路径。
	*/
	void FileTemplateProvider::setTemplateIconPath(const QString& iconPath) {
		d->TemplateIconPath = iconPath;
	}

	/*!
		\since YSS 0.13.0
		return 模板ID
	*/
	QString FileTemplateProvider::getTemplateID() const {
		return d->TemplateID;
	}

	/*!
		\since YSS 0.13.0
		\a id 模板ID，必须唯一

		设置模板ID
	*/
	void FileTemplateProvider::setTemplateID(const QString& id) {
		d->TemplateID = id;
	}

	/*!
		\since YSS 0.13.0
		return 模板名称
	*/
	QString FileTemplateProvider::getTemplateName() const {
		return d->TemplateName;
	}

	/*!
		\since YSS 0.13.0
		\a name 模板名称

		设置模板名称
	*/
	void FileTemplateProvider::setTemplateName(const QString& name) {
		d->TemplateName = name;
	}

	/*!
		\since YSS 0.13.0
		return 模板描述
	*/
	QString FileTemplateProvider::getTemplateDescription() const {
		return d->TemplateDescription;
	}

	/*!
		\since YSS 0.13.0
		\a description 模板描述

		设置模板描述。这个描述会在新建文件界面展示，帮助用户了解这个模板的用途和特点。
	*/
	void FileTemplateProvider::setTemplateDescription(const QString& description) {
		d->TemplateDescription = description;
	}

	/*!
		\since YSS 0.13.0
		return 模板标签列表
	*/
	QStringList FileTemplateProvider::getTemplateTags() const {
		return d->TemplateTags;
	}

	/*!
		\since YSS 0.13.0
		\a tags 模板标签列表

		设置模板标签列表。标签可以帮助用户在新建文件界面通过筛选来快速找到想要的模板。
	*/
	void FileTemplateProvider::setTemplateTags(const QStringList& tags) {
		d->TemplateTags = tags;
	}

	/*!
		\since YSS 0.18.0

		return 模板产出的文件扩展名，不含点。未设置时返回空字符串。
	*/
	QString FileTemplateProvider::getFileExt() const {
		return d->FileExt;
	}

	/*!
		\since YSS 0.18.0
		\a ext 文件扩展名，不含点

		设置模板产出的文件扩展名。默认的 \l{fileInitWidget()} 实现会用它拼目标文件名。
		传入带前导点的值（如 \c{".json"}）也会被规范化成 \c{"json"}。
	*/
	void FileTemplateProvider::setFileExt(const QString& ext) {
		QString normalized = ext.trimmed();
		while (normalized.startsWith(QLatin1Char('.'))) {
			normalized.remove(0, 1);
		}
		d->FileExt = normalized;
	}

	/*!
		\since YSS 0.18.0
		\a targetUri 新文件将被创建到的目录

		return 模板的初始化界面。

		默认实现返回一个只输入文件名的界面，扩展名取自 \l{getFileExt()}，主名称预填一个
		通用默认值（"新建"）。内容为空，因此这类模板不需要写任何代码就能创建空文件。

		需要自定义输入或内容时，重写本函数返回自己的 FileTemplateInitWidget；返回 nullptr
		则表示完全没有界面，由新建文件窗口直接按模板名创建空文件。
	*/
	FileTemplateInitWidget* FileTemplateProvider::fileInitWidget(
		const YSSCore::General::YSSURI& targetUri) {
		return new DefaultFileTemplateInitWidget(targetUri, this->getFileExt(),
			VITRL("Visindigo::general.new"), this->getTemplateName(), nullptr);
	}
}