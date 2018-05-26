// Arduino MEGA 2560
// Victor Samper
// sampervictor@gmail.com
// 27 Feb 2018
// V. 1.0.0


// Stimulus(stim.sti)

// Include the Servo library 
#include <Servo.h> 


// Declare pins
// servo pins for SERVO0 - SERVO7
int SERVO_PINS[] = { 2, 3, 4, 5, 6, 7, 8, 9
};

// cartridge microswitch pins for CARTRIDGE0 - CARTRIDGE7
int CARTRIDGE_uSW_PIN[] = { 20, 21, 22,23, 24, 25, 26, 27
};


int BUTTON_PIN = 28; // from button

// chute optical sensor pins for CHUTE0 - CHUTE7 + DETECTTOR CHUTE (A8)
int CHUTE_SENS_PIN[] = { A0, A1, A2, A3, A4, A5, A6, A7, A8
};

//// chute optical sensor pin for DETECTOR_CHUTE
//int DETECTOR_CHUTE_PIN = A8;

// LED indicator pins for CARTRIDGE0 - CARTRIDGE7
int INDICATOR_PIN[] = { 33, 34, 35, 36, 37, 38, 39, 40
};

// LED indicator pin for DETECTOR_CHUTE
int DETECTOR_CHUTE_LED_PIN = 32;

// LED indicator pins for communication mode
int COMMS_LED_PIN = 29; // LED comms indicator
int COMMS_TX_LED_PIN = 30;
int COMMS_RX_LED_PIN = 31;

int AMBIENT_WITHOUT_CHUTE_PIN = 41; // LED ambient without chute level indicator
int AMBIENT_WITH_CHUTE_PIN = 43; // LED ambient with chute level indicator
int TILE_REFLECTION_PIN = 42; // LED for tile reflection level indicator

int READY_LED_PIN = 47;
int FAIL_LED_PIN = 48;
int ERROR_LED_PIN = 49;

int READY_PIN = 52; // to controller AND to indicator
int FAIL_PIN = 51; // to controller AND to indicator
int ERROR_PIN = 50; // to controller AND to indicator

int CLK_PIN = 1; // from controller
int ACK_PIN = 0; // from controller



// Declare constants
int SERVO_REST = 15;
int SERVO_PUSH = 165;

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
Servo servoX[] = { servo0, servo1, servo2, servo3, servo4, servo5, servo6, servo7 };
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
int chute_sens_ambient[9];
int chute_sens_empty[9];
int chute_sens_full[8];
int cartridge_present[8];
int detector_level;
unsigned long timeout = 5000;
unsigned long time_start;
int mode = 0; // 0=setup; 1=operation; 2=comms; 3=error


        //void update_LEDs(){
        //  digitalWrite(LED_WIDE_PIN, LED_wide);
        //  digitalWrite(LED_OPEN_PIN, LED_open);
        //  digitalWrite(LED_CLOSED_PIN, LED_closed);
        //  digitalWrite(LED_FAIL_PIN, LED_fail);
        //  digitalWrite(LED_GOOD_PIN, LED_good);
        //}


//int flash_state(unsigned long start_time, unsigned long period) {
//  unsigned long number_periods;
////  unsigned long current_time_passed = time_passed(millis());
//  number_periods = current_time_passed / period;
//// check to see if odd or even
//  if ((number_periods % 2) == 0) {
//    return 1;
//  }
//  else{
//    return 0;
//  }
//}



void all_LEDs_on() {
  digitalWrite(COMMS_LED_PIN, LOW);
  digitalWrite(COMMS_TX_LED_PIN, LOW);
  digitalWrite(COMMS_RX_LED_PIN, LOW);

  digitalWrite(AMBIENT_WITHOUT_CHUTE_PIN, LOW);
  digitalWrite(AMBIENT_WITH_CHUTE_PIN, LOW);
  digitalWrite(TILE_REFLECTION_PIN, LOW);

  for (int i = 0; i < 8; i++) {
    digitalWrite(INDICATOR_PIN[i], LOW);
  }

  digitalWrite(DETECTOR_CHUTE_LED_PIN, LOW);

  digitalWrite(ERROR_LED_PIN, LOW);
  digitalWrite(FAIL_LED_PIN, LOW);
  digitalWrite(READY_LED_PIN, LOW);

}


