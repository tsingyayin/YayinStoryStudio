#include <QtCore/qmetaobject.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include "Network/HttpTypes.h"

namespace Visindigo::Network {
	/*!
		\struct Visindigo::Network::RetryPolicy
		\inheaderfile Network/HttpTypes.h
		\since Visindigo 0.17.0
		\inmodule Visindigo
		\brief 请求重试策略。

		RetryPolicy 描述一次请求在失败后是否可以重试、重试几次、以及退避方式。
		它属于纯值类型，可以随意拷贝与修改。

		默认策略只对幂等方法（GET / HEAD / PUT / DELETE / OPTIONS）生效，
		这是 \l RetryIdempotentOnly 的默认值。之所以这样设定，是因为 POST
		重试可能造成服务端重复处理（例如重复下单、重复扣费），而这在传输层
		无法自动判断安全性，因此必须由调用方显式关闭该开关才允许重试。

		\variable MaxAttempts 总尝试次数（含首次），为 1 表示不重试。
		\variable InitialDelayMs 首次重试前的等待毫秒数。
		\variable MaxDelayMs 退避延迟的上限，避免因指数增长而等待过久。
		\variable BackoffMultiplier 退避倍数，每次重试后延迟乘以此值。
		\variable UseJitter 是否在延迟上叠加随机抖动，用于避免大量客户端同时重连造成的惊群。
		\variable RespectRetryAfter 是否优先采纳服务端返回的 Retry-After 头（多见于 429 / 503）。
		\variable RetryOnTimeout 超时是否可重试。
		\variable RetryOnConnectionError 连接类错误（拒绝、DNS 失败、TLS 失败等）是否可重试。
		\variable RetryOnStatusError 命中 \l RetryableStatusCodes 的状态码是否可重试。
		\variable RetryIdempotentOnly 是否只对幂等方法重试。
		\variable RetryableStatusCodes 允许重试的 HTTP 状态码集合。
	*/

	/*!
		\struct Visindigo::Network::HttpError
		\inheaderfile Network/HttpTypes.h
		\since Visindigo 0.17.0
		\inmodule Visindigo
		\brief 网络层统一错误信息。

		网络失败属于可预期结果，而非异常情况，因此 Visindigo 的网络模块不使用
		异常表达错误，而是在结果类型（std::expected）或信号中传递本结构。

		这样做的另一个好处是：错误信息可以跨线程、跨信号槽安全传递，且不会被
		栈展开意外绕过——异常在 Qt 的信号槽中传播本身就是不推荐的用法。

		\variable Type 错误分类，取值见 \l ErrorType。
		\variable HttpStatus 有 HTTP 响应时为状态码，否则为 0。
		\variable Message 面向用户的简短错误描述。
		\variable Detail 服务端返回的原始错误体（已按上限截断），常用于展示 4xx/5xx 的具体原因。
		\variable Retryable 该错误是否可按重试策略重试。
		\variable SuggestedRetryDelayMs 服务端建议的等待毫秒数，来自 Retry-After；无建议时为 -1。
	*/

	/*!
		\enum Visindigo::Network::HttpError::ErrorType
		\since Visindigo 0.17.0

		错误分类。命名参照 Visindigo::General::CommandErrorData::ErrorType，
		以避免与下方同名的成员变量 \c Type 发生名称冲突。

		\value NoError 无错误。
		\value Aborted 调用方主动中止。
		\value Timeout 连接或总时长超时。
		\value IdleTimeout 流式请求中两次数据到达的间隔超过空闲超时。
		\value ConnectionRefused 连接被拒绝。
		\value HostNotFound 域名解析失败。
		\value NetworkUnreachable 网络不可达（常见于断网或路由问题）。
		\value ProxyError 代理相关错误。
		\value TlsError TLS 握手失败。
		\value CertificateError 证书校验失败。此类错误不应被静默忽略，否则等同于放弃中间人防护。
		\value TooManyRedirects 重定向次数超过 setFollowRedirectsMax() 设定的上限。
		\value ProtocolError HTTP 协议层面的错误（响应格式非法等）。
		\value HttpStatusError 服务端返回 4xx 或 5xx。
		\value ResponseTooLarge 响应体超过 setMaxResponseBytes() 或 setMaxBufferBytes() 设定的上限。
		\value LocalIoError 本地 I/O 失败，例如落盘目录无法创建、目标文件无法写入。
		\value ParseError 流式分帧或 JSON 解析失败。
		\value QueueRejected 请求被队列拒绝，通常是命中了并发上限或队列长度上限。
		\value UrlSchemeRejected URL 协议不在中心允许的白名单内。
		\value Unknown 未归类的错误。
	*/

