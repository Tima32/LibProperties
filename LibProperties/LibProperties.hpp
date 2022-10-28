#pragma once
#include <stdint.h>
#include <string>
#include <sstream>
#include <map>

#ifdef LIB_PPROPERTIES
#else
#pragma comment(lib,"LibProperties.lib")
#endif // LIB_PPROPERTIES


namespace lp
{
#ifdef _WIN32
	using lp_string = std::wstring;
	using lp_char = wchar_t;
	using lp_stringstream = std::wstringstream;
#define TEXT(quote) L##quote
#elif __linux__
	using lp_string = std::string;
	using lp_char = char;
#define TEXT(quote) quote
#else
#error Unknown operating system.
#endif

	// Completely parses the file and stores it in memory.
	class LibPropertiesMemory : public std::map<lp_string, lp_string>
	{
	public:
		LibPropertiesMemory(bool save_default = true);
		~LibPropertiesMemory();

		bool loadFromFile(const lp_string& file_name);
		bool insertFromFile(const lp_string& file_name);

		bool saveToFile() const;
		bool saveToFile(const lp_string& file_name) const;

		bool set(const lp_string& name, const lp_string& data);

		bool get(const lp_string& name, lp_string& data);
		bool get(const lp_string& name, size_t& data);

	private:
		lp_string file_name;
		bool save_default;
	};
}