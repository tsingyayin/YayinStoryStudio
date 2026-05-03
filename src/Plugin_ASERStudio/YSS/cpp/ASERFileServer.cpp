#include "YSS/ASERFIleServer.h"
#include <General/YSSProject.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlayout.h>
#include <Utility/JsonConfig.h>
#include <Utility/FileUtility.h>
#include <General/Log.h>
namespace ASERStudio::YSS {
	FileServer_AStoryX::FileServer_AStoryX(YSSCore::Editor::EditorPlugin* plugin) :
		YSSCore::Editor::FileServer("AStory Script File Server", "ASERStudio.FileServer.AStoryX", plugin) {
		setEditorType(EditorType::CodeEditor);
		setSupportedFileExts({ "astoryx" });
	}
	FileServer_AStoryX::~FileServer_AStoryX() {
	}
	FileServer_ASRuleJson::FileServer_ASRuleJson(YSSCore::Editor::EditorPlugin* plugin) :
		YSSCore::Editor::FileServer("ASRule JSON File Server", "ASERStudio.FileServer.ASRuleJson", plugin) {
		setEditorType(EditorType::BuiltInEditor); // TODO: implement a custom editor for ASRule JSON files
		setSupportedFileExts({ "json" });
	}
	FileServer_ASRuleJson::~FileServer_ASRuleJson() {
	}
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
		QList<QLabel*> labels;
		QVBoxLayout* layout;
		QString rawContent;
	};

	EditorWidget_ASRuleJson::EditorWidget_ASRuleJson(QWidget* parent) : YSSCore::Editor::FileEditWidget(parent) {
		d = new EditorWidget_ASRuleJsonPrivate();
		d->layout = new QVBoxLayout(this);
	}

	EditorWidget_ASRuleJson::~EditorWidget_ASRuleJson() {
		delete d;
	}

	bool EditorWidget_ASRuleJson::onOpen(const QString& path) {
		d->rawContent = Visindigo::Utility::FileUtility::readAll(path);
		Visindigo::Utility::JsonConfig config;
		config.parse(d->rawContent);
		auto controllers = config.getArray("rules");
		for (auto controller : controllers) {
			QLabel* label = new QLabel(this);
			QString key = controller.getString("Key");
			QString header = controller.getString("Value.header");
			label->setText(key + " [" + header + "]");
			label->setWordWrap(true);
			d->labels.append(label);
			d->layout->addWidget(label);
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