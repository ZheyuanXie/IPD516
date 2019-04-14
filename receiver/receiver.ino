#define PRINT_ERROR Serial.println("Error!");

#define LED_R 13
#define LED_Y 12
#define LED_G 27
#define LED_W 25

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200);
  
  ledcSetup(0,2000,8);
  ledcAttachPin(32,0);
  
  pinMode(LED_R, OUTPUT);
  pinMode(LED_Y, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_W, OUTPUT);
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
  if (channel == 0) {
    if (vel > 0) {
      ledcWriteTone(0, note_to_frequency(note));
    } else {
      ledcWriteTone(0,0);
    }
  }
}

int note_to_frequency(uint8_t note) {
  switch(note) {
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
    default: return 0;
  }
}
