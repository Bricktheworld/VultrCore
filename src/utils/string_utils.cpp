#include "string_utils.h"

namespace Vultr
{
	bool starts_with(StringView haystack, char needle, CaseSensitivity cs)
	{
		if (haystack.length() == 0)
			return false;
		if (cs == CaseSensitivity::Sensitive)
		{
			return haystack[0] == needle;
		}
		else
		{
			return tolower(haystack[0]) == tolower(needle);
		}
	}

	bool starts_with(StringView string, StringView start, CaseSensitivity cs)
	{
		if (start.length() == 0)
			return true;
		if (string.length() < start.length())
			return false;
		if (string.length() == start.length())
			return string == start;
		if (string.c_str() == start.c_str())
			return true;

		if (cs == CaseSensitivity::Sensitive)
			return !memcmp(string.c_str(), start.c_str(), start.length());

		auto *string_c = string.c_str();
		auto *start_c  = start.c_str();

		for (size_t i = 0, start_i = 0; start_i < start.length(); i++, start_i++)
		{
			if (tolower(string_c[i]) != tolower(start_c[start_i]))
				return false;
		}
		return true;
	}

	bool ends_with(StringView haystack, char needle, CaseSensitivity cs)
	{
		auto len = haystack.length();
		if (len == 0)
			return false;
		if (cs == CaseSensitivity::Sensitive)
		{
			return haystack[len - 1] == needle;
		}
		else
		{
			return tolower(haystack[len - 1]) == tolower(needle);
		}
	}

	bool ends_with(StringView string, StringView end, CaseSensitivity cs)
	{
		if (end.length() == 0)
			return true;
		if (string.length() < end.length())
			return false;
		if (string.length() == end.length())
			return string == end;

		if (cs == CaseSensitivity::Sensitive)
			return !memcmp(string.c_str() + (string.length() - end.length()), end.c_str(), end.length());

		auto *string_c = string.c_str();
		auto *end_c    = end.c_str();

		for (size_t i = string.length() - end.length(), end_i = 0; end_i < end.length(); i++, end_i++)
		{
			if (tolower(string_c[i]) != tolower(end_c[end_i]))
				return false;
		}
		return true;
	}

	bool contains(StringView haystack, char needle, CaseSensitivity cs)
	{
		if (haystack.is_empty())
			return false;
		if (cs == CaseSensitivity::Sensitive)
		{
			for (size_t i = 0; i < haystack.length(); i++)
			{
				if (haystack[i] == needle)
					return true;
			}
			return false;
		}
		else
		{
			auto needle_lower = tolower(needle);
			for (size_t i = 0; i < haystack.length(); i++)
			{
				if (tolower(haystack[i] == needle_lower))
					return true;
			}
			return false;
		}
	}

	bool contains(StringView haystack, StringView needle, CaseSensitivity cs)
	{
		if (haystack.is_empty() || needle.is_empty() || needle.length() > haystack.length())
			return false;
		if (needle.is_empty())
			return true;
		auto *haystack_c = haystack.c_str();
		auto *needle_c   = needle.c_str();
		if (cs == CaseSensitivity::Sensitive)
			return memmem(haystack_c, haystack.length(), needle_c, needle.length()) != nullptr;

		auto needle_first = tolower(needle_c[0]);
		for (size_t hi = 0; hi < haystack.length(); hi++)
		{
			if (tolower(haystack_c[hi]) != needle_first)
				continue;
			for (size_t ni = 0; hi + ni < haystack.length(); ni++)
			{
				if (tolower(haystack_c[hi + ni]) != tolower(needle_c[ni]))
				{
					hi += ni;
					break;
				}
				if (ni + 1 == needle.length())
					return true;
			}
		}
		return false;
	}

	Option<size_t> find(StringView haystack, char needle, size_t start)
	{
		if (start >= haystack.length())
			return None;
		for (size_t i = 0; i < haystack.length(); i++)
		{
			if (haystack[i] == needle)
				return i;
		}
		return None;
	}

	Option<size_t> find(StringView haystack, StringView needle, size_t start)
	{
		if (start >= haystack.length())
			return {};
		auto *index = static_cast<char *>(memmem(haystack.c_str() + start, haystack.length() - start, needle.c_str(), needle.length()));
		if (index == nullptr)
			return None;
		return index - haystack.c_str();
	}

	Option<size_t> find_last(StringView haystack, char needle)
	{
		for (size_t i = haystack.length(); i > 0; --i)
		{
			if (haystack[i - 1] == needle)
				return i - 1;
		}
		return None;
	}
} // namespace Vultr
