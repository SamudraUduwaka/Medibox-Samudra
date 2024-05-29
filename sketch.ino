//******************************//
// *** Uduwaka S. S. *** //
// *** 210663V
// *** ENTC
//*****************************//

//Libararies for functionalities
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHTesp.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

WiFiClient espClient;
PubSubClient mqttClient(espClient);

//define screen details for the OLED display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

//define the Pins
#define BUZZER 5 
#define LED_1 2 

//define the push buttons
#define PB_CANCEL 4 
#define PB_OK 33 
#define PB_UP 32 
#define PB_DOWN 35 

//define DHT 22
#define DHTPIN 12

//define LDRs
#define LDR_LEFT 34
#define LDR_RIGHT 39

//for Servo
Servo servo;

int thetaOffset = 30;
float gammaValue = 0.75;

//for time update
#define NTP_SERVER "pool.ntp.org"
int UTC_OFFSET = 0;
int UTC_OFFSET_DST = 0;

//Declare objects - OLED display and DHT Sensor
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHTesp dhtSensor;

//Global Variables
int days = 0;
int hours = 0;
int minutes = 0;
int seconds = 0;

//for buzzer
int C = 262; //define note C
int D = 294; //define note D
int E = 330; //define note E
int F = 349; //define note F
int G = 392; //define note G
int A = 440; //define note A
int B = 494; //define note B
int C_H = 523; //define note C_H
int notes[] = {C,D,E,F,G,A,B,C_H};
int n = 8;

bool alarm_enabled = true; //turn all alarm on
int n_alarms = 3; // keep track of the alarm
int alarm_hours[] = {0,1,2}; // set hours for two alarms
int alarm_minutes[] = {1,10,20}; // set minutes for two alarms
bool alarm_triggered[] = {false,false,false};//keep the track whether alarm triggered

int current_mode = 0; //current mode in the menu
int max_modes = 5; //number of modes in the menu
String modes[] = {"1 - Set Time Zone", "2 - Set Alarm 1", "3 - Set Alarm 2", 
"4 - Set Alarm 3", "5 - Disable Alarm"};

char tempArr[6];
char ldrLArr[4];
char ldrRArr[4];

/****************************** Setting Up **********************************/                           

void setup() {
  //define the buzzer pin mode as a output
  pinMode(BUZZER, OUTPUT); 
  //define the LED indicator pin mode as a output
  pinMode(LED_1, OUTPUT); 
  //define the Push button pin mode as input
  pinMode(PB_CANCEL,INPUT); 
  pinMode(PB_OK,INPUT);
  pinMode(PB_UP,INPUT);
  pinMode(PB_DOWN,INPUT);
  //define LDRs
  pinMode(LDR_LEFT, INPUT);
  pinMode(LDR_RIGHT, INPUT);

  servo.attach(27);
  servo.write(0);

  //configure the sensor setup
  dhtSensor.setup(DHTPIN, DHTesp::DHT22); 

  //Intialize serial monitor and OLED display
  Serial.begin(9600);

  if (! display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)){
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  //turn on OLED display
  display.display();
  delay(2000);

  setUpWiFi();

  setUpMqtt();

  configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER); //configuer the time from internet

  //clear oled display
  display.clearDisplay();

  printLine("Welcome to Medibox", 10, 20, 2); //Display Welcome message
  display.clearDisplay();

  for(int i=0;i<5;i++){
    tone(BUZZER,notes[i]); // Beep
    delay(100);
  }
  noTone(BUZZER);

  delay(1000);
}

void setUpWiFi(){
  //connect to the WIFI to automatically update the time
  WiFi.begin("Wokwi-GUEST", "", 6 );
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    display.clearDisplay();
    printLine("Connecting to WiFi", 0, 0, 2); //until wifi is connected display the message
  }

  display.clearDisplay();
  printLine("Connected to WiFi", 0, 0, 2); // Display after wifi is connected
}

