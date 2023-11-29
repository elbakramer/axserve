from __future__ import annotations

import win32api
import win32con
import win32job


def CreateJobObjectForCleanUp() -> int:
    jobAttributes = None
    jobName = ""
    hJob = win32job.CreateJobObject(jobAttributes, jobName)
    extendedInfo = win32job.QueryInformationJobObject(
        hJob, win32job.JobObjectExtendedLimitInformation
    )
    basicLimitInformation = extendedInfo["BasicLimitInformation"]
    basicLimitInformation["LimitFlags"] = win32job.JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE
    win32job.SetInformationJobObject(
        hJob,
        win32job.JobObjectExtendedLimitInformation,
        extendedInfo,
    )
    return hJob


def AssignProcessToJobObject(hJob: int, processId: int) -> None:
    assert processId != 0
    desiredAccess = win32con.PROCESS_TERMINATE | win32con.PROCESS_SET_QUOTA
    inheritHandle = False
    hProcess = win32api.OpenProcess(
        desiredAccess,
        inheritHandle,
        processId,
    )
    return win32job.AssignProcessToJobObject(hJob, hProcess)
