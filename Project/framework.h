// header.h : 標準のシステム インクルード ファイルのインクルード ファイル、
// またはプロジェクト専用のインクルード ファイル
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーからほとんど使用されていない部分を除外する
// Windows ヘッダー ファイル
#include <windows.h>
// C ランタイム ヘッダー ファイル
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <comdef.h> 

// デバッグ用のログ出力マクロ
#if defined(_DEBUG)
#include <string>
inline void DbgPrint(const std::wstring& msg)
{
    wprintf(L"%s\n", msg.c_str());
}
#else
#define DbgPrint(msg)
#endif