void setUpMqtt(){
  mqttClient.setServer("test.mosquitto.org",1883);
  mqttClient.setCallback(recieveCallback);
}

/**************************** Looping the Code ***************************/

void loop() {
  update_time_with_check_alarm();

  if(digitalRead(PB_OK)==LOW){
    delay(200);
    go_to_menu();
  }

  if(!mqttClient.connected()){
    connectToBroker();
  }
  mqttClient.loop();

  check_temp_and_humidity();
  updateLight();

}

void connectToBroker(){
  while(!mqttClient.connected()){
    display.println("Attempting MQTT Connection..");
    if(mqttClient.connect("ESP32-12345645454")){
      display.println("Connected");
      mqttClient.subscribe("ENTC-Admin-MinAngleAdjust");
      mqttClient.subscribe("ENTC-Admin-ControFacAdjust");
    }else{
      display.println("Failed");
      Serial.println(mqttClient.state());
      delay(5000);
    }

  }
}

void recieveCallback(char *topic, byte *payload, unsigned int length) {

  char payloadCharAr[length];

  for (int i = 0; i < length; i++) {
    payloadCharAr[i] = (char)payload[i];
  }
  
  if (strcmp(topic, "ENTC-Admin-MinAngleAdjust") == 0) {
    thetaOffset = String(payloadCharAr).toInt();

  } else if (strcmp(topic, "ENTC-Admin-ControFacAdjust") == 0) {
    gammaValue = String(payloadCharAr).toFloat();
  }
}

void updateLight() {

  float leftIntensityValue = analogRead(LDR_LEFT) * 1.00;
  float rightIntensityValue = analogRead(LDR_RIGHT) * 1.00;

  float leftIntensityV = (float)(leftIntensityValue - 4063.00) / (32.00 - 4063.00);
  float rightIntensityV = (float)(rightIntensityValue - 4063.00) / (32.00 - 4063.00);

  //Calling Servo angle update function
  updateAngle(leftIntensityV, rightIntensityV);

  //Connecting with dashboard
  String(leftIntensityV).toCharArray(ldrLArr, 4);
  mqttClient.publish("ENTC-Admin-IntensityL", ldrLArr);
  delay(50);

  String(rightIntensityV).toCharArray(ldrRArr, 4);
  mqttClient.publish("ENTC-Admin-IntensityR", ldrRArr);
  delay(100);
}

void updateAngle(float leftIntensityV, float rightIntensityV) {
  float maxIntensity;
  float dutyValue;

  if (leftIntensityV > rightIntensityV) {
    maxIntensity = leftIntensityV;
    dutyValue = 1.5;
  } else{
    maxIntensity = rightIntensityV;
    dutyValue = 0.5;
  }

  int angleValue = thetaOffset * dutyValue + (180 - thetaOffset) * maxIntensity * gammaValue;
  angleValue = min(angleValue, 180);

  servo.write(angleValue);
}

/******************************* Blink LED ******************************/

void LED_Blink(void){
  digitalWrite(LED_1, HIGH);
  delay(500);
  digitalWrite(LED_1, LOW);
  delay(500);
}


/********************** Function to printLine in OLED ***********************/

void printLine(String text, int column, int row, int text_size){
  display.setTextSize(text_size);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(column,row);
  display.println(text);

  display.display();
}


/***************** Function to Print Current Time in OLED *******************/

void printTimeNow(void){
  display.clearDisplay();
  printLine(String(days),0,0,2);
  printLine(":",20,0,2);
  printLine(String(hours),30,0,2);
  printLine(":",50,0,2);
  printLine(String(minutes),60,0,2);
  printLine(":",80,0,2);
  printLine(String(seconds),90,0,2);
}


/************************** Function to Update Time *************************/

