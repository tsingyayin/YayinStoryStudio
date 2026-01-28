#include "General/CommandHost.h"
#include "General/CommandHandler.h"
#include "General/Log.h"
#include "General/VIApplication.h"
#include "Utility/FileUtility.h"
namespace Visindigo::General {
	class CommandErrorDataPrivate {
		friend class CommandErrorData;
	protected:
		CommandErrorData::ErrorType Type;
		QString Message;
		QString Command;
		qsizetype ErrorPosition;
	};

	CommandErrorData::CommandErrorData(bool noError)
		: d(new CommandErrorDataPrivate()) {
		if (noError) {
			d->Type = NoError;
		} else {
			d->Type = UnknownError;
		}
		d->Message = QString();
		d->Command = QString();
		d->ErrorPosition = -1;
	}

	CommandErrorData::CommandErrorData(CommandErrorData::ErrorType type)
		: d(new CommandErrorDataPrivate()) {
		d->Type = type;
		d->Message = QString();
		d->Command = QString();
		d->ErrorPosition = -1;
	}

	CommandErrorData::CommandErrorData(CommandErrorData::ErrorType type, const QString& message, const QString& command, qsizetype errorPosition)
		: d(new CommandErrorDataPrivate()) {
		d->Type = type;
		d->Message = message;
		d->Command = command;
		d->ErrorPosition = errorPosition;
		
	}
	
	VIMoveable_Impl(CommandErrorData);
	VICopyable_Impl(CommandErrorData);

	CommandErrorData::~CommandErrorData() {
		delete d;
	}

	CommandErrorData::ErrorType CommandErrorData::getErrorType() const {
		return d->Type;
	}

	QString CommandErrorData::getErrorMessage() const {
		return d->Message;
	}

	QString CommandErrorData::getCommand() const {
		return d->Command;
	}

	qsizetype CommandErrorData::getErrorPosition() const {
		return d->ErrorPosition;
	}

	class CommandHostPrivate {
		friend class CommandHost;
	protected:
		bool saveCommandHistory = false;
		QString savedPath;
		QMap<QString, CommandHandler*> CommandMap; // main name -> handler
		QMap<QString, CommandHandler*> AliasMap; // main name & alias -> handler
		qsizetype MaxCommandHistorySize = 100;
		QStringList CommandHistory;
		static CommandHost* Instance;

		void parseCommand(const QString& command, QString* name, QStringList* unnamedArgs, QMap<QString, QString>* namedArgs) {
			QString withSpaceEnd = command.trimmed() + QChar(' ');
			QByteArray cmdBytes = withSpaceEnd.toUtf8();
			const char* ptr = cmdBytes.constData();
			qsizetype length = cmdBytes.size();
			bool inQuotes = false;
			qsizetype index = 0;
			for(index; index < length; ++index) {
				if (*ptr == ' ') {
					*name = QString::fromUtf8(cmdBytes.constData(), index);
					break;
				}
				ptr++;
			}
			ptr++; // skip space
			index++;
			qsizetype argStart = index;
			bool lastWasKey = false;
			QString currentKey;
			for (index; index < length; ++index) {
				if (*ptr == '\"') {
					const char* prevChar = ptr == cmdBytes.constData() ? ptr : ptr - 1;
					if (*prevChar != '\\') {
						inQuotes = !inQuotes;
					}
				} else if (*ptr == ' ' && !inQuotes) {
					if (argStart < index) {
						QString arg = QString::fromUtf8(cmdBytes.constData() + argStart, index - argStart);
						if (arg.startsWith('\"') && arg.endsWith('\"') && arg.size() >= 2) {
							arg = arg.mid(1, arg.size() - 2);
						}
						if (lastWasKey) {
							namedArgs->insert(currentKey, arg);
							lastWasKey = false;
							currentKey.clear();
						} else if (arg.startsWith('-')) {
							currentKey = arg.mid(1);
							lastWasKey = true;
						} else {
							unnamedArgs->append(arg);
						}
					}
					argStart = index + 1;
				}
				ptr++;
			}
		}
	};

