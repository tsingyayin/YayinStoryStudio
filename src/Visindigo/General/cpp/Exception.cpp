#include "../Exception.h"

namespace Visindigo::General {
	Exception Exception::fromStdException(const std::exception& e) {
		return Exception(Exception::Unknown, QString::fromStdString(e.what()), true, "", 0, "");
	}

	class ExceptionPrivate {
		public:
		Exception::Type Type;
		QString Message;
		bool Critical;
		QString File;
		int Line;
		QString Function;
	};

	Exception::Exception(Type type, QString message, bool critical, QString file, int line, QString func) {
		d = new ExceptionPrivate();
		d->Type = type;
		d->Message = message;
		d->Critical = critical;
		d->File = file;
		d->Line = line;
		d->Function = func;
	}

	Exception::Exception(const Exception& other) {
		d = new ExceptionPrivate();
		*d = *(other.d);
	}

	Exception& Exception::operator=(const Exception& other) {
		if (this != &other) {
			delete d;
			*d = *(other.d);
		}
		return *this;
	}
	Exception::Exception(Exception&& other) noexcept {
		d = other.d;
		other.d = nullptr;
	}
	Exception& Exception::operator=(Exception&& other) noexcept {
		if (this != &other) {
			delete d;
			d = other.d;
			other.d = nullptr;
		}
		return *this;
	}


	Exception::~Exception() {
		delete d;
	}

	QString Exception::getMessage() const {
		return d->Message;
	}

	Exception::Type Exception::getType() const {
		return d->Type;
	}

	bool Exception::isCritical() const {
		return d->Critical;
	}

	QString Exception::getFile() const {
		return d->File;
	}

	int Exception::getLine() const {
		return d->Line;
	}

	QString Exception::getFunction() const {
		return d->Function;
	}

	const char* Exception::what() const noexcept {
		return d->Message.toStdString().c_str();
	}
}