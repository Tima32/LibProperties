#pragma once
#include <stdint.h>
#include <string>
#include <sstream>
#include <map>

#ifdef LIB_PPROPERTIES
#else
	#ifdef _WIN64
		#ifdef _DEBUG
			#pragma comment(lib,"LibProperties-x64-d.lib")
		#else
			#pragma comment(lib,"LibProperties-x64.lib")
		#endif
	#elif _WIN32
		#ifdef _DEBUG
			#pragma comment(lib,"LibProperties-x86-d.lib")
		#else
			#pragma comment(lib,"LibProperties-x86.lib")
		#endif
	#endif

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

		LibPropertiesMemory() = default;
		LibPropertiesMemory(const std::map<lp_string, lp_string>& data);
		LibPropertiesMemory(std::map<lp_string, lp_string>&& data) noexcept;
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
	};
}