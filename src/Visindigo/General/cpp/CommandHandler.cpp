#include "General/CommandHandler.h"
#include "General/CommandHost.h"
#include "General/Exception.h"
namespace Visindigo::General {
	class CommandErrorDataPrivate {
		friend class CommandErrorData;
	protected:
		CommandErrorData::ErrorType Type;
		QString Message;
		QString Command;
		qsizetype ErrorPosition;
	};

	/*!
		\class Visindigo::General::CommandErrorData
		\inheaderfile General/CommandHandler.h
		\brief 此类表示命令执行过程中可能发生的错误信息。
		\since Visindigo 0.13.0
		\inmodule Visindigo
		
		命令错误数据类用于封装命令执行过程中产生的错误信息，包括错误类型、错误消息、相关命令和错误位置等。

		\note 这类的内容是不可变的，一旦创建，其内容就不能被修改。
	*/

	/*!
		\since Visindigo 0.13.0
		构造函数。此构造函数适用于从boolean值隐式转换的情况。为true时表示无错误，为false时表示未知错误。

		这构造函数用于方便地从布尔值创建错误数据对象，简化了无错误和未知错误的表示。但一般来说
		仍然建议在有错误时采用更具体的构造函数来提供详细的错误信息。建议只将该构造函数用于无错误情况。
	*/
	CommandErrorData::CommandErrorData(bool noError)
		: d(new CommandErrorDataPrivate()) {
		if (noError) {
			d->Type = NoError;
		}
		else {
			d->Type = UnknownError;
		}
		d->Message = QString();
		d->Command = QString();
		d->ErrorPosition = -1;
	}

	/*!
		\since Visindigo 0.13.0
		构造函数。创建一个指定错误类型的命令错误数据对象。
		\a type 指定错误的类型。
	*/
	CommandErrorData::CommandErrorData(CommandErrorData::ErrorType type)
		: d(new CommandErrorDataPrivate()) {
		d->Type = type;
		d->Message = QString();
		d->Command = QString();
		d->ErrorPosition = -1;
	}

	/*!
		\since Visindigo 0.13.0
		构造函数。创建一个完整的命令错误数据对象。
		\a type 指定错误的类型。
		\a message 指定错误的消息描述。
		\a command 指定相关的命令字符串。
		\a errorPosition 指定错误发生的位置索引。
	*/
	CommandErrorData::CommandErrorData(CommandErrorData::ErrorType type, const QString& message, const QString& command, qsizetype errorPosition)
		: d(new CommandErrorDataPrivate()) {
		d->Type = type;
		d->Message = message;
		d->Command = command;
		d->ErrorPosition = errorPosition;

	}

	/*!
		\fn  Visindigo::General::CommandErrorData::CommandErrorData(CommandErrorData&& other)
		\since Visindigo 0.13.0
		移动构造函数。将另一个命令错误数据对象的内容移动到当前对象中。
	*/

	/*!
		\fn  Visindigo::General::CommandErrorData::CommandErrorData(const CommandErrorData& other)
		\since Visindigo 0.13.0
		复制构造函数。将另一个命令错误数据对象的内容复制到当前对象中。
	*/

	/*!
		\fn Visindigo::General::CommandErrorData::operator=(CommandErrorData&& other)
		\since Visindigo 0.13.0
		移动赋值运算符。将另一个命令错误数据对象的内容移动到当前对象中。
	*/

	/*!
		\fn Visindigo::General::CommandErrorData::operator=(const CommandErrorData& other)
		\since Visindigo 0.13.0
		复制赋值运算符。将另一个命令错误数据对象的内容复制到当前对象中。
	*/
	VIMoveable_Impl(CommandErrorData);
	VICopyable_Impl(CommandErrorData);

	/*!
		\since Visindigo 0.13.0
		析构函数。销毁命令错误数据对象并释放相关资源。
	*/
	CommandErrorData::~CommandErrorData() {
		delete d;
	}

	/*!
		\since Visindigo 0.13.0
		获取错误的类型。
		return 错误的类型枚举值。
	*/
	CommandErrorData::ErrorType CommandErrorData::getErrorType() const {
		return d->Type;
	}

	/*!
		\since Visindigo 0.13.0
		获取错误的消息描述。
		return 错误的消息字符串。
	*/
	QString CommandErrorData::getErrorMessage() const {
		return d->Message;
	}

	/*!
		\since Visindigo 0.13.0
		获取相关的命令字符串。
		return 相关的命令字符串。
	*/
	QString CommandErrorData::getCommand() const {
		return d->Command;
	}

	/*!
		\since Visindigo 0.13.0
		获取错误发生的位置索引。
		return 错误位置的索引值。
	*/
	qsizetype CommandErrorData::getErrorPosition() const {
		return d->ErrorPosition;
	}

	class CommandHandlerPrivate {
		friend class CommandHandler;
	protected:
		QString CommandName;
		QStringList Alias;
	};

	/*!
		\class Visindigo::General::CommandHandler
		\inheaderfile General/CommandHandler.h
		\brief 此类提供了一个命令处理程序的接口，用于处理和补全命令行输入。
		\since Visindigo 0.13.0
		\inmodule Visindigo

		命令处理程序用于定义和管理命令行接口中的命令。每个命令处理程序关联一个命令名称和可选的别名列表。
		用户可以通过实现onCommand 方法来定义命令的行为，并通过onComplete 方法提供命令补全功能。

		值得注意的是，在enable之后，setCommandName 和 setAlias 方法将不再生效。
	*/

