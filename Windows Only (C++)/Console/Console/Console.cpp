// Console.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <iostream>
#include <thread>
#include <string>

using namespace std;

OVERLAPPED olw{};

int main()
{
    HANDLE serial = CreateFile(TEXT("COM5"), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
    if (serial == INVALID_HANDLE_VALUE)
    {
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

    auto rthread = thread([serial]
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
                    cout << "broken" << endl;
                    break;
                }
            }
        });

    auto M_ACT = "\033[38;2;0;0;0;48;2;255;0;0m"s;
    auto M_END = "\033[38;2;255;0;0;49m"s;

    int data[32]{};

    auto clearVisual = []
        {
            cout << "\033[121D\033[11A" << flush;

            for (int i = 0; i < 11; i++)
            {
                cout << "\033[2K" << endl;
            }

            cout << "\033[121D\033[11A" << flush;
        };

    auto updateVisual = [](bool update, int d[32])
        {
            auto M_ACT = "\033[38;2;0;0;0;48;2;255;0;0m";
            auto M_END = "\033[38;2;255;0;0;49m";

            auto P = "  Power  "s;
            auto A = "Activate "s;
            auto X = "Alt. Act."s;

            if (update)
            {
                cout << "\033[121D\033[11A" << flush;
            }

            cout << "=========================================================================================================================" << endl;
            cout << "= Monster 1 = Monster 2 = Monster 3 = Monster 4 = Monster 5 = Monster 6 = Monster 7 = Monster 8 = Monster 9 = Monster10 =" << endl;
            cout << "=========================================================================================================================" << endl;
            cout << "= " << (d[0] == 1 ? M_ACT + P + M_END : P) << " = " << (d[1] == 1 ? M_ACT + P + M_END : P)
                << " = " << (d[2] == 1 ? M_ACT + P + M_END : P) << " = " << (d[3] == 1 ? M_ACT + P + M_END : P)
                << " = " << (d[4] == 1 ? M_ACT + P + M_END : P) << " = " << (d[5] == 1 ? M_ACT + P + M_END : P) 
                << " = " << (d[6] == 1 ? M_ACT + P + M_END : P) << " = " << (d[7] == 1 ? M_ACT + P + M_END : P) 
                << " = " << (d[8] == 1 ? M_ACT + P + M_END : P) << " = " << (d[9] == 1 ? M_ACT + P + M_END : P) << " =" << endl;
            cout << "=========================================================================================================================" << endl;
            cout << "= " << (d[10] == 1 ? M_ACT + A + M_END : A) << " = " << (d[11] == 1 ? M_ACT + A + M_END : A)
                << " = " << (d[12] == 1 ? M_ACT + A + M_END : A) << " = " << (d[13] == 1 ? M_ACT + A + M_END : A)
                << " = " << (d[14] == 1 ? M_ACT + A + M_END : A) << " = " << (d[15] == 1 ? M_ACT + A + M_END : A)
                << " = " << (d[16] == 1 ? M_ACT + A + M_END : A) << " = " << (d[17] == 1 ? M_ACT + A + M_END : A)
                << " = " << (d[18] == 1 ? M_ACT + A + M_END : A) << " = " << (d[19] == 1 ? M_ACT + A + M_END : A) << " =" << endl;
            cout << "=========================================================================================================================" << endl;
            cout << "= " << (d[20] == 1 ? M_ACT + X + M_END : X) << " = " << (d[21] == 1 ? M_ACT + X + M_END : X)
                << " = " << (d[22] == 1 ? M_ACT + X + M_END : X) << " = " << (d[23] == 1 ? M_ACT + X + M_END : X)
                << " = " << (d[24] == 1 ? M_ACT + X + M_END : X) << " = " << (d[25] == 1 ? M_ACT + X + M_END : X)
                << " = " << (d[26] == 1 ? M_ACT + X + M_END : X) << " = " << (d[27] == 1 ? M_ACT + X + M_END : X)
                << " = " << (d[28] == 1 ? M_ACT + X + M_END : X) << " = " << (d[29] == 1 ? M_ACT + X + M_END : X) << " =" << endl;
            cout << "=========================================================================================================================" << endl;
            cout << "                                                                                                "
                << "= " << (d[30] == 1 ? M_ACT + "  Door   "s + M_END : "  Door   ") << " = " << (d[31] == 1 ? M_ACT + " Garbage "s + M_END : " Garbage ") << " =" << endl;
            cout << "=========================================================================================================================" << endl;
        };

    HANDLE hIn;
    HANDLE hOut;
    bool Continue = TRUE;
    INPUT_RECORD InRec;
    DWORD NumRead;

    cout << "WARNING: The layout should look normal, like a rectangle box.  If it looks wonky, the program will look broken, so resize the console to fix it." << endl;
    cout << "JJ's Monster Controller using Visual C++ for Windows" << endl;
    cout << "For help, press escape and type 'help'" << endl;

    cout << "\033[38;2;255;0;0m" << flush;

    updateVisual(false, data);

    hIn = GetStdHandle(STD_INPUT_HANDLE);
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);

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

    int i = 0;

    while (Continue)
    {
        ReadConsoleInput(hIn, &InRec, 1, &NumRead);

        if (InRec.EventType == KEY_EVENT)
        {
            i++;

            OutputDebugStringA(std::to_string(i).c_str());
            OutputDebugStringA(" ");

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
}
