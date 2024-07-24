#define _CRT_SECURE_NO_WARNINGS

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "setupapi.lib")

#include "resource.h"
#include <cmath>
#include <dwmapi.h>
#include <map>
#include <thread>
#include <windowsx.h>
#include <SetupAPI.h>
#include <cfgmgr32.h>
#include <devguid.h>

#pragma region Variables

char wClass[100];
char wTitle[100];
std::map<DWORD, BOOL> keysDown{};
WPARAM vkUpdate = 0;
BOOL SpecialDoor = FALSE;

std::thread PortThread;
void (*asas)(const char* msg);

HWND hWnd;
HINSTANCE HInstance;

HCURSOR NormalCursor = LoadCursor(NULL, IDC_ARROW);
HCURSOR WaitCursor = LoadCursor(NULL, IDC_WAIT);
HCURSOR TextCursor = LoadCursor(NULL, IDC_IBEAM);
HCURSOR ClickCursor = LoadCursor(NULL, IDC_HAND);

COLORREF BlackColor = RGB(0, 0, 0);
COLORREF DarkRedColor = RGB(0x8B, 0, 0);
COLORREF HoverDarkRedColor = RGB(0x25, 0, 0);

HPEN BlackPen = CreatePen(PS_SOLID, 2, BlackColor);
HPEN DarkRedPen = CreatePen(PS_SOLID, 2, DarkRedColor);
HPEN NullPen = CreatePen(PS_NULL, 0, NULL);

HBRUSH BlackBrush = CreateSolidBrush(BlackColor);
HBRUSH DarkRedBrush = CreateSolidBrush(DarkRedColor);
HBRUSH HoverDarkRedBrush = CreateSolidBrush(HoverDarkRedColor);

HFONT Font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
LOGFONTA LogFont;

RECT ClientRegion = { 0, 0, 0, 0 };
RECT prevClientRegion = { 0, 0, 0, 0 };

POINT CursorPos = { 0, 0 };
POINT prevCursorPos = { 0, 0 };
WORD CursorHitPoint = HTNOWHERE;

double wd = 0.0; // Width Divisor
double hd = 0.0; // Height Divisor
double ws = 0.0; // Width Switch Divisor

unsigned int wd1 = 0;
unsigned int wd2 = 0;
unsigned int wd3 = 0;
unsigned int wd4 = 0;
unsigned int wd5 = 0;
unsigned int wd6 = 0;
unsigned int wd7 = 0;
unsigned int wd8 = 0;
unsigned int wd9 = 0;

unsigned int hd1 = 0;
unsigned int hd2 = 0;
unsigned int hd3 = 0;
unsigned int hd4 = 0;
unsigned int hd5 = 0;
unsigned int hd6 = 0;
unsigned int hd7 = 0;

unsigned int ws1 = 0;
unsigned int ws2 = 0;

RECT L1;
RECT L2;
RECT L3;
RECT L4;
RECT L5;
RECT L6;
RECT L7;
RECT L8;
RECT L9;
RECT L10;

RECT P1;
RECT P2;
RECT P3;
RECT P4;
RECT P5;
RECT P6;
RECT P7;
RECT P8;
RECT P9;
RECT P10;

RECT A1;
RECT A2;
RECT A3;
RECT A4;
RECT A5;
RECT A6;
RECT A7;
RECT A8;
RECT A9;
RECT A10;

RECT X1;
RECT X2;
RECT X3;
RECT X4;
RECT X5;
RECT X6;
RECT X7;
RECT X8;
RECT X9;
RECT X10;
RECT XR1;
RECT XR2;
RECT XR3;
RECT XR4;
RECT XR5;
RECT XR6;
RECT XR7;
RECT XR8;
RECT XR9;
RECT XR10;

RECT DoorBtn;
RECT GarbageBtn;

RECT KeyboardSwitch;
RECT GUISwitch;
RECT WebsiteSwitch;

constexpr int maxText = 15;
char L1text[maxText] = "Monster 1"; size_t L1size = 9;
char L2text[maxText] = "Monster 2"; size_t L2size = 9;
char L3text[maxText] = "Monster 3"; size_t L3size = 9;
char L4text[maxText] = "Monster 4"; size_t L4size = 9;
char L5text[maxText] = "Monster 5"; size_t L5size = 9;
char L6text[maxText] = "Monster 6"; size_t L6size = 9;
char L7text[maxText] = "Monster 7"; size_t L7size = 9;
char L8text[maxText] = "Monster 8"; size_t L8size = 9;
char L9text[maxText] = "Monster 9"; size_t L9size = 9;
char L10text[maxText] = "Monster 10"; size_t L10size = 10;

HANDLE serial;
char ComSerText[0xFF]{};
RECT CSTR = { 0, 0, 0, 0 };
BOOL CSTC = TRUE;

unsigned int ButtonsActive[35]
{
    0, // P1
    0, // P2
    0, // P3
    0, // P4
    0, // P5
    0, // P6
    0, // P7
    0, // P8
    0, // P9
    0, // P10

    0, // A1
    0, // A2
    0, // A3
    0, // A4
    0, // A5
    0, // A6
    0, // A7
    0, // A8
    0, // A9
    0, // A10

    0, // X1
    0, // X2
    0, // X3
    0, // X4
    0, // X5
    0, // X6
    0, // X7
    0, // X8
    0, // X9
    0, // X10

    0, // Door
    0, // Garbage

    0, // Keyboard
    1, // GUI
    0 // Website
};
unsigned int selected = 0;
unsigned int prevSelected = 0;
unsigned int down = 0;
unsigned int prevDown = 0;
unsigned int edit = 0;
unsigned int prevEdit = 0;
unsigned int switched = 0;

BOOL move = FALSE;
BOOL shift = FALSE;

BOOL Connected = FALSE;

#pragma endregion

void UpdateCST(HDC hdc)
{
    SetTextColor(hdc, DarkRedColor);
    DrawTextA(hdc, ComSerText, (int)strlen(ComSerText), &CSTR, DT_SINGLELINE);
}

void SerialComAsyncRead()
{
    while (Connected)
    {
        OVERLAPPED overlapped = { 0 };
        char comp[0xFF]{};
        //while ((posTemp = comp.find(L"\r\n")) == std::wstring::npos)
        //{
        //    wchar_t b{};
        //    DWORD bytesRead = 0;
        //    if (ReadFile(serial, &b, 1, &bytesRead, &overlapped))
        //    {
        //        if (b == '\0')
        //            break;
        //        comp.append(1, b);
        //    }
        //    else if (GetLastError() == ERROR_IO_PENDING && GetOverlappedResult(serial, &overlapped, &bytesRead, TRUE))
        //    {
        //        if (b == '\0')
        //            break;
        //        comp.append(1, b);
        //    }
        //    else
        //    {
        //        return;
        //    }
        //}
        char buf[0xFFF]{};
        DWORD bytesRead = 0;
        if (ReadFile(serial, buf, 0xFFF, &bytesRead, &overlapped))
        {
            strcat(comp, buf);
        }
        else if (GetLastError() == ERROR_IO_PENDING && GetOverlappedResult(serial, &overlapped, &bytesRead, TRUE))
        {
            if (strcmp(buf, "") == 0)
            {
                continue;
            }
            strcat(comp, buf);
        }
        else
        {
            Connected = FALSE;
            asas("The Box got disconnected.  Plug it back in and press R.");
            continue;
        }
        OutputDebugStringA(comp);
    }
}

void SerialComWrite(const char* data)
{
    if (Connected)
    {
        OVERLAPPED overlapped = { 0 };
        DWORD nBytesWritten = 0;
        DWORD bytesTransferred = 0;
        DWORD err = 0;
        //OutputDebugStringA("Sending...");
        BOOL res = WriteFile(serial, data, (DWORD)strlen(data), &nBytesWritten, &overlapped); // not working most likely because read file is already happening.
        if (!res)
        {
            err = GetLastError();
        }
    
        //res = GetOverlappedResult(serial, &overlapped, &bytesTransferred, FALSE);
    
        if (!res)
        {
            err = GetLastError();
            if (err != ERROR_IO_INCOMPLETE && err != ERROR_IO_PENDING)
            {
                //MessageBoxA(NULL, std::format("WriteFile failed: {}", err).c_str(), "Writing error", 0);
                char m[0x7F];
                sprintf(m, "WriteFile failed: %d", err);
                MessageBoxA(NULL, m, "Writing error", 0);
            }
        }
    
        //OutputDebugStringA("Done!\n");
    
        //OutputDebugStringA(data);
    }
}

void ActiveMonster(int n, bool on)
{
    //auto asdf = std::format("{}{}{}\n", on ? '1' : '0', n < 10 ? "0" : "", n);
    //std::string asdf = (on ? "1"s : "0") + (n < 10 ? "0" : "") + std::to_string(n) + "\n";
    OutputDebugStringA("in ActiveMonster\n");
    char sr[5];
    sprintf_s(sr, 5, "%d%s%d\n", on ? 1 : 0, n < 10 ? "0" : "", n);
    SerialComWrite(sr);
    OutputDebugStringA("out ActiveMonster\n");
}

void ConnectToPort(const char* com)
{
    serial = CreateFileA(com, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);

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

    Connected = TRUE;

#if _DEBUG
    //SerialComWrite("TSNDINF\n");
#endif

    PortThread = std::thread(SerialComAsyncRead);
    PortThread.detach();

    _memccpy(ComSerText, "Connected!", 0, strlen("Connected!"));
    ComSerText[10] = '\0';
    //ComSerText = "Connected!";
    prevClientRegion = { 0, 0, 0, 0 }; // for full gui reset
    InvalidateRect(hWnd, NULL, FALSE);
    InvalidateRect(hWnd, &CSTR, TRUE);
    UpdateWindow(hWnd);
}

