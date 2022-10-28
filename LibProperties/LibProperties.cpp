#include "LibProperties.hpp"
#include <iostream>
#include <vector>

using namespace std;

namespace lp
{
	//автоматически закрывает файл
	class FileCloseControl
	{
	public:
		FileCloseControl(FILE* _file)
		{
			file = _file;
		}
		~FileCloseControl()
		{
			if (file)
				fclose(file);
			file = nullptr;
		}
	private:
		FILE* file;
	};


	static bool SkipComments(FILE* file)
	{
		lp_char buff;
		while (true)
		{
			if (fread(&buff, 1, sizeof(lp_char), file) == 0)
				return 1;//файл закончился
			if (buff == L'\n')
				return 0;
			if (feof(file))
				return 1;//файл закончился
		}
	}
	static lp_string ReadData(FILE* file)
	{
		lp_string data;
		lp_char buff;

		while (true)
		{
			if (fread(&buff, 1, sizeof(lp_char), file) == 0 ||
				buff == L'\n' || feof(file))
			{
				return data;
			}
			data += buff;
		}
	}


	lp::LibPropertiesMemory::LibPropertiesMemory(bool save_default) :
		save_default(save_default)
	{
	}

	lp::LibPropertiesMemory::~LibPropertiesMemory()
	{
	}

	bool lp::LibPropertiesMemory::loadFromFile(const lp_string& file_name)
	{
		clear();
		return insertFromFile(file_name);
	}
	bool LibPropertiesMemory::insertFromFile(const lp_string& file_name)
	{
		this->file_name = file_name;
		FILE* file{ nullptr };
		auto open_err = _wfopen_s(&file, file_name.c_str(), L"rt, ccs=UTF-8");
		FileCloseControl fc(file);
		if (file == nullptr || open_err)
			return false;

		int read_len;

		lp_char buff;
		lp_string read_name;
		while (true)
		{
			//обработка коментария
			fread(&buff, sizeof(lp_char), 1, file);
			if (feof(file))
			{
				return true;
			}


			if (buff == L'#')
			{
				if (SkipComments(file) || feof(file))
				{
					return true;//файл закончился
				}
			}

			//найден знак конца имени
			else if (buff == L'=')
			{
				set(read_name, ReadData(file));
				read_name.clear();
			}

			//Обработка повриждений файла
			else if ((buff == L'\n' && read_name != L"") || //Обрыв имени
				(buff == L' ' && read_name != L"") ||       //Обрыв имени (не найден знак конца имени)
				(feof(file) && read_name != L"") ||         //Обрыв файла
				(buff == L'=' && read_name == L""))         //Днные указанны без имени
			{
				return false;
			}

			//конец файла
			else if (feof(file))
			{
				return true;
			}

			//запись символов имени
			else if (buff != L' ' &&
				buff != L'\n')
			{
				read_name += buff;
			}

		}

		return true;
	}

	bool LibPropertiesMemory::saveToFile() const
	{
		return saveToFile(file_name);
	}
	bool LibPropertiesMemory::saveToFile(const lp_string& file_name) const
	{
		FILE* file{ nullptr };
		errno_t open_err;
		fpos_t size{ 0 };
		lp_string ft;

		struct pars_str
		{
			lp_string str;
			size_t equal_pos;
			bool comment;
		};
		auto ParseStr = [](const lp_string& str) -> pars_str
		{
			pars_str ps = { .str = str, .comment = false };

			ps.equal_pos = str.find(TEXT("="));
			if (ps.equal_pos == lp_string::npos)
			{
				ps.comment = true;
				return ps;
			}

			auto name = str.substr(0, ps.equal_pos);
			if (name.find(TEXT(" ")) != lp_string::npos)
			{
				ps.comment = true;
				return ps;
			}

			return ps;
		};
		auto GetName = [](const pars_str& s) -> lp_string
		{
			return s.str.substr(0, s.equal_pos);
		};
		auto Replace = [](pars_str& s, const lp_string& ns)
		{
			s.str.replace(s.str.begin() + s.equal_pos + 1, s.str.end(), ns);
		};
		vector<pars_str> strings;

		open_err = _wfopen_s(&file, file_name.c_str(), L"rt, ccs=UTF-8");

		if (open_err == ENOENT)
			goto combining_tables;

		if (file == nullptr || open_err)
			return false;

		// read and parse file
		{
			fseek(file, 0, SEEK_END);
			fgetpos(file, &size);
			fseek(file, 0, SEEK_SET);

			ft.resize(size + 1, 0);
			fread(&ft[0], sizeof(lp_char), size, file);
			fclose(file);

			if (ft[0] == 65279)
				ft.erase(ft.begin(), ft.begin() + 1);

			cout << "--Read file begin--" << endl;
			wcout << ft << endl;
			cout << "--Read file end--" << endl;

			lp_stringstream st{ ft.data() };

			while (1)
			{
				lp_string s;
				getline(st, s);
				if (!st)
					break;
				strings.push_back(ParseStr(s));
				wcout << L"strings[n]: " << strings[strings.size() - 1].str << L" " <<
					strings[strings.size() - 1].str.size() << endl;
			}
		}

	combining_tables:
		for (const auto& e : *this)
		{
			bool find = false;
			for (auto& es : strings)
			{
				if (es.comment)
					continue;
				if (GetName(es) == e.first)
				{
					find = true;
					Replace(es, e.second);
					break;
				}
			}

			if (!find)
			{
				pars_str ps;
				ps.str = e.first + TEXT("=") + e.second;
				ps.equal_pos = e.first.size();
				ps.comment = false;

				strings.push_back(ps);
			}
		}


		open_err = _wfopen_s(&file, file_name.c_str(), L"wt, ccs=UTF-8");
		if (file == nullptr || open_err)
			return false;

		for (const auto& es : strings)
		{
			if (es.str.size())
			{
				wcout << es.str << L" " << es.str.size() << endl;
				fwrite(&es.str[0], sizeof(lp_char), es.str.size(), file);
			}
			fwrite(TEXT("\n"), sizeof(lp_char), 1, file);
		}
		fclose(file);

		return true;
	}
	
	bool LibPropertiesMemory::set(const lp_string& name, const lp_string& data)
	{
		(*this)[name] = data;
		return true;
	}
	
	bool LibPropertiesMemory::get(const lp_string& name, lp_string& data)
	{
		try
		{
			data = at(name);
			return true;
		}
		catch (...)
		{
			return false;
		}

		return true;
	}
	bool LibPropertiesMemory::get(const lp_string& name, size_t& data)
	{
		try
		{
			auto& s = at(name);
			lp_stringstream ss;
			ss.str(s);
			ss >> data;
		}
		catch (...)
		{
			return false;
		}

		return true;
	}
}
