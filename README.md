# The Monster Controller

This is a number of programs, made in different languages, that have the same purpose: to control the monsters.

## Programming Languages

To keep the programs cross-platform, I've been able to use a differnt number of languages:

- C# .NET MAUI (v0.0.5-alpha)
- Tauri v2-beta (v0.1-beta)
- Python (console version)
- Node.js (console version)

The Arduino code is found in the ArduinoSoftware folder.

## For Windows Only

I've made a C++ program only suited for Windows platforms (both console edition and UI).  UI is currently buggy, and console is a working console version.

# The WIP

Here I will show the progress I've made and the progress I want to make with each program:

## C#

Using .NET MAUI Blazor (C# version of React.js basically).

To Do:

- Create Cross-Platform Serial Port code for easy control
- Fix up the UI

## Tauri

Currently using Tauri v2-Beta.

To Do:

- Update Tauri to v2.0-rc
- Setup a functioning Tcp Listener for local network communication
- Give error info in a user-friendly way

## Python and Node.js

Since Python and Node.js are both scripting languages, I'm not gonna put myself through UI fancy things, at least not right now.
For now, Python and Node.js are easy console apps (desktop only) that only have keyboard input support.
Since I'm putting most my efforts in C# and Tauri, there is no current To Do list for these two yet.

## C++ (Windows Only)

C# WinForms was the first coding language I used to make the program (not seen here).  C++ WinForms was the second (seen here).
If you use Windows, I recommend using either of these two programs over cross-platform programs since these call Windows APIs faster and more directly.

**DEPRECATED:** This does use a lot of raw stuff, like the current code doesn't use built-in WinForms controls (buttons, labels, etc.), which is why I have marked this deprecated for now.

To Do:

- Relearn C++ WinForms and Restart from scratch.

## Arduion Software

For right now, it takes simple communication and sends data.

To Do:

- More direct communication (like the program with send data, arduino will send data and the program will respond in someway instead of just printing the data)