# The Monster Controller

This is a number of programs, made in different languages, that have the same purpose: to control the monsters.

## Programming Languages

To keep the programs cross-platform, I've been able to use a differnt number of languages:

- C# (WIP, switching to .NET MAUI soon)
- Tauri (v0.1-beta)
- Python (console version)
- Node.js (console version)

The Arduino code is found in the ArduinoSoftware folder.

## For Windows Only

I've made a C++ program only suited for Windows platforms (both console and UI).  UI is currently buggy, and console is a working console version.

# The WIP

Here I will show the progress I've made and the progress I want to make with each program:

## C#

I have stepped down from what I was using (Avalonia).  Since then, I've wanted to switch it to .NET MAUI for a more flexible program, especially to run on mobile products.
The reason why I haven't wiped C# clean is because Avalonia and MAUI use xml for front-end development, and they look similar.

To Do:

- Switch to .NET MAUI

## Tauri

Currently using Tauri v2-Beta.

To Do:

- Setup a functioning Tcp Listener for local network communication
- Fix minor UI bugs
- Give error info in a user-friendly way

## Python and Node.js

Since Python and Node.js are both scripting languages, I'm not gonna put myself through UI fancy things, at least not right now.
For now, Python and Node.js are easy console apps (desktop only) that only have keyboard input support.
Since I'm putting most my efforts in C# and Tauri, there is no current To Do list for these two yet.

## C++ (Windows Only)

C# WinForms was the first coding language I used to make the program (not seen here).  C++ WinForms was the second (seen here).
If you use Windows, I recommend using either of these two programs over cross-platform programs since these call Windows APIs faster and more directly.

To Do:

- Fix string allocation (causing random crashes)
- Implement Web communication for WinForms

## Arduion Software

For right now, it takes simple communication and sends data.

To Do:

- More direct communication (like the program with send data, arduino will send data and the program will respond in someway instead of just printing the data)