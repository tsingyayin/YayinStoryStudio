#ifndef Visindigo_Agent_private_Dialog_p_h
#define Visindigo_Agent_private_Dialog_p_h
#include <QtCore/qbytearray.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qstring.h>
#include "Agent/Dialog.h"
#include "Agent/MCP.h"
#include "Agent/Model.h"
#include "Agent/Prompt.h"
#include "Agent/Skill.h"
namespace Visindigo::Utility {
	class JsonConfig;
}
namespace Visindigo::Network {
	class HttpResponse;
	class SSEReply;
	struct StreamFrame;
}
namespace Visindigo::Agent {
	class Function;

	class MessagePrivate {
		friend class Message;
	public:
		Message::Role RoleName = Message::Role::User;
		QString Content;
		QString Name;
		QString ToolCallID;
		QString ToolCalls;
		QString Reasoning;
	};

	class DialogPrivate {
		friend class Dialog;
	public:
		// 流式响应中同一个工具调用的分片累加器
		class ToolCallBuilder {
		public:
			QString Id;
			QString Name;
			QString Arguments;
		};
		// 单次执行允许的工具往返上限，用于挡住模型反复调用工具的失控情况
		static constexpr qint32 MaxToolRounds = 8;
	public:
		Dialog* q = nullptr;
		QString Id;
		QString BranchFromId;
		QString ProviderId;
		QString ModelId;
		Model::Capabilities CapabilityRequirement = Model::Capability::Text;
		QList<Message> Messages;
		QList<Skill> Skills;
		QList<MCP> MCPs;
		QList<Prompt> Prompts;
		QMap<QString, Function*> Functions;
		bool Streaming = false;
		QString StreamingBuffer;
		QString ReasoningBuffer;
		Visindigo::Network::SSEReply* PendingReply = nullptr;
		QMap<qint32, ToolCallBuilder> ToolCalls;
		QString FinishReason;
		Model ActiveModel;
		QString ActiveModelId;
		qint32 ToolRound = 0;
	public:
		void startRun();
		bool handleFrame(const Visindigo::Network::StreamFrame& frame);
		void completeRun(const Visindigo::Network::HttpResponse& response);
		void runToolCalls(const QList<ToolCallBuilder>& calls);
		void failRun(const QString& message);
		void abortPendingReply();
		void buildRequestBody(Visindigo::Utility::JsonConfig& body) const;
		void buildTools(Visindigo::Utility::JsonConfig& body) const;
		QList<Skill> GlobalSkills() const;
		QString buildSystemInstruction() const;
		Function* resolveFunction(const QString& id) const;
	};
}
#endif // Visindigo_Agent_private_Dialog_p_h
