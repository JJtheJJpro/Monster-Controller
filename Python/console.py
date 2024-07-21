import os
import serial
import serial.tools
import serial.tools.list_ports
import threading
from pynput.keyboard import Key, Listener, KeyCode

M_ACT = '\033[38;2;0;0;0;48;2;255;0;0m'
M_END = '\033[38;2;255;0;0;49m'

print('WARNING: The layout should look normal, like a rectangle box.  If it looks wonky, the program will look broken, so resize the console to fix it.')
print("JJ's Monster Controller using Python")
print("For help, press escape and type 'help'")

print('\033[38;2;255;0;0m', end='')

ports = serial.tools.list_ports.comports()
port_name = ''
for port in ports:
    if port.vid == 0x2341 and port.pid == 0x0042:
        port_name = port.device
        print('Arduino Found.  Connecting...', end='')

if port_name == '':
    print('Arduino not found.')
    print('Press Enter to continue...', end='')
    input()
    exit()

ser = serial.Serial(port_name)
lkeys = []

monster_data = [
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0
]

def clear_visual():
    print('\033[121D\033[11A', end='')

    for _ in range(11):
        print('\033[2K')

    print('\033[121D\033[11A', end='')

def update_visual(update: bool = False):
    global monster_data
    
    P = "  Power  "
    A = "Activate "
    X = "Alt. Act."

    if update:
        print('\033[121D\033[11A', end='')
    
    print(f'█████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████')
    print(f'█ Monster 1 █ Monster 2 █ Monster 3 █ Monster 4 █ Monster 5 █ Monster 6 █ Monster 7 █ Monster 8 █ Monster 9 █ Monster10 █')
    print(f'█████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████')
    print(f'█ {f"{M_ACT}{P}{M_END}" if monster_data[0] == 1 else P} █ {f"{M_ACT}{P}{M_END}" if monster_data[1] == 1 else P} ' + \
          f'█ {f"{M_ACT}{P}{M_END}" if monster_data[2] == 1 else P} █ {f"{M_ACT}{P}{M_END}" if monster_data[3] == 1 else P} ' + \
          f'█ {f"{M_ACT}{P}{M_END}" if monster_data[4] == 1 else P} █ {f"{M_ACT}{P}{M_END}" if monster_data[5] == 1 else P} ' + \
          f'█ {f"{M_ACT}{P}{M_END}" if monster_data[6] == 1 else P} █ {f"{M_ACT}{P}{M_END}" if monster_data[7] == 1 else P} ' + \
          f'█ {f"{M_ACT}{P}{M_END}" if monster_data[8] == 1 else P} █ {f"{M_ACT}{P}{M_END}" if monster_data[9] == 1 else P} █')
    print(f'█████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████')
    print(f'█ {f"{M_ACT}{A}{M_END}" if monster_data[10] == 1 else A} █ {f"{M_ACT}{A}{M_END}" if monster_data[11] == 1 else A} ' + \
          f'█ {f"{M_ACT}{A}{M_END}" if monster_data[12] == 1 else A} █ {f"{M_ACT}{A}{M_END}" if monster_data[13] == 1 else A} ' + \
          f'█ {f"{M_ACT}{A}{M_END}" if monster_data[14] == 1 else A} █ {f"{M_ACT}{A}{M_END}" if monster_data[15] == 1 else A} ' + \
          f'█ {f"{M_ACT}{A}{M_END}" if monster_data[16] == 1 else A} █ {f"{M_ACT}{A}{M_END}" if monster_data[17] == 1 else A} ' + \
          f'█ {f"{M_ACT}{A}{M_END}" if monster_data[18] == 1 else A} █ {f"{M_ACT}{A}{M_END}" if monster_data[19] == 1 else A} █')
    print(f'█████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████')
    print(f'█ {f"{M_ACT}{X}{M_END}" if monster_data[20] == 1 else X} █ {f"{M_ACT}{X}{M_END}" if monster_data[21] == 1 else X} ' + \
          f'█ {f"{M_ACT}{X}{M_END}" if monster_data[22] == 1 else X} █ {f"{M_ACT}{X}{M_END}" if monster_data[23] == 1 else X} ' + \
          f'█ {f"{M_ACT}{X}{M_END}" if monster_data[24] == 1 else X} █ {f"{M_ACT}{X}{M_END}" if monster_data[25] == 1 else X} ' + \
          f'█ {f"{M_ACT}{X}{M_END}" if monster_data[26] == 1 else X} █ {f"{M_ACT}{X}{M_END}" if monster_data[27] == 1 else X} ' + \
          f'█ {f"{M_ACT}{X}{M_END}" if monster_data[28] == 1 else X} █ {f"{M_ACT}{X}{M_END}" if monster_data[29] == 1 else X} █')
    print(f'█████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████')
    print(f'█                                                                                               ' + \
          f'█ {f"{M_ACT}  Door   {M_END}" if monster_data[30] == 1 else "  Door   "} █ {f"{M_ACT} Garbage {M_END}" if monster_data[31] == 1 else " Garbage "} █')
    print(f'█████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████')

def parse_monster(on: bool, type: int, monster: int):
    global monster_data
    global ser
    assert type == 0 or type == 1 or type == 2
    assert (monster >= 0 and monster <= 9) or ((monster == 30 or monster == 31) and type == 0)
    
    if monster == 30 or monster == 31:
        monster_data[monster] = 1 if on else 0
    else:
        monster += type * 10
        monster_data[monster] = 1 if on else 0
    monster += 1
    m = f'{1 if on else 0}{monster.__str__() if monster > 9 else f"0{monster.__str__()}"}\n'
    ser.write(m.encode())
    update_visual(True)

