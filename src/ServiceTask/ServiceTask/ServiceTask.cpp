// ServiceTask.cpp : This file contains the 'ServiceTask::Run' function. Program execution begins and ends there.
//

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include "ServiceCore.h"
#include "ServiceTask.h"

void ServiceTask::Run()
{
    /// <summary>
    /// 你的服务逻辑在这里编写
    /// </summary>
    while (true)
    {
        std::wstring text = A_To_T(__func__).c_str();
        LOG_INFO(_T("%s:%d Handle task.\r\n"), text.c_str(), __LINE__);
        Sleep(160);
    }
}