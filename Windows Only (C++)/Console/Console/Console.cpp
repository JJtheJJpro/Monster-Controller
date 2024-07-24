// Console.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <SetupAPI.h>
#include <initguid.h>
#include <devguid.h>
#include <iostream>
#include <comdef.h>
#include <WbemIdl.h>
#include <atlbase.h>
#include <cstdlib>
#include <thread>
#include <string>

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "wbemuuid.lib")

using namespace std;

OVERLAPPED olw{};

void ListenForUSB()
{
    //SELECT * FROM Win32_DeviceChangeEvent WHERE EventType = 2

    CComPtr<IWbemLocator> locator;
    CComPtr<IWbemServices> service;
    CComPtr<IEnumWbemClassObject> enumerator;
    CComPtr<IWbemClassObject> processor;

    HRESULT hr = CoInitialize(0);
    if (FAILED(hr))
    {
        printf("err %d : %d", hr, GetLastError());
        exit(0);
    }
    hr = CoCreateInstance(CLSID_WbemAdministrativeLocator, NULL, CLSCTX_INPROC_SERVER, IID_IWbemLocator, reinterpret_cast<void**>(&locator));
    if (FAILED(hr))
    {
        printf("err %d : %d", hr, GetLastError());
        exit(0);
    }
    hr = locator->ConnectServer((BSTR)_T("root\\cimv2"), NULL, NULL, NULL, WBEM_FLAG_CONNECT_USE_MAX_WAIT, NULL, NULL, &service);
    if (FAILED(hr))
    {
        printf("err %d : %d", hr, GetLastError());
        exit(0);
    }
    hr = service->ExecNotificationQuery((BSTR)_T("WQL"), (BSTR)_T("SELECT * FROM Win32_DeviceChangeEvent WHERE EventType = 2"), WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY, NULL, &enumerator);
    if (FAILED(hr))
    {
        printf("err %d : %d", hr, GetLastError());
        exit(0);
    }

    ULONG retcnt;
    hr = enumerator->Next(WBEM_INFINITE, 1L, &processor, &retcnt);
    if (FAILED(hr))
    {
        printf("err %d : %d", hr, GetLastError());
        exit(0);
    }

    processor.Release();
    enumerator.Release();
    service.Release();
    locator.Release();

    CoUninitialize();
}

void GetSerialPortName(char* buf)
{
    HDEVINFO deviceInfoSet = SetupDiGetClassDevsA(&GUID_DEVCLASS_PORTS, nullptr, nullptr, DIGCF_PRESENT | DIGCF_PROFILE);
    if (deviceInfoSet == INVALID_HANDLE_VALUE)
    {
        printf("Error: Unable to get device info set\n");
        return;
    }

    SP_DEVINFO_DATA deviceInfoData{};
    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    for (DWORD i = 0; SetupDiEnumDeviceInfo(deviceInfoSet, i, &deviceInfoData); i++)
    {
        char deviceInstanceId[256];
        if (SetupDiGetDeviceInstanceIdA(deviceInfoSet, &deviceInfoData, deviceInstanceId, sizeof(deviceInstanceId), nullptr))
        {
            const char* vidPos = strstr(deviceInstanceId, "VID_");
            const char* pidPos = strstr(deviceInstanceId, "PID_");

            if (vidPos && pidPos)
            {
                //printf("Device Instance ID: %s\n", deviceInstanceId);

                char svid[5] = { 0 };
                char spid[5] = { 0 };

                strncpy_s(svid, vidPos + 4, 4);
                strncpy_s(spid, pidPos + 4, 4);

                char* end;

                long vid = strtol(svid, &end, 16);
                if (*end != '\0')
                {
                    printf("err\n");
                }
                long pid = strtol(spid, &end, 16);
                if (*end != '\0')
                {
                    printf("err\n");
                }

                //printf("VID: 0x%4X, PID: 0x%4X\n", vid, pid);

                if (vid == 0x2341 && pid == 0x0042)
                {
                    char name[256]{};
                    if (SetupDiGetDeviceRegistryPropertyA(deviceInfoSet, &deviceInfoData, SPDRP_FRIENDLYNAME, nullptr, (PBYTE)name, sizeof(name), nullptr))
                    {
                        char* scom = strstr(name, "COM");
                        if (scom)
                        {
                            size_t l = strstr(scom, ")") - scom;
                            //char com[5];
                            strncpy(buf, scom, l);
                            buf[l] = '\0';
                            SetupDiDestroyDeviceInfoList(deviceInfoSet);
                            return;
                            //buf = com;
                        }
                    }
                }
            }
        }
    }

    SetupDiDestroyDeviceInfoList(deviceInfoSet);
}

