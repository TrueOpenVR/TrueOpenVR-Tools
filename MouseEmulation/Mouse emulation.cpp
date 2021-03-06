#include <windows.h>
#include <math.h>
#include <atlstr.h> 
#include <algorithm>
#include "IniReader\IniReader.h"

typedef struct _HMDData
{
	double	X;
	double	Y;
	double	Z;
	double	Yaw;
	double	Pitch;
	double	Roll;
} THMD, *PHMD;

#define TOVR_SUCCESS 0
#define TOVR_FAILURE 1

typedef DWORD(__stdcall *_GetHMDData)(__out THMD *HMD);
typedef DWORD(__stdcall *_SetCentering)(__in int dwIndex);

_GetHMDData GetHMDData;
_SetCentering SetCentering;

double SensX, SensY;
int last_x = 0, last_y = 0;

int MouseGetDelta(int val, int prev) //Implementation from OpenTrack https://github.com/opentrack/opentrack/blob/unstable/proto-mouse/
{
	const int a = std::abs(val - prev), b = std::abs(val + prev);
	if (b < a)
		return val + prev;
	else
		return val - prev;
}

void MousePose(const double axisX, const double axisY) //Implementation from OpenTrack https://github.com/opentrack/opentrack/blob/unstable/proto-mouse/
{
	int mouse_x = 0, mouse_y = 0;

	mouse_x = round(axisX * SensX * 2);
	mouse_y = round(axisY * SensY * 2);

	const int dx = MouseGetDelta(mouse_x, last_x);
	const int dy = MouseGetDelta(mouse_y, last_y);

	last_x = mouse_x;
	last_y = mouse_y;

	if (dx || dy)
	{
		INPUT input;
		input.type = INPUT_MOUSE;
		MOUSEINPUT& mi = input.mi;
		mi = {};
		mi.dx = dx;
		mi.dy = dy;
		mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_MOVE_NOCOALESCE;

		SendInput(1, &input, sizeof(input));
	}
}

int main()
{
	SetConsoleTitle(_T("TrueOpenVR Mouse emulation"));

	//Read parameters from config
	CIniReader IniFile("Config.ini");
	SensX = IniFile.ReadFloat("Main", "SensX", 4.5);
	SensY = IniFile.ReadFloat("Main", "SensY", 3.5);

	//Read parameters from registry
	CRegKey key;
	TCHAR libPath[MAX_PATH];

	LONG status = key.Open(HKEY_CURRENT_USER, _T("Software\\TrueOpenVR"));
	if (status == ERROR_SUCCESS)
	{
		ULONG libPathSize = sizeof(libPath);

		#ifdef _WIN64
			status = key.QueryStringValue(_T("Library64"), libPath, &libPathSize);
		#else
			status = key.QueryStringValue(_T("Library"), libPath, &libPathSize);
		#endif

		if (status != ERROR_SUCCESS)
		{
			printf("ERROR: TrueOpenVR library path not found");
			return 1;
		}

	}
	key.Close();


	//Load main library
	HMODULE hDll;

	if (PathFileExists(libPath)) {
		hDll = LoadLibrary(libPath);
		if (hDll != NULL) {
			//Load functions
			GetHMDData = (_GetHMDData)GetProcAddress(hDll, "GetHMDData");
			SetCentering = (_SetCentering)GetProcAddress(hDll, "SetCentering");
		}
		else {
			printf("ERROR: unable to load DLL\n");
			return 1;
		}
	}
	else {
		printf("TrueOpenVR library not found");
	}

	//Check functions
	if (GetHMDData == NULL) {
		printf("ERROR: unable to find GetHMDData DLL function\n");
		return 1;
	}

	if (SetCentering == NULL) {
		printf("ERROR: unable to find SetCentering DLL function\n");
		return 1;
	}

	THMD MyHMD;
	if (GetHMDData(&MyHMD) == TOVR_SUCCESS) {
		printf("HMD found, mouse movement activated.\n");
	}
	else
	{
		printf("HMD not found.\n");
		return 1;
	}

	//Get data
	while (true) {
		GetHMDData(&MyHMD);
		MousePose(MyHMD.Yaw, MyHMD.Pitch);

		//Centring
		if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0 && (GetAsyncKeyState(VK_MENU) & 0x8000) != 0 && (GetAsyncKeyState(82) & 0x8000) != 0) //HMD - CTRL + ALT + R 
			SetCentering(0);

	}

	FreeLibrary(hDll);
	hDll = nullptr;

	return 0;
}
