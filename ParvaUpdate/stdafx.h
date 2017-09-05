// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"


//#define WIN32_LEAN_AND_MEAN             // 从 Windows 头中排除极少使用的资料
// Windows 头文件: 
#include <windows.h>
#include <CommCtrl.h>
#include <WinUser.h>

// C 运行时头文件
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


// TODO:  在此处引用程序需要的其他头文件
#define CURL_STATICLIB
//#define _CRYPT  define in complie
#include <assert.h> 
#include <string>

#ifdef _DEBUG
#pragma comment(lib, "libcurld.lib")	  
#else
#pragma comment(lib, "libcurl.lib") 
#endif

#pragma comment ( lib, "ws2_32.lib" )
#pragma comment ( lib, "wldap32.lib" ) 

#ifdef _CRYPT 
#pragma comment (lib , "Crypt32.lib")
#endif // _CYRPT 


