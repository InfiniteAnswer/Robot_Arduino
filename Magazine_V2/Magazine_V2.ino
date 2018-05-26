// Arduino MEGA 2560
// Victor Samper
// sampervictor@gmail.com
// 27 Feb 2018
// V. 1.0.0


// Include the Servo library 
#include <Servo.h> 


// Declare pins
// servo pins for SERVO0 - SERVO7
int SERVO_PINS[] = {2, 3, 4, 5, 6, 7, 8, 9
  };

// cartridge microswitch pins for CARTRIDGE0 - CARTRIDGE7
int CARTRIDGE_uSW_PIN[] = {30, 31, 32, 33, 34, 35, 36, 37
  };

// chute optical sensor pins for CHUTE0 - CHUTE7
int CHUTE_SENS_PIN[] = {A0, A1, A2, A3, A4, A5, A6, A7
  };

// chute optical sensor pin for DETECTOR_CHUTE
int DETECTOR_CHUTE_PIN = A8;

// LED indicator pins for CARTRIDGE0 - CARTRIDGE7
int INDICATOR_PIN[] = {22, 23, 24, 25, 26, 27, 28, 29
  };
// LED indicator pins for communication mode
int INDICATOR_COMMS_PIN = 38;

// LED indicator pin for DETECTOR_CHUTE
int DETECTOR_CHUTE_LED_PIN = 39;

int READY_PIN = 40;
int FAIL_PIN = 41;
int ERROR_PIN = 42;

int CLK_PIN = 43;
int ACK_PIN = 44;
int COMMS_PIN = 45;

int BUTTON_PIN = 46;


// Declare constants
int SERVO_REST = 0;
int SERVO_PUSH = 180;

int SENSOR_TRIGGER_UPPER_DELTA = 20;
int SENSOR_TRIGGER_LOWER_DELTA = 10;

unsigned long RESET_PRESET_TIME = 3000;
boolean ENABLE_SERIAL_MONITOR = false;


// Declare variables
//Servo servoX[8];
Servo servo0;
Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;
Servo servo5;
Servo servo6;
Servo servo7;
long average_optical_SENS = 0;
int indicatorX[8];
unsigned long time_switch_released;
int clk = LOW;
int ack = LOW;
int comm = LOW;
int ready = LOW;
int fail = LOW;
int error = LOW;
int detector_chute_sens_ambient = 0;
int detector_chute_sens_close = 0;
int chute_sens_ambient[8];
int chute_sens_empty[8];
int chute_sens_full[8];
int cartridge_present[8];
int mode = 0; // 0=setup; 1=operation; 2=comms; 3=error


//void update_LEDs(){
//  digitalWrite(LED_WIDE_PIN, LED_wide);
//  digitalWrite(LED_OPEN_PIN, LED_open);
//  digitalWrite(LED_CLOSED_PIN, LED_closed);
//  digitalWrite(LED_FAIL_PIN, LED_fail);
//  digitalWrite(LED_GOOD_PIN, LED_good);
//}


