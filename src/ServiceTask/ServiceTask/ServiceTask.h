// ServiceTask.h : This file contains the 'ServiceTask::Run' function. Program execution begins and ends there.
//
#pragma once

class ServiceTask
{
public:
    /////////////////////////////////////////////////////////////////////////////////////
    FILE* pLog = NULL;
    TCHAR rootPath[MAX_PATH] = { 0 };
    TCHAR logsPath[MAX_PATH] = { 0 };
    /////////////////////////////////////////////////////////////////////////////////////

public:
    void Run();

public:
    static ServiceTask* GetInstance() {
        static ServiceTask serviceTaskInstance;
        return &serviceTaskInstance;
    }
};