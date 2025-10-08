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

// デバッグ用のログ出力マクロ
#if defined(_DEBUG)
#include <string>
inline void DbgPrint(const std::wstring& msg)
{
    OutputDebugStringW((msg + L"\n").c_str()); // メッセージの末尾に改行を追加
}
#else
#define DbgPrint(msg)
#endif