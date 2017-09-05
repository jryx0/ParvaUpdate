// ParvaUpdate.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "ParvaUpdate.h"
#include "cpr/cpr.h"


#include "ParvaImg.h"

#define MAX_LOADSTRING 100

// ȫ�ֱ���: 
HINSTANCE hInst;                                // ��ǰʵ��
HWND    hwndMainDlg;
WCHAR szTitle[MAX_LOADSTRING];                  // �������ı�
WCHAR szWindowClass[MAX_LOADSTRING];            // ����������
CHAR szRemoteServer[MAX_LOADSTRING];			// Ĭ��Զ�̷�������ַ
CHAR szInstallJson[MAX_LOADSTRING];				// Ĭ�����������ļ���


HWND hwndDownloadProgress; //���ؽ�����
HWND hwndSetupProgress;    //��װ������

LRESULT CALLBACK mainDlgProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM);
static void goToScreenCenter(HWND hwnd);


static DWORD WINAPI startInstallProc(void *);
//�ṩ��CURL���ؽ��Ȼص��ĺ��������ڱ������ص����ݵ��ļ�  
static size_t   DownloadCallback(void* pBuffer, size_t nSize, size_t nMemByte, void* pParam);
//�ṩ��CURL���ؽ��Ȼص��ĺ��������ڼ������ؽ���֪ͨ����  
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
			MessageBox(hWndDlg, _T("OK"), _T("���Ա���"), MB_OK);
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
		pi->DrawImage(_T("res\\����.png"), rc);
		delete pi;
		EndPaint(hWndDlg, &ps);*/

		hwndiMG = ::GetDlgItem(hWndDlg, IDC_SHOWIMAGE);

		auto hdc = BeginPaint(hwndiMG, &ps);
		GetWindowRect(hwndiMG, &rc);

		ParvaImg* pi = new ParvaImg(hdc);
		pi->SetDC(hdc);
		//pi->DrawImage(hInst, IDB_PARVAPNG, rc);
		pi->DrawImage(_T("res\\����.png"), rc);
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
	//���ý������ݵĻص�
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, DownloadCallback);
	//���ý��Ȼص�����
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
	//�鿴�Ƿ��г�����Ϣ
	const char* pError = curl_easy_strerror(res);
	::MessageBoxA(hwndMainDlg, pError, "����", MB_OK | MB_ICONERROR);
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
	//�����ص���������׷�ӵķ�ʽд���ļ�(һ��Ҫ��a������ǰ��д������ݾͻᱻ������)  
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
	//֪ͨ�������������ؽ���  
	//::PostMessage(hwndDownloadProgress, WM_USER + 110, nPos, 0);
	//::Sleep(10);  
	return 0;
}



