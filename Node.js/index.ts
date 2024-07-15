import assert from "assert";
import EventEmitter from "events";
import {
    GlobalKeyboardListener,
    IGlobalKey,
    IGlobalKeyDownMap,
    IGlobalKeyEvent
    } from "node-global-key-listener";
import { SerialPort } from "serialport";
//import WebSocket from "ws";
//import mqtt from "async-mqtt";

/*

(async () => {
    //const wss = 'wss://broker.hivemq.com:8884/mqtt';
    //const wss = 'wss://public.mqtthq.com:8084/mqtt';
    const wss = 'wss://8f35b48abd704709b721771de025229e.s1.eu.hivemq.cloud:8884/mqtt';

    //const wssLocal = new WebSocket.Server({ port: 8080 });
    const wssMQTT = mqtt.connect(wss);

    //wssLocal.on('connection', (ws, req) => {
    //    const addr = req.socket.remoteAddress?.replace('::ffff:', '');
    //    let name = '';
    //    console.log(`New Local Client Connected: ${addr}`);
    //    let ips = req.headers['x-forwarded-for'];
    //    if (ips != undefined) {
    //        ips = [...ips];
    //        ips.forEach(ip => {
    //            console.log(`detcennoC tneilC weN: ${ip.split(',')[0].trim()}`);
    //        });
    //    } else {
    //        console.log("detcennoC tneilC weN: undefined");
    //    }
    //    ws.on('message', (message: string) => {
    //        if (message.toString().startsWith("name: ")) {
    //            name = message.at(6)!;
    //        } else {
    //            console.log(`${addr}: ${message}`);
    //        }
    //        //ws.send(`Server received your message: ${message}`);
    //    });
    //    ws.on('close', () => {
    //        console.log(`${addr} disconnected`);
    //    });
    //});

    wssMQTT.on('message', (topic: string, payload: Buffer) => {
        console.log(`message received online: ${payload}`);
    });

    await wssMQTT.subscribe('jj-monster-controller-mega-2341-0042');

    console.log("ready");
})();

*/

