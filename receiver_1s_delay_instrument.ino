#define PRINT_ERROR Serial.println("Error!");

#define DIP_SWITCH_1 36
#define DIP_SWITCH_2 39
#define DIP_SWITCH_3 34
#define DIP_SWITCH_4 35

#include "esp_task_wdt.h"
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"

int cnt = 0;
int count =0; // debug only
/* structure that hold data*/
typedef struct{
  byte info[3];
  unsigned long receivedTime;
}midiMes;

midiMes cmd;

/* this variable hold queue handle */
xQueueHandle xQueue;
//TaskHandle_t xTask1;
TaskHandle_t xTask2;

void setup() {
  Serial.begin(115200);
  //GET MIDI info from receiver you can change the pin here
  Serial1.begin(115200,SERIAL_8N1,14,26);
  Serial2.begin(115200);//DEFAULE PIN 16 AND PIN 17, communicate with motor
  //add pin configuration here
  //example: pinMode();
  
  /* create the queue which size can contains 5 elements of Data */
  xQueue = xQueueCreate(2000, (sizeof(midiMes)));

//  xTaskCreatePinnedToCore(
//      readTask,           /* Task function. */
//      "readTask",        /* name of task. */
//      2000,                    /* Stack size of task */
//      NULL,                     /* parameter of the task */
//      1,                        /* priority of the task */
//      &xTask1,                /* Task handle to keep track of created task */
//      1);                    /* pin task to core 1 */
  xTaskCreatePinnedToCore(
      handleTask,           /* Task function. */
      "handleTask",        /* name of task. */
      20000,                    /* Stack size of task */
      NULL,                     /* parameter of the task */
      1,                        /* priority of the task */
      &xTask2,            /* Task handle to keep track of created task */
      0);                 /* pin task to core 0 */
}

//loop() is also running on core 1
//reading from MIDI
void loop() {
  if (Serial2.available() > 0) {
      process_incoming_byte(Serial2.read());
  }    
}

//void readTask( void * parameter )
//{
//  for(;;){
//    if (Serial2.available() > 0) {
//      Serial.println(Serial2.read());
//      process_incoming_byte(Serial2.read());
//    }
//  }
//}

void handleTask( void * parameter )
{
  BaseType_t xStatus;
  for(;;)
  {
    midiMes receivedCmd;
    /* receive data from the queue */
    xStatus = xQueueReceive( xQueue, &receivedCmd, 10);
    /* check whether receiving is ok or not */
    if(xStatus == pdPASS){  
      //process the midi msg 1 second after receiving
      //make changes here, count the instrument response time
      while ((micros() - receivedCmd.receivedTime)<1000000){
        //feed the dog
        TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
        TIMERG0.wdt_feed=1;
        TIMERG0.wdt_wprotect=0;
      };        
      process_midi_command(receivedCmd.info);
    }
  }  
  vTaskDelete( NULL );
}

void process_incoming_byte(const byte incoming_byte) {
  // you will only get msg of your channel only, the channel byte is for 
  // checking the order of incoming_byte only.
  if ((uint8_t)incoming_byte > 127) {
    cnt = 0;
    cmd.info[cnt] = (incoming_byte & 0x0f);
  } else if (cnt > 3) {
    PRINT_ERROR;
  } else {
    cnt++;
    cmd.info[cnt] = incoming_byte;
    if (cnt == 2) {
      cmd.receivedTime = micros();
      xQueueSendToBack(xQueue, &cmd, 10);
      //Serial1.write(cmd.info,3);       
    }
  }
}

void process_midi_command(byte MIDI[3]) {
  //DEBUG
  count = count +1; 
  Serial.printf("channel:%d, note:%d, velocity:%d， #get%d.\n\r", MIDI[0], MIDI[1], MIDI[2],count);
  uint8_t channel = MIDI[0];
  uint8_t note = MIDI[1];
  uint8_t vel = MIDI[2];

  //----------------PUT YOUT CODE IN HERE (control your INSTRUMENT)------------------
}
