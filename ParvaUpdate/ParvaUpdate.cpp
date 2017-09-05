// ParvaUpdate.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "ParvaUpdate.h"
#include "cpr/cpr.h"


#include "ParvaImg.h"

#define MAX_LOADSTRING 100

// 全局变量: 
HINSTANCE hInst;                                // 当前实例
HWND    hwndMainDlg;
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
CHAR szRemoteServer[MAX_LOADSTRING];			// 默认远程服务器地址
CHAR szInstallJson[MAX_LOADSTRING];				// 默认离线配置文件名


HWND hwndDownloadProgress; //下载进度条
HWND hwndSetupProgress;    //安装进度条

LRESULT CALLBACK mainDlgProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM);
static void goToScreenCenter(HWND hwnd);


static DWORD WINAPI startInstallProc(void *);
//提供给CURL下载进度回调的函数，用于保存下载的数据到文件  
static size_t   DownloadCallback(void* pBuffer, size_t nSize, size_t nMemByte, void* pParam);
//提供给CURL下载进度回调的函数，用于计算下载进度通知界面  
static int ProgressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);



int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	hInst = hInstance;

	memset(szRemoteServer, 0, MAX_LOADSTRING);
	memset(szInstallJson, 0, MAX_LOADSTRING);
	::LoadStringA(hInst, IDS_SERVERADDR, szRemoteServer, MAX_LOADSTRING);
	::LoadStringA(hInst, IDS_INSTALLJSON, szInstallJson, MAX_LOADSTRING);

	::DialogBox(hInst, (LPCTSTR)IDD_MAINDLG, GetDesktopWindow(), reinterpret_cast<DLGPROC>(mainDlgProc));
}



LRESULT CALLBACK mainDlgProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{

	static bool isMouseDown = false;
	static int x, y;
	hwndMainDlg = hWndDlg;
	static HWND hwndiMG;

	switch (Msg)
	{
	case WM_INITDIALOG:
		hwndDownloadProgress = ::GetDlgItem(hWndDlg, IDC_PROGRESSDOWNLOAD);
		SendMessage(hwndDownloadProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100));

		hwndSetupProgress = ::GetDlgItem(hWndDlg, IDC_PROGRESSSETUP);
		SendMessage(hwndSetupProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100));

		/* progress increase*/
		PostMessage(hwndSetupProgress, PBM_SETSTEP, (WPARAM)60, 0);
		PostMessage(hwndSetupProgress, PBM_STEPIT, 0, 0);


		goToScreenCenter(hWndDlg);

		::CreateThread(NULL, 0, startInstallProc, NULL, 0, NULL);

		break;
	case WM_COMMAND:
		switch (wParam)
		{
		case IDOK:
		case IDCANCEL:
			MessageBox(hWndDlg, _T("OK"), _T("测试标题"), MB_OK);
			EndDialog(hWndDlg, 0);
			return TRUE;
		}
		break;


	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		RECT  rc;

		/*	auto hdc = BeginPaint(hWndDlg, &ps);
		GetWindowRect(hWndDlg, &rc);
		ParvaImg* pi = new ParvaImg(hdc);
		pi->DrawImage(_T("res\\背景.png"), rc);
		delete pi;
		EndPaint(hWndDlg, &ps);*/

		hwndiMG = ::GetDlgItem(hWndDlg, IDC_SHOWIMAGE);

		auto hdc = BeginPaint(hwndiMG, &ps);
		GetWindowRect(hwndiMG, &rc);

		ParvaImg* pi = new ParvaImg(hdc);
		pi->SetDC(hdc);
		//pi->DrawImage(hInst, IDB_PARVAPNG, rc);
		pi->DrawImage(_T("res\\背景.png"), rc);
		delete pi;

		EndPaint(hwndiMG, &ps);
	}
	break;

	case WM_LBUTTONDOWN:
		isMouseDown = true;
		x = LOWORD(lParam);
		y = HIWORD(lParam);
		break;
	case WM_MOUSEMOVE:
		if (isMouseDown)
		{
			int xPos = LOWORD(lParam);
			int yPos = HIWORD(lParam);

			RECT rc;
			::GetWindowRect(hWndDlg, &rc);

			rc.left += (xPos - x);
			rc.top += (yPos - y);
			::SetWindowPos(hWndDlg, HWND_TOP, rc.left, rc.top, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);
		}
		break;
	case WM_LBUTTONUP:
		isMouseDown = false;
		break;
	}
	return FALSE;
}


static void goToScreenCenter(HWND hwnd)
{
	RECT screenRc;
	::SystemParametersInfo(SPI_GETWORKAREA, 0, &screenRc, 0);

	POINT center;
	center.x = screenRc.left + (screenRc.right - screenRc.left) / 2;
	center.y = screenRc.top + (screenRc.bottom - screenRc.top) / 2;

	RECT rc;
	::GetWindowRect(hwnd, &rc);
	int x = center.x - (rc.right - rc.left) / 2;
	int y = center.y - (rc.bottom - rc.top) / 2;

	::SetWindowPos(hwnd, HWND_TOP, x, y, rc.right - rc.left, rc.bottom - rc.top, SWP_SHOWWINDOW);
};

static DWORD WINAPI startInstallProc(void *)
{
	/*
	1.init curl
	2.download json file
	3.anlyze json file
	4.download file and save to tmp path
	5.copy or unzip file
	6.set reg or set path
	7.clear file
	8.show success info


	CURL *curl;
	CURLcode res = CURLE_FAILED_INIT;

	curl = curl_easy_init();
	if (curl)
	{
	//设置接收数据的回调
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, DownloadCallback);
	//设置进度回调函数
	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, ProgressCallback);
	curl_easy_setopt(curl, CURLOPT_URL,  szRemoteServer);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0);

	res = curl_easy_perform(curl);
	if (res == CURLE_OK)
	{
	long retcode = 0;
	CURLcode code = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &retcode);
	if (retcode != 200)
	{//error find file!

	}
	}
	else
	{
	//查看是否有出错信息
	const char* pError = curl_easy_strerror(res);
	::MessageBoxA(hwndMainDlg, pError, "错误", MB_OK | MB_ICONERROR);
	}
	curl_easy_cleanup(curl);
	}	*/

	auto r = cpr::Get(cpr::Url{ "http://156.18.1.93/tmpa/login.aspx" });
	if (r.status_code == 200)
	{
	
 	}
	
	r = cpr::Get(cpr::Url{ "http://156.18.1.93/tmpa/OfflineInstall.json" },
		cpr::Authentication{ "user", "pass" });
	r.status_code;                  // 200
	r.header["content-type"];       // application/json; charset=utf-8
	r.text;                         // JSON text string







	

	return 0;
}


static size_t DownloadCallback(void* pBuffer, size_t nSize, size_t nMemByte, void* pParam)
{
	//把下载到的数据以追加的方式写入文件(一定要有a，否则前面写入的内容就会被覆盖了)  
	FILE* fp = NULL;
	fopen_s(&fp, "d:\\test.html", "ab+");
	size_t nWrite = fwrite(pBuffer, nSize, nMemByte, fp);
	fclose(fp);
	return nWrite;
}

static int ProgressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
	if (dltotal > -0.1 && dltotal < 0.1)
		return 0;
	int nPos = (int)((dlnow / dltotal) * 100);
	//通知进度条更新下载进度  
	//::PostMessage(hwndDownloadProgress, WM_USER + 110, nPos, 0);
	//::Sleep(10);  
	return 0;
}



