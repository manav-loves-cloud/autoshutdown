/*************************************************************

	AutoShutdownToolsService.cpp
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
#include <WtsApi32.h>

// Linking
#pragma comment(lib,"WtsApi32.lib")

// Namespace
using namespace std;

// Definitions
constexpr auto SERVICENAME			= L"AutoShutdownToolsService";
constexpr auto SERVICEDISPLAYNAME	= L"Auto Shutdown Tools Service";
constexpr auto LOGFILENAME			= L"AutoShutdownToolsService.log";

// Global variables
wchar_t					LOGFILE[MAX_PATH];
SERVICE_STATUS			gServiceStatus;
SERVICE_STATUS_HANDLE	ghServiceStatusHandle;

// Function prototypes
int			wmain(int argc, wchar_t* argv[]);
bool		InstallService();
bool		UninstallService();
void WINAPI ServiceMain(DWORD argc, LPTSTR *argv);
void WINAPI	ServiceControlHandler(DWORD dwOpCode);
void WINAPI MainWorkerThread(LPVOID lpParam);
void WINAPI ServiceThread(LPVOID lpParam);
void		InitiateShutdown();
void		LogToFile(LPCWSTR pMessage);

/*************************************************************

	Main entry point for the application

*************************************************************/
int wmain(int argc, wchar_t* argv[])
{
	// Handle commands
	if (argc > 1)
	{
		if (_wcsicmp(L"install", argv[1]) == 0)
		{
			if (InstallService())
				wcout << L"Auto Shutdown Tools Service installed." << endl;
			else
				wcout << L"ERROR: Failed to install Auto Shutdown Tools Service." << endl;
		}
		else if (_wcsicmp(L"uninstall", argv[1]) == 0)
		{
			if (UninstallService())
				wcout << L"Auto Shutdown Service uninstalled." << endl;
			else
				wcout << L"ERROR: Failed to uninstall Auto Shutdown Tools Service." << endl;
		}
		else
			wcout << L"Usage: AutoShutdownToolsService [install | uninstall]" << endl;
	}

	// Start the service
	SERVICE_TABLE_ENTRY DispatchTable[] = { {PWSTR(SERVICENAME), ServiceMain}, {NULL, NULL}};
	StartServiceCtrlDispatcher(DispatchTable);

	return 0;
} // end main

/*************************************************************

	Function to install the service

*************************************************************/
bool InstallService()
{
	// Try and get a handle to the service control manager with permissions to create a new service
	SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (!hSCM)
	{
		wcout << L"Failed to get SC_HANDLE: " << GetLastError() << L"For installation ensure you run as administrator" << endl;
		return false;
	}

	// Try to get a handle to the service to see if it is already installed
	SC_HANDLE hService = OpenService(hSCM, SERVICENAME, SERVICE_ALL_ACCESS);
	if (hService)
	{
		wcout << L"The service is already installed." << endl;
		CloseServiceHandle(hService);
		CloseServiceHandle(hSCM);
		return false;
	}

	// Get the full path to this file
	wchar_t cServiceBinary[MAX_PATH];
	if (!GetModuleFileName(NULL, cServiceBinary, MAX_PATH))
	{
		wcout << L"Failed to get service binary path." << endl;
		CloseServiceHandle(hSCM);
		return false;
	}

	// Try and install the service
	hService = CreateService(hSCM, SERVICENAME, SERVICEDISPLAYNAME, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, cServiceBinary, NULL, NULL, NULL, NULL, NULL);
	if (!hService)
	{
		wcout << L"Failed to create service: " << GetLastError() << endl;
		CloseServiceHandle(hSCM);
		return false;
	}

	// Try to start the service
	if (!StartService(hService, 0, NULL))
	{
		wcout << L"Failed to start service: " << GetLastError() << endl;
		CloseServiceHandle(hService);
		CloseServiceHandle(hSCM);
		return false;
	}


	// Cleanup
	CloseServiceHandle(hService);
	CloseServiceHandle(hSCM);
	return true;
} // end installservice

