#include "../DebugServer.h"
#include "Editor/EditorPlugin.h"
namespace YSSCore::Editor {
	class DebugServerPrivate {
		friend class DebugServer;
	protected:
		DebugServer::SupportedDebugFeature Feature = DebugServer::SupportedDebugFeatureFlag::Unknown;
	};

	/*!
		\class YSSCore::Editor::DebugServer
		\brief 这个类提供对外部程序调试的接口。插件可以通过继承这个类来实现对特定类型程序的调试支持。
		\since YSS 0.13.0
		\inmodule YSSCore

		DebugServer类提供了一系列虚函数，插件可以重写这些函数来实现具体的调试功能，
		例如构建、运行、单步调试等。插件还可以通过信号与编辑器进行通信，
		例如在调试过程中发送进度更新或请求编辑器聚焦到特定的代码行。

		YSS不关心这个类打算如何实现这些功能，也不关心这个类是否打算启动一个独立线程
		或进程来处理调试任务。插件可以根据自己的需要来设计DebugServer的实现细节。
		不过考虑到实现这个需求理应需要跨线程或跨进程，因此DebugServer的虚函数都
		被设置为void。如果要通知程序当前调试步骤的变化，可以通过信号来实现。

		为了避免造成歧义，每个调试功能对应的虚函数都会详细解释YSSCore认为的
		该功能应该具有的行为，插件在实现这些函数时，应该尽量遵守这些要求。

		但这里有个例外是获取断点的getBreakpoints函数，他要求立即返回断点
		的相关信息。同样是考虑到DebugServer的实现比较自由，因此没有为它提供
		默认实现，DebugServer本体也不负责保存断点信息。插件需要自己实现断点信息的保存和管理，
		并在getBreakpoints函数中返回当前的断点信息。

		\warning 由于断点相关功能涉及到数据交互和持久化，因此有严格的实现要求，请
		务必仔细阅读setBreakpoint、getBreakpoints和setBreakpoints三个函数的文档说明。

		此外，值得指出的是，这里看上去并不存在一个用于根据程序执行情况来切换编辑器
		光标的信号（你不应该滥用breakpointHited信号来实现这个功能），因为这个
		功能已经在YSSCore::Editor::FileServerManager::focusOnFile中提供。

		
		\note 虽然这类从0.13.0开始提供，但文档从0.15.0开始提供，因此没有记录中途变化历史，
		不能根据此文档使用0.13和0.14中的此类。
	*/

	/*!
		\since YSS 0.13.0
		\enum YSSCore::Editor::DebugServer::SupportedDebugFeatureFlag
		\value Unknown 不支持任何调试功能。
		\value Build 支持构建功能。
		\value Rebuild 支持全部重新构建功能。
		\value Clean 支持清理构建产物功能。
		\value DebugRun 支持以调试模式运行程序功能。
		\value Run 支持以普通模式运行程序功能。
		\value StepInto 支持单步进入功能。
		\value StepOver 支持单步跳过功能。
		\value Suspend 支持暂停功能。
		\value Continue 支持继续功能。
		\value Stop 支持停止功能。
		\value Breakpoint 支持断点功能。
	*/

	/*!
		\typedef YSSCore::Editor::DebugServer::SupportedDebugFeatureFlag YSSCore::Editor::DebugServer::DebugAction
		\since YSS 0.15.0
		将SupportedDebugFeatureFlag赋予别名为DebugAction，以更好地表达它在信号中的含义。

		作为DebugAction使用时，表示正在执行的普遍耗时调试操作，例如构建、清理、运行等。

		有些操作可能只在意外情况下才耗时，例如单步调试时，绝大多数时候都是瞬时完成的，
		但偶尔可能会因为某些原因而变得非常慢，对于那些正常情况下不应耗时的操作，
		YSS约定也应该使用DebugAction来表示它们，以便在它们偶尔变得耗时的时候，编辑器也能正确地响应。
	*/