void setup() {
// open the serial port at 9600 bps:
  if (ENABLE_SERIAL_MONITOR) Serial.begin(9600);  
  
//  for (int i=0; i<8; i++){
//    servoX[i].attach(SERVO_PINS[i]);
//  }
  servo0.attach(SERVO_PINS[0]);
  servo1.attach(SERVO_PINS[1]);
  servo2.attach(SERVO_PINS[2]);
  servo3.attach(SERVO_PINS[3]);
  servo4.attach(SERVO_PINS[4]);
  servo5.attach(SERVO_PINS[5]);
  servo6.attach(SERVO_PINS[6]);
  servo7.attach(SERVO_PINS[7]);
  for (int i=0; i<8; i++){
    pinMode(CARTRIDGE_uSW_PIN[i], INPUT);
    pinMode(INDICATOR_PIN[i], OUTPUT);
  }
  pinMode(INDICATOR_COMMS_PIN, OUTPUT);
  pinMode(DETECTOR_CHUTE_LED_PIN, OUTPUT);
  pinMode(READY_PIN, OUTPUT);
  pinMode(FAIL_PIN, OUTPUT);
  pinMode(ERROR_PIN, OUTPUT);
  pinMode(CLK_PIN, INPUT);
  pinMode(ACK_PIN, INPUT);
  pinMode(COMMS_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  mode = 2;
  comms_mode();
}


void update_cartridge(){
  for (int i=1; i<8; i++){
    cartridge_present[i] = digitalRead(CARTRIDGE_uSW_PIN[i]);
  }
}


unsigned long time_passed(unsigned long start_time){
  unsigned long time_passed;
  unsigned long current_time;
  current_time = millis();
// Check for 40 day (32 bit unsigned long) overflow
  if (current_time<start_time){
    start_time -= pow(2, 15);
    current_time += pow(2, 15);
  }
  time_passed = current_time - start_time;
  return time_passed;
}


void send_state_to_TT(){
  int clk_state = LOW;
  int count = 0;
  int error_count = 0;
  unsigned long timeout = 5000;
  unsigned long dead_time = 100;
  unsigned long time_start;  
//  int previous_clk_state = LOW;
//  unsigned long time_present;
//  unsigned long elapsed_time;

  
  // set mode to comms
  digitalWrite(ERROR_PIN, HIGH);
  digitalWrite(READY_PIN, HIGH);
  
  // wait for 5 HIGH->LOW transitions on CLK pin to indicate start of comms
  clk_state = digitalRead(CLK_PIN);
  time_start = millis();
  while ((time_passed(time_start)<timeout) && (count<5)){
      while ((time_passed(time_start)<timeout) && (clk_state==LOW)){
// in low state
        clk_state = digitalRead(CLK_PIN);
        delay(50);
      }
      while ((time_passed(time_start)<timeout) && (clk_state==HIGH)){
// in high state
        clk_state = digitalRead(CLK_PIN);
        delay(50);
      }
// if reached this line without timout then increase count of HIGH->LOW transitions
    if (time_passed(time_start)<timeout){
      count ++;
    }
  }
// if timeout occurred then ERROR

// ready to start comms?
  if (count==5){
// on each subsequent HIGH->LOW CLK transition, send data during the LOW cycle of CLK
    clk_state = digitalRead(CLK_PIN);
    time_start = millis();  
    count = 0;  
    while ((time_passed(time_start)<timeout) && (count<8)){
// in low state
// wait until dead time is passed
      while(time_passed(time_start)<dead_time){
        delay(10);
      }
// send next cartridge state on fail line
      digitalWrite(FAIL_PIN, cartridge_present[count]);
      count ++;
      while ((time_passed(time_start)<timeout) && (clk_state==LOW)){
        clk_state = digitalRead(CLK_PIN);
        delay(10);
      }
      while ((time_passed(time_start)<timeout) && (clk_state==HIGH)){
// in high state
        clk_state = digitalRead(CLK_PIN);
        delay(10);
      }
// in low state
// wait until dead time is passed
      while (time_passed(time_start)<dead_time){
        delay(10);
      }
// receive signal on ACK pin and copare with sent signal
      if (digitalRead(ACK_PIN) != cartridge_present[count-1]){
        error_count ++;
      }
      while ((time_passed(time_start)<timeout) && (clk_state==LOW)){
// in low state        
        clk_state = digitalRead(CLK_PIN);
        delay(10);
      }
      while ((time_passed(time_start)<timeout) && (clk_state==HIGH)){
// in high state
        clk_state = digitalRead(CLK_PIN);
        delay(10);
      }
    }
  }
// if timeout occurred then ERROR


  // consider variation to allow multiple retries
  // count errors
  // if no errors at end then indicate successful transmission with unique flashing pattern of COMMS LED
  // if errors and number of retries is < 3, resend
  // if errors and number of retries == 3, trigger an error and keep COMM LED on to show error is in comms
  
}


void setup_mode(){
  
}


void operation_mode(){
  
}


void comms_mode(){
  // illuminate comms LED to indicate in COMMS_MODE
  digitalWrite(COMMS_PIN, HIGH);
  
  // scan through 8 uSwitches and store which cartridges are present
  update_cartridge();

  // send cartridge state to TT
  send_state_to_TT();
}


void error_mode(){
  
}


void loop() {
  // put your main code here, to run repeatedly:

}