	CommandHost* CommandHostPrivate::Instance = nullptr;
	/*!
		\class Visindigo::General::CommandHost
		\brief 此类提供了一个命令主机，用于注册和管理命令处理程序。
		\inheaderfile General/CommandHost.h
		\since Visindigo 0.13.0
		\inmodule Visindigo

		\note 这是一个从旧版Visindigo继承下来的功能（VICore/VICommand），但已经重新实现，因此行为和接口与旧版
		有所不同。建议用户参考本类的文档和示例代码，以了解如何使用新版本。

		命令主机是一个单例类，负责维护所有注册的命令处理程序，并提供执行命令的接口。
		
		\section1 命令格式
		Visindigo将命令格式分为两类：匿名参数命令和具名参数命令。
		匿名参数命令的格式为：
		\badcode
		commandName arg1 arg2 ...
		\endcode

		具名参数命令的格式为：
		\badcode
		commandName -key value -key value
		\endcode

		在具名参数命令中，参数以键值对的形式出现，键以短横线（-）开头，后跟对应的值。
		Visindigo不支持某些命令格式中的纯标志设置语法（如--flag），所有具名参数都必须有对应的值。

		Visindigo允许混用具名参数和匿名参数，也不要求参数的前后顺序，因为Visindigo这样解析命令：
		如果一个参数以短横线开头，则将其视为具名参数的键，后续的参数直到下一个空格之前都视为该键的值。
		而在任何值之后的参数都视为匿名参数，直到再次遇到可以视为键名（即以短横线开头）的参数为止。
		例如，在以下命令中：
		\badcode
		commandName -key1 value1 arg1 -key2 value2 arg2 arg3
		\endcode
		Visindigo会将-key1和value1解析为具名参数，将arg1解析为匿名参数，然后将-key2和value2解析为另一个具名参数，最后将arg2和arg3解析为匿名参数。

		不过，这样的写法可能会降低命令的可读性，我们仍然建议用户将具名参数和匿名参数分开书写，以提高命令的清晰度和可维护性。

		对于开发者而言，具名参数和匿名参数在CommandHandler的onCommand 和 onComplete 方法中分别通过namedArgs 和unnamedArgs 参数传递。
		它们分别对应于QMap<QString, QString> 类型的具名参数和QStringList 类型的匿名参数。因此匿名参数是有序的，而具名参数则是无序的。

		如果参数中要出现空格，则整个参数都必须用引号括起来。（这与Windows Batch不同，在bat中你可以只在空格处用引号，
		比如arg" "withspace，但在Visindigo中你必须使用"arg withspace"这种形式。），同样的，如果参数头部要出现短横线，
		也必须使用引号括起来以避免被误解析为具名参数的键名。要使用引号本身，请在引号前加反斜杠进行转义，例如：\"。

		\section1 命令名和别名
		Visindigo中的每个命令处理程序都有一个唯一的命令名（command name），用于标识该命令。
		此外，命令处理程序还可以有一个或多个别名（alias），这些别名也是用于标识该命令的有效名称。
		但是，所有命令名和别名在整个命令主机中必须是唯一的，不能重复注册。如果注册命令时发现
		命令名已被占用，则注册将失败，命令处理程序仍然保持未注册状态。
		
		但重复的别名不影响注册本身，只是后来者的别名将无法使用，因为命令主机会优先匹配最先注册的命令处理程序。
		
		\section1 命名建议
		Visindigo命令处理框架的灵感来源于Minecraft的命令系统，因此在命令命名方面，我们推荐遵循
		Minecraft插件社区大部分作品所采用的一种不成文的规则：
		\list
		\li 1. 命令名应和插件名（或功能模块名）相关联并尽量采取全名。如果插件名为TianyuAreaManager，
		则命令名应为tianyareamanager，而不是tam或tamgr
		\li 2. 命令名（包括别名和命令解析）应全部采用小写字母，也不需要任何连字符或下划线。
		\li 3. 命令名尽量避免使用缩写，除非该缩写已经被广泛接受和使用（例如，cfg代表config）。
		\li 4. 命令名缩写的形式，例如，假设AdvancedCfg插件的作者是Visindigo团队，
		则可以使用viac作为advancedcfg命令的别名。
		\endlist

		\section1 监听stdin和多线程
		Visindigo的命令主机默认并不监听标准输入（stdin），因此用户无法直接在控制台中输入命令。
		如果需要启用stdin监听功能，请参考setStdInListeningEnabled 方法的说明。

		除了stdin之外，在很多情况下命令执行可能需要在其他线程中进行或调用，而
		CommandHost只在主线程中执行命令处理程序的onCommand 和onComplete 方法，也只在
		主线程实现命令解析。不过为了方便起见，executeCommand方法被置为了槽函数，使得
		用户可以通过Qt的信号槽机制在其他线程中调用它来执行命令。但它本身仍然只在主线程中执行。

		\section1 命令提示和自动补全
		Visindigo的命令主机支持命令提示和自动补全功能，这些功能通过CommandHandler的onComplete 方法实现。
		onComplete 方法接受与onCommand 方法相同的参数，但返回一个QStringList，表示可能的补全选项。

		而调用者可以通过CommandHost的completeCommand 方法来获取补全选项。
		这对于实现命令行界面（CLI）或集成开发环境（IDE）中的命令补全功能非常有用。

		\section1 错误处理
		Visindigo的命令主机提供了基本的错误处理机制，通过CommandHost的getLastError 方法获取最后一次命令执行的错误信息。
		如果executeCommand 方法返回false，则可以调用getLastError 方法获取详细的错误信息，
		有关Visindigo::General::CommandErrorData 类的说明，请参考相应的文档。
	*/

