#ifndef Visindigo_Agent_Dialog_h
#define Visindigo_Agent_Dialog_h
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qobject.h>
#include <QtCore/qobjectdefs.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include "Agent/Model.h"
#include "Agent/Prompt.h"
#include "VICompileMacro.h"
// Forward declarations
namespace Visindigo::Utility {
	class JsonConfig;
}
namespace Visindigo::Agent {
	class DialogPrivate;
	class MessagePrivate;
	class Skill;
	class MCP;
	class Function;
}
// Main
namespace Visindigo::Agent {
	class VisindigoAPI Message {
		Q_GADGET;
	public:
		enum class Role {
			System,
			User,
			Assistant,
			Tool,
			Prompt
		};
		Q_ENUM(Role)
	public:
		Message();
		Message(Role role, const QString& content);
		VICopyable(Message);
		VIMoveable(Message);
		~Message();
	public:
		Message& setRole(Role role);
		Message& setContent(const QString& content);
		Message& setName(const QString& name);
		Message& setToolCallID(const QString& id);
		// 原始 JSON 数组文本，原样回传给模型
		Message& setToolCalls(const QString& rawJsonArray);
		// 推理模型的思考内容，只存不发
		Message& setReasoning(const QString& reasoning);
	public:
		Role getRole() const;
		QString getContent() const;
		QString getName() const;
		QString getToolCallID() const;
		QString getToolCalls() const;
		QString getReasoning() const;
		bool isEmpty() const;
	public:
		Visindigo::Utility::JsonConfig toJson() const;
		bool fromJson(const Visindigo::Utility::JsonConfig& json);
	private:
		MessagePrivate* d;
	};

	class VisindigoAPI Dialog : public QObject {
		Q_OBJECT;
		friend class DialogPrivate;
	public:
		Dialog();
		virtual ~Dialog();
	public:
		Dialog& appendMessage(const Message& message);
		Dialog& appendMessage(Message::Role role, const QString& content);
		Dialog& clearMessages();
		QList<Message> getMessages() const;
		Message getLastMessage() const;
		qint32 getMessageCount() const;
		Dialog& truncateTo(qint32 messageCount);
		qint32 findLastAssistantMessage() const;
		// 深拷贝消息与能力配置；不复制 id，target 保留自己的 id
		bool copyTo(Dialog* target) const;
	public:
		void beginAssistantMessage();
		void appendDelta(const QString& delta);
		void endAssistantMessage();
		bool isStreaming() const;
		QString getStreamingContent() const;
		QString getReasoningContent() const;
	public:
		Dialog& addSkill(const Skill& skill);
		Dialog& removeSkill(const QString& name);
		QList<Skill> getSkills() const;
		Dialog& addMCP(const MCP& server);
		Dialog& removeMCP(const QString& name);
		QList<MCP> getMCPs() const;
		// 转移所有权；Dialog 负责释放
		Dialog& addFunction(Function* function);
		Dialog& removeFunction(const QString& id);
		Function* getFunction(const QString& id) const;
		QStringList getFunctionIds() const;
	public:
		Dialog& addPrompt(const Prompt& prompt);
		Dialog& removePrompt(const QString& name);
		QList<Prompt> getPrompts() const;
		Dialog& appendPrompt(const QString& name, const QMap<QString, QString>& variables = {});
	public:
		QString getId() const;
		Dialog& setBranchFromID(const QString& id);
		QString getBranchFromID() const;
		Dialog& setProviderId(const QString& id);
		QString getProviderId() const;
		Dialog& setModelId(const QString& id);
		QString getModelId() const;
		Dialog& setCapabilityRequirement(Model::Capabilities required);
		Model::Capabilities getCapabilityRequirement() const;
		void run();
	public:
		Visindigo::Utility::JsonConfig toJson() const;
		bool fromJson(const Visindigo::Utility::JsonConfig& json);
	signals:
		void messageAppended(qint32 index);
		void streamBegan();
		void streamDelta(const QString& delta);
		void streamReasoning(const QString& delta);
		void streamEnded(const QString& fullContent);
		void runFailed(const QString& message);
	private:
		DialogPrivate* d;
	};
}
#endif // Visindigo_Agent_Dialog_h
