#define P1(on) ((on && (PORTA & B00000001) == B00000001) || (!on && (PORTA & B00000001) == 0)) ? PORTA ^= B00000001 : PORTA = PORTA
#define P2(on) ((on && (PORTA & B00000010) == B00000010) || (!on && (PORTA & B00000010) == 0)) ? PORTA ^= B00000010 : PORTA = PORTA
#define P3(on) ((on && (PORTA & B00000100) == B00000100) || (!on && (PORTA & B00000100) == 0)) ? PORTA ^= B00000100 : PORTA = PORTA
#define P4(on) ((on && (PORTA & B00001000) == B00001000) || (!on && (PORTA & B00001000) == 0)) ? PORTA ^= B00001000 : PORTA = PORTA
#define P5(on) ((on && (PORTA & B00010000) == B00010000) || (!on && (PORTA & B00010000) == 0)) ? PORTA ^= B00010000 : PORTA = PORTA
#define P6(on) ((on && (PORTA & B00100000) == B00100000) || (!on && (PORTA & B00100000) == 0)) ? PORTA ^= B00100000 : PORTA = PORTA
#define P7(on) ((on && (PORTA & B01000000) == B01000000) || (!on && (PORTA & B01000000) == 0)) ? PORTA ^= B01000000 : PORTA = PORTA
#define P8(on) ((on && (PORTA & B10000000) == B10000000) || (!on && (PORTA & B10000000) == 0)) ? PORTA ^= B10000000 : PORTA = PORTA
#define P9(on) ((on && (PORTC & B10000000) == B10000000) || (!on && (PORTC & B10000000) == 0)) ? PORTC ^= B10000000 : PORTG = PORTG
#define P10(on) ((on && (PORTC & B01000000) == B01000000) || (!on && (PORTC & B01000000) == 0)) ? PORTC ^= B01000000 : PORTG = PORTG

#define A1(on) ((on && (PORTC & B00100000) == B00100000) || (!on && (PORTC & B00100000) == 0)) ? PORTC ^= B00100000 : PORTC = PORTC
#define A2(on) ((on && (PORTC & B00010000) == B00010000) || (!on && (PORTC & B00010000) == 0)) ? PORTC ^= B00010000 : PORTC = PORTC
#define A3(on) ((on && (PORTC & B00001000) == B00001000) || (!on && (PORTC & B00001000) == 0)) ? PORTC ^= B00001000 : PORTC = PORTC
#define A4(on) ((on && (PORTC & B00000100) == B00000100) || (!on && (PORTC & B00000100) == 0)) ? PORTC ^= B00000100 : PORTC = PORTC
#define A5(on) ((on && (PORTC & B00000010) == B00000010) || (!on && (PORTC & B00000010) == 0)) ? PORTC ^= B00000010 : PORTC = PORTC
#define A6(on) ((on && (PORTC & B00000001) == B00000001) || (!on && (PORTC & B00000001) == 0)) ? PORTC ^= B00000001 : PORTC = PORTC
#define A7(on) ((on && (PORTD & B10000000) == B10000000) || (!on && (PORTD & B10000000) == 0)) ? PORTD ^= B10000000 : PORTD = PORTD
#define A8(on) ((on && (PORTG & B00000100) == B00000100) || (!on && (PORTG & B00000100) == 0)) ? PORTG ^= B00000100 : PORTG = PORTG
#define A9(on) ((on && (PORTG & B00000010) == B00000010) || (!on && (PORTG & B00000010) == 0)) ? PORTG ^= B00000010 : PORTG = PORTG
#define A10(on) ((on && (PORTG & B00000001) == B00000001) || (!on && (PORTG & B00000001) == 0)) ? PORTG ^= B00000001 : PORTG = PORTG