/*************************************************************

	Function to uninstall the service

*************************************************************/
bool UninstallService()
{
	// Try and get a handle to the service control manager with permissions to create a new service
	SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (!hSCM)
	{
		wcout << L"Failed to get SC_HANDLE: " << GetLastError() << L"For uninstallation ensure you run as administrator" << endl;
		return false;
	}

	// Try to get a handle to the service
	SC_HANDLE hService = OpenService(hSCM, SERVICENAME, SERVICE_ALL_ACCESS);
	if (!hService)
	{
		wcout << L"Failed to get a handle to the service. Is it installed?" << endl;
		CloseServiceHandle(hSCM);
		return false;
	}

	// Get the service state
	SERVICE_STATUS getServiceStatus;
	if (ControlService(hService, SERVICE_CONTROL_INTERROGATE, &getServiceStatus))
	{
		if (getServiceStatus.dwCurrentState == SERVICE_RUNNING)
		{
			// Try and stop the service - if stop on shutdown is enabled the service may not get to uninstall before shutdown
			if (!ControlService(hService, SERVICE_CONTROL_STOP, &getServiceStatus))
			{
				wcout << L"Error stopping service." << endl;
			}
		}
	}
	else
		wcout << L"Error getting the service status." << endl;

	// Try to delete the service
	if (!DeleteService(hService))
	{
		wcout << L"Failed to delete the service: " << GetLastError() << endl;
		CloseServiceHandle(hService);
		CloseServiceHandle(hSCM);
		return false;
	}

	// Cleanup
	CloseServiceHandle(hService);
	CloseServiceHandle(hSCM);
	return true;
} // end uninstallservice

/*************************************************************

	Main entry point for the service

*************************************************************/
void WINAPI ServiceMain(DWORD DwArgc, LPTSTR *argv)
{
	// Setup the log path
	DWORD	dwRetVal;
	wchar_t cTempPath[MAX_PATH];
	dwRetVal = GetTempPath(MAX_PATH, cTempPath);
	if (dwRetVal == 0 || dwRetVal > MAX_PATH)
	{
		wsprintf(LOGFILE, L"C:\\Windows\\Temp\\%s", LOGFILENAME);
		LogToFile(L"Failed to get temp path from environment");
	}
	else
		wsprintf(LOGFILE, L"%s\\%s", cTempPath, LOGFILENAME);

	// Configure the inital service status
	gServiceStatus.dwServiceType				= SERVICE_WIN32;
	gServiceStatus.dwCurrentState				= SERVICE_START_PENDING;
	gServiceStatus.dwControlsAccepted			= SERVICE_ACCEPT_STOP;
	gServiceStatus.dwWin32ExitCode				= 0;
	gServiceStatus.dwServiceSpecificExitCode	= 0;
	gServiceStatus.dwCheckPoint					= 0;
	gServiceStatus.dwWaitHint					= 0;

	// Get the service status handle
	ghServiceStatusHandle = RegisterServiceCtrlHandler(SERVICENAME, ServiceControlHandler);
	if (!ghServiceStatusHandle)
	{
		LogToFile(L"Failed to get the service status handle.");
		return;
	}

	// Set the service state to running
	gServiceStatus.dwCurrentState = SERVICE_RUNNING;
	gServiceStatus.dwCheckPoint = 0;
	gServiceStatus.dwWaitHint = 0;
	if (!SetServiceStatus(ghServiceStatusHandle, &gServiceStatus))
	{
		LogToFile(L"Failed to set the service status.");
		return;
	}

	// Create the main worker thread for the service
	DWORD dwThreadId;
	HANDLE hThread;
	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MainWorkerThread, NULL, 0, &dwThreadId);
	if (!hThread)
	{
		LogToFile(L"Failed to create main worker thread.");
		return;
	}

	// Wait for the service to end
	WaitForSingleObject(hThread, INFINITE);

	// Cleanup
	CloseHandle(hThread);
	hThread = NULL;

} // end servicemain

/*************************************************************

	This function recieves events to change the service state

*************************************************************/
void WINAPI	ServiceControlHandler(DWORD dwOpCode)
{
	// Handle service state requests - this service only accepts stop requests
	switch(dwOpCode)
	{
		case SERVICE_CONTROL_STOP:
			gServiceStatus.dwWin32ExitCode	= 0;
			gServiceStatus.dwCurrentState	= SERVICE_STOPPED;
			gServiceStatus.dwCheckPoint		= 0;
			gServiceStatus.dwWaitHint		= 0;
			SetServiceStatus(ghServiceStatusHandle, &gServiceStatus);
			break;
	}
} //  end servicecontrolhandler