	/*!
		\since Visindigo 0.13.0
		构造函数。创建一个命令处理程序对象，并设置其命令名称和可选的别名列表。
		\a commandName 指定命令的名称，不能为空且不能包含空格。
		\a alias 指定命令的别名列表，默认为空列表。别名不能为空且不能包含空格。
	*/
	CommandHandler::CommandHandler(Plugin* parent, const QString& moduleID, const QString& commandName, const QStringList& alias) :
		PluginModule(parent, moduleID, VIModuleType_CommandHandler, commandName)
	{
		d = new CommandHandlerPrivate();
		if (commandName.isEmpty()) {
			setCommandName(moduleID);
		}
		else {
			setCommandName(commandName);
		}
		setAlias(alias);
	}

	/*!
		\since Visindigo 0.13.0
		设置命令的名称。
		\a commandName 指定命令的名称，不能为空且不能包含空格。

		此函数只能在命令处理程序未启用时调用。一旦命令处理程序通过enable 方法启用，
		命令名称将无法更改。如果尝试在启用后调用此函数，将不会有任何效果。

		此函数在名称无效时会明确抛出Visindigo::General::Exception::Type::InvalidArgument 异常。
	*/
	void CommandHandler::setCommandName(const QString& commandName) {
		if (VISCH->isCommandRegistered(this)) {
			return;
		}
		if (commandName.isEmpty() || commandName.contains(' ')) {
			VI_Throw_ST(Exception::InvalidArgument, "Command name cannot be empty or contain spaces.");
		}
		d->CommandName = commandName;
	}

	/*!
		\since Visindigo 0.13.0
		获取命令的名称。
		return 命令的名称字符串。
	*/
	QString CommandHandler::getCommandName() const {
		return d->CommandName;
	}

	/*!
		\since Visindigo 0.13.0
		设置命令的别名列表。
		\a alias 指定命令的别名列表，不能为空且不能包含空格。

		此函数只能在命令处理程序未启用时调用。一旦命令处理程序通过enable 方法启用，
		别名列表将无法更改。如果尝试在启用后调用此函数，将不会有任何效果。

		此函数在别名无效时会明确抛出Visindigo::General::Exception::Type::InvalidArgument 异常。
	*/
	void CommandHandler::setAlias(const QStringList& alias) {
		if (VISCH->isCommandRegistered(this)) {
			return;
		}
		for (const QString& a : alias) {
			if (a.isEmpty() || a.contains(' ')) {
				VI_Throw_ST(Exception::InvalidArgument, "Alias cannot be empty or contain spaces.");
			}
		}
		d->Alias = alias;
	}

	/*!
		\since Visindigo 0.13.0
		获取命令的别名列表。
		return 命令的别名字符串列表。
	*/
	QStringList CommandHandler::getAlias() const {
		return d->Alias;
	}

	/*!
		\since Visindigo 0.13.0
		启用命令处理程序，将其注册到命令主机中。
		此函数是直接调用Visindigo::General::CommandHost::registerCommand 实现的，请参考该函数的文档了解更多信息。
	*/
	void CommandHandler::enable() {
		VISCH->registerCommand(this);
	}

	/*!
		\since Visindigo 0.13.0
		禁用命令处理程序，将其从命令主机中注销。
		此函数是直接调用Visindigo::General::CommandHost::unregisterCommand 实现的，请参考该函数的文档了解更多信息。
	*/
	void CommandHandler::disable() {
		VISCH->unregisterCommand(this);
	}

	/*!
		\since Visindigo 0.13.0
		检查命令处理程序是否已启用。
		此函数是直接调用Visindigo::General::CommandHost::isCommandRegistered 实现的，请参考该函数的文档了解更多信息。
	*/
	bool CommandHandler::isEnabled() {
		return VISCH->isCommandRegistered(this);
	}

	/*!
		\fn virtual Visindigo::General::CommandErrorData Visindigo::General::CommandHandler::onCommand(const QString& entryName, const QStringList& unnamedArgs, const QMap<QString, QString>& namedArgs) = 0
		\since Visindigo 0.13.0
		命令执行的回调函数。此函数在命令被调用时执行。
		\a entryName 指定调用的命令名称。
		\a unnamedArgs 指定传递给命令的未命名参数列表。
		\a namedArgs 指定传递给命令的命名参数映射。
		此函数是一个纯虚函数，必须由派生类实现。实现此函数以定义命令的具体行为。
		返回值为命令执行结果的错误数据对象，表示命令执行是否成功以及可能的错误信息。
		return 命令执行结果的错误数据对象。
	*/

	/*!
		\since Visindigo 0.13.0
		命令补全的回调函数。此函数在需要提供命令补全建议时执行。
		\a entryName 指定调用的命令名称。
		\a unnamedArgs 指定传递给命令的未命名参数列表。
		\a namedArgs 指定传递给命令的命名参数映射。
		默认实现返回一个空的字符串列表。派生类可以重写此函数以提供具体的补全建议。
		return 命令补全建议的字符串列表。

		默认情况下，此函数返回一个空的字符串列表。派生类可以根据具体需求重写此函数。
	*/
	QStringList CommandHandler::onComplete(const QString& entryName, const QStringList& unnamedArgs, const QMap<QString, QString>& namedArgs) {
		return QStringList();
	}
}