#define X1(on) ((on && (PORTL & B10000000) == B10000000) || (!on && (PORTL & B10000000) == 0)) ? PORTL ^= B10000000 : PORTL = PORTL
#define X2(on) ((on && (PORTL & B01000000) == B01000000) || (!on && (PORTL & B01000000) == 0)) ? PORTL ^= B01000000 : PORTL = PORTL
#define X3(on) ((on && (PORTL & B00100000) == B00100000) || (!on && (PORTL & B00100000) == 0)) ? PORTL ^= B00100000 : PORTL = PORTL
#define X4(on) ((on && (PORTL & B00010000) == B00010000) || (!on && (PORTL & B00010000) == 0)) ? PORTL ^= B00010000 : PORTL = PORTL
#define X5(on) ((on && (PORTL & B00001000) == B00001000) || (!on && (PORTL & B00001000) == 0)) ? PORTL ^= B00001000 : PORTL = PORTL
#define X6(on) ((on && (PORTL & B00000100) == B00000100) || (!on && (PORTL & B00000100) == 0)) ? PORTL ^= B00000100 : PORTL = PORTL
#define X7(on) ((on && (PORTL & B00000010) == B00000010) || (!on && (PORTL & B00000010) == 0)) ? PORTL ^= B00000010 : PORTL = PORTL
#define X8(on) ((on && (PORTL & B00000001) == B00000001) || (!on && (PORTL & B00000001) == 0)) ? PORTL ^= B00000001 : PORTL = PORTL
#define X9(on) ((on && (PORTB & B00001000) == B00001000) || (!on && (PORTB & B00001000) == 0)) ? PORTB ^= B00001000 : PORTB = PORTB
#define X10(on) ((on && (PORTB & B00000100) == B00000100) || (!on && (PORTB & B00000100) == 0)) ? PORTB ^= B00000100 : PORTB = PORTB

#define Door(on) ((on && (PORTB & B01) == B01) || (!on && (PORTB & B01) == 0)) ? PORTB ^= B01 : PORTB = PORTB
#define Garbage(on) ((on && (PORTB & B10) == B10) || (!on && (PORTB & B10) == 0)) ? PORTB ^= B10 : PORTB = PORTB

// waste of time.

