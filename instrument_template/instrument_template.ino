/** Declare your global variable here **/
int inst_id;  // example global variable

/** Implement code for your instrument here **/
void init_inst(){
  inst_id = 0;
}

long expected_delay(uint8_t note, uint8_t velocity) {
  return 0; 
}

void note_on(uint8_t note, uint8_t velocity) {
  Serial.printf("Note_on: %d, %d\n", note, velocity);
  ledcWriteTone(0, note_to_frequency(note));
}

void note_off(uint8_t note) {
  Serial.printf("Note_off: %d\n", note);
  ledcWriteTone(0, 0);
}

/** Try not to modify the code below **/
#define GLOBAL_DELAY 3000000  // global delay in microseconds

typedef struct msg_t {
  uint8_t note;
  uint8_t velocity;
  long ts;
} msg_t;

/* cmd buffer */
uint8_t cmd_buffer[3];

/* msg queue */
#define MSG_Q_SIZE 128
int msg_q_ptr = 0;
int msg_q_occupy = 0;
uint8_t msg_q_bmap[MSG_Q_SIZE];
msg_t msg_q[MSG_Q_SIZE];

void setup() {  
  // buzzer
  ledcSetup(0,2000,8);
  ledcAttachPin(32,0);
  
  memset(msg_q_bmap, 0, sizeof(msg_q_bmap));
  init_inst();
  Serial.begin(115200);
  Serial2.begin(115200);
}

void loop() {
  get_cmd();
  for (int i = 0; i < MSG_Q_SIZE; i++) {
    if (msg_q_bmap[i] == 1) {
      if (micros() - msg_q[i].ts > GLOBAL_DELAY - expected_delay(msg_q[i].note, msg_q[i].velocity)) {
        if (msg_q[i].velocity > 0) note_on(msg_q[i].note, msg_q[i].velocity);
        else note_off(msg_q[i].note);
        msg_q_bmap[i] = 0;
      }
    }
  }
}

bool get_cmd() {
  static int cnt = 0;
  if (Serial2.available() > 0) {
    byte incoming_byte = Serial2.read();
    if ((uint8_t)incoming_byte > 127) {       // The first status byte
      cnt = 0;
      cmd_buffer[cnt] = (incoming_byte & 0x0f);
    } else if (cnt > 3) {                     // Exceed packet length
      Serial.printf("Error: packet length error!\n");
    } else {                                  // data bytes
      cmd_buffer[++cnt] = incoming_byte;
    }
    if (cnt == 2) {                           // add new message to queue
      while(1) {
        if (msg_q_occupy == MSG_Q_SIZE) {
          Serial.printf("Error: message queue is full!\n");
          break;
        }
        if (msg_q_bmap[msg_q_ptr] == 0) {
          msg_q[msg_q_ptr].ts = micros();
          msg_q[msg_q_ptr].note = cmd_buffer[1];
          msg_q[msg_q_ptr].velocity = cmd_buffer[2];
          Serial.printf("New msg added to queue at %d!\n", msg_q[msg_q_ptr].ts);
          msg_q_bmap[msg_q_ptr++] = 1;
          msg_q_occupy++;
          break;
        } else {
          Serial.printf("!\n");
          msg_q_ptr = (msg_q_ptr + 1) % MSG_Q_SIZE;
        }
      }
    }
  }
  return false;
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
