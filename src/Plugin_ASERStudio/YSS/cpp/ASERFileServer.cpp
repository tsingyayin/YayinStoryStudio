#include "YSS/ASERFileServer.h"
#include "YSS/private/ASERFileServer_p.h"
#include <General/YSSProject.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlayout.h>
#include <QtWidgets/qscrollarea.h>
#include <QtWidgets/qlistwidget.h>
#include <QtGui/qfont.h>
#include <Utility/JsonConfig.h>
#include <Utility/FileUtility.h>
#include <General/Log.h>
#include "AStorySyntax/AStoryXRule.h"
#include "AStorySyntax/AStoryXValueMeta.h"
#include <Widgets/BorderFrame.h>
#include <General/TranslationHost.h>
#include <Utility/ColorTool.h>
#include <QtWidgets/qstyleoption.h>
#include <QtGui/qpainter.h>
namespace ASERStudio::YSS {
	FileServer_AStoryX::FileServer_AStoryX(YSSCore::Editor::EditorPlugin* plugin) :
		YSSCore::Editor::FileServer("AStory Script File Server", "ASERStudio.FileServer.AStoryX", plugin) {
		setEditorType(EditorType::CodeEditor);
		setSupportedFileExts({ "astoryx" });
	}
	FileServer_AStoryX::~FileServer_AStoryX() {}
	FileServer_ASRuleJson::FileServer_ASRuleJson(YSSCore::Editor::EditorPlugin* plugin) :
		YSSCore::Editor::FileServer("ASRule JSON File Server", "ASERStudio.FileServer.ASRuleJson", plugin) {
		setEditorType(EditorType::BuiltInEditor); // TODO: implement a custom editor for ASRule JSON files
		setSupportedFileExts({ "json" });
	}
	FileServer_ASRuleJson::~FileServer_ASRuleJson() {}
	qint64 FileServer_ASRuleJson::especiallyFocusFile(const QString& filePath) {
		YSSCore::General::YSSProject* project = YSSCore::General::YSSProject::getCurrentProject();
		QString projectFolder = project->getProjectFolder();
		QString ruleFolder = projectFolder + "/Rules";
		if (filePath.startsWith(ruleFolder) && filePath.endsWith(".json")) {
			vgDebug << "ASRule JSON file focused: " << filePath;
			return std::numeric_limits<qint64>::max();
		}
		else {
			vgDebug << "Normal JSON file:" << filePath;
			return -1;
		}
	}

	YSSCore::Editor::FileEditWidget* FileServer_ASRuleJson::onCreateFileEditWidget() {
		return new EditorWidget_ASRuleJson();
	}

	class EditorWidget_ASRuleJsonPrivate {
		friend class EditorWidget_ASRuleJson;
	protected:
		QString rawContent;
		QListWidget* overviewList = nullptr; // 左侧总览
		QScrollArea* scrollArea = nullptr;
		QWidget* contentWidget = nullptr;
		QVBoxLayout* contentLayout = nullptr;
		QList<Visindigo::Widgets::BorderFrame*> controllerFrames;
	};

	TagLabel::TagLabel(QWidget* parent) : QLabel(parent) {
		setContentsMargins(8, 2, 8, 2);
		setAlignment(Qt::AlignCenter);
	}

	void TagLabel::setFillRole(QPalette::ColorRole role) {
		m_fillRole = role;
		update();
	}

