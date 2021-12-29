#pragma once
#include <errno.h>
#include "string_view.h"

namespace Vultr
{
	struct Error
	{
		const int code = 0;
		const StringView message;

		Error() : message("Error has not been initialized!"), code(-1) {}
		explicit Error(StringView message) : message(message), code(-1) {}
		explicit Error(StringView message, int code) : message(message), code(code) {}

		// Just for macro "check"
		explicit operator bool() const { return true; }
	};

	inline Error error_from_errno(int code) { return Error(strerror(code), code); }
} // namespace Vultr