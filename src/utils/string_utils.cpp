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

	Vector<size_t> find_all(StringView haystack, StringView needle)
	{
		Vector<size_t> positions{};
		size_t current_position = 0;
		while (current_position <= haystack.length())
		{
			auto *res = static_cast<char *>(memmem(haystack.c_str() + current_position, haystack.length() - current_position, needle.c_str(), needle.length()));
			if (res == nullptr)
				break;
			positions.push_back(res - haystack.c_str());
			current_position += res - haystack.c_str() + 1;
		}
		return positions;
	}

	Vector<StringView> split(StringView string, StringView delimiter)
	{
		Vector<StringView> split{};
		str current = string.c_str();
		while (current < string.c_str() + string.length())
		{
			auto *res = static_cast<char *>(memmem(current, string.length() - (current - string.c_str()), delimiter.c_str(), delimiter.length()));
			if (res == nullptr)
			{
				split.push_back({current, string.length() - (current - string.c_str())});
				break;
			}
			split.push_back({current, (size_t)(res - current)});
			current = res + delimiter.length();
		}

		return split;
	}

	String replace_all(StringView haystack, StringView needle, StringView replacement)
	{
		if (haystack.is_empty())
			return String(haystack);

		Vector<size_t> positions = find_all(haystack, needle);
		if (positions.empty())
			return String(haystack);

		String replaced_string;
		size_t last_position = 0;
		for (const auto &position : positions)
		{
			replaced_string += haystack.substr(last_position, position);
			replaced_string += replacement;
			last_position = position + needle.length();
		}
		replaced_string += haystack.substr(last_position, haystack.length());
		return replaced_string;
	}

	ErrorOr<f64> parse_f64(StringView string)
	{
		f64 result        = 0;
		f64 fact          = 1;
		s32 exponent      = 0;
		s32 exponent_fact = 1;
		size_t i          = 0;
		if (string[i] == '-')
		{
			i++;
			fact = -1;
		}

		bool point_seen = false;
		bool e_seen     = false;
		for (; i < string.length(); i++)
		{
			if (string[i] == '.')
			{
				if (e_seen)
					return Error("Cannot have a decimal in the e exponent!");
				point_seen = true;
				continue;
			}

			if (string[i] == 'e')
			{
				if (e_seen)
					return Error("Multiple e's found!");
				e_seen = true;
				continue;
			}

			if (e_seen)
			{
				if (string[i] == '-')
				{
					exponent_fact = -1;
				}
				else
				{
					s32 d = string[i] - '0';
					if (d < 0 || d > 9)
						return Error("Incorrect format, unknown character found!");
					exponent = exponent * 10 + d;
				}
			}
			else
			{
				s32 d = string[i] - '0';
				if (d < 0 || d > 9)
					return Error("Incorrect format, unknown character found!");

				if (point_seen)
					fact /= 10.0f;
				result = result * 10.0f + (f64)d;
			}
		}
		return result * fact * pow(10, exponent * exponent_fact);
	}

	ErrorOr<u64> parse_u64(StringView string)
	{
		u64 result   = 0;
		u64 exponent = 0;
		bool e_seen  = false;
		for (size_t i = 0; i < string.length(); i++)
		{
			if (string[i] == '-')
				return Error("Expected unsigned positive integer instead found negative integer!");

			if (string[i] == 'e')
			{
				if (e_seen)
					return Error("Multiple e's found!");
				e_seen = true;
				continue;
			}

			if (string[i] == '.')
				return Error("Expected unsigned integer, instead found floating point number!");

			s32 d = string[i] - '0';
			if (d < 0 || d > 9)
				return Error("Incorrect format, unknown character found!");
			if (e_seen)
			{
				exponent = exponent * 10 + (u64)d;
			}
			else
			{
				result = result * 10 + (u64)d;
			}
		}
		return result * static_cast<u64>(pow(10, exponent));
	}

	ErrorOr<s64> parse_s64(StringView string)
	{
		s64 fact     = 1;
		s64 result   = 0;
		u64 exponent = 0;
		bool e_seen  = false;
		for (size_t i = 0; i < string.length(); i++)
		{
			if (string[i] == '-')
			{
				if (e_seen)
					return Error("Cannot have negative e!");

				if (fact == -1)
					return Error("Negative symbol found twice!");

				fact = -1;
				continue;
			}

			if (string[i] == 'e')
			{
				if (e_seen)
					return Error("Multiple e's found!");
				e_seen = true;
				continue;
			}

			if (string[i] == '.')
				return Error("Expected unsigned integer, instead found floating point number!");

			s32 d = string[i] - '0';
			if (d < 0 || d > 9)
				return Error("Incorrect format, unknown character found!");

			if (e_seen)
			{
				exponent = exponent * 10 + (u64)d;
			}
			else
			{
				result = result * 10 + (s64)d;
			}
		}
		return result * fact * static_cast<s64>(pow(10, exponent));
	}

	String serialize_f64(f64 value)
	{
		char buf[64];
		snprintf(buf, 64, "%f", value);
		return String(buf);
	}
	String serialize_u64(u64 value)
	{
		char buf[32];
		snprintf(buf, 32, "%lu", value);
		return String(buf);
	}
	String serialize_s64(s64 value)
	{
		char buf[32];
		snprintf(buf, 32, "%lu", value);
		return String(buf);
	}
} // namespace Vultr