	/*!
		\fn YSSCore::Editor::DebugServer::actionStarted(DebugAction action)
		\since YSS 0.15.0
		当一个调试操作开始时发出此信号。理应只有on开始的函数才应该发出对应的信号。

		\a action 正在执行的调试操作，例如构建、运行、单步调试等。
	*/

	/*!
		\fn YSSCore::Editor::DebugServer::actionPercent(DebugAction action, qint32 finished, qint32 total)
		\since YSS 0.15.0
		当一个调试操作有进度更新时发出此信号。理应只有on开始的函数才应该发出对应的信号。

		\a action 正在执行的调试操作，\a finished 已经完成的部分，\a total 总的部分。

		finished和total的具体含义取决于正在执行的调试操作，由实现自行决定，YSS
		使用这个数据绘制进度条。
	*/

	/*!
		\fn YSSCore::Editor::DebugServer::actionMessage(DebugAction action, const QString& message)
		\since YSS 0.15.0
		当一个调试操作有消息更新时发出此信号。理应只有on开始的函数才应该发出对应的信号。
		\a action 正在执行的调试操作，\a message 消息内容。

		message的具体含义取决于正在执行的调试操作，由实现自行决定，YSS
		使用这个数据在输出窗口中输出。
	*/

	/*!
		\fn YSSCore::Editor::DebugServer::actionFinished(DebugAction action, bool success)
		\since YSS 0.15.0
		当一个调试操作完成时发出此信号。理应只有on开始的函数才应该发出对应的信号。
		\a action 正在执行的调试操作，\a success 调试操作是否成功完成。

		如果一个步骤最终未能顺利执行完毕，则应该在适当的时候发出这个信号，
		并将\a success设置为false，以通知编辑器这个步骤未能成功完成。

		请注意，YSS约定，如果收到了\a success为false的该信号，则将
		从actionMessage收到的最后一条信息视作错误信息，并以
		一定策略显示出来。
	*/

	/*!
		\fn YSSCore::Editor::DebugServer::breakpointChanged(const QString& filePath, qint32 lineNumber, bool enabled)
		\since YSS 0.15.0
		当一个断点被添加、删除时发出此信号。有关何时应该发出此信号，请参见setBreakpoint和setBreakpoints函数的文档说明。
		\a filePath 断点所在的文件路径，\a lineNumber 断点所在的行号，\a enabled 断点是否启用。

		由于写入此类的文件路径理应都是绝对路径，因此期望这个函数的\a filePath也是绝对路径。

		此外，YSS约定，行号从0开始计数。
	*/

	/*!
		\fn YSSCore::Editor::DebugServer::breakpointHited(const QString& filePath, qint32 lineNumber)
		\since YSS 0.15.0
		当程序执行过程中命中了一个断点时发出此信号。
		\a filePath 断点所在的文件路径，\a lineNumber 断点所在的行号。

		由于写入此类的文件路径理应都是绝对路径，因此期望这个函数的\a filePath也是绝对路径。

		此外，YSS约定，行号从0开始计数。
	*/

	/*!
		\since YSS 0.13.0
		构造函数
		\a name 模块名称，\a id 模块ID，\a plugin 所属插件。
	*/
	DebugServer::DebugServer(const QString& name, const QString& id, EditorPlugin* plugin)
		: Visindigo::General::PluginModule((Visindigo::General::Plugin*)plugin, id, YSSPluginModule_DebugServer, name ) {
		d = new DebugServerPrivate();
	}

	/*!
		\since YSS 0.13.0
		析构函数
	*/
	DebugServer::~DebugServer() {
		delete d;
	}

	/*!
		\since YSS 0.13.0
		设置这个DebugServer支持的调试功能。
	*/
	void DebugServer::setSupportedFeatures(SupportedDebugFeature features) {
		d->Feature = features;
	}

	/*!
		\since YSS 0.13.0
		获取这个DebugServer支持的调试功能。
	*/
	DebugServer::SupportedDebugFeature DebugServer::getSupportedFeatures() {
		return d->Feature;
	}

