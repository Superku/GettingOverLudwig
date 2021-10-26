///////////////////////////////
#include <windows.h>
#include <d3d9.h>
///////////////////////////////

#define DISPLAY_MODES_COMBINED

typedef struct _MONITOR_HELPER_LIST
{
	void *data;
	struct _MONITOR_HELPER_LIST *next;
} MONITOR_HELPER_LIST;

#undef D3DDISPLAYMODE
typedef struct D3DDISPLAYMODE {
	UINT      Width;
	UINT      Height;
	UINT      RefreshRate;
	D3DFORMAT Format;
} D3DDISPLAYMODE;
typedef D3DDISPLAYMODE DISPLAYMODE;

/*typedef struct
{
	int Width;
	int Height;
} DISPLAYMODE;*/

typedef struct
{
	char DeviceName[32];
	RECT rect, monitorRect;
	int numModes, flags, width, height, frequency, posX, posY, DPI, appScaling;
	//int monitorWidth, monitorHeight;
	HMONITOR hmonitor;
	DISPLAYMODE *displayModes;
} MYMONITORINFO;

typedef struct
{
	int numGraphicscards,currentGraphicscard;
	char** cGraphicscards;
	int currentWidth,currentHeight,currentFrequency;
	int currentAdapterNum,launchAdapterNum;
	int numMonitors;
	MYMONITORINFO* monitorInfo;
	//MYMONITORINFO* monitorInfo2;
	MYMONITORINFO* monitorInfoCombined;
} MULTIMONITORINFO;

MULTIMONITORINFO multiMonitorInfo; // this can be used directly after calling multiMonitorGetInfo() at least once 
int mmInfoLastError = 0;

///////////////////////////////

// main function
int multiMonitorGetInfo();

// access functions

int MMInfoGetNumMonitors() { return multiMonitorInfo.numMonitors; } // used to be defines, didnt work
int MMInfoGetLaunchAdapterNum() { return multiMonitorInfo.launchAdapterNum; } // -mon X. 0 == default
int MMInfoGetCurrentAdapterNum() { return multiMonitorInfo.currentAdapterNum; }
int MMInfoGetNumGraphicscards() { return multiMonitorInfo.numGraphicscards; }
int MMInfoGetCurrentGraphicscardNum() { return multiMonitorInfo.currentGraphicscard; }
int MMInfoGetCurrentWidth() { return multiMonitorInfo.currentWidth; }
int MMInfoGetCurrentHeight() { return multiMonitorInfo.currentHeight; }
int MMInfoGetCurrentFrequency() { return multiMonitorInfo.currentFrequency; }

char* MMInfoGetGraphicscardName(int i);

MYMONITORINFO* MMInfoGetMonitorInfo(int monitorID);

int MMInfoGetDisplayNumModes(int monitorID);

DISPLAYMODE* MMInfoGetDisplayMode(int monitorID, int modeID);

///////////////////////////////
// helper functions

MONITOR_HELPER_LIST* monitorHelperListAdd(MONITOR_HELPER_LIST* list, void* data);

void monitorHelperListDestroy(MONITOR_HELPER_LIST* list, int mode);

int MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, RECT *lprcMonitor, LPARAM dwData);

///////////////////////////////
// Windows and DirectX defines

#define DISPLAY_DEVICE_ATTACHED_TO_DESKTOP      0x00000001
#define DISPLAY_DEVICE_MULTI_DRIVER             0x00000002
#define DISPLAY_DEVICE_PRIMARY_DEVICE           0x00000004
#define DISPLAY_DEVICE_MIRRORING_DRIVER         0x00000008
#define MAX_DEVICE_IDENTIFIER_STRING        512
#define ENUM_CURRENT_SETTINGS -1
#define SM_CMONITORS 80
#define HMONITOR int
#define HDC int
#define CHAR char
#define D3DFMT_X8R8G8B8 22
#define D3D_OK 0

typedef struct LARGE_INTEGER
{
	int QuadPart,extra32bit;
} LARGE_INTEGER;

