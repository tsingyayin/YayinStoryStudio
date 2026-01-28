#include "General/CommandHandler.h"
#include "General/CommandHost.h"
#include "General/Exception.h"
namespace Visindigo::General {
	class CommandHandlerPrivate {
		friend class CommandHandler;
	protected:
		QString CommandName;
		QStringList Alias;
	};

	/*!
		\class Visindigo::General::CommandHandler
		\brief 此类提供了一个命令处理程序的接口，用于处理和补全命令行输入。
		\since Visindigo 0.13.0
		\inmodule Visindigo

		命令处理程序用于定义和管理命令行接口中的命令。每个命令处理程序关联一个命令名称和可选的别名列表。
		用户可以通过实现onCommand 方法来定义命令的行为，并通过onComplete 方法提供命令补全功能。

		值得注意的是，在enable之后，setCommandName 和 setAlias 方法将不再生效。
	*/
	CommandHandler::CommandHandler() {
		
	}

	CommandHandler::CommandHandler(const QString& commandName, const QStringList& alias) {
		d = new CommandHandlerPrivate();
		setCommandName(commandName);
		setAlias(alias);
	}

	void CommandHandler::setCommandName(const QString& commandName) {
		if (VISCH->isCommandRegistered(this)) {
			return;
		}
		if (commandName.isEmpty() || commandName.contains(' ')) {
			VI_Throw_ST(Exception::InvalidArgument, "Command name cannot be empty or contain spaces.");
		}
		d->CommandName = commandName;
	}

	QString CommandHandler::getCommandName() const {
		return d->CommandName;
	}

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

	QStringList CommandHandler::getAlias() const {
		return d->Alias;
	}

	void CommandHandler::enable() {
		VISCH->registerCommand(this);
	}

	void CommandHandler::disable() {
		VISCH->unregisterCommand(this);
	}

	bool CommandHandler::isEnabled() {
		return VISCH->isCommandRegistered(this);
	}

	QStringList CommandHandler::onComplete(const QString& entryName, const QStringList& unnamedArgs, const QMap<QString, QString>& namedArgs) {
		return QStringList();
	}
}