#ifndef Visindigo_Network_HttpTypes_h
#define Visindigo_Network_HttpTypes_h
#include <QtCore/qbytearray.h>
#include <QtCore/qlist.h>
#include <QtCore/qobject.h>
#include <QtCore/qset.h>
#include <QtCore/qstring.h>
#include "VICompileMacro.h"
// Main
namespace Visindigo::Network {
	struct VisindigoAPI RetryPolicy {
		qint32 MaxAttempts = 3;
		qint32 InitialDelayMs = 300;
		qint32 MaxDelayMs = 8000;
		qreal  BackoffMultiplier = 2.0;
		bool   UseJitter = true;
		bool   RespectRetryAfter = true;
		bool   RetryOnTimeout = true;
		bool   RetryOnConnectionError = true;
		bool   RetryOnStatusError = true;
		bool   RetryIdempotentOnly = true;
		QSet<qint32> RetryableStatusCodes = { 408, 425, 429, 500, 502, 503, 504 };
	};

	struct VisindigoAPI HttpError {
		Q_GADGET;
	public:
		enum class ErrorType {
			NoError = 0,
			Aborted,
			Timeout,
			IdleTimeout,
			ConnectionRefused,
			HostNotFound,
			NetworkUnreachable,
			ProxyError,
			TlsError,
			CertificateError,
			TooManyRedirects,
			ProtocolError,
			HttpStatusError,
			ResponseTooLarge,
			LocalIoError,
			ParseError,
			QueueRejected,
			UrlSchemeRejected,
			Unknown
		};
		Q_ENUM(ErrorType)
		ErrorType Type = ErrorType::NoError;
		qint32 HttpStatus = 0;
		QString Message;
		QString Detail;
		bool Retryable = false;
		qint32 SuggestedRetryDelayMs = -1;
	public:
		static QString errorTypeName(ErrorType type);
		bool isError() const;
		QString toString() const;
	};

	struct VisindigoAPI StreamFrame {
		QString Event;
		QByteArray Data;
		QString Id;
		qint32 RetryMs = -1;
	public:
		bool isValid() const;
	};

	class VisindigoAPI IStreamDecoder {
	public:
		virtual ~IStreamDecoder() {}
	public:
		virtual void reset() = 0;
		virtual QList<StreamFrame> feed(const QByteArray& chunk) = 0;
		virtual QList<StreamFrame> finish() = 0;
		virtual qint64 getBufferedBytes() const = 0;
		virtual void setMaxFrameBytes(qint64 bytes) = 0;
		virtual qint64 getMaxFrameBytes() const = 0;
	};

	struct VisindigoAPI HttpTimings {
		qint64 TotalMs = -1;
		qint64 TtfbMs = -1;
		qint64 DnsMs = -1;
		qint64 ConnectMs = -1;
		qint64 TlsMs = -1;
		qint64 UploadMs = -1;
	};
}
#endif // Visindigo_Network_HttpTypes_h