	/*!
		\since Visindigo 0.13.0
		构造一个新的命令主机实例。
	*/
	CommandHost::CommandHost() {
		d = new CommandHostPrivate();
		d->saveCommandHistory = VIApp->getEnvConfig(VIApplication::SaveCommandHistory).toBool();
		d->savedPath = VIApp->getEnvConfig(VIApplication::LogFolderPath).toString() + "/command_history.log";
		if (d->saveCommandHistory) {
			d->CommandHistory = Visindigo::Utility::FileUtility::readLines(d->savedPath);
		}
	}

	/*!
		\since Visindigo 0.13.0
		销毁命令主机实例。
		没有任何情况需要手动调用此函数，它应与应用程序保持相同的生命周期。
	*/
	CommandHost::~CommandHost() {
		if (d->saveCommandHistory) {
			Visindigo::Utility::FileUtility::saveLines(d->savedPath, d->CommandHistory);
		}
		delete d;
	}

	/*!
		\since Visindigo 0.13.0
		获取命令主机的单例实例。
		如果实例尚未创建，则会创建一个新的实例。
		返回命令主机的单例实例指针。
	*/
	CommandHost* CommandHost::getInstance() {
		if (CommandHostPrivate::Instance == nullptr) {
			CommandHostPrivate::Instance = new CommandHost();
		}
		return CommandHostPrivate::Instance;
	}

	/*!
		\since Visindigo 0.13.0
		设置命令历史记录的最大大小。
		当命令历史记录超过此大小时，最旧的命令将被删除。
		\a size 命令历史记录的最大大小。
	*/
	void CommandHost::setMaxCommandHistorySize(qsizetype size) {
		d->MaxCommandHistorySize = size;
	}

	/*!
		\since Visindigo 0.13.0
		获取命令历史记录的最大大小。
	*/
	qsizetype CommandHost::getMaxCommandHistorySize() const {
		return d->MaxCommandHistorySize;
	}

	/*!
		\since Visindigo 0.13.0
		获取命令历史记录的列表。
		返回命令历史记录的字符串列表。
	*/
	QStringList CommandHost::getCommandHistory(const QString& startWith) const {
		if (startWith.isEmpty()) {
			return d->CommandHistory;
		}
		else {
			QStringList result;
			for (const QString& cmd : d->CommandHistory) {
				if (cmd.startsWith(startWith)) {
					result.append(cmd);
				}
			}
			return result;
		}
	}

	/*!
		\since Visindigo 0.13.0
		获取最后执行的命令。
		如果没有命令被执行过，则返回一个空字符串。
		返回最后执行的命令字符串。
	*/
	QString CommandHost::getLastCommand() const {
		if (d->CommandHistory.isEmpty()) {
			return QString();
		}
		return d->CommandHistory.last();
	}

	/*!
		\since Visindigo 0.13.0
		注册一个命令处理程序。
		\a handler 要注册的命令处理程序指针。
		如果命令处理程序的命令名已被占用，则注册将失败，命令处理程序仍然保持未注册状态。
		各CommandHandler的enable方法本质是调用此函数。
	*/
	void CommandHost::registerCommand(CommandHandler* handler) {
		if (handler == nullptr) {
			return;
		}
		QString commandName = handler->getCommandName();
		if (commandName.isEmpty()) {
			return;
		}
		if (d->CommandMap.contains(commandName)) {
			return;
		}
		d->CommandMap.insert(commandName, handler);
		d->AliasMap.insert(commandName, handler);
		QStringList aliasList;
		for (const QString& alias : handler->getAlias()) {
			if (!alias.isEmpty() && !d->AliasMap.contains(alias)) {
				d->AliasMap.insert(alias, handler);
				aliasList.append(alias);
			}
		}
		vgSuccessF << "Registered command:" << commandName << "with aliases:" << aliasList;
	}