void all_LEDs_off() {
  for(int i=0; i<3; i++){
  digitalWrite(COMMS_LED_PIN, HIGH);
  delay(25);
  digitalWrite(COMMS_TX_LED_PIN, HIGH);
   delay(25);
  digitalWrite(COMMS_RX_LED_PIN, HIGH);
 delay(25);
   digitalWrite(COMMS_LED_PIN, LOW);
  delay(25);
  digitalWrite(COMMS_TX_LED_PIN, LOW);
   delay(25);
  digitalWrite(COMMS_RX_LED_PIN, LOW);
 delay(25);
  }
 delay(500);

 
   for(int i=0; i<3; i++){
  digitalWrite(AMBIENT_WITHOUT_CHUTE_PIN, HIGH);
   delay(25);
  digitalWrite(AMBIENT_WITH_CHUTE_PIN, HIGH);
   delay(25);
  digitalWrite(TILE_REFLECTION_PIN, HIGH);
   delay(25);
     digitalWrite(AMBIENT_WITHOUT_CHUTE_PIN, LOW);
   delay(25);
  digitalWrite(AMBIENT_WITH_CHUTE_PIN, LOW);
   delay(25);
  digitalWrite(TILE_REFLECTION_PIN, LOW);
   delay(25);
   }
delay(500);


   for(int i=0; i<3; i++){
  digitalWrite(DETECTOR_CHUTE_LED_PIN, HIGH);
   delay(10);
   
  for (int i = 0; i < 8; i++) {
    digitalWrite(INDICATOR_PIN[i], HIGH);
     delay(10);
  }
    digitalWrite(DETECTOR_CHUTE_LED_PIN, LOW);
   delay(10);
   
  for (int i = 0; i < 8; i++) {
    digitalWrite(INDICATOR_PIN[i], LOW);
     delay(10);
  }
   }
 delay(500);


 
    for (int i = 0; i < 3; i++) {
  digitalWrite(ERROR_LED_PIN, HIGH);
   delay(250);
  digitalWrite(FAIL_LED_PIN, HIGH);
   delay(25);
  digitalWrite(READY_LED_PIN, HIGH);
   delay(25);
     digitalWrite(ERROR_LED_PIN, LOW);
   delay(25);
  digitalWrite(FAIL_LED_PIN, LOW);
   delay(25);
  digitalWrite(READY_LED_PIN, LOW);
   delay(25);
}
delay(500);



    for(int i=0; i<3; i++){
  digitalWrite(DETECTOR_CHUTE_LED_PIN, HIGH);
   
  for (int i = 0; i < 8; i++) {
    digitalWrite(INDICATOR_PIN[i], HIGH);
  }
    digitalWrite(AMBIENT_WITHOUT_CHUTE_PIN, HIGH);
  digitalWrite(AMBIENT_WITH_CHUTE_PIN, HIGH);
  digitalWrite(TILE_REFLECTION_PIN, HIGH);
  digitalWrite(COMMS_LED_PIN, HIGH);
  digitalWrite(COMMS_TX_LED_PIN, HIGH);
  digitalWrite(COMMS_RX_LED_PIN, HIGH);
  digitalWrite(ERROR_LED_PIN, HIGH);
  digitalWrite(FAIL_LED_PIN, HIGH);
  digitalWrite(READY_LED_PIN, HIGH);

  delay(250);

    digitalWrite(DETECTOR_CHUTE_LED_PIN, LOW);
   
  for (int i = 0; i < 8; i++) {
    digitalWrite(INDICATOR_PIN[i], LOW);
  }
    digitalWrite(AMBIENT_WITHOUT_CHUTE_PIN, LOW);
  digitalWrite(AMBIENT_WITH_CHUTE_PIN, LOW);
  digitalWrite(TILE_REFLECTION_PIN, LOW);
  digitalWrite(COMMS_LED_PIN, LOW);
  digitalWrite(COMMS_TX_LED_PIN, LOW);
  digitalWrite(COMMS_RX_LED_PIN, LOW);
  digitalWrite(ERROR_LED_PIN, LOW);
  digitalWrite(FAIL_LED_PIN, LOW);
  digitalWrite(READY_LED_PIN, LOW);
delay(250);
   }

}



unsigned long button_press() {
  int time_start = millis();
  int button_state = digitalRead(BUTTON_PIN);
  while (button_state == HIGH) {
    button_state = digitalRead(BUTTON_PIN);
    delay(10);
  }
  while (button_state == LOW) {
    button_state = digitalRead(BUTTON_PIN);
    delay(10);
  }
 // return time_passed(time_start);
}



void setup() {
  delay(1000);
  Serial.begin(9600);
  unsigned long time_button_pressed;

  // open the serial port at 9600 bps:
  if (ENABLE_SERIAL_MONITOR) Serial.begin(9600);

  for (int i = 0; i < 8; i++) {
    pinMode(CARTRIDGE_uSW_PIN[i], INPUT);
    pinMode(INDICATOR_PIN[i], OUTPUT);
  }
  pinMode(BUTTON_PIN, INPUT);
  pinMode(DETECTOR_CHUTE_LED_PIN, OUTPUT);

  pinMode(COMMS_LED_PIN, OUTPUT);
  pinMode(COMMS_TX_LED_PIN, OUTPUT);
  pinMode(COMMS_RX_LED_PIN, OUTPUT);

  pinMode(READY_LED_PIN, OUTPUT);
  pinMode(FAIL_LED_PIN, OUTPUT);
  pinMode(ERROR_LED_PIN, OUTPUT);

  pinMode(AMBIENT_WITHOUT_CHUTE_PIN, OUTPUT);
  pinMode(AMBIENT_WITH_CHUTE_PIN, OUTPUT);
  pinMode(TILE_REFLECTION_PIN, OUTPUT);

  pinMode(READY_PIN, OUTPUT);
  pinMode(FAIL_PIN, OUTPUT);
  pinMode(ERROR_PIN, OUTPUT);
//  pinMode(CLK_PIN, OUTPUT);
//  pinMode(ACK_PIN, INPUT);

   for (int i=0; i<8; i++){
      servoX[i].attach(SERVO_PINS[i]);
      servoX[i].write(15);
    }

  // initialise LED indicator status
  all_LEDs_on();
  delay(1000);
  all_LEDs_off();
  button_press();

int p=0;
while (p<2){

for (int i=0; i<8; i++){
  servoX[i].write(15);
  delay(350);
  servoX[i].write(165);
  delay(350);
  servoX[i].write(15);
}
 p++;
  
}

}

void loop() {
  for (int i=0; i<8; i++){
  servoX[i].write(5);
  delay(500);
  servoX[i].write(170);
  delay(500);
  servoX[i].write(5);
  Serial.println(millis());
  Serial.println("Switches");
  for (int i=0; i<8; i++){
    Serial.println(digitalRead(CARTRIDGE_uSW_PIN[i]));
  }
  Serial.println("Sensors");
    for (int i=0; i<9; i++){
    Serial.println(analogRead(CHUTE_SENS_PIN[i]));
  }
  Serial.println(millis());
  }
}