/*************************************************************

	This function does all the work

*************************************************************/
void WINAPI MainWorkerThread(LPVOID lpParam)
{
	DWORD	dwRetVal;							// Store the return value
	HKEY	hKey;								// Store the handle to the open registry key
	DWORD	dwSize = sizeof(DWORD);				// Store the size of the registry value
	long	nIdleTimeout = 15;					// Default timeout value (minutes)
	long	nIdleCounter;						// Idle counter
	long	nActivityWait = 5;					// Defeault time to wait for a reponse (minutes)
	long	nActiveTimeout = 600000;			// Default timeout (milliseconds) 8 hours / 480 mins / 28,800 seconds
	long	nActiveTimeoutIncrement = 600000;	// How much to increment to the active time timeout by (milliseconds) 60 minutes / 3,600 seconds
	bool	bShutdownOnStop = false;			// Shutdown on stop
	bool	bDebugLogging = true;				// Log debug messages to file

	//
	// Open the registry key
	dwRetVal = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\AutoShutdownToolsService\\Parameters", 0, KEY_QUERY_VALUE, &hKey);

	//
	// Handle errors
	if (dwRetVal != ERROR_SUCCESS)
	{
		if (bDebugLogging)
			LogToFile(L"No configuration, using default value");
	}
	else
	{
		//
		// Variables for reading the registry
		long	nRegIdleTimeout;					// Timeout value in registry
		long	nRegActiveTimeout;					// Active timeout value in registry
		long	nRegActiveTimeoutIncrement;			// Active timeout increment in registry
		long	nRegShutdownOnStop;					// Shutdown on stop value in registry
		long	nRegActivityWait;
		long	nDebugLog = 0;

		// Check for a debug logging registry value - 0 = false, >0 = true
		dwRetVal = RegQueryValueExW(hKey, L"DebugLogging", NULL, NULL, (LPBYTE)&nDebugLog, &dwSize);
		if (dwRetVal != ERROR_SUCCESS)
		{
			if (bDebugLogging)
				LogToFile(L"Unable to read debug logging, disabling debug logging");
			bDebugLogging = false;
		}
		else
		{
			if (bDebugLogging)
				LogToFile(L"Using debug logging from registry");
			if (nDebugLog > 0)
				bDebugLogging = true;
			else
				bDebugLogging = false;
		}

		// Check for an idle timeout registry value - in minutes
		dwRetVal = RegQueryValueExW(hKey, L"Timeout", NULL, NULL, (LPBYTE)&nRegIdleTimeout, &dwSize);
		if (dwRetVal != ERROR_SUCCESS)
		{
			if (bDebugLogging)
				LogToFile(L"Unable to read timeout, using default value");
		}
		else
		{
			if (bDebugLogging)
				LogToFile(L"Using timeout from registry");
			nIdleTimeout = nRegIdleTimeout;
		}
		// Check for an idle timeout registry value - in minutes
		dwRetVal = RegQueryValueExW(hKey, L"ActivityWait", NULL, NULL, (LPBYTE)&nRegActivityWait, &dwSize);
		if (dwRetVal != ERROR_SUCCESS)
		{
			if (bDebugLogging)
				LogToFile(L"Unable to read Activity Wait, using default value");
		}
		else
		{
			if (bDebugLogging)
				LogToFile(L"Using Activity Wait from registry");
			nActivityWait = nRegActivityWait;
		}
		// Check for an active timeout registry value - in milliseconds (tested against boot time)
		dwRetVal = RegQueryValueExW(hKey, L"ActiveTimeout", NULL, NULL, (LPBYTE)&nRegActiveTimeout, &dwSize);
		if (dwRetVal != ERROR_SUCCESS)
		{
			if (bDebugLogging)
				LogToFile(L"Unable to read active timeout, using default value");
		}
		else
		{
			if (bDebugLogging)
				LogToFile(L"Using active timeout from registry");
			nActiveTimeout = nRegActiveTimeout;
		}

		// Check for an active timeout increment registry value - in milliseconds
		dwRetVal = RegQueryValueExW(hKey, L"ActiveTimeoutIncrement", NULL, NULL, (LPBYTE)&nRegActiveTimeoutIncrement, &dwSize);
		if (dwRetVal != ERROR_SUCCESS)
		{
			if (bDebugLogging)
				LogToFile(L"Unable to read active timeout increment, using default value");
		}
		else
		{
			if (bDebugLogging)
				LogToFile(L"Using active timeout increment from registry");
			nActiveTimeoutIncrement = nRegActiveTimeoutIncrement;
		}

		// Check for a shutdown on stop registry value - 0 = false, >0 = true
		dwRetVal = RegQueryValueExW(hKey, L"ShutdownOnStop", NULL, NULL, (LPBYTE)&nRegShutdownOnStop, &dwSize);
		if (dwRetVal != ERROR_SUCCESS)
		{
			if (bDebugLogging)
				LogToFile(L"Unable to read shutdown on stop, using default value");
		}
		else
		{
			if (bDebugLogging)
				LogToFile(L"Using shutdown on stop from registry");
			if (nRegShutdownOnStop > 0)
				bShutdownOnStop = true;
		}

		//
		// Close the registry key and set the pointer to NULL
		RegCloseKey(hKey);
		hKey = NULL;
	}

	//
	// Set idle counter
	nIdleCounter = nIdleTimeout;

	// Periodically check if the service is stopping.
	while (gServiceStatus.dwCurrentState == SERVICE_RUNNING)
	{
		// One minute of looping and checking the service state every 5 seconds - prevent the service being unresponsive to stop requests
		for (int x = 0; x < 12; x++)
		{
			::Sleep(5000);
			if (gServiceStatus.dwCurrentState != SERVICE_RUNNING)
				goto ServiceStopped;
		}

		// Loop variables
		PWTS_SESSION_INFO	pSessionInfo = 0;
		DWORD				dwCount = 0;
		int					nActiveCount = 0;

		// Get the current sessions and check for active
		if (WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pSessionInfo, &dwCount))
		{
			for (DWORD i = 0; i < dwCount; ++i)
			{
				WTS_SESSION_INFO si = pSessionInfo[i];
				if (si.State == WTSActive)
				{
					nActiveCount++;
					if (bDebugLogging)
						LogToFile(L"Active connection detected");
					break;
				}
			}
		}
		else
		{
			LogToFile(L"WTSEnumerateSessions Failed");
		}

		WTSFreeMemory(pSessionInfo);

		if (nActiveCount > 0)	//	At least one active session - reset the counter
		{
			nIdleCounter = nIdleTimeout;
			if (bDebugLogging)
				LogToFile(L"Idle counter reset");
		}
		else					// No active sessions - reduce idle counter
		{
			nIdleCounter--;
			if (bDebugLogging)
				LogToFile(L"Idle counter reduced");
		}

		// Check if the counter has expired
		if (nIdleCounter <= 0)
		{
			// Initiate shutdown here
			LogToFile(L"Idle Counter Timeout");
			InitiateShutdown();
			return;
		}

		// Check if the active timeout has been reached
		if ((long)GetTickCount64() >= nActiveTimeout)
		{
			if (bDebugLogging)
				LogToFile(L"Active Timeout Reached");

			DWORD dwValue = 0;

			// Signal to the UI app to display user message
			dwRetVal = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"Software\\AutoShutdownToolsService\\AutoShutdownToolsUI", 0, KEY_ALL_ACCESS, &hKey);
			if (dwRetVal != ERROR_SUCCESS)
			{
				LogToFile(L"Failed to open UI registry key");
				InitiateShutdown();
				return;
			}
			else
			{
				// Write the registry value
				dwValue = 1;
				dwRetVal = RegSetValueExW(hKey, L"Timeout", 0, REG_DWORD, (const BYTE*)&dwValue, sizeof(dwValue));
				if (dwRetVal != ERROR_SUCCESS)
				{
					LogToFile(L"Unable to write timeout state");
				}
				else
				{
					long nRegActiveState = 0;

					// Loop waiting for a user response - check service status so we can stop
					for (int y = 0; y < nActivityWait; y++)
					{
						for (int z = 0; z < 12; z++)
						{
							::Sleep(5000);
							if (gServiceStatus.dwCurrentState != SERVICE_RUNNING)
								goto ServiceStopped;
						}

						// Check for user response
						dwRetVal = RegQueryValueExW(hKey, L"Active", NULL, NULL, (LPBYTE)&nRegActiveState, &dwSize);
						if (dwRetVal != ERROR_SUCCESS)
						{
							LogToFile(L"Unable to read active state");
						}
						else
						{
							if (nRegActiveState > 0)	// User is active
							{
								if (bDebugLogging)
									LogToFile(L"Active state > 0");
								y = nActivityWait + 1;
								break;
							}
							else						// No response yet
							{
								if (bDebugLogging)
									LogToFile(L"Active state NOT > 0");
							}
						}

						// Set timeout registry key on each loop
						dwValue = 1;
						dwRetVal = RegSetValueExW(hKey, L"Timeout", 0, REG_DWORD, (const BYTE*)&dwValue, sizeof(dwValue));
						if (dwRetVal != ERROR_SUCCESS)
						{
							LogToFile(L"Unable to write timeout state");
						}
					}

					if (nRegActiveState > 0)	// The user has been active so extend the active shutdown timer and continue
					{
						// Extend
						nActiveTimeout = nActiveTimeout + nActiveTimeoutIncrement;
						if (bDebugLogging)
							LogToFile(L"Increase active timeout");

						// Reset the registry flags
						dwValue = 0;
						dwRetVal = RegSetValueExW(hKey, L"Timeout", 0, REG_DWORD, (const BYTE*)&dwValue, sizeof(dwValue));
						if (dwRetVal != ERROR_SUCCESS)
						{
							LogToFile(L"Unable to write timeout state");
						}
						dwRetVal = RegSetValueExW(hKey, L"Active", 0, REG_DWORD, (const BYTE*)&dwValue, sizeof(dwValue));
						if (dwRetVal != ERROR_SUCCESS)
						{
							LogToFile(L"Unable to write active state");
						}
					}
					else						// The user is inactive so shutdown
					{
						RegCloseKey(hKey);
						hKey = NULL;
						LogToFile(L"Timeout Counter and No Activity");
						InitiateShutdown();
						return;
					}

					RegCloseKey(hKey);
					hKey = NULL;
				}
			}
		}
	}

