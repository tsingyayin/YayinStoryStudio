#ifndef Visinidgo_Utility_Optional_h
#define Visinidgo_Utility_Optional_h
#include <type_traits>
namespace Visindigo::Utility {
	template<typename ErrorCodeType> concept StandardErrorCodeType = requires(ErrorCodeType errorCode) {
		requires std::is_enum_v<ErrorCodeType>;
		ErrorCodeType::Success;
		ErrorCodeType::Unknown;
	};

	template<typename ValueType, typename StandardErrorCodeType>
	class Optional {
	public:
		Optional() {};
		Optional(ValueType value) : value(value) {};
		Optional(StandardErrorCodeType errorCode) : errorCode(errorCode) {};
		~Optional() noexcept = default;
	public:
		bool hasValue() const {
			return ErrorCode == StandardErrorCodeType::Success;
		}
		ValueType value() const {
			return Value;
		}
		StandardErrorCodeType errorCode() const {
			return ErrorCode;
		}
		bool hasError() const {
			return ErrorCode != StandardErrorCodeType::Success;
		}
		void reset() {
			Value = ValueType{};
			ErrorCode = StandardErrorCodeType::Success;
		}
	private:
		ValueType Value = ValueType{};
		StandardErrorCodeType ErrorCode = StandardErrorCodeType::Success;
	};
}
#endif // Visinidgo_Utility_Optional_h