int main(void) {
  init();

  Serial.begin(9600);
  Serial.setTimeout(-1);

  // P1-P8 : DDRA B11111111 (in order)
  // P9 : DDRC B10000000
  // P10 : DDRC B01000000

  // A1 : DDRC B00100000
  // A2 : DDRC B00010000
  // A3 : DDRC B00001000
  // A4 : DDRC B00000100
  // A5 : DDRC B00000010
  // A6 : DDRC B00000001
  // A7 : DDRD B10000000
  // A8 : DDRG B00000100
  // A9 : DDRG B00000010
  // A10 : DDRG B00000001

  // X1 : DDRL B10000000
  // X2 : DDRL B01000000
  // X3 : DDRL B00100000
  // X4 : DDRL B00010000
  // X5 : DDRL B00001000
  // X6 : DDRL B00000100
  // X7 : DDRL B00000010
  // X8 : DDRL B00000001
  // X9 : DDRB B00001000
  // X10 : DDRB B00000100

  // Garbage : DDRB B0010
  // Door : DDRB B0001

  DDRA = B11111111;
  DDRB = B00001111;
  DDRC = B11111111;
  DDRD = B10000000;
  DDRG = B00000111;
  DDRL = B11111111;

  PORTA = B11111111;
  PORTB = B00001111;
  PORTC = B11111111;
  PORTD = B10000000;
  PORTG = B00000111;
  PORTL = B11111111;

  int sendInfo = 0;

  Serial.println("Ready");

  while (true) {
    String v = Serial.readStringUntil('\n');

    if (v == "") {
      Serial.println("Nothing was received.");
    } else if (v == "On") {
      Serial.println("Yes");
    } else if (v.length() == 3) {
      bool on = v[0] == '1';
      int num = v.toInt();
      if (num == 0) {
        continue;
      }
      if (on) {
        num -= 100;
      }

      if (sendInfo) {
        Serial.print("Data: ");
        Serial.println(v);
        char temp[0xF] = "";
        sprintf(temp, "on : %d", on);
        Serial.println(temp);
        sprintf(temp, "number : %d", num);
        Serial.println(temp);
        Serial.println();
      }

      switch (num) {
        case 1:
          P1(on);
          break;
        case 2:
          P2(on);
          break;
        case 3:
          P3(on);
          break;
        case 4:
          P4(on);
          break;
        case 5:
          P5(on);
          break;
        case 6:
          P6(on);
          break;
        case 7:
          P7(on);
          break;
        case 8:
          P8(on);
          break;
        case 9:
          P9(on);
          break;
        case 10:
          P10(on);
          break;
        case 11:
          A1(on);
          break;
        case 12:
          A2(on);
          break;
        case 13:
          A3(on);
          break;
        case 14:
          A4(on);
          break;
        case 15:
          A5(on);
          break;
        case 16:
          A6(on);
          break;
        case 17:
          A7(on);
          break;
        case 18:
          A8(on);
          break;
        case 19:
          A9(on);
          break;
        case 20:
          A10(on);
          break;
        case 21:
          X1(on);
          break;
        case 22:
          X2(on);
          break;
        case 23:
          X3(on);
          break;
        case 24:
          X4(on);
          break;
        case 25:
          X5(on);
          break;
        case 26:
          X6(on);
          break;
        case 27:
          X7(on);
          break;
        case 28:
          X8(on);
          break;
        case 29:
          X9(on);
          break;
        case 30:
          X10(on);
          break;
        case 31:
          Door(on);
          break;
        case 32:
          Garbage(on);
          break;
        default:
          Serial.println("Unknown monster");
          break;
      }
    } else if (v == "RESET") {
      PORTA = B11111111;
      PORTB = B00001111;
      PORTC = B11111111;
      PORTD = B10000000;
      PORTG = B00000111;
      PORTL = B11111111;
    } else if (v == "TSNDINF") {
      sendInfo = sendInfo == 0 ? 1 : 0;
      Serial.print("sendInfo set to ");
      Serial.println(sendInfo == 0 ? "false" : "true");
    } else if (v == "TEST") {
      Serial.println("testing...");
      P1(1);
      delay(250);
      P1(0);
      A1(1);
      delay(250);
      A1(0);
      X1(1);
      delay(250);
      X1(0);
      P2(1);
      delay(250);
      P2(0);
      A2(1);
      delay(250);
      A2(0);
      X2(1);
      delay(250);
      X2(0);
      P3(1);
      delay(250);
      P3(0);
      A3(1);
      delay(250);
      A3(0);
      X3(1);
      delay(250);
      X3(0);
      P4(1);
      delay(250);
      P4(0);
      A4(1);
      delay(250);
      A4(0);
      X4(1);
      delay(250);
      X4(0);
      P5(1);
      delay(250);
      P5(0);
      A5(1);
      delay(250);
      A5(0);
      X5(1);
      delay(250);
      X5(0);
      P6(1);
      delay(250);
      P6(0);
      A6(1);
      delay(250);
      A6(0);
      X6(1);
      delay(250);
      X6(0);
      P7(1);
      delay(250);
      P7(0);
      A7(1);
      delay(250);
      A7(0);
      X7(1);
      delay(250);
      X7(0);
      P8(1);
      delay(250);
      P8(0);
      A8(1);
      delay(250);
      A8(0);
      X8(1);
      delay(250);
      X8(0);
      P9(1);
      delay(250);
      P9(0);
      A9(1);
      delay(250);
      A9(0);
      X9(1);
      delay(250);
      X9(0);
      P10(1);
      delay(250);
      P10(0);
      A10(1);
      delay(250);
      A10(0);
      X10(1);
      delay(250);
      X10(0);
      Garbage(1);
      delay(250);
      Garbage(0);
      Door(1);
      delay(250);
      Door(0);
      Serial.println("done");
    } else if (sendInfo) {
      Serial.print("Unknown data: ");
      Serial.println(v);
    }
  }
}