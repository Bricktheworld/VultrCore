#pragma once
#include <errno.h>
#include "string_view.h"

namespace Vultr
{
	struct Error
	{
		const int code           = 0;
		const StringView message = nullptr;

		Error() : message(nullptr), code(0) {}
		explicit Error(StringView message) : message(message), code(-1) {}
		explicit Error(StringView message, int code) : message(message), code(code) {}

		bool ok() const { return message == nullptr; }
		explicit operator bool() const { return ok(); }
	};

	inline Error error_from_errno(int code) { return Error(strerror(code), code); }
} // namespace Vultr