	/*!
		\since Visindigo 0.17.0

		返回枚举值对应的名字字符串；传入未收录的取值时返回 \c "Unknown"。

		实现走 QMetaEnum 反射而不是手写 switch，这是本结构声明为 Q_GADGET 的直接
		收益：枚举名只有一个来源（即声明本身），将来往 ErrorType 里增加取值时
		不必记得同步修改映射表，也就不存在"新增了错误类型但日志里显示 Unknown"
		这类遗漏。同时它对调用方是公开的，日志与界面可以直接拿它做展示。
	*/
	QString HttpError::errorTypeName(ErrorType type) {
		const QMetaEnum metaEnum = QMetaEnum::fromType<ErrorType>();
		const char* key = metaEnum.valueToKey(static_cast<int>(type));
		return key != nullptr ? QString::fromUtf8(key) : QStringLiteral("Unknown");
	}

	/*!
		\since Visindigo 0.17.0

		判断本结构是否真的表示一个错误，即 \l Type 不等于 \l ErrorType::NoError。
	*/
	bool HttpError::isError() const {
		return Type != ErrorType::NoError;
	}

	/*!
		\since Visindigo 0.17.0

		生成适合写入日志的单行描述，形如：
		\code
			[Timeout] Connection timed out (HTTP 504)
		\endcode
		仅在 \l HttpStatus 不为 0 时附加状态码部分。若 \l Detail 非空，
		调用方可自行决定是否展示，本函数不将其包含在内，以免日志被超长文本淹没。
	*/
	QString HttpError::toString() const {
		QString result = QStringLiteral("[%1] %2").arg(errorTypeName(Type), Message);
		if (HttpStatus != 0) {
			result += QStringLiteral(" (HTTP %1)").arg(HttpStatus);
		}
		return result;
	}

	/*!
		\struct Visindigo::Network::StreamFrame
		\inheaderfile Network/HttpTypes.h
		\since Visindigo 0.17.0
		\inmodule Visindigo
		\brief 与具体协议无关的流帧。

		请求中心不关心帧来自哪种文本流格式，只负责把字节流切成帧，
		切分规则由 \l IStreamDecoder 的实现决定：

		\list
		\li SSE（W3C text/event-stream）：\l Event 为 \c event: 字段（可能为空），
			\l Data 为多行 \c data: 字段拼接后的结果，\l Id 与 \l RetryMs 对应
			\c id: 与 \c retry: 字段。
		\li NDJSON（JSON Lines）：\l Event、\l Id、\l RetryMs 均为空，\l Data 为整行内容。
		\endlist

		\note 本结构不含任何厂商语义。诸如 \c "data: [DONE]" 这类结束约定由上层协议
		实现自行判断，中心与解码器均不关心，这是为了保证本模块对各家服务保持中立。
	*/

	/*!
		\since Visindigo 0.17.0

		判断本帧是否携带了有效内容，即 \l Event 或 \l Data 至少有一个非空。

		之所以两者都算，是因为标准允许只带事件名的帧（例如 \c "event: ping"），
		这种帧没有数据体但依然是有意义的心跳信号，不应被当作空帧丢弃。
	*/
	bool StreamFrame::isValid() const {
		return !Event.isEmpty() || !Data.isEmpty();
	}

	/*!
		\class Visindigo::Network::IStreamDecoder
		\inheaderfile Network/HttpTypes.h
		\since Visindigo 0.17.0
		\inmodule Visindigo
		\brief 流帧解码器接口。

		IStreamDecoder 把"持续到达的字节流"切成一个个完整的 Visindigo::Network::StreamFrame。
		请求中心只依赖本接口，对具体文本格式（SSE、NDJSON，或调用方自定义的格式）
		保持中立：新增一种流格式只需要实现本接口，中心与句柄都不必改动。

		实现本接口时必须满足以下约定，否则会在真实网络环境中出错：

		\list
		\li \b{必须能处理任意切分点}。TCP 是字节流协议，分包边界与帧边界毫无关系，
			一个帧可能被拆到任意多次 \l feed() 中到达，也可能一次到达包含多个帧。
			实现必须自行缓存不完整的尾部，而不能假设"每次调用恰好是一个完整帧"。
		\li \b{状态必须可重置}。请求因重试或重定向需要重发时，中心会调用 \l reset()
			让解码器丢掉上一跳的残留缓冲，因此实现不能把"第一次调用"作为初始状态
			的来源。
		\li \b{必须尊重单帧上限}。当缓存超过 \l setMaxFrameBytes() 设定的长度仍未
			出现分帧符时，实现应当丢弃该部分而不是继续累积——否则一个始终不发换行
			的恶意响应就能把进程内存吃光。本模块自带的两个实现都采用"清空缓冲"的
			策略。
		\endlist

		\note 同一实例不会被两个请求同时使用，因此实现无需考虑并发访问；但也不应
		依赖"只会被一个请求用到底"，重试会复用同一个 shared_ptr 实例，这也是要求
		实现支持 \l reset() 的原因。

		内置实现见 Visindigo::Network::SseDecoder（W3C text/event-stream）与
		Visindigo::Network::JsonLinesDecoder（NDJSON）。

		\sa Visindigo::Network::StreamFrame
		\sa Visindigo::Network::HttpRequest::setStreamDecoder
	*/

