#define PRINT_ERROR Serial.println("Error!");

#define LED_R 13
#define LED_Y 12
#define LED_G 27
#define LED_W 25

#define DIP_SWITCH_1 36
#define DIP_SWITCH_2 39
#define DIP_SWITCH_3 34
#define DIP_SWITCH_4 35

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, 14, 26);
  Serial2.begin(115200);
  
  ledcSetup(0,2000,8);
  ledcAttachPin(32,0);
  
  pinMode(LED_R, OUTPUT);
  pinMode(LED_Y, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_W, OUTPUT);
  pinMode(DIP_SWITCH_1, INPUT);
  pinMode(DIP_SWITCH_2, INPUT);
  pinMode(DIP_SWITCH_3, INPUT);
  pinMode(DIP_SWITCH_4, INPUT);
}

void loop() {
  if (Serial2.available() > 0) {
    process_incoming_byte(Serial2.read());
  }
}

byte last_incoming_byte;
byte cmd[3];
int cnt = 0;

void process_incoming_byte(const byte incoming_byte) {
  if ((uint8_t)incoming_byte > 127) {
    cnt = 0;
    cmd[cnt] = (incoming_byte & 0x0f);
  } else if (cnt > 3) {
    PRINT_ERROR;
  } else {
    cnt++;
    cmd[cnt] = incoming_byte;
    if (cnt == 2) process_midi_command();
  }
}

void process_midi_command() {
  Serial.printf("channel:%d, note:%d, velocity:%d\n\r",cmd[0],cmd[1],cmd[2]);
  uint8_t channel = cmd[0];
  uint8_t note = cmd[1];
  uint8_t vel = cmd[2];
  
  // Drumset
  if (channel == 9) {
    if (note > 42 && vel > 0) digitalWrite(LED_R, HIGH);
    if (note > 42 && vel == 0) digitalWrite(LED_R, LOW);
    if (note == 35 && vel > 0) digitalWrite(LED_G, HIGH);
    if (note == 35 && vel == 0) digitalWrite(LED_G, LOW);
    if (note == 37 && vel > 0) digitalWrite(LED_Y, HIGH);
    if (note == 37 && vel == 0) digitalWrite(LED_Y, LOW);
    if (note == 42 && vel > 0) digitalWrite(LED_W, HIGH);
    if (note == 42 && vel == 0) digitalWrite(LED_W, LOW);
  }
  
  // Piano
  if (channel == get_channel()) {
    cmd[0] = cmd[0] | 0x80;
    Serial1.write(cmd, 3);
    if (vel > 0) {
      ledcWriteTone(0, note_to_frequency(note));
    } else {
      ledcWriteTone(0,0);
    }
  }
}

uint8_t get_channel() {
  uint8_t res = digitalRead(DIP_SWITCH_1) * 8 + digitalRead(DIP_SWITCH_2) * 4 + digitalRead(DIP_SWITCH_3) * 2 + digitalRead(DIP_SWITCH_4);
  Serial.printf("DIP SWITCH: %d\n", res);
  return res;
}

int note_to_frequency(uint8_t note) {
  switch(note) {
    case 36: return 65;
    case 37: return 69;
    case 38: return 73;
    case 39: return 78;
    case 40: return 82;
    case 41: return 87;
    case 42: return 93;
    case 43: return 98;
    case 44: return 104;
    case 45: return 110;
    case 46: return 117;
    case 47: return 123;
    case 48: return 131;
    case 49: return 139;
    case 50: return 147;
    case 51: return 156;
    case 52: return 165;
    case 53: return 175;
    case 54: return 185;
    case 55: return 196;
    case 56: return 207;
    case 57: return 220;
    case 58: return 233;
    case 59: return 247;
    case 60: return 262;
    case 61: return 277;
    case 62: return 294;
    case 63: return 311;
    case 64: return 330;
    case 65: return 349;
    case 66: return 370;
    case 67: return 392;
    case 68: return 415;
    case 69: return 440;
    case 70: return 466;
    case 71: return 494;
    case 72: return 523;
    case 73: return 554;
    case 74: return 587;
    case 75: return 622;
    case 76: return 659;
    case 77: return 698;
    case 78: return 740;
    case 79: return 784;
    case 80: return 831;
    case 81: return 880;
    case 82: return 932;
    case 83: return 988;
    case 84: return 1046;
    case 85: return 1109;
    case 86: return 1175;
    case 87: return 1245;
    case 88: return 1319;
    case 89: return 1397;
    case 90: return 1480;
    case 91: return 1568;
    case 92: return 1661;
    case 93: return 1760;
    case 94: return 1865;
    case 95: return 1976;
    case 96: return 2093;
    case 97: return 2217;
    case 98: return 2349;
    case 99: return 2489;
    case 100: return 2637;
    case 101: return 2794;
    case 102: return 2960;
    case 103: return 3136;
    case 104: return 3322;
    case 105: return 3520;
    case 106: return 3729;
    case 107: return 3951;
    case 108: return 4186;
    case 109: return 4435;
    case 110: return 4699;
    case 111: return 4978;
    case 112: return 5274;
    case 113: return 5588;
    case 114: return 5920;
    case 115: return 6272;
    case 116: return 6645;
    case 117: return 7040;
    case 118: return 7459;
    case 119: return 7902;
    case 120: return 8372;
    case 121: return 8870;
    case 122: return 9397;
    case 123: return 9956;
    case 124: return 10548;
    case 125: return 11175;
    case 126: return 11840;
    case 127: return 12544;
    default: return 0;
  }
}
