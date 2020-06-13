// ServiceCore.h : This file contains the 'main' function. Program execution begins and ends there.
//

#define _CRT_SECURE_NO_WARNINGS

#pragma once

// kbf.cpp : Defines the entry point for the application.
//
#include <string>

#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <userenv.h>
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#pragma comment(lib,"userenv")

class ServiceLogs
{
#define LOG_INFO(fmt, ...) {if (ServiceLogs::GetInstance()->pLog != nullptr){_ftprintf(ServiceLogs::GetInstance()->pLog, fmt, ##__VA_ARGS__);fflush(ServiceLogs::GetInstance()->pLog);}}
public:
    virtual ~ServiceLogs()
    {
        LogCleanup();
    }
public:
    /////////////////////////////////////////////////////////////////////////////////////
    FILE* pLog = nullptr;
    TCHAR rootPath[MAX_PATH] = { 0 };
    TCHAR logsPath[MAX_PATH] = { 0 };
    /////////////////////////////////////////////////////////////////////////////////////

public:
    void LogStartup(LPCTSTR lpFileName, LPCTSTR lpMode=_T("wb"))
    {
        LogCleanup();
        pLog = _tfopen(lpFileName, lpMode);
        if (sizeof(TCHAR) > sizeof(CHAR) && _tcscmp(lpMode, _T("wb")) == 0)
        {
            fwrite("\xFF\xFE", 2, 1, pLog);
        }
    }
    void LogCleanup()
    {
        if (pLog != nullptr)
        {
            fclose(pLog);
            pLog = nullptr;
        }
    }

public:
    static ServiceLogs* GetInstance() {
        static ServiceLogs serviceLogsInstance;
        return &serviceLogsInstance;
    }
};

//通用版将wstring转化为string
__inline std::string W_To_A(const std::wstring& wstr, unsigned int codepage = CP_ACP)
{
    int nwstrlen = WideCharToMultiByte(codepage, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    if (nwstrlen > 0)
    {
        std::string str(nwstrlen, '\0');
        WideCharToMultiByte(codepage, 0, wstr.c_str(), -1, (LPSTR)str.c_str(), nwstrlen, NULL, NULL);
        return std::move(str);
    }
    return ("");
}
//通用版将string转化为wstring
__inline std::wstring A_To_W(const std::string& str, unsigned int codepage = CP_ACP)
{
    int nstrlen = MultiByteToWideChar(codepage, 0, str.c_str(), -1, NULL, 0);
    if (nstrlen > 0)
    {
        std::wstring wstr(nstrlen, L'\0');
        MultiByteToWideChar(codepage, 0, str.c_str(), -1, (LPWSTR)wstr.c_str(), nstrlen);
        return std::move(wstr);
    }
    return (L"");
}
__inline static
#if !defined(UNICODE) && !defined(_UNICODE)
std::string
#else
std::wstring
#endif
A_To_T(const std::string& str)
{
#if !defined(UNICODE) && !defined(_UNICODE)
    return str;
#else
    return A_To_W(str);
#endif
}
__inline static
#if !defined(UNICODE) && !defined(_UNICODE)
std::string
#else
std::wstring
#endif
W_To_T(const std::wstring& wstr)
{
#if !defined(UNICODE) && !defined(_UNICODE)
    return W_To_A(wstr);
#else
    return wstr;
#endif
}
__inline static std::string T_To_A(
    const
#if !defined(UNICODE) && !defined(_UNICODE)
    std::string
#else
    std::wstring
#endif
    & tsT)
{
#if !defined(UNICODE) && !defined(_UNICODE)
    return tsT;
#else
    return W_To_A(tsT);
#endif
}
__inline static std::wstring T_To_W(
    const
#if !defined(UNICODE) && !defined(_UNICODE)
    std::string
#else
    std::wstring
#endif
    & tsT)
{
#if !defined(UNICODE) && !defined(_UNICODE)
    return A_To_W(tsT);
#else
    return tsT;
#endif
}
//将From编码转化为To编码
__inline static std::string CodePage_FromTo(const std::string& str,
    unsigned int from_codepage, unsigned int to_codepage)
{
    return W_To_A(A_To_W(str, from_codepage), to_codepage);
}
//将UTF8转化为ANSI
__inline static std::string UTF8ToANSI(const std::string& str)
{
    return CodePage_FromTo(str, CP_UTF8, CP_ACP);
}
//将ANSI转化为UTF8
__inline static std::string ANSIToUTF8(const std::string& str)
{
    return CodePage_FromTo(str, CP_ACP, CP_UTF8);
}

__inline static BOOL CreateProcessAsAdministrator(LPCTSTR lpProcessName, LPPROCESS_INFORMATION process)
{
    BOOL bResult = FALSE;
    HANDLE hToken = NULL;
    STARTUPINFO si = { 0 };
    DWORD dwSessionId = 0L;
    LPVOID pEnvironment = NULL;
    HANDLE hTokenDuplicate = NULL;

    if (!lpProcessName || !(*lpProcessName))
    {
        goto __LEAVE_CLEAN__;
    }

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken))
    {
        goto __LEAVE_CLEAN__;
    }

    if (!DuplicateTokenEx(hToken, TOKEN_ALL_ACCESS, NULL, SecurityAnonymous, TokenPrimary, &hTokenDuplicate))
    {
        goto __LEAVE_CLEAN__;
    }

    dwSessionId = WTSGetActiveConsoleSessionId();
    if (!SetTokenInformation(hTokenDuplicate, TokenSessionId, &dwSessionId, sizeof(dwSessionId)))
    {
        CloseHandle(hToken);
        CloseHandle(hTokenDuplicate);
        return FALSE;
    }

    if (!CreateEnvironmentBlock(&pEnvironment, hTokenDuplicate, FALSE))
    {
        goto __LEAVE_CLEAN__;
    }

    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.wShowWindow = SW_HIDE;// SW_SHOW;
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.lpDesktop = (LPTSTR)_T("WinSta0\\Default");
    if (!CreateProcessAsUser(hTokenDuplicate, lpProcessName, NULL, NULL, NULL, FALSE,
        NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW/*| CREATE_NEW_CONSOLE*/ | CREATE_UNICODE_ENVIRONMENT,
        pEnvironment, NULL, &si, process))
    {
        goto __LEAVE_CLEAN__;
    }

    bResult = TRUE;

__LEAVE_CLEAN__:

    if (pEnvironment != NULL)
    {
        DestroyEnvironmentBlock(pEnvironment);
        pEnvironment = NULL;
    }

    if (hToken != NULL)
    {
        CloseHandle(hToken);
        hToken = NULL;
    }
    if (hTokenDuplicate != NULL)
    {
        CloseHandle(hTokenDuplicate);
        hTokenDuplicate = NULL;
    }

    return bResult;
}

#include "ServiceTask.h"
