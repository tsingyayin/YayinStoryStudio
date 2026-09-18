#include "Editor/MainEditor/DocumentMessageTracer.h"
#include "Editor/MainEditor/MessageCenter.h"

/*!
	\class YSS::Editor::DocumentMessageTracer
	\brief 跟踪某一行消息所在行号的变化，并记下这一行的消息数量。
	\since YSS 0.17.0
	\inmodule YayinStoryStudio

	它挂在“带消息的那一行”对应的文本块上，所以行被插入、删除或合并时它会跟着走，
	不需要任何搬运。当文档结构变化、行号不再一致时，它会在被框架同步时发出 lineChanged，
	界面据此只更新受影响的那些行，而不必重建整个消息列表。

	它同时是本行消息数量的存放处：消息层按行号增删消息时同步维护这里的三个计数，
	所以总数是加减出来的，行被移到别的行号上也不会影响统计，不需要重新数一遍全文档。
	它随块被释放时会在析构里通知消息层，把本行贡献的计数从总数里扣掉。
*/

/*!
	\since YSS 0.17.0
	构造函数。
*/
YSS::Editor::DocumentMessageTracer::DocumentMessageTracer() {
}

/*!
	\since YSS 0.17.0
	析构函数。
*/
YSS::Editor::DocumentMessageTracer::~DocumentMessageTracer() {
	MessageCenter::getInstance()->onTracerDestroyed(this);
}

/*!
	\since YSS 0.17.0
	\return 界面上当前显示的行号。未显示时返回 -1。
*/
qint32 YSS::Editor::DocumentMessageTracer::getShownLine() const {
	return ShownLine;
}

/*!
	\since YSS 0.17.0
	\a line 界面上正在显示的行号。

	之后的位移判断都以它为基准。
*/
void YSS::Editor::DocumentMessageTracer::setShownLine(qint32 line) {
	ShownLine = line;
}

/*!
	\since YSS 0.17.0
	\return 这一行上错误消息的数量。
*/
qint32 YSS::Editor::DocumentMessageTracer::getErrorCount() const {
	return ErrorCount;
}

/*!
	\since YSS 0.17.0
	\return 这一行上警告消息的数量。
*/
qint32 YSS::Editor::DocumentMessageTracer::getWarningCount() const {
	return WarningCount;
}

/*!
	\since YSS 0.17.0
	\return 这一行上信息消息的数量。
*/
qint32 YSS::Editor::DocumentMessageTracer::getInfoCount() const {
	return InfoCount;
}

/*!
	\since YSS 0.17.0
	\a error 这一行上错误消息的数量。
	\a warning 这一行上警告消息的数量。
	\a info 这一行上信息消息的数量。

	只应由消息层在重新统计完这一行之后调用。
*/
void YSS::Editor::DocumentMessageTracer::setMessageCount(qint32 error, qint32 warning, qint32 info) {
	ErrorCount = error;
	WarningCount = warning;
	InfoCount = info;
}

/*!
	\since YSS 0.17.0
	\a newBlockNumber 该行现在所在的行号。

	框架发现行号与上次记录的不同时调用它，它会把新行号记下来并发出 lineChanged。
*/
void YSS::Editor::DocumentMessageTracer::onBlockNumberChanged(qint32 newBlockNumber) {
	qint32 oldLine = ShownLine;
	ShownLine = newBlockNumber;
	if (oldLine != newBlockNumber) {
		emit lineChanged(oldLine, newBlockNumber);
	}
}
