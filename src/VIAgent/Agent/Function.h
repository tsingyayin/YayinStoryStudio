#ifndef Visindigo_Agent_Function_h
#define Visindigo_Agent_Function_h
#include <functional>
#include <QtCore/qstring.h>
#include "VIAgentCompileMacro.h"
// Forward declarations
namespace Visindigo::Utility {
	class JsonConfig;
}
// Main
namespace Visindigo::Agent {
	class VIAgentAPI Function {
	public:
		virtual ~Function();
	public:
		// 唯一标识，同时作为发给模型的工具名；须匹配 ^[a-zA-Z0-9_-]{1,64}$
		virtual QString getId() const = 0;
		virtual QString getName() const = 0;
		virtual QString getDescription() const = 0;
		virtual Visindigo::Utility::JsonConfig getParameterSchema() const = 0;
		virtual Visindigo::Utility::JsonConfig invoke(const Visindigo::Utility::JsonConfig& arguments) = 0;
		virtual void invokeAsync(const Visindigo::Utility::JsonConfig& arguments,
			std::function<void(const Visindigo::Utility::JsonConfig&)> callback);
	};
}
#endif // Visindigo_Agent_Function_h
