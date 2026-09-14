#ifndef Visindigo_Network_private_HttpReply_p_h
#define Visindigo_Network_private_HttpReply_p_h
#include <QtCore/qbytearray.h>
#include <QtCore/qelapsedtimer.h>
#include <QtCore/qpointer.h>
#include <QtCore/qstring.h>
#include <QtCore/qurl.h>
#include <memory>
#include "Network/HttpRequest.h"
#include "Network/HttpTypes.h"
#include "Network/private/HttpRequest_p.h"

class QNetworkReply;
class QSaveFile;
class QTimer;

namespace Visindigo::Network {
	class HttpCenterPrivate;

	class HttpReplyPrivate {
	public:
		// 响应体的去向。在响应头到达时决定：3xx 与 4xx/5xx 一律只读进内存
		// 或直接丢弃，绝不落盘，避免把跳转提示页或错误页写成下载结果。
		enum class BodyMode {
			Discard,
			Buffer,
			ToFile
		};
	public:
		HttpReplyPrivate(HttpReply* owner, HttpCenterPrivate* center, const HttpRequest& request);
		~HttpReplyPrivate();
	public:
		void start();
		void onMetaDataChanged();
		void onReadyRead();
		void onTransportFinished();
		void consumeChunk(const QByteArray& chunk);
		void feedDecoder(const QByteArray& chunk, bool finishing);
		void finalizeSuccess();
		void finalizeFailure(const HttpError& error, bool allowRetry);
		void emitFinishedSignals();
		void cleanupTransport();
		bool shouldRetry(const HttpError& error) const;
		void scheduleRetry(const HttpError& error);
		// 连入队都没能进去（例如队列已满）时直接判定失败。交由句柄自己发信号，
		// 而不是由中心从外部发射另一个对象的信号——Qt 的信号是受保护成员。
		void failImmediately(const HttpError& error);
		HttpError mapTransportError() const;
		static qint32 retryDelayFor(qint32 attempt, const RetryPolicy& policy, qint32 suggestedMs);
	public:
		HttpReply* Q = nullptr;
		HttpCenterPrivate* Center = nullptr;
		quint64 Id = 0;
		QString TraceId;
		HttpRequest Request;
		HttpResponse Response;
		HttpReply::State State = HttpReply::State::Queued;
		bool AutoDelete = true;
		bool AbortRequested = false;
		bool Completed = false;
		QPointer<QNetworkReply> Transport;
		QUrl CurrentUrl;
		HttpRequest::Method CurrentMethod = HttpRequest::Method::Get;
		QByteArray CurrentBody;
		QByteArray CurrentContentType;
		qint32 RedirectCount = 0;
		qint32 Attempt = 0;
		quint64 RetryCount = 0;
		QTimer* RetryTimer = nullptr;
		QTimer* TotalTimer = nullptr;
		QTimer* IdleTimer = nullptr;
		QElapsedTimer Elapsed;
		bool FirstByteSeen = false;
		// 全局证书校验开关与请求级开关的与结果，在每次 start() 时重算。
		// 缓存下来是因为 sslErrors 回调也需要用到同一个判定结果。
		bool VerifyTlsEffective = true;
		BodyMode Mode = BodyMode::Buffer;
		QSaveFile* Sink = nullptr;
		std::shared_ptr<IStreamDecoder> Decoder;
		bool DecoderFinished = false;
		qint64 SinkBytes = 0;
	};
}
#endif // Visindigo_Network_private_HttpReply_p_h
