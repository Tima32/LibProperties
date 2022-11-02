#include <iostream>
#include "LibProperties/LibProperties.hpp"

using namespace std;

int main()
{
	lp::LibPropertiesMemory p;

	p[TEXT("login")] = TEXT("root");
	p[TEXT("password")] = TEXT("root");
	p[TEXT("read_counter")] = TEXT("0");

	if (!p.insertFromFile(TEXT("config.properties")))
	{
		cout << "Error open." << endl;
	}

	lp::lp_string login, password;
	size_t read_counter{ 0 };

	login = p[TEXT("login")];
	password = p[TEXT("password")];
	if (!p.get(TEXT("read_counter"), read_counter))
	{
		wcout << L"Error geting read_counter" << endl;
	}

	lp::lp_cout << login << TEXT(" ") << password << TEXT(" ") << read_counter << endl;

	read_counter++;
#ifdef _WIN32
		p[TEXT("read_counter")] = to_wstring(read_counter);
#elif __linux__
		p[TEXT("read_counter")] = to_string(read_counter);
#endif

	p.saveToFile();
	
	return 0;
}