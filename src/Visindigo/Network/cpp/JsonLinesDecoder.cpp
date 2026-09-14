#include <QtCore/qbytearray.h>
#include <QtCore/qlist.h>
#include "Network/HttpRequest.h"
#include "Network/HttpTypes.h"

namespace Visindigo::Network {
	class JsonLinesDecoderPrivate {
	public:
		QByteArray Buffer;
		qint64 MaxFrameBytes = 1024 * 1024;
	};

	/*!
		\class Visindigo::Network::JsonLinesDecoder
		\inheaderfile Network/HttpRequest.h
		\since Visindigo 0.17.0
		\inmodule Visindigo
		\brief NDJSON（JSON Lines）流解码器。

		以换行符分帧，每一行构成一个完整帧，帧的 \l StreamFrame::Data 即该行内容
		（不含换行），\l StreamFrame::Event 与 \l StreamFrame::Id 恒为空。
		空行被跳过，行尾的 \c \\r 会被剥除，因此 Windows 与 Unix 换行都能正确处理。

		这一格式常见于本地推理服务与事件推送接口，典型用法是把它挂到请求上：
		\code
			request.setStreamDecoder(std::make_shared<JsonLinesDecoder>());
			center->request(request)->onFrame(this, [](const StreamFrame& frame) {
				// frame.Data 是一行 JSON
				return true;
			});
		\endcode

		\note 本类只负责切分，不解析 JSON。行内容是否合法由调用方判断，
		这样解码器无需关心负载语义，也不会因为 JSON 库的选择而影响接口。

		\note 当累计缓冲超过 \l setMaxFrameBytes 设定的上限仍未见换行时，
		超限部分会被丢弃，以避免无换行的响应把内存吃光。
	*/

	JsonLinesDecoder::JsonLinesDecoder() {
		d = new JsonLinesDecoderPrivate();
	}

	JsonLinesDecoder::~JsonLinesDecoder() {
		delete d;
	}

	/*!
		\since Visindigo 0.17.0

		清空内部缓冲，使解码器回到初始状态。重试或重定向后会调用它，
		以便下一跳的字节从头开始分帧。
	*/
	void JsonLinesDecoder::reset() {
		d->Buffer.clear();
	}

	/*!
		\since Visindigo 0.17.0

		送入新增字节，返回本次能够完整切出的所有帧。未以换行结尾的残余部分
		会被保留到下次调用，因此调用方无需关心网络分包的位置。
	*/
	QList<StreamFrame> JsonLinesDecoder::feed(const QByteArray& chunk) {
		QList<StreamFrame> frames;
		d->Buffer.append(chunk);
		qsizetype index = d->Buffer.indexOf('\n');
		while (index >= 0) {
			QByteArray line = d->Buffer.left(index);
			d->Buffer.remove(0, index + 1);
			if (line.endsWith('\r')) {
				line.chop(1);
			}
			if (!line.isEmpty()) {
				StreamFrame frame;
				frame.Data = line;
				frames.append(frame);
			}
			index = d->Buffer.indexOf('\n');
		}
		if (d->MaxFrameBytes > 0 && d->Buffer.size() > d->MaxFrameBytes) {
			// 迟迟不见换行说明对端可能根本不发换行，此时继续累积只会耗尽内存。
			d->Buffer.clear();
		}
		return frames;
	}

	/*!
		\since Visindigo 0.17.0

		流结束时调用。若缓冲区还剩下未以换行结尾的内容，它会被当作最后一个帧返回；
		这是必要的，因为服务端完全可以在最后一行的末尾省略换行。
	*/
	QList<StreamFrame> JsonLinesDecoder::finish() {
		QList<StreamFrame> frames;
		if (!d->Buffer.isEmpty()) {
			QByteArray line = d->Buffer;
			d->Buffer.clear();
			if (line.endsWith('\r')) {
				line.chop(1);
			}
			if (!line.isEmpty()) {
				StreamFrame frame;
				frame.Data = line;
				frames.append(frame);
			}
		}
		return frames;
	}

	/*!
		\since Visindigo 0.17.0

		返回尚未切分成帧的字节数。
	*/
	qint64 JsonLinesDecoder::getBufferedBytes() const {
		return d->Buffer.size();
	}

	/*!
		\since Visindigo 0.17.0

		设置单帧缓冲上限，默认 1 MiB。请参阅类说明中关于超限行为的解释。
	*/
	void JsonLinesDecoder::setMaxFrameBytes(qint64 bytes) {
		d->MaxFrameBytes = bytes;
	}

	/*!
		\since Visindigo 0.17.0

		返回当前的单帧缓冲上限。
	*/
	qint64 JsonLinesDecoder::getMaxFrameBytes() const {
		return d->MaxFrameBytes;
	}
}