	/*!
		\since Visindigo 0.13.0
		注销一个命令处理程序。
		\a handler 要注销的命令处理程序指针。
		各CommandHandler的disable方法本质是调用此函数。
	*/
	void CommandHost::unregisterCommand(CommandHandler* handler) {
		if (handler == nullptr) {
			return;
		}
		QString commandName = handler->getCommandName();
		if (commandName.isEmpty()) {
			return;
		}
		d->CommandMap.remove(commandName);
		d->AliasMap.remove(commandName);
		for (const QString& alias : handler->getAlias()) {
			d->AliasMap.remove(alias);
		}
	}

	/*!
		\since Visindigo 0.13.0
		检查一个命令处理程序是否已注册。
		\a handler 要检查的命令处理程序指针。
		如果命令处理程序已注册，则返回true；否则返回false。
		各CommandHandler的isEnabled方法本质是调用此函数。
	*/
	bool CommandHost::isCommandRegistered(CommandHandler* handler) {
		if (handler == nullptr) {
			return false;
		}
		QString commandName = handler->getCommandName();
		if (commandName.isEmpty()) {
			return false;
		}
		return d->CommandMap.value(commandName, nullptr) == handler;
	}

	/*!
		\since Visindigo 0.13.0
		根据命令名获取对应的命令处理程序。
		\a commandName 要查找的命令名字符串。
		如果找到对应的命令处理程序，则返回其指针；否则返回nullptr。
	*/
	CommandHandler* CommandHost::getCommandHandler(const QString& commandName) {
		return d->AliasMap.value(commandName, nullptr);
	}

	/*!
		\since Visindigo 0.13.0
		执行一个命令行字符串。
		\a commandLine 要执行的命令行字符串。
		此函数为了便于跨线程调用而置为槽函数，因此没有返回值。
		如果命令执行出现问题，则会通过errorOccurred 信号发出错误信息。
	*/
	void CommandHost::executeCommand(const QString& commandLine) {
		vgDebug << "Executing command:" << commandLine;
		QString cmdName = QString();
		QStringList unnamedArgs;
		QMap<QString, QString> namedArgs;
		d->parseCommand(commandLine, &cmdName, &unnamedArgs, &namedArgs);
		vgDebug << "Parsed command name:" << cmdName << "unnamed args:" << unnamedArgs << "named args:" << namedArgs;
		CommandHandler* handler = d->AliasMap.value(cmdName, nullptr);
		if (handler == nullptr) {
			vgError << "Failed to execute command, command not found:" << commandLine;
			emit errorOccurred(CommandErrorData(CommandErrorData::ErrorType::UnknownCommand, QString("Command not found: %1").arg(cmdName), commandLine, 0));
			d->CommandHistory.append(commandLine);
			if (d->CommandHistory.size() > d->MaxCommandHistorySize) {
				d->CommandHistory.removeFirst();
			}
			return;
		}
		CommandErrorData errorData = handler->onCommand(cmdName, unnamedArgs, namedArgs);
		if (errorData.getErrorType() != CommandErrorData::ErrorType::NoError) {
			vgError << "Command execution failed:" << commandLine << "\nError:" << errorData.getErrorMessage();
			emit errorOccurred(errorData);
		}
		d->CommandHistory.append(commandLine);
		if (d->CommandHistory.size() > d->MaxCommandHistorySize) {
			d->CommandHistory.removeFirst();
		}
	}

	/*!
		\since Visindigo 0.13.0
		启用或禁用标准输入（stdin）监听功能。
		\a enabled 如果为true，则启用stdin监听；如果为false，则禁用stdin监听。
	*/
	void CommandHost::setStdInListeningEnabled(bool enabled) {
		// TODO
	}

	/*!
		\since Visindigo 0.13.0
		获取所有已注册的命令名列表。
		返回已注册命令名的字符串列表。
	*/
	QStringList CommandHost::getRegisteredCommandNames() const {
		return d->CommandMap.keys();
	}

	/*!
		\since Visindigo 0.13.0
		根据部分命令行字符串获取可能的命令补全选项。
		\a commandLine 部分命令行字符串。
		返回可能的补全选项字符串列表。
	*/
	QStringList CommandHost::completeCommand(const QString& commandLine) {
		QString cmdName = QString();
		QStringList unnamedArgs;
		QMap<QString, QString> namedArgs;
		d->parseCommand(commandLine, &cmdName, &unnamedArgs, &namedArgs);
		CommandHandler* handler = d->AliasMap.value(cmdName, nullptr);
		if (handler == nullptr) {
			return QStringList();
		}
		return handler->onComplete(commandLine, unnamedArgs, namedArgs);
	}
}