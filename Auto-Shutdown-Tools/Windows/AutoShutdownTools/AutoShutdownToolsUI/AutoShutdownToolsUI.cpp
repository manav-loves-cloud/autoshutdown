/*************************************************************

	AutoShutdownToolsUI.cpp
	Copyright 2022 Sophos

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

		https://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.

*************************************************************/

// Dependencies
#include <windows.h>
#include <iostream>

// Namespace
using namespace std;

// Definitions
constexpr auto LOGFILENAME = L"AutoShutdownToolsUI.log";

// Global variables
wchar_t LOGFILE[MAX_PATH];

// Function prototypes
int WINAPI	WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
void		LogToFile(LPCWSTR pMessage);

/*************************************************************

	Main entry point for the application

*************************************************************/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// Local variables
	DWORD dwRetVal;
	DWORD dwSize;
	DWORD dwValue = 0;
	HKEY hKey;
	wchar_t cTempPath[MAX_PATH];

	// Setup the log path
	dwRetVal = GetTempPath(MAX_PATH, cTempPath);
	if (dwRetVal == 0 || dwRetVal > MAX_PATH)
	{
		MessageBox(NULL, L"Error starting.", L"Auto Shutdown UI", MB_ICONERROR);
		return 1;
	}
	wsprintf(LOGFILE, L"%s\\%s", cTempPath, LOGFILENAME);

	// Create or open the registry key
	dwRetVal = RegCreateKeyEx(HKEY_LOCAL_MACHINE, L"Software\\AutoShutdownToolsService\\AutoShutdownToolsUI", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	if (dwRetVal != ERROR_SUCCESS)
	{
		LogToFile(L"Error creating/opening registry key.");
		return 1;
	}
	else
	{
		// Setup registry keys
		dwRetVal = RegSetValueEx(hKey, L"Timeout", 0, REG_DWORD, (const BYTE*)&dwValue, sizeof(dwValue));
		if (dwRetVal != ERROR_SUCCESS)
			LogToFile(L"Error writing to registry: (timeout)");
		dwRetVal = RegSetValueEx(hKey, L"Active", 0, REG_DWORD, (const BYTE*)&dwValue, sizeof(dwValue));
		if (dwRetVal != ERROR_SUCCESS)
			LogToFile(L"Error writing to registry: (active)");
	}
	RegCloseKey(hKey);
	hKey = NULL;

	// Control loop
	long nRegTimeoutState = 0;
	while (true)
	{
		// One minute of looping and checking for shutdown every 5 seconds
		for (int x = 0; x < 12; x++)
		{
			::Sleep(5000);
			//
			// Check for shutdown
			if (GetSystemMetrics(SM_SHUTTINGDOWN) != 0)
			{
				goto ShuttingDownNow;
			}
		}

		dwRetVal = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"Software\\AutoShutdownToolsService\\AutoShutdownToolsUI", 0, KEY_ALL_ACCESS, &hKey);
		if (dwRetVal == ERROR_SUCCESS)
		{
			dwRetVal = RegQueryValueEx(hKey, L"Timeout", NULL, NULL, (LPBYTE)&nRegTimeoutState, &dwSize);
			if (dwRetVal == ERROR_SUCCESS)
			{
				if (nRegTimeoutState > 0)
				{
					// Display the dialog box to see if the computer is still being used
					if (MessageBox(NULL, L"If you are still using this VM please click OK.\r\rIf you do not click OK this VM will shutdown\rin 5 minutes.", L"Auto Shutdown Tools UI", MB_OK | MB_ICONSTOP | MB_SYSTEMMODAL | MB_SETFOREGROUND | MB_TOPMOST) == IDOK)
					{
						dwValue = 1;
						dwRetVal = RegSetValueEx(hKey, L"Active", 0, REG_DWORD, (const BYTE*)&dwValue, sizeof(dwValue));
						dwValue = 0;
						dwRetVal = RegSetValueEx(hKey, L"Timeout", 0, REG_DWORD, (const BYTE*)&dwValue, sizeof(dwValue));
					}
				}
			}
			RegCloseKey(hKey);
			hKey = NULL;
		}
	}

ShuttingDownNow:
	return 0;
} // end winmain

/*************************************************************

	Write a message to the log file

*************************************************************/
void LogToFile(LPCWSTR pMessage)
{
	try
	{
		SYSTEMTIME sysTime;
		GetLocalTime(&sysTime);
		FILE* fpLog;
		_wfopen_s(&fpLog, LOGFILE, L"a");
		if (fpLog)
		{
			fwprintf(fpLog, L"%02d/%02d/%04d, %02d:%02d:%02d - %s\n", sysTime.wDay, sysTime.wMonth, sysTime.wYear, sysTime.wHour, sysTime.wMinute, sysTime.wSecond, pMessage);
			fclose(fpLog);
		}
	}
	catch (...) {}
} // end logtofile