// ServiceTask.h : This file contains the 'ServiceTask::Run' function. Program execution begins and ends there.
//
#pragma once

class ServiceTask
{
public:
    void Run();

public:
    static ServiceTask* GetInstance() {
        static ServiceTask serviceTaskInstance;
        return &serviceTaskInstance;
    }
};