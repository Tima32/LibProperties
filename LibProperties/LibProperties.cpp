#include "LibProperties.hpp"
#include <iostream>
#include <vector>

using namespace std;

// #ifdef __unix
// #define fopen_s(pFile,filename,mode) ((*(pFile))=fopen((filename),(mode)))==NULL
// #endif

static int fopen_s(FILE **f, const char *name, const char *mode) {
    int ret = 0;
    *f = fopen(name, mode);
    /* Can't be sure about 1-to-1 mapping of errno and MS' errno_t */
    if (!*f)
        ret = errno;
    return ret;
}

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

	LibPropertiesMemory::LibPropertiesMemory(std::map<lp_string, lp_string>&& data) noexcept :
		std::map<lp_string, lp_string>(data)
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

#ifdef _WIN32
		auto open_err = _wfopen_s(&file, file_name.c_str(), L"rt, ccs=UTF-8");
#elif __linux__
		int open_err = fopen_s(&file, file_name.c_str(), "rt, ccs=UTF-8");
#endif
		
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
			else if ((buff == TEXT('\n') && read_name != TEXT("")) || //Обрыв имени
				(buff == TEXT(' ') && read_name != TEXT("")) ||       //Обрыв имени (не найден знак конца имени)
				(feof(file) && read_name != TEXT("")) ||         //Обрыв файла
				(buff == TEXT('=') && read_name == TEXT("")))         //Днные указанны без имени
			{
				return false;
			}

			//конец файла
			else if (feof(file))
			{
				return true;
			}

			//запись символов имени
			else if (buff != TEXT(' ') &&
				buff != TEXT('\n'))
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
		int open_err;
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

#ifdef _WIN32
		open_err = _wfopen_s(&file, file_name.c_str(), L"rt, ccs=UTF-8");
#elif __linux__
		open_err = fopen_s(&file, file_name.c_str(), "rt");
#endif

		if (open_err == ENOENT)
			goto combining_tables;

		if (file == nullptr || open_err)
			return false;

		// read and parse file
		{
			fseek(file, 0, SEEK_END);
			fgetpos(file, &size);
			fseek(file, 0, SEEK_SET);

#ifdef _WIN32
		ft.resize(size + 1, 0);
		fread(&ft[0], sizeof(lp_char), size, file);
		fclose(file);
#elif __linux__
		ft.resize(size.__pos, 0);
		fread(&ft[0], sizeof(lp_char), size.__pos, file);
		fclose(file);
#endif

			if (ft[0] == 65279)
				ft.erase(ft.begin(), ft.begin() + 1);

			lp_stringstream st{ ft.data() };

			while (1)
			{
				lp_string s;
				getline(st, s);
				if (!st)
					break;
				strings.push_back(ParseStr(s));
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


#ifdef _WIN32
		open_err = _wfopen_s(&file, file_name.c_str(), L"wt, ccs=UTF-8");
#elif __linux__
		open_err = fopen_s(&file, file_name.c_str(), "wt");
#endif
		if (file == nullptr || open_err)
			return false;

		for (const auto& es : strings)
		{
			if (es.str.size())
			{
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