	/*!
		\since YSS 0.13.0
		当构建操作开始时调用此函数。插件可以重写此函数来实现具体的构建功能。
		\a rebuild 是否是全部重新构建。

		对YSS而言，构建指的是，通过一定步骤将项目已有内容进行转换，
		以进行后续操作的过程。从“已有内容”到“后续操作”间的这个“构建”
		步骤理应是无法被跳过的。

		为了性能考虑，构建操作并不总是一定要通过处理整个项目进行构建，
		这个时候\a rebuild即是false，指示实现可以根据需要有选择的进行构建。
		相反，如果\a rebuild是true，则指示实现需要进行全部重新构建。

		实现应首先保证全部重新构建的功能完整实现，然后再考虑在\a rebuild为false时的优化实现。

		构建开始时，发出actionStarted信号，构建过程中根据需要发出actionPercent和actionMessage
		信号，构建完成时发出actionFinished信号。
	*/
	void DebugServer::onBuild(bool rebuild) {
	
	}

	/*!
		\since YSS 0.13.0
		当清理构建产物操作开始时调用此函数。插件可以重写此函数来实现具体的清理构建产物功能。
		\a includeTarget 是否包括输出文件。

		这个函数被调用时，如果\a includeTarget为false，则只应该清理
		构建过程中产生的中间文件，而不应该清理输出文件。相反，如果\a includeTarget为true，
		则应该清理包括输出文件在内的所有构建产物。

		实现应首先保证包括输出文件在内的全部清理功能完整实现，然后再考虑在\a includeTarget为false时的优化实现。

		清理开始时，发出actionStarted信号，清理过程中根据需要发出actionPercent和actionMessage
		信号，清理完成时发出actionFinished信号。
	*/
	void DebugServer::onClean(bool includeTarget) {
	
	}

	/*!
		\since YSS 0.13.0
		当运行程序操作开始时调用此函数。插件可以重写此函数来实现具体的运行程序功能。
		\a debug 是否以调试模式运行。

		YSS不要求优先实现带有调试的运行。对于调试功能的具体实现视实际情况而定。

		值得指出的常见误区是，对于程序编译而言，调试模式运行不等于Debug方案编译，
		Release编译的程序同样可以以调试模式运行，反之亦然。因此这个函数的实现不应该
		将其连带视作对编译方案的要求，而应该仅仅关注于运行时的调试功能。

		运行开始时，发出actionStarted信号，运行完成时发出actionFinished信号。

		运行过程中，可以通过targetStdOutput和targetStdError信号来输出目标的标准输出和标准错误。

		如果运行有错，可以通过actionMessage信号来输出错误信息，并在适当的时候发出actionFinished信号，且将success设置为false。

		YSS不处理DebugAction为Run或DebugRun时的actionPercent信号。
	*/
	void DebugServer::onRun(bool debug) {
	
	}

	/*!
		\since YSS 0.13.0
		当单步进入操作开始时调用此函数。插件可以重写此函数来实现具体的单步进入功能。

		单步操作开始时，发出actionStarted信号，单步操作完成时发出actionFinished信号。

		如果单步操作有错，可以通过actionMessage信号来输出错误信息，并在适当的时候发出actionFinished信号，且将success设置为false。

		YSS不处理DebugAction为StepInto时的actionPercent信号。
	*/
	void DebugServer::onStepInto() {
	
	}

	/*!
		\since YSS 0.13.0
		当单步跳过操作开始时调用此函数。插件可以重写此函数来实现具体的单步跳过功能。

		单步操作开始时，发出actionStarted信号，单步操作完成时发出actionFinished信号。

		如果单步操作有错，可以通过actionMessage信号来输出错误信息，并在适当的时候发出actionFinished信号，且将success设置为false。

		YSS不处理DebugAction为StepOver时的actionPercent信号。
	*/
	void DebugServer::onStepOver() {
	
	}