int main()
{
    auto STDIN = GetStdHandle(STD_INPUT_HANDLE);
    INPUT_RECORD InRec;
    DWORD NumRead;
    bool found = false;

    loop:
    char name[5];
    GetSerialPortName(name);
    if (!name)
    {
        printf("\033[2KListening for Arduino (press esc to exit)...");
        auto tmp = thread([&InRec, &NumRead, &found, STDIN]
            {
                while (!found)
                {
                    ReadConsoleInputA(STDIN, &InRec, 1, &NumRead);
                    if (found)
                    {
                        WriteConsoleInputA(STDIN, &InRec, 1, &NumRead);
                    }
                    else if (InRec.EventType == KEY_EVENT && InRec.Event.KeyEvent.uChar.AsciiChar == 27)
                    {
                        exit(0);
                    }
                }
            }); 
        tmp.detach();
        ListenForUSB();
        goto loop;
    }

    found = true;

    HANDLE serial = CreateFileA(name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
    if (serial == INVALID_HANDLE_VALUE)
    {
        printf(name);
        printf("Failed to connect to the arduino.");
        return 1;
    }

    DCB serialParams{};
    serialParams.DCBlength = sizeof(serialParams);
    GetCommState(serial, &serialParams);
    if (serialParams.BaudRate != CBR_9600 || serialParams.ByteSize != 8 || serialParams.fRtsControl != 1)
    {
        serialParams.BaudRate = CBR_9600;
        serialParams.ByteSize = 8;
        serialParams.fRtsControl = 1;
        SetCommState(serial, &serialParams);
        Sleep(100);
    }

    COMMTIMEOUTS timeout{};
    GetCommTimeouts(serial, &timeout);
    if (timeout.ReadIntervalTimeout != 1 || timeout.ReadTotalTimeoutConstant != 0 || timeout.ReadTotalTimeoutMultiplier != 0)
    {
        timeout.ReadIntervalTimeout = 1;
        timeout.ReadTotalTimeoutConstant = 0;
        timeout.ReadTotalTimeoutMultiplier = 0;
        SetCommTimeouts(serial, &timeout);
    }

    EscapeCommFunction(serial, SETDTR);

    bool Continue = TRUE;

    auto clearVisual = []
        {
            printf("\033[121D\033[11A");

            for (int i = 0; i < 11; i++)
            {
                printf("\033[2K\n");
            }

            printf("\033[121D\033[11A");
        };

    auto rthread = thread([serial, clearVisual, &Continue]
        {
            OVERLAPPED ol{};
            string comp;
            while (true)
            {
                char buf[1]{};
                DWORD bytesRead = 0;
                if (ReadFile(serial, buf, 1, &bytesRead, &ol))
                {
                    comp += buf[0];
                }
                else if (GetLastError() == ERROR_IO_PENDING && GetOverlappedResult(serial, &ol, &bytesRead, TRUE))
                {
                    comp += buf[0];
                }
                else
                {
                    clearVisual();
                    printf("Arduino Disconnected.  Press any key to exit...");
                    Continue = FALSE;
                    break;
                }
            }
        });

    int data[32]{};

    auto updateVisual = [](bool update, int d[32])
        {
            auto P = L"  Power  ";
            auto A = L"Activate ";
            auto X = L"Alt. Act.";
            auto sP = L"\033[38;2;0;0;0;48;2;255;0;0m  Power  \033[38;2;255;0;0;49m";
            auto sA = L"\033[38;2;0;0;0;48;2;255;0;0mActivate \033[38;2;255;0;0;49m";
            auto sX = L"\033[38;2;0;0;0;48;2;255;0;0mAlt. Act.\033[38;2;255;0;0;49m";
            auto sD = L"\033[38;2;0;0;0;48;2;255;0;0m  Door   \033[38;2;255;0;0;49m";
            auto sG = L"\033[38;2;0;0;0;48;2;255;0;0m Garbage \033[38;2;255;0;0;49m";

            if (update)
            {
                printf("\033[121D\033[11A");
            }

            _cwprintf(L"█████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████\n");
            _cwprintf(L"█ Monster 1 █ Monster 2 █ Monster 3 █ Monster 4 █ Monster 5 █ Monster 6 █ Monster 7 █ Monster 8 █ Monster 9 █ Monster10 █\n");
            _cwprintf(L"█████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████\n");
            _cwprintf(L"█ %s █ %s █ %s █ %s █ %s █ %s █ %s █ %s █ %s █ %s █\n",
                d[0] == 1 ? sP : P, d[1] == 1 ? sP : P, d[2] == 1 ? sP : P, d[3] == 1 ? sP : P, d[4] == 1 ? sP : P, d[5] == 1 ? sP : P, d[6] == 1 ? sP : P, d[7] == 1 ? sP : P, d[8] == 1 ? sP : P, d[9] == 1 ? sP : P);
            _cwprintf(L"█████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████\n");
            _cwprintf(L"█ %s █ %s █ %s █ %s █ %s █ %s █ %s █ %s █ %s █ %s █\n",
                d[10] == 1 ? sA : A, d[11] == 1 ? sA : A, d[12] == 1 ? sA : A, d[13] == 1 ? sA : A, d[14] == 1 ? sA : A, d[15] == 1 ? sA : A, d[16] == 1 ? sA : A, d[17] == 1 ? sA : A, d[18] == 1 ? sA : A, d[19] == 1 ? sA : A);
            _cwprintf(L"█████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████\n");
            _cwprintf(L"█ %s █ %s █ %s █ %s █ %s █ %s █ %s █ %s █ %s █ %s █\n",
                d[20] == 1 ? sX : X, d[21] == 1 ? sX : X, d[22] == 1 ? sX : X, d[23] == 1 ? sX : X, d[24] == 1 ? sX : X, d[25] == 1 ? sX : X, d[26] == 1 ? sX : X, d[27] == 1 ? sX : X, d[28] == 1 ? sX : X, d[29] == 1 ? sX : X);
            _cwprintf(L"█████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████\n");
            _cwprintf(L"█                                                                                               █ %s █ %s █\n", d[30] == 1 ? sD : L"  Door   ", d[31] == 1 ? sG : L" Garbage ");
            _cwprintf(L"█████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████\n");
        };

    printf("WARNING: The layout should look normal, like a rectangle box.  If it looks wonky, the program will look broken, so resize the console to fix it.\n");
    printf("JJ's Monster Controller using Visual C++ for Windows\n");
    printf("For help, press escape and type 'help'\n");

    printf("\033[38;2;255;0;0m");

    updateVisual(false, data);

    //hIn = GetStdHandle(STD_INPUT_HANDLE);
    //hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    bool door = false;
    int tdoor = 0;

    auto twrite = [serial](const char* d)
        {
            DWORD nBytesWritten = 0;
            if (!WriteFile(serial, d, (DWORD)strlen(d), &nBytesWritten, &olw))
            {
                if (GetLastError() == ERROR_IO_PENDING && GetOverlappedResult(serial, &olw, &nBytesWritten, TRUE))
                {

                }
            }
        };

    while (Continue)
    {
        ReadConsoleInputA(STDIN, &InRec, 1, &NumRead);

        if (!Continue)
        {
            break;
        }

        if (InRec.EventType == KEY_EVENT)
        {
            if (InRec.Event.KeyEvent.bKeyDown)
            {
                if (InRec.Event.KeyEvent.wRepeatCount <= 1)
                {
                    switch (InRec.Event.KeyEvent.uChar.AsciiChar)
                    {
                    case '1':
                        if (data[0] == 0)
                        {
                            data[0] = 1;
                            twrite("101\n");
                            updateVisual(true, data);
                        }
                        break;
                    case '2':
                        if (data[1] == 0)
                        {
                            data[1] = 1;
                            twrite("102\n");
                            updateVisual(true, data);
                        }
                        break;
                    case '3':
                        if (data[2] == 0)
                        {
                            data[2] = 1;
                            twrite("103\n");
                            updateVisual(true, data);
                        }
                        break;
                    case '4':
                        if (data[3] == 0)
                        {
                            data[3] = 1;
                            twrite("104\n");
                            updateVisual(true, data);
                        }
                        break;
                    case '5':
                        if (data[4] == 0)
                        {
                            data[4] = 1;
                            twrite("105\n");
                            updateVisual(true, data);
                        }
                        break;
                    case '6':
                        if (data[5] == 0)
                        {
                            data[5] = 1;
                            twrite("106\n");
                            updateVisual(true, data);
                        }
                        break;
                    case '7':
                        if (data[6] == 0)
                        {
                            data[6] = 1;
                            twrite("107\n");
                            updateVisual(true, data);
                        }
                        break;
                    case '8':
                        if (data[7] == 0)
                        {
                            data[7] = 1;
                            twrite("108\n");
                            updateVisual(true, data);
                        }
                        break;
                    case '9':
                        if (data[8] == 0)
                        {
                            data[8] = 1;
                            twrite("109\n");
                            updateVisual(true, data);
                        }
                        break;
                    case '0':
                        if (data[9] == 0)
                        {
                            data[9] = 1;
                            twrite("110\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 'q':
                        if (data[10] == 0)
                        {
                            data[10] = 1;
                            twrite("111\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 'w':
                        if (data[11] == 0)
                        {
                            data[11] = 1;
                            twrite("112\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 'e':
                        if (data[12] == 0)
                        {
                            data[12] = 1;
                            twrite("113\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 'r':
                        if (data[13] == 0)
                        {
                            data[13] = 1;
                            twrite("114\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 't':
                        if (data[14] == 0)
                        {
                            data[14] = 1;
                            twrite("115\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 'y':
                        if (data[15] == 0)
                        {
                            data[15] = 1;
                            twrite("116\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 'u':
                        if (data[16] == 0)
                        {
                            data[16] = 1;
                            twrite("117\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 'i':
                        if (data[17] == 0)
                        {
                            data[17] = 1;
                            twrite("118\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 'o':
                        if (data[18] == 0)
                        {
                            data[18] = 1;
                            twrite("119\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 'p':
                        if (data[19] == 0)
                        {
                            data[19] = 1;
                            twrite("120\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 'a':
                        if (data[20] == 0)
                        {
                            data[20] = 1;
                            twrite("121\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 's':
                        if (data[21] == 0)
                        {
                            data[21] = 1;
                            twrite("122\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 'd':
                        if (data[22] == 0)
                        {
                            data[22] = 1;
                            twrite("123\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 'f':
                        if (data[23] == 0)
                        {
                            data[23] = 1;
                            twrite("124\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 'g':
                        if (data[24] == 0)
                        {
                            data[24] = 1;
                            twrite("125\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 'h':
                        if (data[25] == 0)
                        {
                            data[25] = 1;
                            twrite("126\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 'j':
                        if (data[26] == 0)
                        {
                            data[26] = 1;
                            twrite("127\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 'k':
                        if (data[27] == 0)
                        {
                            data[27] = 1;
                            twrite("128\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 'l':
                        if (data[28] == 0)
                        {
                            data[28] = 1;
                            twrite("129\n");
                            updateVisual(true, data);
                        }
                        break;
                    case ';':
                        if (data[29] == 0)
                        {
                            data[29] = 1;
                            twrite("130\n");
                            updateVisual(true, data);
                        }
                        break;
                    case ' ':
                        if (!door)
                        {
                            door = true;
                            tdoor ^= 1;
                            data[30] = tdoor;
                            twrite(data[30] == 1 ? "131\n" : "031\n");
                            updateVisual(true, data);
                        }
                        break;
                    case '.':
                        if (data[31] == 0)
                        {
                            data[31] = 1;
                            twrite("132\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 27:
                        clearVisual();
                        printf("> ");
                        {
                            char input[128];
                            fgets(input, 128, stdin);
                            input[strcspn(input, "\n")] = '\0';
                            auto psblCmd = strstr(input, "arduino ");
                            if (strcmp(input, "clear") == 0)
                            {
                                system("cls");
                            }
                            else if (strcmp(input, "exit") == 0)
                            {
                                Continue = FALSE;
                                continue;
                            }
                            else if (strcmp(input, "help") == 0)
                            {
                                printf("To get to the command line again, press escape again.\n");
                                printf("\n");
                                printf("exit:           exits this program\n");
                                printf("help:           prints this list of commands\n");
                                printf("clear:          clears the console\n");
                                printf("arduino <data>: sends <data> to the arduino\n");
                            }
                            else if (strcmp(input, "arduino") == 0)
                            {
                                printf("command requires 1 parameter <data>: 'arduino <data>'\n");
                            }
                            else if (psblCmd)
                            {
                                //char cmd[32];
                                memmove(psblCmd, psblCmd + 8, strlen(psblCmd) - 7);
                                sprintf(psblCmd, "%s\n", psblCmd);
                                twrite(psblCmd);
                            }
                            else
                            {
                                printf("Unknown command.\n");
                            }
                            updateVisual(false, data);
                        }
                        break;
                    default:
                        break;
                    }
                }
            }
            else
            {
                if (InRec.Event.KeyEvent.wRepeatCount <= 1)
                {
                    switch (InRec.Event.KeyEvent.uChar.AsciiChar)
                    {
                    case '1':
                        if (data[0] == 1)
                        {
                            data[0] = 0;
                            twrite("001\n");
                            updateVisual(true, data);
                        }
                        break;
                    case '2':
                        if (data[1] == 1)
                        {
                            data[1] = 0;
                            twrite("002\n");
                            updateVisual(true, data);
                        }
                        break;
                    case '3':
                        if (data[2] == 1)
                        {
                            data[2] = 0;
                            twrite("003\n");
                            updateVisual(true, data);
                        }
                        break;
                    case '4':
                        if (data[3] == 1)
                        {
                            data[3] = 0;
                            twrite("004\n");
                            updateVisual(true, data);
                        }
                        break;
                    case '5':
                        if (data[4] == 1)
                        {
                            data[4] = 0;
                            twrite("005\n");
                            updateVisual(true, data);
                        }
                        break;
                    case '6':
                        if (data[5] == 1)
                        {
                            data[5] = 0;
                            twrite("006\n");
                            updateVisual(true, data);
                        }
                        break;
                    case '7':
                        if (data[6] == 1)
                        {
                            data[6] = 0;
                            twrite("007\n");
                            updateVisual(true, data);
                        }
                        break;
                    case '8':
                        if (data[7] == 1)
                        {
                            data[7] = 0;
                            twrite("008\n");
                            updateVisual(true, data);
                        }
                        break;
                    case '9':
                        if (data[8] == 1)
                        {
                            data[8] = 0;
                            twrite("009\n");
                            updateVisual(true, data);
                        }
                        break;
                    case '0':
                        if (data[9] == 1)
                        {
                            data[9] = 0;
                            twrite("010\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 'q':
                        if (data[10] == 1)
                        {
                            data[10] = 0;
                            twrite("011\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 'w':
                        if (data[11] == 1)
                        {
                            data[11] = 0;
                            twrite("012\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 'e':
                        if (data[12] == 1)
                        {
                            data[12] = 0;
                            twrite("013\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 'r':
                        if (data[13] == 1)
                        {
                            data[13] = 0;
                            twrite("014\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 't':
                        if (data[14] == 1)
                        {
                            data[14] = 0;
                            twrite("015\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 'y':
                        if (data[15] == 1)
                        {
                            data[15] = 0;
                            twrite("016\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 'u':
                        if (data[16] == 1)
                        {
                            data[16] = 0;
                            twrite("017\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 'i':
                        if (data[17] == 1)
                        {
                            data[17] = 0;
                            twrite("018\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 'o':
                        if (data[18] == 1)
                        {
                            data[18] = 0;
                            twrite("019\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 'p':
                        if (data[19] == 1)
                        {
                            data[19] = 0;
                            twrite("020\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 'a':
                        if (data[20] == 1)
                        {
                            data[20] = 0;
                            twrite("021\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 's':
                        if (data[21] == 1)
                        {
                            data[21] = 0;
                            twrite("022\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 'd':
                        if (data[22] == 1)
                        {
                            data[22] = 0;
                            twrite("023\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 'f':
                        if (data[23] == 1)
                        {
                            data[23] = 0;
                            twrite("024\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 'g':
                        if (data[24] == 1)
                        {
                            data[24] = 0;
                            twrite("025\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 'h':
                        if (data[25] == 1)
                        {
                            data[25] = 0;
                            twrite("026\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 'j':
                        if (data[26] == 1)
                        {
                            data[26] = 0;
                            twrite("027\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 'k':
                        if (data[27] == 1)
                        {
                            data[27] = 0;
                            twrite("028\n");
                            updateVisual(true, data);
                        }
                        break;
                    case 'l':
                        if (data[28] == 1)
                        {
                            data[28] = 0;
                            twrite("029\n");
                            updateVisual(true, data);
                        }
                        break;
                    case ';':
                        if (data[29] == 1)
                        {
                            data[29] = 0;
                            twrite("030\n");
                            updateVisual(true, data);
                        }
                        break;
                    case ' ':
                        door = false;
                        break;
                    case '.':
                        if (data[31] == 1)
                        {
                            data[31] = 0;
                            twrite("032\n");
                            updateVisual(true, data);
                        }
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }
    twrite("RESET\n");
    rthread.detach();
    printf("\033[0mexiting...");
    return 0;
}
