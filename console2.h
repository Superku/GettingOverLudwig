// made by Rackscha, adapted by Superku

//#define DEVTRUE

#ifndef Console_h
	#define Console_h
	#include <windows.h>

	long WINAPI WriteConsole(int Handle, char* Buffer, int CharsToWrite, int* CharsWritten, int reserved);
	long WINAPI CreateConsoleScreenBuffer(long dwDesiredAccess, long dwShareMode, long *lpSecurityAttributes, long dwFlags, long lpScreenBufferData);
	long WINAPI SetConsoleActiveScreenBuffer(long hConsoleOutput);
	long GConsoleBuffer;
	int consoleInitialized = 0;
	#ifdef DEVTRUE
		int consolePrintTrue = 1;
		int consolePrintTarget = 2;
		#else
		int consolePrintTrue = 0;
		int consolePrintTarget = 1; // 2
	#endif

	void consoleInit()
	{
		if(consoleInitialized) return;
		consoleInitialized = 1;
		AllocConsole();
		GConsoleBuffer = CreateConsoleScreenBuffer(GENERIC_WRITE, FILE_SHARE_READ, 0, CONSOLE_TEXTMODE_BUFFER, 0);
		SetConsoleActiveScreenBuffer(GConsoleBuffer);	
		//SetWindowPos( GConsoleBuffer, 0, -800, -100, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
		//MoveWindow( GConsoleBuffer, -600, -100, 0, 0, TRUE);
	}

	void cdiag(char* AText)
	{
		if(!consolePrintTrue) return;
		if(consolePrintTarget) diag(AText);
		if(consolePrintTarget == 0 || consolePrintTarget == 2)
		{
			if(!consoleInitialized) consoleInit();
			WriteConsole(GConsoleBuffer, AText, str_len(AText), NULL, 0);
		}
	}
	
	#define cprintf0(str) cdiag(_chr(str))
	#define cprintf1(str,arg1) cdiag(_chr(str_printf(NULL,str,arg1)))
	#define cprintf2(str,arg1,arg2) cdiag(_chr(str_printf(NULL,str,arg1,arg2)))
	#define cprintf3(str,arg1,arg2,arg3) cdiag(_chr(str_printf(NULL,str,arg1,arg2,arg3)))
	#define cprintf4(str,arg1,arg2,arg3,arg4) cdiag(_chr(str_printf(NULL,str,arg1,arg2,arg3,arg4)))
	#define cprintf5(str,arg1,arg2,arg3,arg4,arg5) cdiag(_chr(str_printf(NULL,str,arg1,arg2,arg3,arg4,arg5)))
	#define cprintf6(str,arg1,arg2,arg3,arg4,arg5,arg6) cdiag(_chr(str_printf(NULL,str,arg1,arg2,arg3,arg4,arg5,arg6)))
	#define cprintf7(str,arg1,arg2,arg3,arg4,arg5,arg6,arg7) cdiag(_chr(str_printf(NULL,str,arg1,arg2,arg3,arg4,arg5,arg6,arg7)))
	#define cprintf8(str,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8) cdiag(_chr(str_printf(NULL,str,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8)))
	#define cprintf9(str,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9) cdiag(_chr(str_printf(NULL,str,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9)))
	#define cprintf10(str,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10) cdiag(_chr(str_printf(NULL,str,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10)))
	
	STRING* cdiagDummyStr = "";
	#define CDIAG_IF_SLOW(time, cstr) { double dd1 = dtimer()*0.001; if(dd1 > time) cdiag(_chr(str_printf(cdiagDummyStr,"\n CDIAG(%s) at frame %d: %.3f > %.3f",cstr,(int)total_frames,dd1,(double)time))); }
	int cdiagIfSlow(double threshold, char *cstr)
	{
		double dd1 = dtimer()*0.001;
		if(dd1 > threshold)
		{
			cdiag(_chr(str_printf(cdiagDummyStr,"\n CDIAG(%s) at frame %d: %.3f > %.3f",cstr,(int)total_frames,dd1,threshold)));
			return 1;
		}
		return 0;
	}
#endif