#define MAX_DEVICE_IDENTIFIER_STRING        512
#undef D3DADAPTER_IDENTIFIER9
typedef struct D3DADAPTER_IDENTIFIER9 {
	char          Driver[MAX_DEVICE_IDENTIFIER_STRING];
	char          Description[MAX_DEVICE_IDENTIFIER_STRING];
	char          DeviceName[32];
	LARGE_INTEGER DriverVersion;
	DWORD         DriverVersionLowPart;
	DWORD         DriverVersionHighPart;
	DWORD         VendorId;
	DWORD         DeviceId;
	DWORD         SubSysId;
	DWORD         Revision;
	GUID          DeviceIdentifier;
	DWORD         WHQLLevel;
} D3DADAPTER_IDENTIFIER9;
D3DADAPTER_IDENTIFIER9 d3dAdapterIdentifier;

typedef struct _devicemode 
{ 
	char  dmDeviceName[32]; 
	WORD   dmSpecVersion; 
	WORD   dmDriverVersion; 
	WORD   dmSize; 
	WORD   dmDriverExtra; 
	DWORD  dmFields; 
	short dmOrientation;
	short dmPaperSize;
	short dmPaperLength;
	short dmPaperWidth;
	short dmScale; 
	short dmCopies; 
	short dmDefaultSource; 
	short dmPrintQuality; 
	short  dmColor; 
	short  dmDuplex; 
	short  dmYResolution; 
	short  dmTTOption; 
	short  dmCollate; 
	BYTE  dmFormName[32]; 
	WORD  dmLogPixels; 
	DWORD  dmBitsPerPel; 
	DWORD  dmPelsWidth; 
	DWORD  dmPelsHeight; 
	DWORD  dmDisplayFlags; 
	DWORD  dmDisplayFrequency; 
	DWORD  dmICMMethod;
	DWORD  dmICMIntent;
	DWORD  dmMediaType;
	DWORD  dmDitherType;
	DWORD  dmReserved1;
	DWORD  dmReserved2;
	DWORD  dmPanningWidth;
	DWORD  dmPanningHeight;
} SCRMODE; 
SCRMODE resolution;

typedef struct _DISPLAY_DEVICEA
{
	DWORD cb;
	CHAR  DeviceName[32];
	CHAR  DeviceString[128];
	DWORD StateFlags;
	CHAR  DeviceID[128];
	CHAR  DeviceKey[128];
} DISPLAY_DEVICEA;


// -
// CheckDeviceType GetAdapterDisplayMode

#undef D3DDEVICE_CREATION_PARAMETERS
typedef struct
{
	UINT       AdapterOrdinal;
	D3DDEVTYPE DeviceType;
	HWND       hFocusWindow;
	DWORD      BehaviorFlags;
} D3DDEVICE_CREATION_PARAMETERS;


typedef struct
{
	DWORD cbSize;
	RECT  rcMonitor;
	RECT  rcWork;
	DWORD dwFlags;
} MONITORINFO;

typedef struct
{
	DWORD cbSize;
	RECT  rcMonitor;
	RECT  rcWork;
	DWORD dwFlags;
	char szDevice[32];
} MONITORINFOEX;


typedef struct _devicemodeA {
  BYTE  dmDeviceName[CCHDEVICENAME]; // CCHDEVICENAME
  WORD  dmSpecVersion;
  WORD  dmDriverVersion;
  WORD  dmSize;
  WORD  dmDriverExtra;
  DWORD dmFields;
  POINTL dmPosition;
  long dummyFillerA, dummyFillerB;
  short dmColor;
  short dmDuplex;
  short dmYResolution;
  short dmTTOption;
  short dmCollate;
  BYTE  dmFormName[CCHFORMNAME];
  WORD  dmLogPixels;
  DWORD dmBitsPerPel;
  DWORD dmPelsWidth;
  DWORD dmPelsHeight;
  DWORD dmDisplayFlags;
  DWORD dmDisplayFrequency;
  DWORD dmICMMethod;
  DWORD dmICMIntent;
  DWORD dmMediaType;
  DWORD dmDitherType;
  DWORD dmReserved1;
  DWORD dmReserved2;
  DWORD dmPanningWidth;
  DWORD dmPanningHeight;
} DEVMODEA;
DEVMODEA devmodeAInstance;


///////////////////////////////

var menu_monitor_identification = 2;
int resolutionIDWanted = -2;
int primMonitorID = 0;

#include "kuGetDisplayModes.c"