void updateTime(void){
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  
  char timeHour[3];
  strftime(timeHour, 3, "%H", &timeinfo);
  hours = atoi(timeHour);

  char timeMinute[3];
  strftime(timeMinute, 3, "%M", &timeinfo);
  minutes = atoi(timeMinute);

  char timeSecond[3];
  strftime(timeSecond, 3, "%S", &timeinfo);
  seconds = atoi(timeSecond);

  char timeDay[3];
  strftime(timeDay, 3, "%d", &timeinfo);
  days = atoi(timeDay);
  
}


/************************** Function to Ring Alarms **************************/

void ring_alarm(void){
  display.clearDisplay();
  printLine("MEDICINE TIME!",0,0,2); //Display when alarm triggered

  LED_Blink(); //Blink the LED when alarm triggered

  bool break_happened = false; //alarm status

  while(break_happened == false && digitalRead(PB_CANCEL)==HIGH){
    for (int i = 0; i < n; i++ ){
      if (digitalRead(PB_CANCEL)==LOW){
        delay(200);
        break_happened = true;//stop the alarm when PB_cancle is pressed
        break;
      }
      tone(BUZZER, notes[i]); //ring the buzzer
      delay(500);
      noTone(BUZZER);
      delay(2);
    }
  }
  
  digitalWrite(LED_1,LOW);
  display.clearDisplay();
}


/************* Function to Updating the time with Checking Alarm *************/

void update_time_with_check_alarm(void){
  updateTime();
  printTimeNow();

  if (alarm_enabled == true){
    for (int i=0; i<n_alarms; i++){
      if (alarm_triggered[i] == false && alarm_hours[i] == hours && alarm_minutes[i] == minutes){
        ring_alarm();
        alarm_triggered[i] = true;
      }
    }
  }
}


/******************* Function to Waiting for the button press ****************/

int wait_for_button_press(){
  //function for buttons
  while(true){ //infinite loop until any button is pressed
    if (digitalRead(PB_UP) == LOW){
      delay(200);
      return PB_UP;
    }
    else if (digitalRead(PB_DOWN) == LOW){
      delay(200);
      return PB_DOWN;
    }
    else if (digitalRead(PB_OK) == LOW){
      delay(200);
      return PB_OK;
    }
    else if (digitalRead(PB_CANCEL) == LOW){
      delay(200);
      return PB_CANCEL;
    }
  }
}


/*************************** Function to Access Menu *************************/

void go_to_menu(){
  //function for the menu access

  current_mode = 0; 
  //When we stay on another mode and then pressed cancel, 
  //if menu is pressed again, then menu is started from that lasted mode.
  // that is fixed here.

  display.clearDisplay();
  printLine("Menu",0,2,2);
  delay(1000);

  while (digitalRead(PB_CANCEL) == HIGH){
    display.clearDisplay();
    printLine(modes[current_mode], 0, 0, 2);

    int pressed = wait_for_button_press();

    if (pressed == PB_UP){
      delay(200);
      current_mode += 1;
      current_mode = current_mode % max_modes;
    }
    else if (pressed == PB_DOWN){
      delay(200);
      current_mode -= 1;
      if (current_mode < 0){
        current_mode = max_modes - 1;
      }
    }
    else if (pressed == PB_OK){
      delay(200);
      Serial.println(current_mode);
      run_mode(current_mode);
    }
    else if (pressed == PB_CANCEL){
      delay(200);
      break;
    }
  }

}


/********************* Function to Set the Alarms ****************************/