// Jump here if the service is being stopped
ServiceStopped:
	LogToFile(L"Service stopping");
	if (bShutdownOnStop) {
		LogToFile(L"Shutting down due to service stop");
		InitiateShutdown();
	}
} // end servicethread

/*************************************************************

	This function will try to shutdown the host

*************************************************************/
void InitiateShutdown()
{
	HANDLE				hToken;
	TOKEN_PRIVILEGES	tPriv;


	// Get the current process token handle so we can the shutdown privilege
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		LogToFile(L"Failed to get the current process token handle");
	}
	else
	{

		// Get the LUID for the shutdown privilege
		LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tPriv.Privileges[0].Luid);
		tPriv.PrivilegeCount = 1; 
		tPriv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		// Get the shutdown priviege for this process
		AdjustTokenPrivileges(hToken, FALSE, &tPriv, 0, (PTOKEN_PRIVILEGES)NULL, 0);

		if (GetLastError() != ERROR_SUCCESS)
		{
			LogToFile(L"Failed to adjust token privilege");
		}
		else
		{

			// Initiate the shutdown
			if (!InitiateSystemShutdownEx(NULL,	NULL, 0, FALSE, FALSE, SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_MINOR_OTHER | SHTDN_REASON_FLAG_PLANNED))
				LogToFile(L"Failed to initiate shutdown");
			else
				LogToFile(L"Shutdown initiated");

			tPriv.Privileges[0].Attributes = 0;
			AdjustTokenPrivileges(hToken, FALSE, &tPriv, 0, (PTOKEN_PRIVILEGES)NULL, 0);

		}
	}
} // end initiateshutdown

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
			fwprintf(fpLog, L"%02d/%02d/%04d %02d:%02d:%02d %s\n", sysTime.wDay, sysTime.wMonth, sysTime.wYear, sysTime.wHour, sysTime.wMinute, sysTime.wSecond, pMessage);
			fclose(fpLog);
		}
	}
	catch (...) {}
} // end logtofile