(async () => {
    const M_ACT = '\x1b[38;2;0;0;0;48;2;255;0;0m';
    const M_END = '\x1b[38;2;255;0;0;49m';

    const readLine = async () => {
        const errBlock = new EventEmitter();
        let ret = '';
        process.stdin.resume();
        process.stdin.addListener('data', d => {
            ret = d.toString();
            errBlock.emit('read');
        });
        await new Promise(resolve => errBlock.once('read', resolve));
        process.stdin.pause();
        return ret;
    };
    const reset = () => {
        process.stdout.write('\x1b[0m');
    };
    const clearVisual = () => {
        process.stdout.write('\x1b[121D\x1b[11A');

        for (let i = 0; i < 11; i++) {
            console.log('\x1b[2K');
        }

        process.stdout.write('\x1b[121D\x1b[11A');
    };
    const updateVisual = (update = false) => {
        const P = '  Power  ';
        const A = 'Activate ';
        const X = 'Alt. Act.';

        if (update) {
            process.stdout.write('\x1b[121D\x1b[11A');
        }

        console.log('█████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████');
        console.log('█ Monster 1 █ Monster 2 █ Monster 3 █ Monster 4 █ Monster 5 █ Monster 6 █ Monster 7 █ Monster 8 █ Monster 9 █ Monster10 █');
        console.log('█████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████');
        console.log(`█ ${monsterData[0] == 1 ? M_ACT + P + M_END : P} █ ${monsterData[1] == 1 ? M_ACT + P + M_END : P} ` +
                    `█ ${monsterData[2] == 1 ? M_ACT + P + M_END : P} █ ${monsterData[3] == 1 ? M_ACT + P + M_END : P} ` +
                    `█ ${monsterData[4] == 1 ? M_ACT + P + M_END : P} █ ${monsterData[5] == 1 ? M_ACT + P + M_END : P} ` +
                    `█ ${monsterData[6] == 1 ? M_ACT + P + M_END : P} █ ${monsterData[7] == 1 ? M_ACT + P + M_END : P} ` +
                    `█ ${monsterData[8] == 1 ? M_ACT + P + M_END : P} █ ${monsterData[9] == 1 ? M_ACT + P + M_END : P} █`);
        console.log('█████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████');
        console.log(`█ ${monsterData[10] == 1 ? M_ACT + A + M_END : A} █ ${monsterData[11] == 1 ? M_ACT + A + M_END : A} ` +
                    `█ ${monsterData[12] == 1 ? M_ACT + A + M_END : A} █ ${monsterData[13] == 1 ? M_ACT + A + M_END : A} ` +
                    `█ ${monsterData[14] == 1 ? M_ACT + A + M_END : A} █ ${monsterData[15] == 1 ? M_ACT + A + M_END : A} ` +
                    `█ ${monsterData[16] == 1 ? M_ACT + A + M_END : A} █ ${monsterData[17] == 1 ? M_ACT + A + M_END : A} ` +
                    `█ ${monsterData[18] == 1 ? M_ACT + A + M_END : A} █ ${monsterData[19] == 1 ? M_ACT + A + M_END : A} █`);
        console.log('█████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████');
        console.log(`█ ${monsterData[20] == 1 ? M_ACT + X + M_END : X} █ ${monsterData[21] == 1 ? M_ACT + X + M_END : X} ` +
                    `█ ${monsterData[22] == 1 ? M_ACT + X + M_END : X} █ ${monsterData[23] == 1 ? M_ACT + X + M_END : X} ` +
                    `█ ${monsterData[24] == 1 ? M_ACT + X + M_END : X} █ ${monsterData[25] == 1 ? M_ACT + X + M_END : X} ` +
                    `█ ${monsterData[26] == 1 ? M_ACT + X + M_END : X} █ ${monsterData[27] == 1 ? M_ACT + X + M_END : X} ` +
                    `█ ${monsterData[28] == 1 ? M_ACT + X + M_END : X} █ ${monsterData[29] == 1 ? M_ACT + X + M_END : X} █`);
        console.log('█████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████');
        console.log('█                                                                                               ' +
                    `█ ${monsterData[30] == 1 ? M_ACT + '  Door   ' + M_END : '  Door   '} █ ${monsterData[31] ? M_ACT + ' Garbage ' + M_END : ' Garbage '} █`);
        console.log('█████████████████████████████████████████████████████████████████████████████████████████████████████████████████████████');
    };
    const parseMonster = (on: boolean, type: number, monster: number) => {
        assert(type == 0 || type == 1 || type == 2);
        assert((monster >= 0 && monster <= 9) || ((monster == 30 || monster == 31) && type == 0));

        if (monster >= 0 && monster <= 9) {
            monster += type * 10;
        }
        if (monsterData[monster] == 1 && on) {
            return;
        }
        monsterData[monster] = on ? 1 : 0;
        monster++;
        let m = `${on ? 1 : 0}${monster > 9 ? monster : `0${monster}`}\n`;
        ser.write(m);
        updateVisual(true);
    };

    console.log('WARNING: The layout should look normal, like a rectangle box.  If it looks wonky, the program will look broken, so resize the console to fix it.');
    console.log("JJ's Monster Controller using Node.js");
    console.log("For help, press escape and type 'help'");

    process.stdout.write('\x1b[38;2;255;0;0m');

    let monsterData = [
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0
    ];

    let ports = await SerialPort.list();
    let portName = '';

    for (let i = 0; i < ports.length; i++) {
        let port = ports[i];
        if (port.vendorId == '2341' && port.productId == '0042') {
            portName = port.path;
            break;
        }
    }

    if (portName == '') {
        reset();
        console.error('Arduino not found.');
        process.stdout.write('Press Enter to continue...');
        process.stdin.resume();
        process.stdin.addListener('data', () => {
            process.exit();
        });
        return;
    }

    let ser: SerialPort;
    {
        const errBlock = new EventEmitter();
        let err = 0; // apparently tsc is being a butthole and not letting me use boolean.

        ser = new SerialPort({ path: portName, baudRate: 9600 }, e => {
            if (e != null) {
                err = 1;
                console.error(e);
            }
            errBlock.emit('errsig');
        });
        await new Promise(resolve => errBlock.once('errsig', resolve));
        if (err == 1) {
            return;
        }
    }

    let sb = '';
    ser.on('data', d => {
        try {
            let data = d as Buffer | null;
            if (data == null) {
                return;
            }
            sb += data.toString();
            //process.stdout.write(data.toString());
            if (sb.endsWith('\r\n')) {
                switch (sb.replace('\r\n', '')) {
                    case "Ready":
                        console.log('Connected!');
                        updateVisual();
                        break;
                }
                sb = '';
            }
        } catch {
            return;
        }
    });

    //let lkeys = [];

    while (true) {
        const v = new GlobalKeyboardListener();
        const block = new EventEmitter();
        const listener = (e: IGlobalKeyEvent) => {
            switch (e.vKey) {

                case 49: // 1
                    parseMonster(e.state == 'DOWN', 0, 0);
                    break;
                case 50: // 2
                    parseMonster(e.state == 'DOWN', 0, 1);
                    break;
                case 51: // 3
                    parseMonster(e.state == 'DOWN', 0, 2);
                    break;
                case 52: // 4
                    parseMonster(e.state == 'DOWN', 0, 3);
                    break;
                case 53: // 5
                    parseMonster(e.state == 'DOWN', 0, 4);
                    break;
                case 54: // 6
                    parseMonster(e.state == 'DOWN', 0, 5);
                    break;
                case 55: // 7
                    parseMonster(e.state == 'DOWN', 0, 6);
                    break;
                case 56: // 8
                    parseMonster(e.state == 'DOWN', 0, 7);
                    break;
                case 57: // 9
                    parseMonster(e.state == 'DOWN', 0, 8);
                    break;
                case 48: // 0
                    parseMonster(e.state == 'DOWN', 0, 9);
                    break;

                case 81: // q
                    parseMonster(e.state == 'DOWN', 1, 0);
                    break;
                case 87: // w
                    parseMonster(e.state == 'DOWN', 1, 1);
                    break;
                case 69: // e
                    parseMonster(e.state == 'DOWN', 1, 2);
                    break;
                case 82: // r
                    parseMonster(e.state == 'DOWN', 1, 3);
                    break;
                case 84: // t
                    parseMonster(e.state == 'DOWN', 1, 4);
                    break;
                case 89: // y
                    parseMonster(e.state == 'DOWN', 1, 5);
                    break;
                case 85: // u
                    parseMonster(e.state == 'DOWN', 1, 6);
                    break;
                case 73: // i
                    parseMonster(e.state == 'DOWN', 1, 7);
                    break;
                case 79: // o
                    parseMonster(e.state == 'DOWN', 1, 8);
                    break;
                case 80: // p
                    parseMonster(e.state == 'DOWN', 1, 9);
                    break;

                case 65: // a
                    parseMonster(e.state == 'DOWN', 2, 0);
                    break;
                case 83: // s
                    parseMonster(e.state == 'DOWN', 2, 1);
                    break;
                case 68: // d
                    parseMonster(e.state == 'DOWN', 2, 2);
                    break;
                case 70: // f
                    parseMonster(e.state == 'DOWN', 2, 3);
                    break;
                case 71: // g
                    parseMonster(e.state == 'DOWN', 2, 4);
                    break;
                case 72: // h
                    parseMonster(e.state == 'DOWN', 2, 5);
                    break;
                case 74: // j
                    parseMonster(e.state == 'DOWN', 2, 6);
                    break;
                case 75: // k
                    parseMonster(e.state == 'DOWN', 2, 7);
                    break;
                case 76: // l
                    parseMonster(e.state == 'DOWN', 2, 8);
                    break;
                case 186: // ;
                    parseMonster(e.state == 'DOWN', 2, 9);
                    break;

                case 32: // enter (door)
                    if (e.state == 'DOWN') {
                        parseMonster(!monsterData[30], 0, 30);
                    }
                    break;

                case 190: // period (garbage)
                    parseMonster(e.state == 'DOWN', 0, 31);
                    break;

                case 27: // escape
                    block.emit('continue');
                    break;
            }
        };
        v.addListener(listener);
        await new Promise(resolve => block.once('continue', resolve));
        v.removeListener(listener);

        process.stdin.setRawMode(false);
        clearVisual();
        process.stdout.write('> ');
        let input = (await readLine()).trim();

        if (input == "clear") {
            console.clear();
        } else if (input == "exit") {
            break;
        } else if (input == "help") {
            console.log('To get to the command line again, press escape again');
            console.log();
            console.log('exit:           exits this program');
            console.log('help:           prints this list of commands');
            console.log('clear:          clears the console');
            console.log('arduino <data>: sends <data> to the arduino');
        } else if (input == "arduino" || input == "arduino ") {
            console.log("command requires 1 parameter <data>: 'arduino <data>'");
        } else if (input.startsWith('arduino ')) {
            ser.write(`${input.replace('arduino ', '')}\n`);
        } else {
            console.log('Unknown command');
        }
        updateVisual();
    }

    ser.write('RESET\n');
    ser.close();
    process.stdout.write('\x1b[0m');
    console.log('exiting...');
})();
