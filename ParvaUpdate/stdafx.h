// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"


//#define WIN32_LEAN_AND_MEAN             // �� Windows ͷ���ų�����ʹ�õ�����
// Windows ͷ�ļ�: 
#include <windows.h>
#include <CommCtrl.h>
#include <WinUser.h>

// C ����ʱͷ�ļ�
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


// TODO:  �ڴ˴����ó�����Ҫ������ͷ�ļ�
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


