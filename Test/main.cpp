#include <iostream>
#include "LibProperties\LibProperties.hpp"

using namespace std;

int main()
{
	lp::LibPropertiesMemory p;

	p[TEXT("login")] = TEXT("root");
	p[TEXT("password")] = TEXT("root");
	p[TEXT("read_counter")] = TEXT("0");

	if (!p.insertFromFile(L"config.properties"))
	{
		cout << "Error open." << endl;
	}

	wstring login, password;
	size_t read_counter{ 0 };

	login = p[TEXT("login")];
	password = p[TEXT("password")];
	if (!p.get(TEXT("read_counter"), read_counter))
	{
		wcout << L"Error geting read_counter" << endl;
	}

	wcout << login << L" " << password << L" " << read_counter << endl;

	read_counter++;
	p[TEXT("read_counter")] = to_wstring(read_counter);

	p.saveToFile();
	
	return 0;
}