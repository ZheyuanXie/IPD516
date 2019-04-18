
//byte cmd[3];
int cnt = 0;
int count =0;
/* structure that hold data*/
typedef struct{
  byte note;
  byte vel;
  unsigned long receivedTime;
}midiMes;

midiMes cmd;
/* this variable hold queue handle */
xQueueHandle xQueue;
TaskHandle_t xTask1;
TaskHandle_t xTask2;

void setup() {
  Serial.begin(115200);//one commnicate with midi receiver
  Serial2.begin(115200);//one communucate with Motor or solenoid 
  
//  ledcSetup(0, 2000, 8);
//  ledcAttachPin(32, 0);

//  pinMode(LED_R, OUTPUT);
//  pinMode(LED_Y, OUTPUT);
//  pinMode(LED_G, OUTPUT);
//  pinMode(LED_W, OUTPUT);
  /* create the queue which size can contains 5 elements of Data */
  xQueue = xQueueCreate(1000, sizeof(midiMes));

  xTaskCreatePinnedToCore(
      sendTask,           /* Task function. */
      "sendTask",        /* name of task. */
      10000,                    /* Stack size of task */
      NULL,                     /* parameter of the task */
      1,                        /* priority of the task */
      &xTask1,                /* Task handle to keep track of created task */
      1);                    /* pin task to core 0 */
  xTaskCreatePinnedToCore(
      receiveTask,           /* Task function. */
      "receiveTask",        /* name of task. */
      10000,                    /* Stack size of task */
      NULL,                     /* parameter of the task */
      1,                        /* priority of the task */
      &xTask2,            /* Task handle to keep track of created task */
      0);                 /* pin task to core 1 */   
}

void loop() {
      
}

void sendTask( void * parameter )
{
  for(;;){
    if (Serial2.available() > 0) {
//      Serial.println(Serial2.read());
      process_incoming_byte(Serial2.read());
    }
  }
}

void receiveTask( void * parameter )
{
  BaseType_t xStatus;
  for(;;)
  {
    byte receivedCmd[3];
  /* keep the status of receiving data */  
    /* receive data from the queue */
    xStatus = xQueueReceive( xQueue, &receivedCmd, 10);
    /* check whether receiving is ok or not */
    if(xStatus == pdPASS){    
      process_midi_command(receivedCmd);
    }
  }  
  vTaskDelete( NULL );
}

void process_incoming_byte(const byte incoming_byte) {
  switch (cnt) {
    case 0:
      if (incoming_byte <  128) {
          cmd.note = incoming_byte;
          cnt = 1;
  //        xQueueSendToBack(xQueue, &cmd, 10);      
        }else{
          cnt = 0;
        }      
      break;
    case 1:
      cnt = 0;
      if (incoming_byte < 128) {
          cmd.vel = incoming_byte;
          cnt = 0;
          cmd.receivedTime = micros();
          xQueueSendToBack(xQueue, &cmd, 10);      
        }
      break;
    default:
      cnt = 0;
      break;
   }
}

void process_midi_command(midiMes) {
  //handel your instrument here
}
