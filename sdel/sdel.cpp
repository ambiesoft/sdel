// sdel.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "../../lsMisc/CommandLineParser.h"
#include "../../lsMisc/SHMoveFile.h"
#include "../../lsMisc/stdwin32/stdwin32.h"

using namespace Ambiesoft;
using namespace stdwin32;

using std::wcin;
using std::wcout;
using std::wcerr;
using std::endl;
using std::wstring;
using std::vector;

typedef std::vector<std::wstring> StringVector;

void ShowHelp()
{
	const static wchar_t *p2spaces = L"  ";
	
	wcout << I18N(L"Recyclebins one or more files or directories.") << endl;
	wcout << endl;
	wcout << L"SDEL [/P] names" << endl;
	wcout << endl;
	wcout << p2spaces << L"names" << L"         " << I18N(L"Specifies a list of one or more files or directories.") << endl;
	wcout << p2spaces << L"/P" << L"            " << I18N(L"Promts for confirmation before recyclebins") << endl;
}

// https://stackoverflow.com/a/2209277
bool PromptForChar( const wchar_t* prompt, wchar_t& readch )
{
    std::wstring tmp;
    std::wcout << prompt; // << std::endl;
    if (std::getline(std::wcin, tmp))
    {
        // Only accept single character input
        if (tmp.length() == 1)
        {
            readch = tmp[0];
        }
        else
        {
            // For most input, char zero is an appropriate sentinel
            readch = L'\0';
        }
        return true;
    }
    return false;
}
int _tmain(int argc, _TCHAR* argv[])
{
	CCommandLineParser parser(CaseFlags_Insensitive);
	COption mainArgs(L"");
	parser.AddOption(&mainArgs);

	bool isHelp;
	parser.AddOption(L"/h", L"/?", 0, &isHelp);
	
	bool isPrompt;
	parser.AddOption(L"/p", 0, &isPrompt);

	bool isNoPreserveParent;
	parser.AddOption(L"/NoPreserveParent", 0, &isNoPreserveParent);

	bool isNoPreserveCurrent;
	parser.AddOption(L"/NoPreserveCurrent", 0, &isNoPreserveCurrent);

	parser.Parse();

	if(isHelp)
	{
		ShowHelp();
		return 0;
	}
	if(!parser.getUnknowOptionStrings().empty())
	{
		wcerr << I18N(L"Unknown options:") << parser.getUnknowOptionStrings() << endl;
		return 1;
	}
	if(!mainArgs.hadValue())
	{
		wcerr << I18N(L"No Input Files") << endl;
		return 1;
	}

	StringVector sv;
	for(size_t i=0 ; i < mainArgs.getValueCount() ;++i)
	{
		wstring t = mainArgs.getValue(i);
		t = stdwin32::trimW(t, L"\\");
		if(t==L"." && !isNoPreserveCurrent)
		{
			wcerr << I18N(L"\".\" is not allowed, use /NoPreserveCurrent to do it.") << endl;
			return 1;
		}
		if(t==L".." && !isNoPreserveParent)
		{
			wcerr << I18N(L"\"..\" is not allowed, use /NoPreserveParent to do it.") << endl;
			return 1;
		}

		if(isPrompt)
		{
			wchar_t responce=0;
			wstring fullt=stdGetFullPathName(t.c_str());
			PromptForChar(string_format(I18N(L"Are you sure to sdel '%s'? "), fullt.c_str()).c_str(), responce);
			if( responce != L'y' && responce != L'Y')
				return 0;
		}

		sv.push_back(t);
	}

	FILEOP_FLAGS flags = FOF_ALLOWUNDO |
		(isPrompt ? 0 : FOF_NOCONFIRMATION) |
		0;
	
	int nRet=SHDeleteFile(sv, flags);
	if(nRet != 0)
	{
		wcerr << GetSHFileOpErrorString(nRet) << endl;
		return nRet;
	}
	return nRet;
}