	void TagLabel::paintEvent(QPaintEvent* event) {
		QPainter painter(this);
		QStyleOptionFrame opt;
		opt.initFrom(this);
		opt.frameShape = QFrame::Box;
		opt.features = QStyleOptionFrame::Rounded;
		const int lineWidth = qMax(1, style()->pixelMetric(QStyle::PM_DefaultFrameWidth, nullptr, this));
		opt.lineWidth = lineWidth;
		opt.midLineWidth = 0;
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.setPen(Qt::NoPen);
		painter.setBrush(visibleFillColor());
		painter.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), 4, 4);
		style()->drawPrimitive(QStyle::PE_Frame, &opt, &painter, this);
		QLabel::paintEvent(event);
	}

	QSize TagLabel::sizeHint() const {
		return paddedSize(QLabel::sizeHint());
	}

	QSize TagLabel::minimumSizeHint() const {
		return paddedSize(QLabel::minimumSizeHint());
	}

	QColor TagLabel::visibleFillColor() const {
		QColor fill = palette().color(m_fillRole);
		const QColor windowColor = palette().color(QPalette::Window);
		if (!Visindigo::Utility::ColorTool::isLightColor(windowColor)) {
			return fill.lighter(140);
		}
		return fill.darker(120);
	}

	QSize TagLabel::paddedSize(const QSize& sz) const {
		const int lineWidth = qMax(1, style()->pixelMetric(QStyle::PM_DefaultFrameWidth, nullptr, this));
		const QMargins margins = contentsMargins();
		return QSize(sz.width() + 2 * lineWidth + margins.left() + margins.right(),
			sz.height() + 2 * lineWidth + margins.top() + margins.bottom());
	}

	namespace {
		QString localizedParameterType(const ASERStudio::AStorySyntax::AStoryXValueMeta& valueMeta) {
			QString typeString = valueMeta.getTypeString();
			static const QStringList knownTypes = { "Integer", "Float", "String", "Bool", "Enum", "Vector" };
			if (!knownTypes.contains(typeString)) {
				return typeString;
			}
			QString i18n = VITR(QString("ASERStudio::value.%1").arg(typeString.toLower()));
			return i18n.isEmpty() ? typeString : i18n;
		}

		QWidget* createParameterRowWidget(const QString& controllerTypeString, const QString& paramName,
			const ASERStudio::AStorySyntax::AStoryXValueMeta& valueMeta, const QString& metaTag = QString(),
			const QString& metaToolTip = QString()) {
			QWidget* row = new QWidget();
			QHBoxLayout* layout = new QHBoxLayout(row);
			layout->setContentsMargins(0, 0, 0, 0);
			layout->setSpacing(6);

			TagLabel* nameLabel = new TagLabel(row);
			nameLabel->setText(paramName);
			layout->addWidget(nameLabel);
			layout->addSpacing(8);

			if (!metaTag.isEmpty()) {
				TagLabel* metaLabel = new TagLabel(row);
				metaLabel->setText(metaTag);
				if (!metaToolTip.isEmpty()) {
					metaLabel->setToolTip(metaToolTip);
				}
				layout->addWidget(metaLabel);
			}

			TagLabel* typeLabel = new TagLabel(row);
			typeLabel->setText(localizedParameterType(valueMeta));
			layout->addWidget(typeLabel);

			QLabel* descLabel = new QLabel(
				VITR(QString("ASERStudio::document.%1.%2").arg(controllerTypeString).arg(paramName)), row);
			descLabel->setWordWrap(true);
			layout->addWidget(descLabel, 1);
			return row;
		}

		Visindigo::Widgets::BorderFrame* createControllerFrame(ASERStudio::AStorySyntax::AStoryXController& controller) {
			QString typeString = controller.getControllerTypeString();
			QString header = controller.getHeader();

			Visindigo::Widgets::BorderFrame* frame = new Visindigo::Widgets::BorderFrame();
			QVBoxLayout* frameLayout = new QVBoxLayout(frame);
			frameLayout->setContentsMargins(12, 12, 12, 12);
			frameLayout->setSpacing(8);

			QWidget* titleRow = new QWidget(frame);
			QHBoxLayout* titleLayout = new QHBoxLayout(titleRow);
			titleLayout->setContentsMargins(0, 0, 0, 0);
			titleLayout->setSpacing(6);

			QFont titleFont = titleRow->font();
			titleFont.setPointSize(titleFont.pointSize() + 5);
			titleFont.setBold(true);

			QLabel* idLabel = new QLabel(header.isEmpty() ? typeString : (typeString + " - "), titleRow);
			idLabel->setFont(titleFont);
			titleLayout->addWidget(idLabel);

			if (!header.isEmpty()) {
				TagLabel* headerLabel = new TagLabel(titleRow);
				headerLabel->setFillRole(QPalette::AlternateBase);
				headerLabel->setText(header);
				titleLayout->addWidget(headerLabel);
			}
			titleLayout->addStretch();
			frameLayout->addWidget(titleRow);

			QLabel* descLabel = new QLabel(
				VITR(QString("ASERStudio::document.%1.__description").arg(typeString)), frame);
			descLabel->setWordWrap(true);
			frameLayout->addWidget(descLabel);

			QString requiredName = controller.getRequiredParameterName();
			if (!requiredName.isEmpty()) {
				QLabel* requiredHeader = new QLabel(VITR("ASERStudio::ruleEditor.requiredParameters"), frame);
				QFont headerFont = requiredHeader->font();
				headerFont.setBold(true);
				requiredHeader->setFont(headerFont);
				frameLayout->addWidget(requiredHeader);

				QString separatorTag;
				QString separatorToolTip;
				QString requiredSeparator = controller.getRequiredParameterSeparator();
				if (!requiredSeparator.isEmpty()) {
					QString displaySeparator = requiredSeparator;
					displaySeparator.replace("\t", "\\t");
					separatorTag = displaySeparator;
					separatorToolTip = VITR("ASERStudio::ruleEditor.internalSeparator");
				}
				frameLayout->addWidget(createParameterRowWidget(typeString, requiredName,
					controller.getRequiredParameterValue(), separatorTag, separatorToolTip));
			}

			QStringList optionalNames = controller.getOptionalParameterNames();
			QStringList optionalPrefixes = controller.getOptionalParameterPrefixes();
			if (!optionalNames.isEmpty()) {
				QLabel* optionalHeader = new QLabel(VITR("ASERStudio::ruleEditor.optionalParameters"), frame);
				QFont headerFont = optionalHeader->font();
				headerFont.setBold(true);
				optionalHeader->setFont(headerFont);
				frameLayout->addWidget(optionalHeader);
				QMap<QString, ASERStudio::AStorySyntax::AStoryXValueMeta> optionalValues = controller.getOptionalParameterValues();
				for (int i = 0; i < optionalNames.size(); ++i) {
					QString prefixTag;
					QString prefixToolTip;
					const QString prefix = optionalPrefixes.value(i);
					if (!prefix.isEmpty()) {
						prefixTag = prefix;
						prefixToolTip = VITR("ASERStudio::ruleEditor.prefix");
					}
					frameLayout->addWidget(createParameterRowWidget(typeString, optionalNames[i],
						optionalValues.value(optionalNames[i]), prefixTag, prefixToolTip));
				}
			}
			frameLayout->addStretch();
			return frame;
		}
	}

	EditorWidget_ASRuleJson::EditorWidget_ASRuleJson(QWidget* parent) : YSSCore::Editor::FileEditWidget(parent) {
		d = new EditorWidget_ASRuleJsonPrivate();

		d->overviewList = new QListWidget(this);
		d->overviewList->setFixedWidth(220);
		d->overviewList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		connect(d->overviewList, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
			int index = d->overviewList->row(item);
			if (index >= 0 && index < d->controllerFrames.size()) {
				d->scrollArea->ensureWidgetVisible(d->controllerFrames[index]);
			}
			});

		d->scrollArea = new QScrollArea(this);
		d->scrollArea->setWidgetResizable(true);
		d->scrollArea->setFrameShape(QFrame::NoFrame);
		d->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		d->contentWidget = new QWidget();
		d->contentLayout = new QVBoxLayout(d->contentWidget);
		d->contentLayout->setContentsMargins(8, 8, 8, 8);
		d->contentLayout->setSpacing(8);
		d->scrollArea->setWidget(d->contentWidget);
		d->contentLayout->addStretch();

		QHBoxLayout* mainLayout = new QHBoxLayout(this);
		mainLayout->setContentsMargins(0, 0, 0, 0);
		mainLayout->setSpacing(0);
		mainLayout->addWidget(d->overviewList);
		mainLayout->addWidget(d->scrollArea, 1);
	}

	EditorWidget_ASRuleJson::~EditorWidget_ASRuleJson() {
		delete d;
	}

	bool EditorWidget_ASRuleJson::onOpen(const QString& path) {
		d->rawContent = Visindigo::Utility::FileUtility::readAll(path);

		for (auto frame : d->controllerFrames) {
			d->contentLayout->removeWidget(frame);
			frame->deleteLater();
		}
		d->controllerFrames.clear();
		d->overviewList->clear();

		ASERStudio::AStorySyntax::AStoryXRule rule("default");
		if (!rule.parseJson(d->rawContent)) {
			vgWarning << "Failed to parse ASRule JSON: " << path;
		}
		auto controllers = rule.getAvailableControllers();
		int insertIndex = d->contentLayout->count() - 1;
		for (auto& controller : controllers) {
			if (!controller.isValid()) {
				continue;
			}
			Visindigo::Widgets::BorderFrame* frame = createControllerFrame(controller);
			d->controllerFrames.append(frame);
			d->contentLayout->insertWidget(insertIndex++, frame);

			QString typeString = controller.getControllerTypeString();
			QString header = controller.getHeader();
			QString overviewText = header.isEmpty() ? typeString : (typeString + " " + header);
			d->overviewList->addItem(overviewText);
		}
		return true;
	}

	bool EditorWidget_ASRuleJson::onClose() {
		return true;
	}

	bool EditorWidget_ASRuleJson::onSave(const QString& path) {
		Visindigo::Utility::FileUtility::saveAll(path, d->rawContent);
		return true;
	}

	bool EditorWidget_ASRuleJson::onReload() {
		return true;
	}

	bool EditorWidget_ASRuleJson::onCopy() {
		return true;
	}

	bool EditorWidget_ASRuleJson::onCut() {
		return true;
	}

	bool EditorWidget_ASRuleJson::onPaste() {
		return true;
	}

	bool EditorWidget_ASRuleJson::onUndo() {
		return true;
	}

	bool EditorWidget_ASRuleJson::onRedo() {
		return true;
	}

	bool EditorWidget_ASRuleJson::onSelectAll() {
		return true;
	}
}