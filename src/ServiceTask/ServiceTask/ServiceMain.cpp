// ServiceMain.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "ServiceCore.h"

#define DEF_APP         "APP"
#define WAIT_HINT_SECS  10000   //10s
#define SERVICE_NAME    "" DEF_APP "_SERVICE"

INT threadStatus = 0;
SC_HANDLE schService = NULL;
SC_HANDLE schSCManager = NULL;
TCHAR szSvcName[MAXCHAR] = { 0 };
SERVICE_STATUS ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE hServiceStatusHandle = NULL;
void WINAPI ServiceMainProc(int argc, char** argv);
void WINAPI ServiceHandler(DWORD fdwControl);

DWORD WINAPI ServiceCoreThread(LPVOID param)
{
    while (threadStatus != 1)
    {
        PROCESS_INFORMATION pi = { 0 };
        TCHAR progName[MAX_PATH] = { 0 };
        _sntprintf(progName, sizeof(progName) / sizeof(*progName), TEXT("%s\\" DEF_APP ".exe"), ServiceLogs::GetInstance()->rootPath);
        if (CreateProcessAsAdministrator(progName, &pi) && (pi.hProcess != NULL))
        {
            LOG_INFO(_T("%s:%d create process %d success\r\n"), A_To_T(__func__).c_str(), __LINE__, pi.dwProcessId);
            (WaitForSingleObject(pi.hProcess, INFINITE));
            LOG_INFO(_T("%s:%d destroy process %d success\r\n"), A_To_T(__func__).c_str(), __LINE__, pi.dwProcessId);
        }
        Sleep(1);
    }
    return NULL;
}

void WINAPI ServiceHandler(DWORD fdwControl)
{
    switch (fdwControl)
    {
    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:
    {
        ServiceStatus.dwWin32ExitCode = 0;
        ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        ServiceStatus.dwCheckPoint = 0;
        ServiceStatus.dwWaitHint = 0;
        threadStatus = 1;
        //add you quit code here
    }
    break;
    default:
    {
        return;
    }
    break;
    };
    if (!SetServiceStatus(hServiceStatusHandle, &ServiceStatus))
    {
        LOG_INFO(_T("%s:%d SetServiceStatus failed(%d)\r\n"), A_To_T(__func__).c_str(), __LINE__, GetLastError());
        return;
    }
}

void WINAPI ServiceMainProc(int argc, char** argv)
{
    ServiceStatus.dwServiceType = SERVICE_WIN32;
    ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_PAUSE_CONTINUE;
    ServiceStatus.dwWin32ExitCode = 0;
    ServiceStatus.dwServiceSpecificExitCode = 0;
    ServiceStatus.dwCheckPoint = 0;
    ServiceStatus.dwWaitHint = 0;
    hServiceStatusHandle = RegisterServiceCtrlHandler(_T(SERVICE_NAME), ServiceHandler);
    if (hServiceStatusHandle == NULL)
    {
        LOG_INFO(_T("%s:%d RegisterServiceCtrlHandler failed(%d)\r\n"), A_To_T(__func__).c_str(), __LINE__, GetLastError());
        return;
    }

    //add your service thread here
    HANDLE task_handle = CreateThread(NULL, NULL, ServiceCoreThread, NULL, NULL, NULL);
    if (task_handle == NULL)
    {
        LOG_INFO(_T("%s:%d CreateThread ServiceCoreThread failed(%d)\r\n"), A_To_T(__func__).c_str(), __LINE__, GetLastError());
        return;
    }

    // Initialization complete - report running status 
    ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    ServiceStatus.dwCheckPoint = 0;
    ServiceStatus.dwWaitHint = WAIT_HINT_SECS;
    if (!SetServiceStatus(hServiceStatusHandle, &ServiceStatus))
    {
        LOG_INFO(_T("%s:%d SetServiceStatus failed(%d)\r\n"), A_To_T(__func__).c_str(), __LINE__, GetLastError());
        return;
    }
}
/////////////////////////////////////////////////////////////////////////////////////

INT APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPTSTR    lpCmdLine,
    _In_ INT       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
    // Initialize global strings
    _tcsncpy(ServiceLogs::GetInstance()->rootPath, *__targv, sizeof(ServiceLogs::GetInstance()->rootPath) / sizeof(*ServiceLogs::GetInstance()->rootPath));
    *_tcsrchr(ServiceLogs::GetInstance()->rootPath, _T('\\')) = _T('\0');

    if (__argc == 1)
    {
        HANDLE hMutex = CreateMutex(NULL, FALSE, _T("__" DEF_APP "_SINGLE_INSTANCE__"));

        if (hMutex != NULL)
        {
            //如果程序已经存在并且正在运行
            if (GetLastError() != ERROR_ALREADY_EXISTS)
            {
                _sntprintf(ServiceLogs::GetInstance()->logsPath, sizeof(ServiceLogs::GetInstance()->logsPath) / sizeof(*ServiceLogs::GetInstance()->logsPath) - 1, _T("%s\\" DEF_APP ".log\0"), ServiceLogs::GetInstance()->rootPath);

                ServiceLogs::GetInstance()->LogStartup(ServiceLogs::GetInstance()->logsPath);

                LOG_INFO(_T("%s:%d Task handler started\r\n"), A_To_T(__func__).c_str(), __LINE__);

                SetCurrentDirectory(ServiceLogs::GetInstance()->rootPath);
            
                ServiceTask::GetInstance()->Run();
            }
            CloseHandle(hMutex);
        }
    }
    else
    {
        _sntprintf(ServiceLogs::GetInstance()->logsPath, sizeof(ServiceLogs::GetInstance()->logsPath) / sizeof(*ServiceLogs::GetInstance()->logsPath) - 1, _T("%s\\" DEF_APP "-service.log\0"), ServiceLogs::GetInstance()->rootPath);

        ServiceLogs::GetInstance()->LogStartup(ServiceLogs::GetInstance()->logsPath);

        LOG_INFO(_T("%s:%d service started\r\n"), A_To_T(__func__).c_str(), __LINE__);

        SERVICE_TABLE_ENTRY ServiceStartTable[2] = { 0 };

        ServiceStartTable[0].lpServiceName = (LPTSTR)_T(SERVICE_NAME);
        ServiceStartTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMainProc;

        ServiceStartTable[1].lpServiceName = NULL;
        ServiceStartTable[1].lpServiceProc = NULL;
        // 启动服务的控制分派机线程
        StartServiceCtrlDispatcher(ServiceStartTable);
    }

    ServiceLogs::GetInstance()->LogCleanup();

    return (INT)(0);
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