def read_port():
    global ser
    while ser.is_open:
        try:
            data = ser.readline().decode().removesuffix('\r\n')
            match data:
                case "Ready":
                    print('Connected!')
                    update_visual()
                case _:
                    pass #print(data)
        except:
            break

def on_press(key: Key | KeyCode):
    if not lkeys.__contains__(key):
        lkeys.append(key)
        if type(key) is KeyCode:
            match key.char:
                case '1':
                    parse_monster(True, 0, 0)
                case '2':
                    parse_monster(True, 0, 1)
                case '3':
                    parse_monster(True, 0, 2)
                case '4':
                    parse_monster(True, 0, 3)
                case '5':
                    parse_monster(True, 0, 4)
                case '6':
                    parse_monster(True, 0, 5)
                case '7':
                    parse_monster(True, 0, 6)
                case '8':
                    parse_monster(True, 0, 7)
                case '9':
                    parse_monster(True, 0, 8)
                case '0':
                    parse_monster(True, 0, 9)
                case 'q':
                    parse_monster(True, 1, 0)
                case 'w':
                    parse_monster(True, 1, 1)
                case 'e':
                    parse_monster(True, 1, 2)
                case 'r':
                    parse_monster(True, 1, 3)
                case 't':
                    parse_monster(True, 1, 4)
                case 'y':
                    parse_monster(True, 1, 5)
                case 'u':
                    parse_monster(True, 1, 6)
                case 'i':
                    parse_monster(True, 1, 7)
                case 'o':
                    parse_monster(True, 1, 8)
                case 'p':
                    parse_monster(True, 1, 9)
                case 'a':
                    parse_monster(True, 2, 0)
                case 's':
                    parse_monster(True, 2, 1)
                case 'd':
                    parse_monster(True, 2, 2)
                case 'f':
                    parse_monster(True, 2, 3)
                case 'g':
                    parse_monster(True, 2, 4)
                case 'h':
                    parse_monster(True, 2, 5)
                case 'j':
                    parse_monster(True, 2, 6)
                case 'k':
                    parse_monster(True, 2, 7)
                case 'l':
                    parse_monster(True, 2, 8)
                case ';':
                    parse_monster(True, 2, 9)
                case '.':
                    parse_monster(True, 0, 31)
        elif type(key) is Key:
            if key == Key.space:
                parse_monster(not monster_data[30], 0, 30)

def on_release(key: Key | KeyCode):
    global ready_cmd
    
    if lkeys.__contains__(key):
        lkeys.remove(key)
        if type(key) is KeyCode:
            match key.char:
                case '1':
                    parse_monster(False, 0, 0)
                case '2':
                    parse_monster(False, 0, 1)
                case '3':
                    parse_monster(False, 0, 2)
                case '4':
                    parse_monster(False, 0, 3)
                case '5':
                    parse_monster(False, 0, 4)
                case '6':
                    parse_monster(False, 0, 5)
                case '7':
                    parse_monster(False, 0, 6)
                case '8':
                    parse_monster(False, 0, 7)
                case '9':
                    parse_monster(False, 0, 8)
                case '0':
                    parse_monster(False, 0, 9)
                case 'q':
                    parse_monster(False, 1, 0)
                case 'w':
                    parse_monster(False, 1, 1)
                case 'e':
                    parse_monster(False, 1, 2)
                case 'r':
                    parse_monster(False, 1, 3)
                case 't':
                    parse_monster(False, 1, 4)
                case 'y':
                    parse_monster(False, 1, 5)
                case 'u':
                    parse_monster(False, 1, 6)
                case 'i':
                    parse_monster(False, 1, 7)
                case 'o':
                    parse_monster(False, 1, 8)
                case 'p':
                    parse_monster(False, 1, 9)
                case 'a':
                    parse_monster(False, 2, 0)
                case 's':
                    parse_monster(False, 2, 1)
                case 'd':
                    parse_monster(False, 2, 2)
                case 'f':
                    parse_monster(False, 2, 3)
                case 'g':
                    parse_monster(False, 2, 4)
                case 'h':
                    parse_monster(False, 2, 5)
                case 'j':
                    parse_monster(False, 2, 6)
                case 'k':
                    parse_monster(False, 2, 7)
                case 'l':
                    parse_monster(False, 2, 8)
                case ';':
                    parse_monster(False, 2, 9)
                case '.':
                    parse_monster(False, 0, 31)
        elif type(key) is Key:
            if key == Key.esc:
                return False

r_thread = threading.Thread(target=read_port)
r_thread.start()

while True:
    with Listener(on_press=on_press, on_release=on_release) as listener:
        listener.join()
    clear_visual()
    print('> ', end='')
    read = input()
    if read == "clear":
        # actually call the os version
        os.system('cls' if os.name == 'nt' else 'clear')
    elif read == "exit":
        break
    elif read == "help":
        print('To get to the command line again, press escape again')
        print()
        print('exit:           exits this program')
        print('help:           prints this list of commands')
        print('clear:          clears the console')
        print('arduino <data>: sends <data> to the arduino')
    elif read == "arduino":
        print("command requires 1 parameter <data>: 'arduino <data>'")
    elif read.startswith('arduino '):
        ser.write(f'{read.removeprefix("arduino ")}\n'.encode())
    else:
        print('Unknown command')
    update_visual()
            
ser.write('RESET\n'.encode())
ser.close()
print('\033[0m', end='')
print('exiting...')
