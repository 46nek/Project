#pragma once
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN
// Windows
#include <windows.h>
// C Runtime
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <comdef.h>
// C++
#include <string>

// デバッグ用のログ出力マクロ
#if defined(_DEBUG)
inline void DbgPrint(const std::wstring& msg)
{
	wprintf(L"%s\n", msg.c_str());
}
#else
#define DbgPrint(msg)
#endif