	/*!
		\fn Visindigo::Network::IStreamDecoder::~IStreamDecoder()
		\since Visindigo 0.17.0

		虚析构，使调用方可以通过本接口指针安全销毁具体解码器。

		中心以 std::shared_ptr 持有解码器，因此继承本接口的类只需正常实现析构即可，
		不必自行管理生命周期。
	*/

	/*!
		\fn void Visindigo::Network::IStreamDecoder::reset()
		\since Visindigo 0.17.0

		清空全部内部状态，使解码器回到刚构造时的样子。

		中心在重试与重定向之后、把下一跳的字节交给解码器之前调用它。如果不重置，
		上一跳半途而废的字节会与下一跳的开头拼接在一起，凭空造出一个从未存在过的帧。
	*/

	/*!
		\fn QList<Visindigo::Network::StreamFrame> Visindigo::Network::IStreamDecoder::feed(const QByteArray& chunk)
		\since Visindigo 0.17.0

		送入一段新到达的字节，返回本次能够完整切出的所有帧。

		\a chunk 的边界没有任何语义，实现不得对其做任何假设：它可能是半个帧，
		也可能包含若干个完整帧。未被切出的残余部分应留在内部缓冲中，等待后续
		调用补齐。

		返回的列表可以为空，表示本次到达的字节尚不足以构成一个完整帧。空列表是
		常态而非异常，中心收到空列表时什么也不做。
	*/

	/*!
		\fn QList<Visindigo::Network::StreamFrame> Visindigo::Network::IStreamDecoder::finish()
		\since Visindigo 0.17.0

		在流结束时调用一次，返回缓冲中剩余内容所能构成的帧。

		之所以需要这个独立的入口，是因为不少服务端在发出最后一个事件后直接断开
		连接，而不补上作为结束标记的空行（SSE）或换行（NDJSON）。若只靠 \l feed()
		分帧，这最后一份数据就会被永远留在缓冲区里。
	*/

	/*!
		\fn qint64 Visindigo::Network::IStreamDecoder::getBufferedBytes() const
		\since Visindigo 0.17.0

		返回尚未切分成帧的字节数。主要供诊断使用——例如排查"流一直没动静"时，
		可以用它区分"对端没有发数据"与"对端发了数据但始终不构成完整帧"。
	*/

	/*!
		\fn void Visindigo::Network::IStreamDecoder::setMaxFrameBytes(qint64 bytes)
		\since Visindigo 0.17.0

		设置内部缓冲的长度上限，单位为字节。详见类说明中关于超限处理的约定。
	*/

	/*!
		\fn qint64 Visindigo::Network::IStreamDecoder::getMaxFrameBytes() const
		\since Visindigo 0.17.0

		返回当前的缓冲长度上限。
	*/

	/*!
		\struct Visindigo::Network::HttpTimings
		\inheaderfile Network/HttpTypes.h
		\since Visindigo 0.17.0
		\inmodule Visindigo
		\brief 请求耗时统计。

		各字段的单位均为毫秒。字段为 -1 表示该项无法观测，而不是"耗时为负"——
		调用方在计算与展示前应当先判断取值是否有效。

		\note 目前只有 \l TotalMs 与 \l TtfbMs 能被稳定填充，其余细分项需要拿到
		连接复用、DNS、TLS 握手的独立时刻，而 Qt 的公开接口并不暴露它们。
		本结构保留这些字段是为了让将来替换传输后端时不必改动调用方代码，
		因此现在读到 -1 属于预期行为，不应视作缺陷。

		\variable TotalMs 从请求发出到结束的总耗时。
		\variable TtfbMs 首字节到达时刻（Time To First Byte），是判断服务端响应快慢
			的关键指标，它排除了下载大响应体所消耗的时间。
		\variable DnsMs 域名解析耗时。
		\variable ConnectMs TCP 建连耗时。
		\variable TlsMs TLS 握手耗时。
		\variable UploadMs 请求体上传耗时。
	*/
}