	/*!
		\since YSS 0.13.0
		当暂停操作开始时调用此函数。插件可以重写此函数来实现具体的暂停功能。

		暂停操作开始时，发出actionStarted信号，暂停操作完成时发出actionFinished信号。

		如果暂停操作有错，可以通过actionMessage信号来输出错误信息，并在适当的时候发出actionFinished信号，且将success设置为false。

		YSS不处理DebugAction为Suspend时的actionPercent信号。
	*/
	void DebugServer::onSuspend() {
	
	}

	/*!
		\since YSS 0.13.0
		当继续操作开始时调用此函数。插件可以重写此函数来实现具体的继续功能。

		继续操作开始时，发出actionStarted信号，继续操作完成时发出actionFinished信号。

		如果继续操作有错，可以通过actionMessage信号来输出错误信息，并在适当的时候发出actionFinished信号，且将success设置为false。

		YSS不处理DebugAction为Continue时的actionPercent信号。
	*/
	void DebugServer::onContinue() {
	
	}

	/*!
		\since YSS 0.13.0
		当停止操作开始时调用此函数。插件可以重写此函数来实现具体的停止功能。

		停止操作开始时，发出actionStarted信号，停止操作完成时发出actionFinished信号。

		如果停止操作有错，可以通过actionMessage信号来输出错误信息，并在适当的时候发出actionFinished信号，且将success设置为false。

		YSS不处理DebugAction为Stop时的actionPercent信号。
	*/
	void DebugServer::onStop() {
	
	}

	/*!
		\since YSS 0.13.0
		设置断点。插件可以重写此函数来实现具体的设置断点功能。
		\a filePath 断点所在的文件路径，\a lineNumber 断点所在的行号，\a enabled 断点是否启用。
		YSS约定，行号从0开始计数。
		当一个断点被添加、删除时，应该发出breakpointChanged信号，以通知编辑器断点信息的变化。

		传入这个函数的文件路径理应是绝对路径。
	*/
	void DebugServer::setBreakpoint(const QString& filePath, qint32 lineNumber, bool enabled) {
	
	}

	/*!
		\since YSS 0.15.0
		获取断点信息。插件可以重写此函数来实现具体的获取断点信息功能。
		返回一个QMap，键是文件路径，值是该文件中所有断点所在的行号列表。
		YSS约定，行号从0开始计数。

		由于setBreakpoint时使用的理应是绝对路径，因此期望这里返回的也是绝对路径。
		
		YSS会在保存项目时，通过调用此函数保存断点信息。不过在保存时，会
		自动根据文件是项目内文件还是项目外文件，来将路径转换为相对路径或绝对路径
		存储，避免项目移动后丢失断点信息。
	*/
	QMap<QString, QList<qint32>> DebugServer::getBreakpoints() {
		return QMap<QString, QList<qint32>>();
	}

	/*!
		\since YSS 0.15.0
		设置断点信息。插件可以重写此函数来实现具体的设置断点信息功能。
		\a breakpoints 断点信息，键是文件路径，值是该文件中所有断点所在的行号列表。

		这个函数传入的路径理应是绝对路径。

		对于传入的断点信息，插件应该将其保存起来，并对其中的每一条断点信息，
		发出breakpointChanged信号，以通知编辑器断点信息的变化。
		
		YSS会在加载项目时，通过调用此函数加载断点信息。在加载时，会自动将
		所有路径转换为绝对路径。

		此外，YSS约定，行号从0开始计数。
	*/
	void DebugServer::setBreakpoints(const QMap<QString, QList<qint32>>& breakpoints) {
	
	}

	/*!
		\since YSS 0.15.0
		YSS通过此函数将一些输入发送给正在调试的程序。插件可以重写此函数来实现具体的向目标程序发送输入的功能。
		\a input 发送的输入内容。

	*/
	void DebugServer::writeTargetStdInput(const QString& input) {
	
	}
}