void set_alarm(int alarm){
  //function to set the alarm from menu
  int temp_hour = alarm_hours[alarm];
  while(true){
    display.clearDisplay();
    printLine("Enter alarm hour: "+String(temp_hour),0,0,2);

    int pressed = wait_for_button_press();

    if (pressed == PB_UP){
      delay(200);
      temp_hour += 1;
      temp_hour = temp_hour % 24;
    }
    else if (pressed == PB_DOWN){
      delay(200);
      temp_hour -= 1;
      if (temp_hour < 0){
        temp_hour = 23;
      }
    }
    else if (pressed == PB_OK){
      delay(200);
      alarm_hours[alarm] = temp_hour;
      break;
    }

    else if (pressed == PB_CANCEL){
      delay(200);
      break;
    }
  }

  int temp_minute = alarm_minutes[alarm];
  while(true){
    display.clearDisplay();
    printLine("Enter alarm minutes: "+String(temp_minute),0,0,2);

    int pressed = wait_for_button_press();

    if (pressed == PB_UP){
      delay(200);
      temp_minute += 1;
      temp_minute = temp_minute % 60;
    }
    else if (pressed == PB_DOWN){
      delay(200);
      temp_minute -= 1;
      if (temp_minute < 0){
        temp_minute = 59;
      }
    }
    else if (pressed == PB_OK){
      delay(200);
      alarm_minutes[alarm] = temp_minute;
      break;
    }

    else if (pressed == PB_CANCEL){
      delay(200);
      break;
    }
  }

  display.clearDisplay();
  printLine("Alarm is set",0,0,2);
  delay(1000);
}


/******************** Function to define the modes in Menu ******************/

void run_mode(int mode){
  //function to identify the mode
  if (mode == 0){
    set_time_zone();
  }
  if (mode == 1 || mode == 2 || mode == 3){
    set_alarm(mode-1);
  }
  if (mode == 4){
    alarm_enabled =false;
  }
}


/**************** Function to Check Temperature and Humidity *****************/

void check_temp_and_humidity(){
  // function to check the temprature and humidity
  TempAndHumidity data = dhtSensor.getTempAndHumidity();

  String(data.temperature,2).toCharArray(tempArr,6);
  mqttClient.publish("ENTC-Admin-Temperature",tempArr);

  if (data.temperature > 32){
    display.clearDisplay();
    printLine("TEMP_HIGH",0,40,1);
    LED_Blink();
  } 
  else if (data.temperature < 26){
    display.clearDisplay();
    printLine("TEMP_LOW",0,40,1);
    LED_Blink();
  }
  if (data.humidity > 80){
    display.clearDisplay();
    printLine("HUMIDITY_HIGH",0,50,1);
    LED_Blink();
  } 
  else if (data.humidity < 60){
    display.clearDisplay();
    printLine("HUMIDITY_LOW",0,50,1);
    LED_Blink();
  }  
}


/******************** Function to Set the Time Zone *************************/

void set_time_zone(){
  int temp_hour = 0;

  while(true){
    display.clearDisplay();
    printLine("Enter hour: " + String(temp_hour),0,0,2);

    int pressed = wait_for_button_press();

      if(pressed == PB_UP){
        delay(200);
        temp_hour += 1;
        temp_hour = temp_hour % 24;
      }
      else if(pressed == PB_DOWN){
        delay(200);
        temp_hour -= 1;
      }
      else if(pressed == PB_OK){
        delay(200);
        UTC_OFFSET = (3600 * temp_hour);
        break;
      }
      else if(pressed == PB_CANCEL){
        delay(200);
        break;
      }
  }

  int temp_minute = 0;

  while(true){
    display.clearDisplay();
    printLine("Enter Minute: " + String(temp_minute),0,0,2);

    int pressed = wait_for_button_press();

      if(pressed == PB_UP){
        delay(200);
        temp_minute += 30;
        temp_minute = temp_minute % 60;
      }
      else if(pressed == PB_DOWN){
        delay(200);
        temp_minute -= 30;
        if(temp_minute <= -1){
          temp_minute = 59;
        }
      }
      else if(pressed == PB_OK){
        delay(200);
        if(temp_hour >= 0){
          UTC_OFFSET = UTC_OFFSET+(60 * temp_minute);
        }
        else{
          UTC_OFFSET = UTC_OFFSET-(60 * temp_minute);
        }
        break;
      }
      else if(pressed == PB_CANCEL){
        delay(200);
        break;
      }
  }

  configTime(UTC_OFFSET,UTC_OFFSET_DST,NTP_SERVER);


  display.clearDisplay();
  printLine("Time Zone is set",0,0,2); 
  delay(1000);

}