void FindPort(char* name)
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
                    char tname[256]{};
                    if (SetupDiGetDeviceRegistryPropertyA(deviceInfoSet, &deviceInfoData, SPDRP_FRIENDLYNAME, nullptr, (PBYTE)tname, sizeof(tname), nullptr))
                    {
                        char* scom = strstr(tname, "COM");
                        if (scom)
                        {
                            size_t l = strstr(scom, ")") - scom;
                            //char com[5];
                            strncpy(name, scom, l);
                            name[l] = '\0';
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

void CycleFont(LONG h, HDC hdc)
{
    LogFont.lfHeight = h;
    HFONT discard = Font;
    Font = CreateFontIndirectA(&LogFont);
    DeleteObject(discard);
    SelectObject(hdc, Font);
}

void AlternateActivateCalc(HDC hdc)
{
    XR1 = X1;
    XR2 = X2;
    XR3 = X3;
    XR4 = X4;
    XR5 = X5;
    XR6 = X6;
    XR7 = X7;
    XR8 = X8;
    XR9 = X9;
    XR10 = X10;

    DrawText(hdc, L"Alternate\r\nActivate", -1, &XR1, DT_CALCRECT | DT_WORDBREAK);
    DrawText(hdc, L"Alternate\r\nActivate", -1, &XR2, DT_CALCRECT | DT_WORDBREAK);
    DrawText(hdc, L"Alternate\r\nActivate", -1, &XR3, DT_CALCRECT | DT_WORDBREAK);
    DrawText(hdc, L"Alternate\r\nActivate", -1, &XR4, DT_CALCRECT | DT_WORDBREAK);
    DrawText(hdc, L"Alternate\r\nActivate", -1, &XR5, DT_CALCRECT | DT_WORDBREAK);
    DrawText(hdc, L"Alternate\r\nActivate", -1, &XR6, DT_CALCRECT | DT_WORDBREAK);
    DrawText(hdc, L"Alternate\r\nActivate", -1, &XR7, DT_CALCRECT | DT_WORDBREAK);
    DrawText(hdc, L"Alternate\r\nActivate", -1, &XR8, DT_CALCRECT | DT_WORDBREAK);
    DrawText(hdc, L"Alternate\r\nActivate", -1, &XR9, DT_CALCRECT | DT_WORDBREAK);
    DrawText(hdc, L"Alternate\r\nActivate", -1, &XR10, DT_CALCRECT | DT_WORDBREAK);

    unsigned int wc1 = (unsigned int)round((X1.right - XR1.right) / 2.0);
    unsigned int hc1 = (unsigned int)round((X1.bottom - XR1.bottom) / 2.0);
    unsigned int wc2 = (unsigned int)round((X1.right - XR1.right) / 2.0);
    unsigned int hc2 = (unsigned int)round((X1.bottom - XR1.bottom) / 2.0);
    unsigned int wc3 = (unsigned int)round((X1.right - XR1.right) / 2.0);
    unsigned int hc3 = (unsigned int)round((X1.bottom - XR1.bottom) / 2.0);
    unsigned int wc4 = (unsigned int)round((X1.right - XR1.right) / 2.0);
    unsigned int hc4 = (unsigned int)round((X1.bottom - XR1.bottom) / 2.0);
    unsigned int wc5 = (unsigned int)round((X1.right - XR1.right) / 2.0);
    unsigned int hc5 = (unsigned int)round((X1.bottom - XR1.bottom) / 2.0);
    unsigned int wc6 = (unsigned int)round((X1.right - XR1.right) / 2.0);
    unsigned int hc6 = (unsigned int)round((X1.bottom - XR1.bottom) / 2.0);
    unsigned int wc7 = (unsigned int)round((X1.right - XR1.right) / 2.0);
    unsigned int hc7 = (unsigned int)round((X1.bottom - XR1.bottom) / 2.0);
    unsigned int wc8 = (unsigned int)round((X1.right - XR1.right) / 2.0);
    unsigned int hc8 = (unsigned int)round((X1.bottom - XR1.bottom) / 2.0);
    unsigned int wc9 = (unsigned int)round((X1.right - XR1.right) / 2.0);
    unsigned int hc9 = (unsigned int)round((X1.bottom - XR1.bottom) / 2.0);
    unsigned int wc10 = (unsigned int)round((X1.right - XR1.right) / 2.0);
    unsigned int hc10 = (unsigned int)round((X1.bottom - XR1.bottom) / 2.0);

    XR1.left += wc1;
    XR1.top += hc1;
    XR1.right += wc1;
    XR1.bottom += hc1;
    XR2.left += wc2;
    XR2.top += hc2;
    XR2.right += wc2;
    XR2.bottom += hc2;
    XR3.left += wc3;
    XR3.top += hc3;
    XR3.right += wc3;
    XR3.bottom += hc3;
    XR4.left += wc4;
    XR4.top += hc4;
    XR4.right += wc4;
    XR4.bottom += hc4;
    XR5.left += wc5;
    XR5.top += hc5;
    XR5.right += wc5;
    XR5.bottom += hc5;
    XR6.left += wc6;
    XR6.top += hc6;
    XR6.right += wc6;
    XR6.bottom += hc6;
    XR7.left += wc7;
    XR7.top += hc7;
    XR7.right += wc7;
    XR7.bottom += hc7;
    XR8.left += wc8;
    XR8.top += hc8;
    XR8.right += wc8;
    XR8.bottom += hc8;
    XR9.left += wc9;
    XR9.top += hc9;
    XR9.right += wc9;
    XR9.bottom += hc9;
    XR10.left += wc10;
    XR10.top += hc10;
    XR10.right += wc10;
    XR10.bottom += hc10;
}

void UpdateMath(HDC hdc)
{
    if (hdc == NULL)
    {
        wd = ((double)ClientRegion.right) / 10.0;
        hd = ((double)ClientRegion.bottom) / 8.0;
        ws = ((double)ClientRegion.right) / 3.0;

        wd1 = (unsigned int)round(wd);
        wd2 = (unsigned int)round(wd * 2);
        wd3 = (unsigned int)round(wd * 3);
        wd4 = (unsigned int)round(wd * 4);
        wd5 = (unsigned int)round(wd * 5);
        wd6 = (unsigned int)round(wd * 6);
        wd7 = (unsigned int)round(wd * 7);
        wd8 = (unsigned int)round(wd * 8);
        wd9 = (unsigned int)round(wd * 9);

        hd1 = (unsigned int)round(hd);
        hd2 = (unsigned int)round(hd * 2);
        hd3 = (unsigned int)round(hd * 3);
        hd4 = (unsigned int)round(hd * 4);
        hd5 = (unsigned int)round(hd * 5);
        hd6 = (unsigned int)round(hd * 6);
        hd7 = (unsigned int)round(hd * 7);

        ws1 = (unsigned int)round(ws);
        ws2 = (unsigned int)round(ws * 2);

        L1.left = P1.left = A1.left = X1.left = KeyboardSwitch.left = 0;
        L2.left = P2.left = A2.left = X2.left = (L1.right = P1.right = A1.right = X1.right = wd1 - 1) + 2;
        L3.left = P3.left = A3.left = X3.left = (L2.right = P2.right = A2.right = X2.right = wd2 - 1) + 2;
        L4.left = P4.left = A4.left = X4.left = (L3.right = P3.right = A3.right = X3.right = wd3 - 1) + 2;
        L5.left = P5.left = A5.left = X5.left = (L4.right = P4.right = A4.right = X4.right = wd4 - 1) + 2;
        L6.left = P6.left = A6.left = X6.left = (L5.right = P5.right = A5.right = X5.right = wd5 - 1) + 2;
        L7.left = P7.left = A7.left = X7.left = (L6.right = P6.right = A6.right = X6.right = wd6 - 1) + 2;
        L8.left = P8.left = A8.left = X8.left = (L7.right = P7.right = A7.right = X7.right = wd7 - 1) + 2;
        L9.left = P9.left = A9.left = X9.left = DoorBtn.left = (L8.right = P8.right = A8.right = X8.right = wd8 - 1) + 2;
        L10.left = P10.left = A10.left = X10.left = GarbageBtn.left = (L9.right = P9.right = A9.right = X9.right = DoorBtn.right = wd9 - 1) + 2;
        L10.right = P10.right = A10.right = X10.right = GarbageBtn.right = WebsiteSwitch.right = ClientRegion.right;
        GUISwitch.left = (KeyboardSwitch.right = ws1 - 1) + 2;
        WebsiteSwitch.left = (GUISwitch.right = ws2 - 1) + 2;

        L1.top = L2.top = L3.top = L4.top = L5.top = L6.top = L7.top = L8.top = L9.top = L10.top = hd2 - 40;
        P1.top = P2.top = P3.top = P4.top = P5.top = P6.top = P7.top = P8.top = P9.top = P10.top = (L1.bottom = L2.bottom = L3.bottom = L4.bottom = L5.bottom = L6.bottom = L7.bottom = L8.bottom = L9.bottom = L10.bottom = hd2 - 1) + 2;
        A1.top = A2.top = A3.top = A4.top = A5.top = A6.top = A7.top = A8.top = A9.top = A10.top = (P1.bottom = P2.bottom = P3.bottom = P4.bottom = P5.bottom = P6.bottom = P7.bottom = P8.bottom = P9.bottom = P10.bottom = hd3 - 1) + 2;
        X1.top = X2.top = X3.top = X4.top = X5.top = X6.top = X7.top = X8.top = X9.top = X10.top = (A1.bottom = A2.bottom = A3.bottom = A4.bottom = A5.bottom = A6.bottom = A7.bottom = A8.bottom = A9.bottom = A10.bottom = hd4 - 1) + 2;
        DoorBtn.top = GarbageBtn.top = (X1.bottom = X2.bottom = X3.bottom = X4.bottom = X5.bottom = X6.bottom = X7.bottom = X8.bottom = X9.bottom = X10.bottom = hd5 - 1) + 2;
        DoorBtn.bottom = GarbageBtn.bottom = hd6 - 1;
        KeyboardSwitch.top = GUISwitch.top = WebsiteSwitch.top = hd7 + 1;
        KeyboardSwitch.bottom = GUISwitch.bottom = WebsiteSwitch.bottom = ClientRegion.bottom;

        CSTR.left = CSTR.top = 100;
        CSTR.right = ClientRegion.right - 100;
        CSTR.bottom = 200;
    }
    else
    {
        AlternateActivateCalc(hdc);
    }
}

void DrawMainLines(HDC hdc, BOOL e = FALSE)
{
    if (e)
    {
        SelectObject(hdc, BlackPen);

        switch (prevEdit)
        {
        case 1:
        {
            MoveToEx(hdc, -1, hd2 - 40, NULL);
            LineTo(hdc, wd1, hd2 - 40);
            LineTo(hdc, wd1, hd2 - 2);
        }
        break;
        case 2:
        {
            MoveToEx(hdc, wd1, hd2 - 2, NULL);
            LineTo(hdc, wd1, hd2 - 41);
            LineTo(hdc, wd2, hd2 - 40);
            LineTo(hdc, wd2, hd2 - 2);
        }
        break;
        case 3:
        {
            MoveToEx(hdc, wd2, hd2 - 2, NULL);
            LineTo(hdc, wd2, hd2 - 41);
            LineTo(hdc, wd3, hd2 - 40);
            LineTo(hdc, wd3, hd2 - 2);
        }
        break;
        case 4:
        {
            MoveToEx(hdc, wd3, hd2 - 2, NULL);
            LineTo(hdc, wd3, hd2 - 41);
            LineTo(hdc, wd4, hd2 - 40);
            LineTo(hdc, wd4, hd2 - 2);
        }
        break;
        case 5:
        {
            MoveToEx(hdc, wd4, hd2 - 2, NULL);
            LineTo(hdc, wd4, hd2 - 41);
            LineTo(hdc, wd5, hd2 - 40);
            LineTo(hdc, wd5, hd2 - 2);
        }
        break;
        case 6:
        {
            MoveToEx(hdc, wd5, hd2 - 2, NULL);
            LineTo(hdc, wd5, hd2 - 41);
            LineTo(hdc, wd6, hd2 - 40);
            LineTo(hdc, wd6, hd2 - 2);
        }
        break;
        case 7:
        {
            MoveToEx(hdc, wd6, hd2 - 2, NULL);
            LineTo(hdc, wd6, hd2 - 41);
            LineTo(hdc, wd7, hd2 - 40);
            LineTo(hdc, wd7, hd2 - 2);
        }
        break;
        case 8:
        {
            MoveToEx(hdc, wd7, hd2 - 2, NULL);
            LineTo(hdc, wd7, hd2 - 41);
            LineTo(hdc, wd8, hd2 - 40);
            LineTo(hdc, wd8, hd2 - 2);
        }
        break;
        case 9:
        {
            MoveToEx(hdc, wd8, hd2 - 2, NULL);
            LineTo(hdc, wd8, hd2 - 41);
            LineTo(hdc, wd9, hd2 - 40);
            LineTo(hdc, wd9, hd2 - 2);
        }
        break;
        case 10:
        {
            MoveToEx(hdc, wd9, hd2 - 2, NULL);
            LineTo(hdc, wd9, hd2 - 41);
            LineTo(hdc, ClientRegion.right, hd2 - 40);
        }
        break;
        }

        prevEdit = edit;
        SelectObject(hdc, DarkRedPen);

        switch (prevEdit)
        {
        case 1:
        {
            MoveToEx(hdc, -1, hd2 - 40, NULL);
            LineTo(hdc, wd1, hd2 - 40);
            LineTo(hdc, wd1, hd2 - 2);
        }
        break;
        case 2:
        {
            MoveToEx(hdc, wd1, hd2 - 2, NULL);
            LineTo(hdc, wd1, hd2 - 41);
            LineTo(hdc, wd2, hd2 - 40);
            LineTo(hdc, wd2, hd2 - 2);
        }
        break;
        case 3:
        {
            MoveToEx(hdc, wd2, hd2 - 2, NULL);
            LineTo(hdc, wd2, hd2 - 41);
            LineTo(hdc, wd3, hd2 - 40);
            LineTo(hdc, wd3, hd2 - 2);
        }
        break;
        case 4:
        {
            MoveToEx(hdc, wd3, hd2 - 2, NULL);
            LineTo(hdc, wd3, hd2 - 41);
            LineTo(hdc, wd4, hd2 - 40);
            LineTo(hdc, wd4, hd2 - 2);
        }
        break;
        case 5:
        {
            MoveToEx(hdc, wd4, hd2 - 2, NULL);
            LineTo(hdc, wd4, hd2 - 41);
            LineTo(hdc, wd5, hd2 - 40);
            LineTo(hdc, wd5, hd2 - 2);
        }
        break;
        case 6:
        {
            MoveToEx(hdc, wd5, hd2 - 2, NULL);
            LineTo(hdc, wd5, hd2 - 41);
            LineTo(hdc, wd6, hd2 - 40);
            LineTo(hdc, wd6, hd2 - 2);
        }
        break;
        case 7:
        {
            MoveToEx(hdc, wd6, hd2 - 2, NULL);
            LineTo(hdc, wd6, hd2 - 41);
            LineTo(hdc, wd7, hd2 - 40);
            LineTo(hdc, wd7, hd2 - 2);
        }
        break;
        case 8:
        {
            MoveToEx(hdc, wd7, hd2 - 2, NULL);
            LineTo(hdc, wd7, hd2 - 41);
            LineTo(hdc, wd8, hd2 - 40);
            LineTo(hdc, wd8, hd2 - 2);
        }
        break;
        case 9:
        {
            MoveToEx(hdc, wd8, hd2 - 2, NULL);
            LineTo(hdc, wd8, hd2 - 41);
            LineTo(hdc, wd9, hd2 - 40);
            LineTo(hdc, wd9, hd2 - 2);
        }
        break;
        case 10:
        {
            MoveToEx(hdc, wd9, hd2 - 2, NULL);
            LineTo(hdc, wd9, hd2 - 41);
            LineTo(hdc, ClientRegion.right, hd2 - 40);
        }
        break;
        }
    }
    else
    {
        SelectObject(hdc, DarkRedPen);

        // Horizontol
        MoveToEx(hdc, -1, hd2, NULL);
        LineTo(hdc, ClientRegion.right, hd2);
        MoveToEx(hdc, -1, hd3, NULL);
        LineTo(hdc, ClientRegion.right, hd3);
        MoveToEx(hdc, -1, hd4, NULL);
        LineTo(hdc, ClientRegion.right, hd4);
        MoveToEx(hdc, -1, hd5, NULL);
        LineTo(hdc, ClientRegion.right, hd5);
        MoveToEx(hdc, wd8, hd6, NULL);
        LineTo(hdc, ClientRegion.right, hd6);
        MoveToEx(hdc, -1, hd7, NULL);
        LineTo(hdc, ClientRegion.right, hd7);

        // Vertical
        MoveToEx(hdc, wd1, hd2, NULL);
        LineTo(hdc, wd1, hd5);
        MoveToEx(hdc, wd2, hd2, NULL);
        LineTo(hdc, wd2, hd5);
        MoveToEx(hdc, wd3, hd2, NULL);
        LineTo(hdc, wd3, hd5);
        MoveToEx(hdc, wd4, hd2, NULL);
        LineTo(hdc, wd4, hd5);
        MoveToEx(hdc, wd5, hd2, NULL);
        LineTo(hdc, wd5, hd5);
        MoveToEx(hdc, wd6, hd2, NULL);
        LineTo(hdc, wd6, hd5);
        MoveToEx(hdc, wd7, hd2, NULL);
        LineTo(hdc, wd7, hd5);
        MoveToEx(hdc, wd8, hd2, NULL);
        LineTo(hdc, wd8, hd6);
        MoveToEx(hdc, wd9, hd2, NULL);
        LineTo(hdc, wd9, hd6);

        // Switch Vertical
        MoveToEx(hdc, ws1, hd7, NULL);
        LineTo(hdc, ws1, ClientRegion.bottom);
        MoveToEx(hdc, ws2, hd7, NULL);
        LineTo(hdc, ws2, ClientRegion.bottom);
    }

    SelectObject(hdc, NullPen);
}

void UpdateButtonsLabelsFull(HDC hdc)
{
    BOOL kActive = ButtonsActive[32] == TRUE;
    BOOL gActive = ButtonsActive[33] == TRUE;

    FillRect(hdc, &P1, ButtonsActive[0] == TRUE && gActive ? DarkRedBrush : ButtonsActive[0] == TRUE ? HoverDarkRedBrush : BlackBrush);
    SetTextColor(hdc, !gActive ? HoverDarkRedColor : ButtonsActive[0] == TRUE ? BlackColor : DarkRedColor);
    DrawText(hdc, kActive ? L"1" : L"Power", -1, &P1, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    FillRect(hdc, &P2, ButtonsActive[1] == TRUE && gActive ? DarkRedBrush : ButtonsActive[1] == TRUE ? HoverDarkRedBrush : BlackBrush);
    SetTextColor(hdc, !gActive ? HoverDarkRedColor : ButtonsActive[1] == TRUE ? BlackColor : DarkRedColor);
    DrawText(hdc, kActive ? L"2" : L"Power", -1, &P2, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    FillRect(hdc, &P3, ButtonsActive[2] == TRUE && gActive ? DarkRedBrush : ButtonsActive[2] == TRUE ? HoverDarkRedBrush : BlackBrush);
    SetTextColor(hdc, !gActive ? HoverDarkRedColor : ButtonsActive[2] == TRUE ? BlackColor : DarkRedColor);
    DrawText(hdc, kActive ? L"3" : L"Power", -1, &P3, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    FillRect(hdc, &P4, ButtonsActive[3] == TRUE && gActive ? DarkRedBrush : ButtonsActive[3] == TRUE ? HoverDarkRedBrush : BlackBrush);
    SetTextColor(hdc, !gActive ? HoverDarkRedColor : ButtonsActive[3] == TRUE ? BlackColor : DarkRedColor);
    DrawText(hdc, kActive ? L"4" : L"Power", -1, &P4, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    FillRect(hdc, &P5, ButtonsActive[4] == TRUE && gActive ? DarkRedBrush : ButtonsActive[4] == TRUE ? HoverDarkRedBrush : BlackBrush);
    SetTextColor(hdc, !gActive ? HoverDarkRedColor : ButtonsActive[4] == TRUE ? BlackColor : DarkRedColor);
    DrawText(hdc, kActive ? L"5" : L"Power", -1, &P5, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    FillRect(hdc, &P6, ButtonsActive[5] == TRUE && gActive ? DarkRedBrush : ButtonsActive[5] == TRUE ? HoverDarkRedBrush : BlackBrush);
    SetTextColor(hdc, !gActive ? HoverDarkRedColor : ButtonsActive[5] == TRUE ? BlackColor : DarkRedColor);
    DrawText(hdc, kActive ? L"6" : L"Power", -1, &P6, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    FillRect(hdc, &P7, ButtonsActive[6] == TRUE && gActive ? DarkRedBrush : ButtonsActive[6] == TRUE ? HoverDarkRedBrush : BlackBrush);
    SetTextColor(hdc, !gActive ? HoverDarkRedColor : ButtonsActive[6] == TRUE ? BlackColor : DarkRedColor);
    DrawText(hdc, kActive ? L"7" : L"Power", -1, &P7, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    FillRect(hdc, &P8, ButtonsActive[7] == TRUE && gActive ? DarkRedBrush : ButtonsActive[7] == TRUE ? HoverDarkRedBrush : BlackBrush);
    SetTextColor(hdc, !gActive ? HoverDarkRedColor : ButtonsActive[7] == TRUE ? BlackColor : DarkRedColor);
    DrawText(hdc, kActive ? L"8" : L"Power", -1, &P8, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    FillRect(hdc, &P9, ButtonsActive[8] == TRUE && gActive ? DarkRedBrush : ButtonsActive[8] == TRUE ? HoverDarkRedBrush : BlackBrush);
    SetTextColor(hdc, !gActive ? HoverDarkRedColor : ButtonsActive[8] == TRUE ? BlackColor : DarkRedColor);
    DrawText(hdc, kActive ? L"9" : L"Power", -1, &P9, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    FillRect(hdc, &P10, ButtonsActive[9] == TRUE && gActive ? DarkRedBrush : ButtonsActive[9] == TRUE ? HoverDarkRedBrush : BlackBrush);
    SetTextColor(hdc, !gActive ? HoverDarkRedColor : ButtonsActive[9] == TRUE ? BlackColor : DarkRedColor);
    DrawText(hdc, kActive ? L"0" : L"Power", -1, &P10, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    FillRect(hdc, &A1, ButtonsActive[10] == TRUE && gActive ? DarkRedBrush : ButtonsActive[10] == TRUE ? HoverDarkRedBrush : BlackBrush);
    SetTextColor(hdc, !gActive ? HoverDarkRedColor : ButtonsActive[10] == TRUE ? BlackColor : DarkRedColor);
    DrawText(hdc, kActive ? L"Q" : L"Activate", -1, &A1, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    FillRect(hdc, &A2, ButtonsActive[11] == TRUE && gActive ? DarkRedBrush : ButtonsActive[11] == TRUE ? HoverDarkRedBrush : BlackBrush);
    SetTextColor(hdc, !gActive ? HoverDarkRedColor : ButtonsActive[11] == TRUE ? BlackColor : DarkRedColor);
    DrawText(hdc, kActive ? L"W" : L"Activate", -1, &A2, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    FillRect(hdc, &A3, ButtonsActive[12] == TRUE && gActive ? DarkRedBrush : ButtonsActive[12] == TRUE ? HoverDarkRedBrush : BlackBrush);
    SetTextColor(hdc, !gActive ? HoverDarkRedColor : ButtonsActive[12] == TRUE ? BlackColor : DarkRedColor);
    DrawText(hdc, kActive ? L"E" : L"Activate", -1, &A3, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    FillRect(hdc, &A4, ButtonsActive[13] == TRUE && gActive ? DarkRedBrush : ButtonsActive[13] == TRUE ? HoverDarkRedBrush : BlackBrush);
    SetTextColor(hdc, !gActive ? HoverDarkRedColor : ButtonsActive[13] == TRUE ? BlackColor : DarkRedColor);
    DrawText(hdc, kActive ? L"R" : L"Activate", -1, &A4, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    FillRect(hdc, &A5, ButtonsActive[14] == TRUE && gActive ? DarkRedBrush : ButtonsActive[14] == TRUE ? HoverDarkRedBrush : BlackBrush);
    SetTextColor(hdc, !gActive ? HoverDarkRedColor : ButtonsActive[14] == TRUE ? BlackColor : DarkRedColor);
    DrawText(hdc, kActive ? L"T" : L"Activate", -1, &A5, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    FillRect(hdc, &A6, ButtonsActive[15] == TRUE && gActive ? DarkRedBrush : ButtonsActive[15] == TRUE ? HoverDarkRedBrush : BlackBrush);
    SetTextColor(hdc, !gActive ? HoverDarkRedColor : ButtonsActive[15] == TRUE ? BlackColor : DarkRedColor);
    DrawText(hdc, kActive ? L"Y" : L"Activate", -1, &A6, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    FillRect(hdc, &A7, ButtonsActive[16] == TRUE && gActive ? DarkRedBrush : ButtonsActive[16] == TRUE ? HoverDarkRedBrush : BlackBrush);
    SetTextColor(hdc, !gActive ? HoverDarkRedColor : ButtonsActive[16] == TRUE ? BlackColor : DarkRedColor);
    DrawText(hdc, kActive ? L"U" : L"Activate", -1, &A7, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    FillRect(hdc, &A8, ButtonsActive[17] == TRUE && gActive ? DarkRedBrush : ButtonsActive[17] == TRUE ? HoverDarkRedBrush : BlackBrush);
    SetTextColor(hdc, !gActive ? HoverDarkRedColor : ButtonsActive[17] == TRUE ? BlackColor : DarkRedColor);
    DrawText(hdc, kActive ? L"I" : L"Activate", -1, &A8, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    FillRect(hdc, &A9, ButtonsActive[18] == TRUE && gActive ? DarkRedBrush : ButtonsActive[18] == TRUE ? HoverDarkRedBrush : BlackBrush);
    SetTextColor(hdc, !gActive ? HoverDarkRedColor : ButtonsActive[18] == TRUE ? BlackColor : DarkRedColor);
    DrawText(hdc, kActive ? L"O" : L"Activate", -1, &A9, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    FillRect(hdc, &A10, ButtonsActive[19] == TRUE && gActive ? DarkRedBrush : ButtonsActive[19] == TRUE ? HoverDarkRedBrush : BlackBrush);
    SetTextColor(hdc, !gActive ? HoverDarkRedColor : ButtonsActive[19] == TRUE ? BlackColor : DarkRedColor);
    DrawText(hdc, kActive ? L"P" : L"Activate", -1, &A10, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    FillRect(hdc, &X1, ButtonsActive[20] == TRUE && gActive ? DarkRedBrush : ButtonsActive[20] == TRUE ? HoverDarkRedBrush : BlackBrush);
    SetTextColor(hdc, !gActive ? HoverDarkRedColor : ButtonsActive[20] == TRUE ? BlackColor : DarkRedColor);
    DrawText(hdc, kActive ? L"A" : L"Alternate\r\nActivate", -1, kActive ? &X1 : &XR1, kActive ? DT_CENTER | DT_VCENTER | DT_SINGLELINE : DT_CENTER | DT_WORDBREAK);
    FillRect(hdc, &X2, ButtonsActive[21] == TRUE && gActive ? DarkRedBrush : ButtonsActive[21] == TRUE ? HoverDarkRedBrush : BlackBrush);
    SetTextColor(hdc, !gActive ? HoverDarkRedColor : ButtonsActive[21] == TRUE ? BlackColor : DarkRedColor);
    DrawText(hdc, kActive ? L"S" : L"Alternate\r\nActivate", -1, kActive ? &X2 : &XR2, kActive ? DT_CENTER | DT_VCENTER | DT_SINGLELINE : DT_CENTER | DT_WORDBREAK);
    FillRect(hdc, &X3, ButtonsActive[22] == TRUE && gActive ? DarkRedBrush : ButtonsActive[22] == TRUE ? HoverDarkRedBrush : BlackBrush);
    SetTextColor(hdc, !gActive ? HoverDarkRedColor : ButtonsActive[22] == TRUE ? BlackColor : DarkRedColor);
    DrawText(hdc, kActive ? L"D" : L"Alternate\r\nActivate", -1, kActive ? &X3 : &XR3, kActive ? DT_CENTER | DT_VCENTER | DT_SINGLELINE : DT_CENTER | DT_WORDBREAK);
    FillRect(hdc, &X4, ButtonsActive[23] == TRUE && gActive ? DarkRedBrush : ButtonsActive[23] == TRUE ? HoverDarkRedBrush : BlackBrush);
    SetTextColor(hdc, !gActive ? HoverDarkRedColor : ButtonsActive[23] == TRUE ? BlackColor : DarkRedColor);
    DrawText(hdc, kActive ? L"F" : L"Alternate\r\nActivate", -1, kActive ? &X4 : &XR4, kActive ? DT_CENTER | DT_VCENTER | DT_SINGLELINE : DT_CENTER | DT_WORDBREAK);
    FillRect(hdc, &X5, ButtonsActive[24] == TRUE && gActive ? DarkRedBrush : ButtonsActive[24] == TRUE ? HoverDarkRedBrush : BlackBrush);
    SetTextColor(hdc, !gActive ? HoverDarkRedColor : ButtonsActive[24] == TRUE ? BlackColor : DarkRedColor);
    DrawText(hdc, kActive ? L"G" : L"Alternate\r\nActivate", -1, kActive ? &X5 : &XR5, kActive ? DT_CENTER | DT_VCENTER | DT_SINGLELINE : DT_CENTER | DT_WORDBREAK);
    FillRect(hdc, &X6, ButtonsActive[25] == TRUE && gActive ? DarkRedBrush : ButtonsActive[25] == TRUE ? HoverDarkRedBrush : BlackBrush);
    SetTextColor(hdc, !gActive ? HoverDarkRedColor : ButtonsActive[25] == TRUE ? BlackColor : DarkRedColor);
    DrawText(hdc, kActive ? L"H" : L"Alternate\r\nActivate", -1, kActive ? &X6 : &XR6, kActive ? DT_CENTER | DT_VCENTER | DT_SINGLELINE : DT_CENTER | DT_WORDBREAK);
    FillRect(hdc, &X7, ButtonsActive[26] == TRUE && gActive ? DarkRedBrush : ButtonsActive[26] == TRUE ? HoverDarkRedBrush : BlackBrush);
    SetTextColor(hdc, !gActive ? HoverDarkRedColor : ButtonsActive[26] == TRUE ? BlackColor : DarkRedColor);
    DrawText(hdc, kActive ? L"J" : L"Alternate\r\nActivate", -1, kActive ? &X7 : &XR7, kActive ? DT_CENTER | DT_VCENTER | DT_SINGLELINE : DT_CENTER | DT_WORDBREAK);
    FillRect(hdc, &X8, ButtonsActive[27] == TRUE && gActive ? DarkRedBrush : ButtonsActive[27] == TRUE ? HoverDarkRedBrush : BlackBrush);
    SetTextColor(hdc, !gActive ? HoverDarkRedColor : ButtonsActive[27] == TRUE ? BlackColor : DarkRedColor);
    DrawText(hdc, kActive ? L"K" : L"Alternate\r\nActivate", -1, kActive ? &X8 : &XR8, kActive ? DT_CENTER | DT_VCENTER | DT_SINGLELINE : DT_CENTER | DT_WORDBREAK);
    FillRect(hdc, &X9, ButtonsActive[28] == TRUE && gActive ? DarkRedBrush : ButtonsActive[28] == TRUE ? HoverDarkRedBrush : BlackBrush);
    SetTextColor(hdc, !gActive ? HoverDarkRedColor : ButtonsActive[28] == TRUE ? BlackColor : DarkRedColor);
    DrawText(hdc, kActive ? L"L" : L"Alternate\r\nActivate", -1, kActive ? &X9 : &XR9, kActive ? DT_CENTER | DT_VCENTER | DT_SINGLELINE : DT_CENTER | DT_WORDBREAK);
    FillRect(hdc, &X10, ButtonsActive[29] == TRUE && gActive ? DarkRedBrush : ButtonsActive[29] == TRUE ? HoverDarkRedBrush : BlackBrush);
    SetTextColor(hdc, !gActive ? HoverDarkRedColor : ButtonsActive[29] == TRUE ? BlackColor : DarkRedColor);
    DrawText(hdc, kActive ? L";" : L"Alternate\r\nActivate", -1, kActive ? &X10 : &XR10, kActive ? DT_CENTER | DT_VCENTER | DT_SINGLELINE : DT_CENTER | DT_WORDBREAK);

    FillRect(hdc, &DoorBtn, ButtonsActive[30] == TRUE && gActive ? DarkRedBrush : ButtonsActive[30] == TRUE ? HoverDarkRedBrush : BlackBrush);
    SetTextColor(hdc, !gActive ? HoverDarkRedColor : ButtonsActive[30] == TRUE ? BlackColor : DarkRedColor);
    DrawText(hdc, kActive ? L"Door (space)" : L"Door", -1, &DoorBtn, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    FillRect(hdc, &GarbageBtn, ButtonsActive[31] == TRUE && gActive ? DarkRedBrush : ButtonsActive[31] == TRUE ? HoverDarkRedBrush : BlackBrush);
    SetTextColor(hdc, !gActive ? HoverDarkRedColor : ButtonsActive[31] == TRUE ? BlackColor : DarkRedColor);
    DrawText(hdc, kActive ? L"Garbage (.)" : L"Garbage", -1, &GarbageBtn, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    FillRect(hdc, &KeyboardSwitch, kActive ? DarkRedBrush : BlackBrush);
    SetTextColor(hdc, kActive ? BlackColor : DarkRedColor);
    DrawText(hdc, L"Keyboard", -1, &KeyboardSwitch, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    FillRect(hdc, &GUISwitch, gActive ? DarkRedBrush : BlackBrush);
    SetTextColor(hdc, gActive ? BlackColor : DarkRedColor);
    DrawText(hdc, L"GUI", -1, &GUISwitch, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    FillRect(hdc, &WebsiteSwitch, ButtonsActive[34] == TRUE ? DarkRedBrush : BlackBrush);
    SetTextColor(hdc, ButtonsActive[34] ? BlackColor : DarkRedColor);
    DrawText(hdc, L"Website", -1, &WebsiteSwitch, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    SetTextColor(hdc, DarkRedColor);
    FillRect(hdc, &L1, BlackBrush);
    DrawTextA(hdc, L1text, -1, &L1, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    FillRect(hdc, &L2, BlackBrush);
    DrawTextA(hdc, L2text, -1, &L2, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    FillRect(hdc, &L3, BlackBrush);
    DrawTextA(hdc, L3text, -1, &L3, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    FillRect(hdc, &L4, BlackBrush);
    DrawTextA(hdc, L4text, -1, &L4, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    FillRect(hdc, &L5, BlackBrush);
    DrawTextA(hdc, L5text, -1, &L5, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    FillRect(hdc, &L6, BlackBrush);
    DrawTextA(hdc, L6text, -1, &L6, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    FillRect(hdc, &L7, BlackBrush);
    DrawTextA(hdc, L7text, -1, &L7, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    FillRect(hdc, &L8, BlackBrush);
    DrawTextA(hdc, L8text, -1, &L8, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    FillRect(hdc, &L9, BlackBrush);
    DrawTextA(hdc, L9text, -1, &L9, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    FillRect(hdc, &L10, BlackBrush);
    DrawTextA(hdc, L10text, -1, &L10, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void DoSpecBtn(HDC hdc, UINT btn, COLORREF textColor, HBRUSH rectBrush)
{
    SetTextColor(hdc, textColor);
    switch (btn)
    {
    case 1:
    {
        FillRect(hdc, &P1, rectBrush);
        DrawText(hdc, ButtonsActive[32] == 1 ? L"1" : L"Power", -1, &P1, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    break;
    case 2:
    {
        FillRect(hdc, &P2, rectBrush);
        DrawText(hdc, ButtonsActive[32] == 1 ? L"2" : L"Power", -1, &P2, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    break;
    case 3:
    {
        FillRect(hdc, &P3, rectBrush);
        DrawText(hdc, ButtonsActive[32] == 1 ? L"3" : L"Power", -1, &P3, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    break;
    case 4:
    {
        FillRect(hdc, &P4, rectBrush);
        DrawText(hdc, ButtonsActive[32] == 1 ? L"4" : L"Power", -1, &P4, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    break;
    case 5:
    {
        FillRect(hdc, &P5, rectBrush);
        DrawText(hdc, ButtonsActive[32] == 1 ? L"5" : L"Power", -1, &P5, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    break;
    case 6:
    {
        FillRect(hdc, &P6, rectBrush);
        DrawText(hdc, ButtonsActive[32] == 1 ? L"6" : L"Power", -1, &P6, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    break;
    case 7:
    {
        FillRect(hdc, &P7, rectBrush);
        DrawText(hdc, ButtonsActive[32] == 1 ? L"7" : L"Power", -1, &P7, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    break;
    case 8:
    {
        FillRect(hdc, &P8, rectBrush);
        DrawText(hdc, ButtonsActive[32] == 1 ? L"8" : L"Power", -1, &P8, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    break;
    case 9:
    {
        FillRect(hdc, &P9, rectBrush);
        DrawText(hdc, ButtonsActive[32] == 1 ? L"9" : L"Power", -1, &P9, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    break;
    case 10:
    {
        FillRect(hdc, &P10, rectBrush);
        DrawText(hdc, ButtonsActive[32] == 1 ? L"0" : L"Power", -1, &P10, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    break;
    case 11:
    {
        FillRect(hdc, &A1, rectBrush);
        DrawText(hdc, ButtonsActive[32] == 1 ? L"Q" : L"Activate", -1, &A1, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    break;
    case 12:
    {
        FillRect(hdc, &A2, rectBrush);
        DrawText(hdc, ButtonsActive[32] == 1 ? L"W" : L"Activate", -1, &A2, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    break;
    case 13:
    {
        FillRect(hdc, &A3, rectBrush);
        DrawText(hdc, ButtonsActive[32] == 1 ? L"E" : L"Activate", -1, &A3, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    break;
    case 14:
    {
        FillRect(hdc, &A4, rectBrush);
        DrawText(hdc, ButtonsActive[32] == 1 ? L"R" : L"Activate", -1, &A4, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    break;
    case 15:
    {
        FillRect(hdc, &A5, rectBrush);
        DrawText(hdc, ButtonsActive[32] == 1 ? L"T" : L"Activate", -1, &A5, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    break;
    case 16:
    {
        FillRect(hdc, &A6, rectBrush);
        DrawText(hdc, ButtonsActive[32] == 1 ? L"Y" : L"Activate", -1, &A6, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    break;
    case 17:
    {
        FillRect(hdc, &A7, rectBrush);
        DrawText(hdc, ButtonsActive[32] == 1 ? L"U" : L"Activate", -1, &A7, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    break;
    case 18:
    {
        FillRect(hdc, &A8, rectBrush);
        DrawText(hdc, ButtonsActive[32] == 1 ? L"I" : L"Activate", -1, &A8, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    break;
    case 19:
    {
        FillRect(hdc, &A9, rectBrush);
        DrawText(hdc, ButtonsActive[32] == 1 ? L"O" : L"Activate", -1, &A9, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    break;
    case 20:
    {
        FillRect(hdc, &A10, rectBrush);
        DrawText(hdc, ButtonsActive[32] == 1 ? L"P" : L"Activate", -1, &A10, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    break;
    case 21:
    {
        FillRect(hdc, &X1, rectBrush);
        DrawText(hdc, ButtonsActive[32] == 1 ? L"A" : L"Alternate\r\nActivate", -1, ButtonsActive[32] == 1 ? &X1 : &XR1, ButtonsActive[32] == 1 ? DT_CENTER | DT_VCENTER | DT_SINGLELINE : DT_CENTER | DT_WORDBREAK);
    }
    break;
    case 22:
    {
        FillRect(hdc, &X2, rectBrush);
        DrawText(hdc, ButtonsActive[32] == 1 ? L"S" : L"Alternate\r\nActivate", -1, ButtonsActive[32] == 1 ? &X2 : &XR2, ButtonsActive[32] == 1 ? DT_CENTER | DT_VCENTER | DT_SINGLELINE : DT_CENTER | DT_WORDBREAK);
    }
    break;
    case 23:
    {
        FillRect(hdc, &X3, rectBrush);
        DrawText(hdc, ButtonsActive[32] == 1 ? L"D" : L"Alternate\r\nActivate", -1, ButtonsActive[32] == 1 ? &X3 : &XR3, ButtonsActive[32] == 1 ? DT_CENTER | DT_VCENTER | DT_SINGLELINE : DT_CENTER | DT_WORDBREAK);
    }
    break;
    case 24:
    {
        FillRect(hdc, &X4, rectBrush);
        DrawText(hdc, ButtonsActive[32] == 1 ? L"F" : L"Alternate\r\nActivate", -1, ButtonsActive[32] == 1 ? &X4 : &XR4, ButtonsActive[32] == 1 ? DT_CENTER | DT_VCENTER | DT_SINGLELINE : DT_CENTER | DT_WORDBREAK);
    }
    break;
    case 25:
    {
        FillRect(hdc, &X5, rectBrush);
        DrawText(hdc, ButtonsActive[32] == 1 ? L"G" : L"Alternate\r\nActivate", -1, ButtonsActive[32] == 1 ? &X5 : &XR5, ButtonsActive[32] == 1 ? DT_CENTER | DT_VCENTER | DT_SINGLELINE : DT_CENTER | DT_WORDBREAK);
    }
    break;
    case 26:
    {
        FillRect(hdc, &X6, rectBrush);
        DrawText(hdc, ButtonsActive[32] == 1 ? L"H" : L"Alternate\r\nActivate", -1, ButtonsActive[32] == 1 ? &X6 : &XR6, ButtonsActive[32] == 1 ? DT_CENTER | DT_VCENTER | DT_SINGLELINE : DT_CENTER | DT_WORDBREAK);
    }
    break;
    case 27:
    {
        FillRect(hdc, &X7, rectBrush);
        DrawText(hdc, ButtonsActive[32] == 1 ? L"J" : L"Alternate\r\nActivate", -1, ButtonsActive[32] == 1 ? &X7 : &XR7, ButtonsActive[32] == 1 ? DT_CENTER | DT_VCENTER | DT_SINGLELINE : DT_CENTER | DT_WORDBREAK);
    }
    break;
    case 28:
    {
        FillRect(hdc, &X8, rectBrush);
        DrawText(hdc, ButtonsActive[32] == 1 ? L"K" : L"Alternate\r\nActivate", -1, ButtonsActive[32] == 1 ? &X8 : &XR8, ButtonsActive[32] == 1 ? DT_CENTER | DT_VCENTER | DT_SINGLELINE : DT_CENTER | DT_WORDBREAK);
    }
    break;
    case 29:
    {
        FillRect(hdc, &X9, rectBrush);
        DrawText(hdc, ButtonsActive[32] == 1 ? L"L" : L"Alternate\r\nActivate", -1, ButtonsActive[32] == 1 ? &X9 : &XR9, ButtonsActive[32] == 1 ? DT_CENTER | DT_VCENTER | DT_SINGLELINE : DT_CENTER | DT_WORDBREAK);
    }
    break;
    case 30:
    {
        FillRect(hdc, &X10, rectBrush);
        DrawText(hdc, ButtonsActive[32] == 1 ? L";" : L"Alternate\r\nActivate", -1, ButtonsActive[32] == 1 ? &X10 : &XR10, ButtonsActive[32] == 1 ? DT_CENTER | DT_VCENTER | DT_SINGLELINE : DT_CENTER | DT_WORDBREAK);
    }
    break;
    case 31:
    {
        FillRect(hdc, &DoorBtn, rectBrush);
        DrawText(hdc, ButtonsActive[32] == 1 ? L"Door (space)" : L"Door", -1, &DoorBtn, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    break;
    case 32:
    {
        FillRect(hdc, &GarbageBtn, rectBrush);
        DrawText(hdc, ButtonsActive[32] == 1 ? L"Garbage (.)" : L"Garbage", -1, &GarbageBtn, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    break;
    case 33:
    {
        FillRect(hdc, &KeyboardSwitch, rectBrush);
        DrawText(hdc, L"Keyboard", -1, &KeyboardSwitch, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    break;
    case 34:
    {
        FillRect(hdc, &GUISwitch, rectBrush);
        DrawText(hdc, L"GUI", -1, &GUISwitch, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    break;
    case 35:
    {
        FillRect(hdc, &WebsiteSwitch, rectBrush);
        DrawText(hdc, L"Website", -1, &WebsiteSwitch, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    break;
    }
}

void UpdateHoverDown(HDC hdc)
{
    if (CursorHitPoint == HTCLIENT)
    {
        if (prevSelected != selected || prevDown != down)
        {
            DoSpecBtn(hdc, prevSelected, prevSelected != 0 && ButtonsActive[prevSelected - 1] == TRUE ? BlackColor : DarkRedColor, prevSelected != 0 && ButtonsActive[prevSelected - 1] == TRUE ? DarkRedBrush : BlackBrush);
            prevSelected = selected;
            prevDown = down;
            DoSpecBtn(hdc, prevSelected, down == selected ? HoverDarkRedColor : DarkRedColor, down == selected ? DarkRedBrush : HoverDarkRedBrush);
        }
    }
    else
    {
        if (prevSelected != selected)
        {
            DoSpecBtn(hdc, prevSelected, prevSelected != 0 && ButtonsActive[prevSelected - 1] == TRUE ? BlackColor : DarkRedColor, prevSelected != 0 && ButtonsActive[prevSelected - 1] == TRUE ? DarkRedBrush : BlackBrush);
        }
        prevSelected = 0;
    }
}

void KeyColorUpdate(HDC hdc)
{
    switch (vkUpdate)
    {
    case '1':
        DoSpecBtn(hdc, 1, ButtonsActive[0] == TRUE ? BlackColor : HoverDarkRedColor, ButtonsActive[0] == TRUE ? HoverDarkRedBrush : BlackBrush);
        break;
    case '2':
        DoSpecBtn(hdc, 2, ButtonsActive[1] == TRUE ? BlackColor : HoverDarkRedColor, ButtonsActive[1] == TRUE ? HoverDarkRedBrush : BlackBrush);
        break;
    case '3':
        DoSpecBtn(hdc, 3, ButtonsActive[2] == TRUE ? BlackColor : HoverDarkRedColor, ButtonsActive[2] == TRUE ? HoverDarkRedBrush : BlackBrush);
        break;
    case '4':
        DoSpecBtn(hdc, 4, ButtonsActive[3] == TRUE ? BlackColor : HoverDarkRedColor, ButtonsActive[3] == TRUE ? HoverDarkRedBrush : BlackBrush);
        break;
    case '5':
        DoSpecBtn(hdc, 5, ButtonsActive[4] == TRUE ? BlackColor : HoverDarkRedColor, ButtonsActive[4] == TRUE ? HoverDarkRedBrush : BlackBrush);
        break;
    case '6':
        DoSpecBtn(hdc, 6, ButtonsActive[5] == TRUE ? BlackColor : HoverDarkRedColor, ButtonsActive[5] == TRUE ? HoverDarkRedBrush : BlackBrush);
        break;
    case '7':
        DoSpecBtn(hdc, 7, ButtonsActive[6] == TRUE ? BlackColor : HoverDarkRedColor, ButtonsActive[6] == TRUE ? HoverDarkRedBrush : BlackBrush);
        break;
    case '8':
        DoSpecBtn(hdc, 8, ButtonsActive[7] == TRUE ? BlackColor : HoverDarkRedColor, ButtonsActive[7] == TRUE ? HoverDarkRedBrush : BlackBrush);
        break;
    case '9':
        DoSpecBtn(hdc, 9, ButtonsActive[8] == TRUE ? BlackColor : HoverDarkRedColor, ButtonsActive[8] == TRUE ? HoverDarkRedBrush : BlackBrush);
        break;
    case '0':
        DoSpecBtn(hdc, 10, ButtonsActive[9] == TRUE ? BlackColor : HoverDarkRedColor, ButtonsActive[9] == TRUE ? HoverDarkRedBrush : BlackBrush);
        break;
    case 'Q':
        DoSpecBtn(hdc, 11, ButtonsActive[10] == TRUE ? BlackColor : HoverDarkRedColor, ButtonsActive[10] == TRUE ? HoverDarkRedBrush : BlackBrush);
        break;
    case 'W':
        DoSpecBtn(hdc, 12, ButtonsActive[11] == TRUE ? BlackColor : HoverDarkRedColor, ButtonsActive[11] == TRUE ? HoverDarkRedBrush : BlackBrush);
        break;
    case 'E':
        DoSpecBtn(hdc, 13, ButtonsActive[12] == TRUE ? BlackColor : HoverDarkRedColor, ButtonsActive[12] == TRUE ? HoverDarkRedBrush : BlackBrush);
        break;
    case 'R':
        DoSpecBtn(hdc, 14, ButtonsActive[13] == TRUE ? BlackColor : HoverDarkRedColor, ButtonsActive[13] == TRUE ? HoverDarkRedBrush : BlackBrush);
        break;
    case 'T':
        DoSpecBtn(hdc, 15, ButtonsActive[14] == TRUE ? BlackColor : HoverDarkRedColor, ButtonsActive[14] == TRUE ? HoverDarkRedBrush : BlackBrush);
        break;
    case 'Y':
        DoSpecBtn(hdc, 16, ButtonsActive[15] == TRUE ? BlackColor : HoverDarkRedColor, ButtonsActive[15] == TRUE ? HoverDarkRedBrush : BlackBrush);
        break;
    case 'U':
        DoSpecBtn(hdc, 17, ButtonsActive[16] == TRUE ? BlackColor : HoverDarkRedColor, ButtonsActive[16] == TRUE ? HoverDarkRedBrush : BlackBrush);
        break;
    case 'I':
        DoSpecBtn(hdc, 18, ButtonsActive[17] == TRUE ? BlackColor : HoverDarkRedColor, ButtonsActive[17] == TRUE ? HoverDarkRedBrush : BlackBrush);
        break;
    case 'O':
        DoSpecBtn(hdc, 19, ButtonsActive[18] == TRUE ? BlackColor : HoverDarkRedColor, ButtonsActive[18] == TRUE ? HoverDarkRedBrush : BlackBrush);
        break;
    case 'P':
        DoSpecBtn(hdc, 20, ButtonsActive[19] == TRUE ? BlackColor : HoverDarkRedColor, ButtonsActive[19] == TRUE ? HoverDarkRedBrush : BlackBrush);
        break;
    case 'A':
        DoSpecBtn(hdc, 21, ButtonsActive[20] == TRUE ? BlackColor : HoverDarkRedColor, ButtonsActive[20] == TRUE ? HoverDarkRedBrush : BlackBrush);
        break;
    case 'S':
        DoSpecBtn(hdc, 22, ButtonsActive[21] == TRUE ? BlackColor : HoverDarkRedColor, ButtonsActive[21] == TRUE ? HoverDarkRedBrush : BlackBrush);
        break;
    case 'D':
        DoSpecBtn(hdc, 23, ButtonsActive[22] == TRUE ? BlackColor : HoverDarkRedColor, ButtonsActive[22] == TRUE ? HoverDarkRedBrush : BlackBrush);
        break;
    case 'F':
        DoSpecBtn(hdc, 24, ButtonsActive[23] == TRUE ? BlackColor : HoverDarkRedColor, ButtonsActive[23] == TRUE ? HoverDarkRedBrush : BlackBrush);
        break;
    case 'G':
        DoSpecBtn(hdc, 25, ButtonsActive[24] == TRUE ? BlackColor : HoverDarkRedColor, ButtonsActive[24] == TRUE ? HoverDarkRedBrush : BlackBrush);
        break;
    case 'H':
        DoSpecBtn(hdc, 26, ButtonsActive[25] == TRUE ? BlackColor : HoverDarkRedColor, ButtonsActive[25] == TRUE ? HoverDarkRedBrush : BlackBrush);
        break;
    case 'J':
        DoSpecBtn(hdc, 27, ButtonsActive[26] == TRUE ? BlackColor : HoverDarkRedColor, ButtonsActive[26] == TRUE ? HoverDarkRedBrush : BlackBrush);
        break;
    case 'K':
        DoSpecBtn(hdc, 28, ButtonsActive[27] == TRUE ? BlackColor : HoverDarkRedColor, ButtonsActive[27] == TRUE ? HoverDarkRedBrush : BlackBrush);
        break;
    case 'L':
        DoSpecBtn(hdc, 29, ButtonsActive[28] == TRUE ? BlackColor : HoverDarkRedColor, ButtonsActive[28] == TRUE ? HoverDarkRedBrush : BlackBrush);
        break;
    case VK_OEM_1:
        DoSpecBtn(hdc, 30, ButtonsActive[29] == TRUE ? BlackColor : HoverDarkRedColor, ButtonsActive[29] == TRUE ? HoverDarkRedBrush : BlackBrush);
        break;
    case VK_SPACE:
        DoSpecBtn(hdc, 31, ButtonsActive[30] == TRUE ? BlackColor : HoverDarkRedColor, ButtonsActive[30] == TRUE ? HoverDarkRedBrush : BlackBrush);
        break;
    case VK_OEM_PERIOD:
        DoSpecBtn(hdc, 32, ButtonsActive[31] == TRUE ? BlackColor : HoverDarkRedColor, ButtonsActive[31] == TRUE ? HoverDarkRedBrush : BlackBrush);
        break;
    }

    vkUpdate = 0;
}

void CalculationsAndDrawings(HDC hdc)
{
    SetBkMode(hdc, TRANSPARENT);

    if (Connected)
    {
        if (prevClientRegion.left != ClientRegion.left ||
            prevClientRegion.top != ClientRegion.top ||
            prevClientRegion.right != ClientRegion.right ||
            prevClientRegion.bottom != ClientRegion.bottom)
        {
            prevClientRegion = ClientRegion;
            UpdateMath(hdc);
            DrawMainLines(hdc);
            UpdateButtonsLabelsFull(hdc);
            UpdateCST(hdc);
        }
        else if (move)
        {
            move = FALSE;
            DrawMainLines(hdc);
            UpdateButtonsLabelsFull(hdc);
        }
        else if (switched)
        {
            switched = FALSE;
            UpdateButtonsLabelsFull(hdc);
        }
        else if (prevSelected != selected || prevDown != down)
        {
            //prevCursorPos = CursorPos;
            UpdateHoverDown(hdc);
        }
        else if (prevEdit != edit || edit != 0)
        {
            SetTextColor(hdc, DarkRedColor);

            switch (edit)
            {
            case 1:
            {
                FillRect(hdc, &L1, BlackBrush);
                DrawTextA(hdc, L1text, -1, &L1, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
            break;
            case 2:
            {
                FillRect(hdc, &L2, BlackBrush);
                DrawTextA(hdc, L2text, -1, &L2, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
            break;
            case 3:
            {
                FillRect(hdc, &L3, BlackBrush);
                DrawTextA(hdc, L3text, -1, &L3, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
            break;
            case 4:
            {
                FillRect(hdc, &L4, BlackBrush);
                DrawTextA(hdc, L4text, -1, &L4, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
            break;
            case 5:
            {
                FillRect(hdc, &L5, BlackBrush);
                DrawTextA(hdc, L5text, -1, &L5, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
            break;
            case 6:
            {
                FillRect(hdc, &L6, BlackBrush);
                DrawTextA(hdc, L6text, -1, &L6, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
            break;
            case 7:
            {
                FillRect(hdc, &L7, BlackBrush);
                DrawTextA(hdc, L7text, -1, &L7, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
            break;
            case 8:
            {
                FillRect(hdc, &L8, BlackBrush);
                DrawTextA(hdc, L8text, -1, &L8, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
            break;
            case 9:
            {
                FillRect(hdc, &L9, BlackBrush);
                DrawTextA(hdc, L9text, -1, &L9, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
            break;
            case 10:
            {
                FillRect(hdc, &L10, BlackBrush);
                DrawTextA(hdc, L10text, -1, &L10, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
            break;
            }

            DrawMainLines(hdc, TRUE);
        }
        else if (ButtonsActive[32] == TRUE)
        {
            KeyColorUpdate(hdc);
        }
    }
    else if (CSTC)
    {
        CSTC = FALSE;
        UpdateCST(hdc);
    }
}

void MousePosCalc(BOOL dblclk = FALSE)
{
    if (dblclk == 2 && ((CursorHitPoint != HTCLIENT) || !(hd2 - 40 < (UINT)CursorPos.y && (UINT)CursorPos.y <= hd2)))
    {
        edit = 0;
        return;
    }
    if (prevCursorPos.x != CursorPos.x || prevCursorPos.y != CursorPos.y || dblclk)
    {
        prevCursorPos = CursorPos;
        if (CursorHitPoint == HTCLIENT)
        {
            if ((hd2 - 40 < (UINT)CursorPos.y && (UINT)CursorPos.y <= hd2) || (hd7 < (UINT)CursorPos.y && CursorPos.y <= ClientRegion.bottom))
            {
                selected = 0;
                if (hd2 - 40 < (UINT)CursorPos.y && (UINT)CursorPos.y <= hd2)
                {
                    SetCursor(ClickCursor);
                    if (0 <= CursorPos.x && (UINT)CursorPos.x <= wd1)
                    {
                        edit = dblclk ? 1 : edit;
                    }
                    else if (wd1 < (UINT)CursorPos.x && (UINT)CursorPos.x <= wd2)
                    {
                        edit = dblclk ? 2 : edit;
                    }
                    else if (wd2 < (UINT)CursorPos.x && (UINT)CursorPos.x <= wd3)
                    {
                        edit = dblclk ? 3 : edit;
                    }
                    else if (wd3 < (UINT)CursorPos.x && (UINT)CursorPos.x <= wd4)
                    {
                        edit = dblclk ? 4 : edit;
                    }
                    else if (wd4 < (UINT)CursorPos.x && (UINT)CursorPos.x <= wd5)
                    {
                        edit = dblclk ? 5 : edit;
                    }
                    else if (wd5 < (UINT)CursorPos.x && (UINT)CursorPos.x <= wd6)
                    {
                        edit = dblclk ? 6 : edit;
                    }
                    else if (wd6 < (UINT)CursorPos.x && (UINT)CursorPos.x <= wd7)
                    {
                        edit = dblclk ? 7 : edit;
                    }
                    else if (wd7 < (UINT)CursorPos.x && (UINT)CursorPos.x <= wd8)
                    {
                        edit = dblclk ? 8 : edit;
                    }
                    else if (wd8 < (UINT)CursorPos.x && (UINT)CursorPos.x <= wd9)
                    {
                        edit = dblclk ? 9 : edit;
                    }
                    else if (wd9 < (UINT)CursorPos.x && CursorPos.x <= ClientRegion.right)
                    {
                        edit = dblclk ? 10 : edit;
                    }
                }
                else if (hd7 < (UINT)CursorPos.y && CursorPos.y <= ClientRegion.bottom)
                {
                    SetCursor(NormalCursor);
                    if (0 <= CursorPos.x && (UINT)CursorPos.x <= ws1)
                    {
                        selected = 33;
                    }
                    else if (ws1 < (UINT)CursorPos.x && (UINT)CursorPos.x <= ws2)
                    {
                        selected = 34;
                    }
                    else if (ws2 < (UINT)CursorPos.x && CursorPos.x <= ClientRegion.right)
                    {
                        selected = 35;
                    }
                }
            }
            else if (ButtonsActive[33] == 1 && hd2 < (UINT)CursorPos.y && (UINT)CursorPos.y <= hd6)
            {
                selected = 0;
                if (hd2 < (UINT)CursorPos.y && (UINT)CursorPos.y <= hd3)
                {
                    SetCursor(NormalCursor);
                    if (0 <= CursorPos.x && (UINT)CursorPos.x <= wd1)
                    {
                        selected = 1;
                    }
                    else if (wd1 < (UINT)CursorPos.x && (UINT)CursorPos.x <= wd2)
                    {
                        selected = 2;
                    }
                    else if (wd2 < (UINT)CursorPos.x && (UINT)CursorPos.x <= wd3)
                    {
                        selected = 3;
                    }
                    else if (wd3 < (UINT)CursorPos.x && (UINT)CursorPos.x <= wd4)
                    {
                        selected = 4;
                    }
                    else if (wd4 < (UINT)CursorPos.x && (UINT)CursorPos.x <= wd5)
                    {
                        selected = 5;
                    }
                    else if (wd5 < (UINT)CursorPos.x && (UINT)CursorPos.x <= wd6)
                    {
                        selected = 6;
                    }
                    else if (wd6 < (UINT)CursorPos.x && (UINT)CursorPos.x <= wd7)
                    {
                        selected = 7;
                    }
                    else if (wd7 < (UINT)CursorPos.x && (UINT)CursorPos.x <= wd8)
                    {
                        selected = 8;
                    }
                    else if (wd8 < (UINT)CursorPos.x && (UINT)CursorPos.x <= wd9)
                    {
                        selected = 9;
                    }
                    else if (wd9 < (UINT)CursorPos.x && CursorPos.x <= ClientRegion.right)
                    {
                        selected = 10;
                    }
                }
                else if (hd3 < (UINT)CursorPos.y && (UINT)CursorPos.y <= hd4)
                {
                    SetCursor(NormalCursor);
                    if (0 <= CursorPos.x && (UINT)CursorPos.x <= wd1)
                    {
                        selected = 11;
                    }
                    else if (wd1 < (UINT)CursorPos.x && (UINT)CursorPos.x <= wd2)
                    {
                        selected = 12;
                    }
                    else if (wd2 < (UINT)CursorPos.x && (UINT)CursorPos.x <= wd3)
                    {
                        selected = 13;
                    }
                    else if (wd3 < (UINT)CursorPos.x && (UINT)CursorPos.x <= wd4)
                    {
                        selected = 14;
                    }
                    else if (wd4 < (UINT)CursorPos.x && (UINT)CursorPos.x <= wd5)
                    {
                        selected = 15;
                    }
                    else if (wd5 < (UINT)CursorPos.x && (UINT)CursorPos.x <= wd6)
                    {
                        selected = 16;
                    }
                    else if (wd6 < (UINT)CursorPos.x && (UINT)CursorPos.x <= wd7)
                    {
                        selected = 17;
                    }
                    else if (wd7 < (UINT)CursorPos.x && (UINT)CursorPos.x <= wd8)
                    {
                        selected = 18;
                    }
                    else if (wd8 < (UINT)CursorPos.x && (UINT)CursorPos.x <= wd9)
                    {
                        selected = 19;
                    }
                    else if (wd9 < (UINT)CursorPos.x && CursorPos.x <= ClientRegion.right)
                    {
                        selected = 20;
                    }
                }
                else if (hd4 < (UINT)CursorPos.y && (UINT)CursorPos.y <= hd5)
                {
                    SetCursor(NormalCursor);
                    if (0 <= CursorPos.x && (UINT)CursorPos.x <= wd1)
                    {
                        selected = 21;
                    }
                    else if (wd1 < (UINT)CursorPos.x && (UINT)CursorPos.x <= wd2)
                    {
                        selected = 22;
                    }
                    else if (wd2 < (UINT)CursorPos.x && (UINT)CursorPos.x <= wd3)
                    {
                        selected = 23;
                    }
                    else if (wd3 < (UINT)CursorPos.x && (UINT)CursorPos.x <= wd4)
                    {
                        selected = 24;
                    }
                    else if (wd4 < (UINT)CursorPos.x && (UINT)CursorPos.x <= wd5)
                    {
                        selected = 25;
                    }
                    else if (wd5 < (UINT)CursorPos.x && (UINT)CursorPos.x <= wd6)
                    {
                        selected = 26;
                    }
                    else if (wd6 < (UINT)CursorPos.x && (UINT)CursorPos.x <= wd7)
                    {
                        selected = 27;
                    }
                    else if (wd7 < (UINT)CursorPos.x && (UINT)CursorPos.x <= wd8)
                    {
                        selected = 28;
                    }
                    else if (wd8 < (UINT)CursorPos.x && (UINT)CursorPos.x <= wd9)
                    {
                        selected = 29;
                    }
                    else if (wd9 < (UINT)CursorPos.x && CursorPos.x <= ClientRegion.right)
                    {
                        selected = 30;
                    }
                }
                else if (hd5 < (UINT)CursorPos.y && (UINT)CursorPos.y <= hd6)
                {
                    SetCursor(NormalCursor);
                    if (wd8 < (UINT)CursorPos.x && (UINT)CursorPos.x <= wd9)
                    {
                        selected = 31;
                    }
                    else if (wd9 < (UINT)CursorPos.x && CursorPos.x <= ClientRegion.right)
                    {
                        selected = 32;
                    }
                }
            }
            else
            {
                selected = 0;
                SetCursor(NormalCursor);
            }
        }
        else
        {
            selected = 0;
        }
    }
}

void StartThread(const char* msg)
{
    _memccpy(ComSerText, msg, 0, strlen(msg));
    //ComSerText = msg;
    PortThread = std::thread([]
        {
            CSTC = TRUE;
            InvalidateRect(hWnd, &CSTR, TRUE);
            UpdateWindow(hWnd);
            char v[0xF]{};
            FindPort(v);
            if (strcmp(v, "") == 0)
            {
                CSTC = TRUE;
                //ComSerText = L"The Control Box is not connected.  Press R to retry.";
                InvalidateRect(hWnd, NULL, TRUE);
                UpdateWindow(hWnd);
            }
            else
            {
                ConnectToPort(v);
                auto asdf = "\033[39m";
            }
        });
    PortThread.detach();
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //OutputDebugString(std::format(L"uMsg: {}\r\nwParam: {}\r\nlParam: {}\r\n", uMsg, wParam, lParam).c_str());
    switch (uMsg)
    {
    case WM_WINDOWPOSCHANGED:
    {
        WINDOWPOS wPos = *(WINDOWPOS*)(void*)lParam;

        if ((wPos.flags & SWP_SHOWWINDOW) != 0)
        {
            GetClientRect(hwnd, &ClientRegion);
            UpdateMath(NULL);
            //InvalidateRect(hwnd, NULL, FALSE);
            //UpdateWindow(hwnd);
        }
        else if ((wPos.flags & SWP_NOSIZE) == 0)
        {
            GetClientRect(hwnd, &ClientRegion);
            UpdateMath(NULL);
            InvalidateRect(hwnd, NULL, FALSE);
            UpdateWindow(hwnd);
        }
        else if ((wPos.flags & SWP_NOMOVE) == 0)
        {
            //InvalidateRect(hwnd, NULL, FALSE);
            move = TRUE;
            UpdateWindow(hwnd);
        }
    }
    break;
    case WM_SETCURSOR:
        CursorHitPoint = LOWORD(lParam);
        break;
    case WM_NCMOUSEMOVE:
    case WM_MOUSEMOVE:
    {
        CursorPos.x = GET_X_LPARAM(lParam);
        CursorPos.y = GET_Y_LPARAM(lParam);
        MousePosCalc();
        if (prevSelected != selected)
        {
            InvalidateRect(hwnd, NULL, FALSE);
            UpdateWindow(hwnd);
        }
    }
    break;
    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    {
        //OutputDebugString(std::format(L"edit: {}\r\n", edit).c_str());
        if (edit != 0 && !(hd2 - 40 < (UINT)CursorPos.y && (UINT)CursorPos.y <= hd2))
        {
            MousePosCalc(2);
            InvalidateRect(hwnd, NULL, FALSE);
            UpdateWindow(hwnd);
        }
        if (selected != 0)
        {
            down = selected;
            if (down == 32)
            {
                ButtonsActive[selected - 1] ^= 1;
                OutputDebugStringA("going into ActiveMonster (LBUTTONCLICK)\n");
                ActiveMonster(selected, ButtonsActive[selected - 1] == TRUE);
                OutputDebugStringA("done with ActiveMonster (LBUTTONCLICK)\n");
            }
            InvalidateRect(hwnd, NULL, FALSE);
            UpdateWindow(hwnd);
        }
        else if (uMsg == WM_LBUTTONDBLCLK)
        {
            MousePosCalc(TRUE);
            InvalidateRect(hwnd, NULL, FALSE);
            UpdateWindow(hwnd);
        }
    }
    break;
    case WM_LBUTTONUP:
    {
        if (selected != 0 && down == selected)
        {
            down = 0;
            switch (selected)
            {
            case 33:
                if (ButtonsActive[32] == 0)
                {
                    for (int i = 0; i < 32; i++)
                    {
                        ButtonsActive[i] = 0;
                    }
                    ButtonsActive[32] = 1;
                    ButtonsActive[33] = 0;
                    ButtonsActive[34] = 0;
                    switched = TRUE;
                }
                break;
            case 34:
                if (ButtonsActive[33] == 0)
                {
                    for (int i = 0; i < 32; i++)
                    {
                        ButtonsActive[i] = 0;
                    }
                    ButtonsActive[32] = 0;
                    ButtonsActive[33] = 1;
                    ButtonsActive[34] = 0;
                    switched = TRUE;
                }
                break;
            case 35:
                if (ButtonsActive[34] == 0)
                {
                    for (int i = 0; i < 32; i++)
                    {
                        ButtonsActive[i] = 0;
                    }
                    ButtonsActive[32] = 0;
                    ButtonsActive[33] = 0;
                    ButtonsActive[34] = 1;
                    switched = TRUE;
                }
                break;
            default:
            {
                ButtonsActive[selected - 1] ^= 1;
                OutputDebugStringA("going into ActiveMonster (LBUTTONUP)\n");
                ActiveMonster(selected, ButtonsActive[selected - 1] == TRUE);
                OutputDebugStringA("done with ActiveMonster (LBUTTONUP)\n");
            }
            break;
            }
            InvalidateRect(hwnd, NULL, FALSE);
            UpdateWindow(hwnd);
        }
    }
    break;
    case WM_KEYDOWN:
    {
        if (Connected)
        {
            if (edit != 0)
            {
                switch (wParam)
                {
                case VK_BACK:
                    switch (edit)
                    {
                    case 1:
                        if (L1size != 0)
                        {
                            L1text[--L1size] = '\0';
                        }
                        break;
                    case 2:
                        if (L2size != 0)
                        {
                            L2text[--L2size] = '\0';
                        }
                        break;
                    case 3:
                        if (L3size != 0)
                        {
                            L3text[--L3size] = '\0';
                        }
                        break;
                    case 4:
                        if (L4size != 0)
                        {
                            L4text[--L4size] = '\0';
                        }
                        break;
                    case 5:
                        if (L5size != 0)
                        {
                            L5text[--L5size] = '\0';
                        }
                        break;
                    case 6:
                        if (L6size != 0)
                        {
                            L6text[--L6size] = '\0';
                        }
                        break;
                    case 7:
                        if (L7size != 0)
                        {
                            L7text[--L7size] = '\0';
                        }
                        break;
                    case 8:
                        if (L8size != 0)
                        {
                            L8text[--L8size] = '\0';
                        }
                        break;
                    case 9:
                        if (L9size != 0)
                        {
                            L9text[--L9size] = '\0';
                        }
                        break;
                    case 10:
                        if (L10size != 0)
                        {
                            L10text[--L10size] = '\0';
                        }
                        break;
                    }
                    break;
                case VK_CAPITAL:
                    shift ^= 1;
                    break;
                case VK_CONTROL:
                    break;
                case VK_SHIFT:
                    shift ^= 1;
                    break;
                case VK_RETURN:
                    edit = 0;
                    break;
                default:
                    switch (edit)
                    {
                    case 1:
                        if (L1size < maxText)
                        {
                            char c = MapVirtualKeyA((UINT)wParam, MAPVK_VK_TO_CHAR);
                            //OutputDebugString(std::format(L"converted: {}\r\n", c).c_str());
                            L1text[L1size] = shift ? c : tolower(c);
                            L1size++;
                            break;
                        }
                        else
                        {
                            return DefWindowProcA(hWnd, uMsg, wParam, lParam);
                        }
                    case 2:
                        if (L2size < maxText)
                        {
                            char c = MapVirtualKeyA((UINT)wParam, MAPVK_VK_TO_CHAR);
                            //OutputDebugString(std::format(L"converted: {}\r\n", c).c_str());
                            L2text[L2size] = shift ? c : tolower(c);
                            L2size++;
                            break;
                        }
                        else
                        {
                            return DefWindowProcA(hWnd, uMsg, wParam, lParam);
                        }
                    case 3:
                        if (L3size < maxText)
                        {
                            char c = MapVirtualKeyA((UINT)wParam, MAPVK_VK_TO_CHAR);
                            //OutputDebugString(std::format(L"converted: {}\r\n", c).c_str());
                            L3text[L3size] = shift ? c : tolower(c);
                            L3size++;
                            break;
                        }
                        else
                        {
                            return DefWindowProcA(hWnd, uMsg, wParam, lParam);
                        }
                    case 4:
                        if (L4size < maxText)
                        {
                            char c = MapVirtualKeyA((UINT)wParam, MAPVK_VK_TO_CHAR);
                            //OutputDebugString(std::format(L"converted: {}\r\n", c).c_str());
                            L4text[L4size] = shift ? c : tolower(c);
                            L4size++;
                            break;
                        }
                        else
                        {
                            return DefWindowProcA(hWnd, uMsg, wParam, lParam);
                        }
                    case 5:
                        if (L5size < maxText)
                        {
                            char c = MapVirtualKeyA((UINT)wParam, MAPVK_VK_TO_CHAR);
                            //OutputDebugString(std::format(L"converted: {}\r\n", c).c_str());
                            L5text[L5size] = shift ? c : tolower(c);
                            L5size++;
                            break;
                        }
                        else
                        {
                            return DefWindowProcA(hWnd, uMsg, wParam, lParam);
                        }
                    case 6:
                        if (L6size < maxText)
                        {
                            char c = MapVirtualKeyA((UINT)wParam, MAPVK_VK_TO_CHAR);
                            //OutputDebugString(std::format(L"converted: {}\r\n", c).c_str());
                            L6text[L6size] = shift ? c : tolower(c);
                            L6size++;
                            break;
                        }
                        else
                        {
                            return DefWindowProcA(hWnd, uMsg, wParam, lParam);
                        }
                    case 7:
                        if (L7size < maxText)
                        {
                            char c = MapVirtualKeyA((UINT)wParam, MAPVK_VK_TO_CHAR);
                            //OutputDebugString(std::format(L"converted: {}\r\n", c).c_str());
                            L7text[L7size] = shift ? c : tolower(c);
                            L7size++;
                            break;
                        }
                        else
                        {
                            return DefWindowProcA(hWnd, uMsg, wParam, lParam);
                        }
                    case 8:
                        if (L8size < maxText)
                        {
                            char c = MapVirtualKeyA((UINT)wParam, MAPVK_VK_TO_CHAR);
                            //OutputDebugString(std::format(L"converted: {}\r\n", c).c_str());
                            L8text[L8size] = shift ? c : tolower(c);
                            L8size++;
                            break;
                        }
                        else
                        {
                            return DefWindowProcA(hWnd, uMsg, wParam, lParam);
                        }
                    case 9:
                        if (L9size < maxText)
                        {
                            char c = MapVirtualKeyA((UINT)wParam, MAPVK_VK_TO_CHAR);
                            //OutputDebugString(std::format(L"converted: {}\r\n", c).c_str());
                            L9text[L9size] = shift ? c : tolower(c);
                            L9size++;
                            break;
                        }
                        else
                        {
                            return DefWindowProcA(hWnd, uMsg, wParam, lParam);
                        }
                    case 10:
                        if (L10size < maxText)
                        {
                            char c = MapVirtualKeyA((UINT)wParam, MAPVK_VK_TO_CHAR);
                            //OutputDebugString(std::format(L"converted: {}\r\n", c).c_str());
                            L10text[L10size] = shift ? c : tolower(c);
                            L10size++;
                            break;
                        }
                        else
                        {
                            return DefWindowProcA(hWnd, uMsg, wParam, lParam);
                        }
                    }
                    break;
                }
                InvalidateRect(hwnd, NULL, FALSE);
                UpdateWindow(hwnd);
            }
            else if (ButtonsActive[32] == 1 && keysDown[(unsigned long)wParam] == FALSE)
            {
                int n = 0;
                switch (wParam)
                {
                case '1':
                    ButtonsActive[(n = 1) - 1] = TRUE;
                    break;
                case '2':
                    ButtonsActive[(n = 2) - 1] = TRUE;
                    break;
                case '3':
                    ButtonsActive[(n = 3) - 1] = TRUE;
                    break;
                case '4':
                    ButtonsActive[(n = 4) - 1] = TRUE;
                    break;
                case '5':
                    ButtonsActive[(n = 5) - 1] = TRUE;
                    break;
                case '6':
                    ButtonsActive[(n = 6) - 1] = TRUE;
                    break;
                case '7':
                    ButtonsActive[(n = 7) - 1] = TRUE;
                    break;
                case '8':
                    ButtonsActive[(n = 8) - 1] = TRUE;
                    break;
                case '9':
                    ButtonsActive[(n = 9) - 1] = TRUE;
                    break;
                case '0':
                    ButtonsActive[(n = 10) - 1] = TRUE;
                    break;
                case 'Q':
                    ButtonsActive[(n = 11) - 1] = TRUE;
                    break;
                case 'W':
                    ButtonsActive[(n = 12) - 1] = TRUE;
                    break;
                case 'E':
                    ButtonsActive[(n = 13) - 1] = TRUE;
                    break;
                case 'R':
                    ButtonsActive[(n = 14) - 1] = TRUE;
                    break;
                case 'T':
                    ButtonsActive[(n = 15) - 1] = TRUE;
                    break;
                case 'Y':
                    ButtonsActive[(n = 16) - 1] = TRUE;
                    break;
                case 'U':
                    ButtonsActive[(n = 17) - 1] = TRUE;
                    break;
                case 'I':
                    ButtonsActive[(n = 18) - 1] = TRUE;
                    break;
                case 'O':
                    ButtonsActive[(n = 19) - 1] = TRUE;
                    break;
                case 'P':
                    ButtonsActive[(n = 20) - 1] = TRUE;
                    break;
                case 'A':
                    ButtonsActive[(n = 21) - 1] = TRUE;
                    break;
                case 'S':
                    ButtonsActive[(n = 22) - 1] = TRUE;
                    break;
                case 'D':
                    ButtonsActive[(n = 23) - 1] = TRUE;
                    break;
                case 'F':
                    ButtonsActive[(n = 24) - 1] = TRUE;
                    break;
                case 'G':
                    ButtonsActive[(n = 25) - 1] = TRUE;
                    break;
                case 'H':
                    ButtonsActive[(n = 26) - 1] = TRUE;
                    break;
                case 'J':
                    ButtonsActive[(n = 27) - 1] = TRUE;
                    break;
                case 'K':
                    ButtonsActive[(n = 28) - 1] = TRUE;
                    break;
                case 'L':
                    ButtonsActive[(n = 29) - 1] = TRUE;
                    break;
                case VK_OEM_1:
                    ButtonsActive[(n = 30) - 1] = TRUE;
                    break;
                case VK_SPACE:
                    ButtonsActive[(n = 31) - 1] = (SpecialDoor = !SpecialDoor);
                    break;
                case VK_OEM_PERIOD: // 190
                    ButtonsActive[(n = 32) - 1] = TRUE;
                    break;
                default:
                    return DefWindowProc(hwnd, uMsg, wParam, lParam);
                }
                OutputDebugStringA("going into ActiveMonster (KEYDOWN)\n");
                ActiveMonster(n, n == 31 ? SpecialDoor : 1);
                OutputDebugStringA("done with ActiveMonster (KEYDOWN)\n");
                keysDown[(unsigned long)wParam] = TRUE;
                vkUpdate = wParam;
                InvalidateRect(hwnd, NULL, FALSE);
                UpdateWindow(hwnd);
            }
        }
        else if (wParam == 'R')
        {
            PortThread = std::thread([]
                {
                    CSTC = TRUE;
                    _memccpy(ComSerText, "Connecting to Control Box...", 0, strlen("Connecting to Control Box..."));
                    //ComSerText = "Connecting to Control Box...";
                    InvalidateRect(hWnd, &CSTR, TRUE);
                    UpdateWindow(hWnd);
                    char v[0xF]{};
                    FindPort(v);
                    if (strcmp(v, "") == 0)
                    {
                        CSTC = TRUE;
                        _memccpy(ComSerText, "The Control Box is not connected.  Press R to retry.", 0, strlen("The Control Box is not connected.  Press R to retry."));
                        //ComSerText = "The Control Box is not connected.  Press R to retry.";
                        InvalidateRect(hWnd, &CSTR, TRUE);
                        UpdateWindow(hWnd);
                    }
                    else
                    {
                        ConnectToPort(v);
                    }
                });
            PortThread.detach();
        }
    }
    break;
    case WM_KEYUP:
    {
        if (Connected)
        {
            if (edit != 0)
            {
                switch (wParam)
                {
                case VK_SHIFT:
                    shift ^= 1;
                    break;
                }
            }
            else if (ButtonsActive[32] == 1)
            {
                int n = 0;
                switch (wParam)
                {
                case '1':
                    ButtonsActive[(n = 1) - 1] = FALSE;
                    break;
                case '2':
                    ButtonsActive[(n = 2) - 1] = FALSE;
                    break;
                case '3':
                    ButtonsActive[(n = 3) - 1] = FALSE;
                    break;
                case '4':
                    ButtonsActive[(n = 4) - 1] = FALSE;
                    break;
                case '5':
                    ButtonsActive[(n = 5) - 1] = FALSE;
                    break;
                case '6':
                    ButtonsActive[(n = 6) - 1] = FALSE;
                    break;
                case '7':
                    ButtonsActive[(n = 7) - 1] = FALSE;
                    break;
                case '8':
                    ButtonsActive[(n = 8) - 1] = FALSE;
                    break;
                case '9':
                    ButtonsActive[(n = 9) - 1] = FALSE;
                    break;
                case '0':
                    ButtonsActive[(n = 10) - 1] = FALSE;
                    break;
                case 'Q':
                    ButtonsActive[(n = 11) - 1] = FALSE;
                    break;
                case 'W':
                    ButtonsActive[(n = 12) - 1] = FALSE;
                    break;
                case 'E':
                    ButtonsActive[(n = 13) - 1] = FALSE;
                    break;
                case 'R':
                    ButtonsActive[(n = 14) - 1] = FALSE;
                    break;
                case 'T':
                    ButtonsActive[(n = 15) - 1] = FALSE;
                    break;
                case 'Y':
                    ButtonsActive[(n = 16) - 1] = FALSE;
                    break;
                case 'U':
                    ButtonsActive[(n = 17) - 1] = FALSE;
                    break;
                case 'I':
                    ButtonsActive[(n = 18) - 1] = FALSE;
                    break;
                case 'O':
                    ButtonsActive[(n = 19) - 1] = FALSE;
                    break;
                case 'P':
                    ButtonsActive[(n = 20) - 1] = FALSE;
                    break;
                case 'A':
                    ButtonsActive[(n = 21) - 1] = FALSE;
                    break;
                case 'S':
                    ButtonsActive[(n = 22) - 1] = FALSE;
                    break;
                case 'D':
                    ButtonsActive[(n = 23) - 1] = FALSE;
                    break;
                case 'F':
                    ButtonsActive[(n = 24) - 1] = FALSE;
                    break;
                case 'G':
                    ButtonsActive[(n = 25) - 1] = FALSE;
                    break;
                case 'H':
                    ButtonsActive[(n = 26) - 1] = FALSE;
                    break;
                case 'J':
                    ButtonsActive[(n = 27) - 1] = FALSE;
                    break;
                case 'K':
                    ButtonsActive[(n = 28) - 1] = FALSE;
                    break;
                case 'L':
                    ButtonsActive[(n = 29) - 1] = FALSE;
                    break;
                case VK_OEM_1:
                    ButtonsActive[(n = 30) - 1] = FALSE;
                    break;
                case VK_SPACE:
                    keysDown[(unsigned long)wParam] = FALSE;
                    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
                case VK_OEM_PERIOD: // 190
                    ButtonsActive[(n = 32) - 1] = FALSE;
                    break;
                default:
                    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
                }
                OutputDebugStringA("going into ActiveMonster (KEYUP)\n");
                ActiveMonster(n, 0);
                OutputDebugStringA("done with ActiveMonster (KEYUP)\n");
                keysDown[(unsigned long)wParam] = FALSE;
                vkUpdate = wParam;
                InvalidateRect(hwnd, NULL, FALSE);
                UpdateWindow(hwnd);
            }
        }
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        CycleFont(35, hdc);
        CalculationsAndDrawings(hdc);
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
    {
        SerialComWrite("RESET\n");
        PostQuitMessage(0);
    }
    return 0;
    }
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);

    if (lpCmdLine == L"FIREWALLALLOW")
    {
        return 0;
    }

    HInstance = hInstance;

    asas = StartThread;

    GetObjectA(Font, sizeof(LOGFONT), &LogFont);

    shift = GetKeyState(VK_CAPITAL);

    LoadStringA(NULL, CLASS_NAME, wClass, 100);
    LoadStringA(NULL, IDS_TITLE, wTitle, 100);

    WNDCLASSEXA wndClass = {};

    wndClass.cbSize = sizeof(WNDCLASSEXA);

    wndClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wndClass.lpfnWndProc = WindowProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = hInstance;
    wndClass.hIcon = LoadIconA(hInstance, MAKEINTRESOURCEA(IDI_ICONJJ));
    //wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = CreateSolidBrush(BlackColor);// BB GG RR
    wndClass.lpszMenuName = MAKEINTRESOURCEA(IDC_WINDOWCONTROL);
    wndClass.lpszClassName = wClass;

    RegisterClassExA(&wndClass);

    DWORD style = WS_OVERLAPPEDWINDOW;

    int width = 2000;
    int height = 1400;

    RECT rect{};
    rect.left = 250;
    rect.top = 250;
    rect.right = rect.left + width;
    rect.bottom = rect.top + height;

    //AdjustWindowRect(&rect, style, true);

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);

    hWnd = CreateWindowExA(
        0,
        wClass,
        wTitle,
        style,
        rect.left,
        rect.top,
        rect.right - rect.left,
        rect.bottom - rect.top,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    BOOL True = TRUE;
    DwmSetWindowAttribute(hWnd, DWMWA_BORDER_COLOR, &DarkRedColor, 4);
    if (DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE - 1, &True, 4) == 0)
    {
        DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &True, 4);
    }

    PortThread = std::thread([]
        {
            _memccpy(ComSerText, "Connecting to Control Box...", 0, strlen("Connecting to Control Box..."));
            //ComSerText = "Connecting to Control Box...";
            char v[0xF]{};
            FindPort(v);
            if (strcmp(v, "") == 0)
            {
                CSTC = TRUE;
                _memccpy(ComSerText, "The Control Box is not connected.  Press R to retry.", 0, strlen("The Control Box is not connected.  Press R to retry."));
                //ComSerText = "The Control Box is not connected.  Press R to retry.";
                InvalidateRect(hWnd, &CSTR, TRUE);
                UpdateWindow(hWnd);
            }
            else
            {
                ConnectToPort(v);
            }
        });
    PortThread.detach();

    ShowWindow(hWnd, SW_SHOW);

    //UpdateWindow(hWnd);
    MSG msg = {};
    while (GetMessageA(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return 0;
}