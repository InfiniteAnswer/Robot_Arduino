// Arduino MEGA 2560
// Victor Samper
// sampervictor@gmail.com
// 10 April 2018
// V. 1.1.2a

// Include the Servo library 
#include <Servo.h> 


// Declare pins
// servo pins for SERVO0 - SERVO7
//int SERVO_PINS[] = { 2, 3, 4, 5, 6, 7, 8, 9
int SERVO_PINS[] = {9, 8, 7, 6, 5, 4, 3, 2
};

// microswitch pins for CARTRIDGE0 - CARTRIDGE7
int CARTRIDGE_uSW_PIN[] = { 24, 25, 26, 27, 20, 21, 22, 23
};

// push button pin
int BUTTON_PIN = 28;

// chute optical sensor pins for CHUTE0 - CHUTE7 + DETECTOR CHUTE (A8)
int CHUTE_SENS_PIN[] = { A7, A6, A5, A4, A3, A2, A1, A0, A8
};

// LED indicator pins for CARTRIDGE0 - CARTRIDGE7
int INDICATOR_PIN[] = { 33, 34, 35, 36, 37, 38, 39, 40
};

// LED indicator pin for DETECTOR_CHUTE
int DETECTOR_CHUTE_LED_PIN = 32;

// LED indicator pins for communication mode
int COMMS_LED_PIN = 29;
int COMMS_TX_LED_PIN = 30;
int COMMS_RX_LED_PIN = 31;

// LED pins for ambient setting indicators
int AMBIENT_WITHOUT_CHUTE_PIN = 41; // LED ambient without chute level indicator
int AMBIENT_WITH_CHUTE_PIN = 43; // LED ambient with chute level indicator
int TILE_REFLECTION_PIN = 42; // LED for tile reflection level indicator

// LED pins for status indicators
int READY_LED_PIN = 49;
int FAIL_LED_PIN = 48;
int ERROR_LED_PIN = 47;

// CONNECTIONS TO MAIN CONTROLLER
int READY_PIN = 52; // to controller AND to indicator
int FAIL_PIN = 51; // to controller AND to indicator
int ERROR_PIN = 50; // to controller AND to indicator

// USART TX RX
int TX_PIN = 1; // transmit
int RX_PIN = 0; // receive


// Declare constants
int SERVO_REST = 15;
int SERVO_PUSH = 165;
int SENSOR_TRIGGER_UPPER_DELTA = 30;
int SENSOR_TRIGGER_LOWER_DELTA = 30;
boolean ENABLE_SERIAL_MONITOR = false;
unsigned long SHORT_PRESS_DURATION = 3000;
int RESCAN_DELAY = 500;


// Declare variables
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
unsigned long time_switch_released;
int comm = LOW;
int tx = LOW;
int rx = LOW;
int ready = LOW;
int fail = LOW;
int error = LOW;
int ambient_without_chute = LOW;
int ambient_with_chute = LOW;
int tile_reflection = LOW;
int indicator[8] = { LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW };
int detector_chute = LOW;
int detector_chute_sens_ambient = 0;
int detector_chute_sens_close = 0;
int chute_sens_ambient[9];
int chute_sens_empty[9];
int chute_sens_full[8];
int cartridge_present[8];
int current_chute_scan[8];
int previous_chute_scan[8];
int detector_level;
unsigned long timeout = 5000;
unsigned long time_start;
int button_state;
int first_empty_chute;
int last_chute_refilled;
boolean armed = true;
int mode = 0; // 0=setup; 1=operation; 2=comms; 3=error


// Setup cartridges present
void setup_cartridges() {
  for (int i = 0; i<8; i++) {
    cartridge_present[i] = !digitalRead(CARTRIDGE_uSW_PIN[i]);
    indicator[i] = cartridge_present[i];
  }
}


// Update all LED and output pin states
void update_LEDs() {
  digitalWrite(COMMS_LED_PIN, !comm);
  digitalWrite(COMMS_TX_LED_PIN, !tx);
  digitalWrite(COMMS_RX_LED_PIN, !rx);
  digitalWrite(AMBIENT_WITHOUT_CHUTE_PIN, !ambient_without_chute);
  digitalWrite(AMBIENT_WITH_CHUTE_PIN, !ambient_with_chute);
  digitalWrite(TILE_REFLECTION_PIN, !tile_reflection);
  digitalWrite(READY_LED_PIN, !ready);
  digitalWrite(READY_PIN, ready);
  digitalWrite(FAIL_LED_PIN, !fail);
  digitalWrite(FAIL_PIN, fail);
  digitalWrite(ERROR_LED_PIN, !error);
  digitalWrite(ERROR_PIN, error);
  digitalWrite(DETECTOR_CHUTE_LED_PIN, !detector_chute);
  for (int i = 0; i<8; i++) {
    digitalWrite(INDICATOR_PIN[i], !indicator[i]);
  }
}


// Calculate time passed in ms, including 40 day (32 bit unsigned long) overflow
unsigned long time_passed(unsigned long start_time) {
  unsigned long elapsed_time;
  unsigned long current_time;
  current_time = millis();
  if (current_time<start_time) {
    start_time -= pow(2, 31);
    current_time += pow(2, 31);
  }
  elapsed_time = current_time - start_time;
  return elapsed_time;
}


// Calculate flashing state based on period and start time
int flash_state(unsigned long period, unsigned long start_time) {
  unsigned long elapsed_time = time_passed(start_time);
  unsigned long count = elapsed_time / period;
  int state;
  if (count % 2 == 0) {
    state = LOW;
  }
  else {
    state = HIGH;
  }
  return state;
}


// Time a button press
unsigned long time_button_pressed(unsigned long start_time) {
  int button_state = digitalRead(BUTTON_PIN);
  while (button_state == LOW) {
    button_state = digitalRead(BUTTON_PIN);
    delay(50);
  }
  return time_passed(start_time);
}


// Wait for button press, flash LEDs to indicate waiting, return button press duration
unsigned long button_press(boolean flash) {
  int timing[6][4] = {
    { 100, LOW, HIGH, HIGH },
  { 200, HIGH, LOW, HIGH },
  { 300, HIGH, HIGH, LOW },
  { 500, HIGH, HIGH, HIGH },
  { 700, HIGH, HIGH, LOW },
  { 900, HIGH, HIGH, HIGH }
  };
  unsigned long time_start = millis();
  unsigned long current_time = millis();
  int timing_index = 0;
  int button_state = digitalRead(BUTTON_PIN);
  while (button_state == HIGH) {
    if (flash) {
      digitalWrite(READY_LED_PIN, timing[timing_index][1]);
      digitalWrite(FAIL_LED_PIN, timing[timing_index][2]);
      digitalWrite(ERROR_LED_PIN, timing[timing_index][3]);
      if (time_passed(current_time) > timing[timing_index][0]) {
        timing_index++;
      }
      if (timing_index == 6) {
        timing_index = 0;
        current_time = millis();
      }
    }
    button_state = digitalRead(BUTTON_PIN);
    delay(10);
  }
  while (button_state == LOW) {
    button_state = digitalRead(BUTTON_PIN);
    delay(10);
  }
  update_LEDs();
  return time_passed(time_start);
}


// Setup optical sensor average levels for 3 conditions
void setup_ambient_levels() {
  unsigned long start_time = millis();
  unsigned long PERIOD = 200;
  int SAMPLE_TIME = 2000;
  int number_measurements = 0;
  unsigned long running_total;
  unsigned long measurement_time;
  int LED_state = HIGH;
  int sensor_value;

  // Update and display state
  error = LOW;
  fail = LOW;
  ready = LOW;
  ambient_without_chute = LOW;
  ambient_with_chute = LOW;
  tile_reflection = LOW;
  update_LEDs();

  // Flash READY/FAIL/ERROR LEDs to indicate waiting for button press
  // Send (TRUE) to button_press() to select this mode
  button_press(true);

  // indicate that measuring the ambient without chute by flashing the LED
  // cycle through the 9 measurements, recording signal for 2 seconds each time
  // average and store
  for (int i = 0; i < 9; i++) {
      if (i<8) {
          digitalWrite(INDICATOR_PIN[i], LOW);
      }
      else {
          digitalWrite(DETECTOR_CHUTE_LED_PIN, LOW);
      }
    measurement_time = millis();
    running_total = 0;
    number_measurements = 0;
    while (time_passed(measurement_time) < SAMPLE_TIME) {
      sensor_value = analogRead(CHUTE_SENS_PIN[i]);
//      Serial.println(sensor_value);
      running_total += sensor_value;
      number_measurements++;
      digitalWrite(AMBIENT_WITHOUT_CHUTE_PIN, flash_state(PERIOD, start_time));
      if (i<8) {
          digitalWrite(INDICATOR_PIN[i], flash_state(PERIOD, start_time));
      }
      else {
          digitalWrite(DETECTOR_CHUTE_LED_PIN, flash_state(PERIOD, start_time));
      }
      delay(50);
    }
    chute_sens_ambient[i] = running_total / number_measurements;
//    Serial.print("Amb: ");
//    Serial.println(i);
//    Serial.println(running_total);
//    Serial.println(number_measurements);
//    Serial.println(chute_sens_ambient[i]);
    if (i<8) {
          digitalWrite(INDICATOR_PIN[i], HIGH);
      }
      else {
          digitalWrite(DETECTOR_CHUTE_LED_PIN, HIGH);
      }
  }

  // indicate that ambient without chute measurement is complete with solid LED
  ambient_without_chute = HIGH;
  digitalWrite(AMBIENT_WITHOUT_CHUTE_PIN, !ambient_without_chute);

  // wait for button press before advancing
  button_press(true);

  // indicate that measuring the ambient with chute by flashing the LED
  for (int i = 0; i < 9; i++) {
      if (i<8) {
          digitalWrite(INDICATOR_PIN[i], LOW);
      }
      else {
          digitalWrite(DETECTOR_CHUTE_LED_PIN, LOW);
      }
    measurement_time = millis();
    running_total = 0;
    number_measurements = 0;
    while (time_passed(measurement_time) < SAMPLE_TIME) {
      sensor_value = analogRead(CHUTE_SENS_PIN[i]);
//      Serial.println(sensor_value);
      running_total += sensor_value;
      number_measurements++;
      digitalWrite(AMBIENT_WITH_CHUTE_PIN, flash_state(PERIOD, start_time));
      if (i<8) {
          digitalWrite(INDICATOR_PIN[i], flash_state(PERIOD, start_time));
      }
      else {
          digitalWrite(DETECTOR_CHUTE_LED_PIN, flash_state(PERIOD, start_time));
      }
      delay(50);
    }
    chute_sens_empty[i] = running_total / number_measurements;
//    Serial.print("Emp: ");
//    Serial.println(i);
//    Serial.println(running_total);
//    Serial.println(number_measurements);
//    Serial.println(chute_sens_empty[i]);
    if (i<8) {
          digitalWrite(INDICATOR_PIN[i], HIGH);
      }
      else {
          digitalWrite(DETECTOR_CHUTE_LED_PIN, HIGH);
      }
  }

  // indicate that ambient with chute measurement is complete with solid LED
  ambient_with_chute = HIGH;
  digitalWrite(AMBIENT_WITH_CHUTE_PIN, !ambient_with_chute);

  // wait for button press before advancing
  button_press(true);

  // indicate that measuring the reflected tile level by flashing the LED
  // cycle through the 8 measurements instead of 9 for previous 2 measurements
  for (int i = 0; i < 8; i++) {
      digitalWrite(INDICATOR_PIN[i], LOW);
    measurement_time = millis();
    running_total = 0;
    number_measurements = 0;
    while (time_passed(measurement_time) < SAMPLE_TIME) {
      sensor_value = analogRead(CHUTE_SENS_PIN[i]);
//      Serial.println(sensor_value);
      running_total += sensor_value;
      number_measurements++;
      digitalWrite(TILE_REFLECTION_PIN, flash_state(PERIOD, start_time));
      if (i<8) {
          digitalWrite(INDICATOR_PIN[i], flash_state(PERIOD, start_time));
      }
      else {
          digitalWrite(DETECTOR_CHUTE_LED_PIN, flash_state(PERIOD, start_time));
      }
      delay(50);
    }
    chute_sens_full[i] = running_total / number_measurements;
//    Serial.print("Ful: ");
//    Serial.println(i);
//    Serial.println(running_total);
//    Serial.println(number_measurements);
//    Serial.println(chute_sens_full[i]);
    digitalWrite(INDICATOR_PIN[i], HIGH);
  }

  // indicate that reflected tile level measurement is complete with solid LED
  tile_reflection = HIGH;
  digitalWrite(TILE_REFLECTION_PIN, !tile_reflection);

  // wait for button press before advancing
  button_press(true);

  error = LOW;
  fail = LOW;
  ready = HIGH;
  update_LEDs();

  // exit averaging mode
}


// Startup check
void startup_check() {
  delay(1000);
  for (int i = 0; i<3; i++) {
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
  delay(250);

  for (int i = 0; i<3; i++) {
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
  delay(250);

  for (int i = 0; i<3; i++) {
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
  delay(250);

  for (int i = 0; i < 3; i++) {
    digitalWrite(ERROR_LED_PIN, HIGH);
    delay(25);
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
  delay(250);

  for (int i=0; i<8; i++) {
    servoX[i].write(170);
    delay(500);
    servoX[i].write(5);
    delay(500);
  }
  
  for (int i = 0; i<3; i++) {
    digitalWrite(COMMS_LED_PIN, LOW);
    digitalWrite(COMMS_TX_LED_PIN, LOW);
    digitalWrite(COMMS_RX_LED_PIN, LOW);
    digitalWrite(DETECTOR_CHUTE_LED_PIN, LOW);
    for (int i = 0; i < 8; i++) {
      digitalWrite(INDICATOR_PIN[i], LOW);
    }
    digitalWrite(AMBIENT_WITHOUT_CHUTE_PIN, LOW);
    digitalWrite(AMBIENT_WITH_CHUTE_PIN, LOW);
    digitalWrite(TILE_REFLECTION_PIN, LOW);
    digitalWrite(ERROR_LED_PIN, LOW);
    digitalWrite(FAIL_LED_PIN, LOW);
    digitalWrite(READY_LED_PIN, LOW);
    delay(250);

    digitalWrite(COMMS_LED_PIN, HIGH);
    digitalWrite(COMMS_TX_LED_PIN, HIGH);
    digitalWrite(COMMS_RX_LED_PIN, HIGH);
    digitalWrite(DETECTOR_CHUTE_LED_PIN, HIGH);
    for (int i = 0; i < 8; i++) {
      digitalWrite(INDICATOR_PIN[i], HIGH);
    }
    digitalWrite(AMBIENT_WITHOUT_CHUTE_PIN, HIGH);
    digitalWrite(AMBIENT_WITH_CHUTE_PIN, HIGH);
    digitalWrite(TILE_REFLECTION_PIN, HIGH);
    digitalWrite(ERROR_LED_PIN, HIGH);
    digitalWrite(FAIL_LED_PIN, HIGH);
    digitalWrite(READY_LED_PIN, HIGH);
    delay(250);
  }
}


void communicate_status(String msg_type) {
    String msg;
    comm = HIGH;
    tx = HIGH;
    // "s" for status
  if (msg_type == "s") {
      msg = "ms";
      for (int i=0; i<8; i++) {
          msg += cartridge_present[i];
      }
      msg += "$";
  }
  
  // "j" for jam
  if (msg_type == "j") {
      msg = "mj";
      msg += last_chute_refilled;
      msg += "$";
      fail = HIGH;
      ready = LOW;
  }
  
  // "e" for empty
  if (msg_type == "e") {
      msg = "me";
      msg += first_empty_chute;
      msg += "$";
      fail = HIGH;
      ready = LOW;
   }
   update_LEDs();
   Serial.println(msg);
   tx = LOW;
   comm = LOW;
   digitalWrite(COMMS_LED_PIN, !comm);
   digitalWrite(COMMS_TX_LED_PIN, !tx);
}
   
      
void short_press() {
    button_press(true);
    setup_cartridges();
    ready = HIGH;
    fail = LOW;
    armed = true;
    last_chute_refilled = 8;
    communicate_status("s");
    button_press(true);
}


void long_press() {
    startup_check();
    setup_cartridges();
  setup_ambient_levels();
  communicate_status("s");
  button_press(true);
}


int scan_for_detector_chute() {
  int value = analogRead(CHUTE_SENS_PIN[8]);
  int state = LOW;
  long difference_ambient = abs(chute_sens_ambient[8] - value);
  long difference_empty = abs(chute_sens_empty[8] - value);
  if (difference_ambient < difference_empty) {
      state = LOW;
  }
  else {
      state = HIGH;
  }
//  Serial.println(value);
//  Serial.println(state);
//  Serial.println("___");
  return state;
}


int scan_for_empty_chute() {
  int value;
  int state = 9;
  long difference_empty;
  long difference_full;
  boolean compare_lower;
  for (int i = 0; i < 8; i++) {
    current_chute_scan[i] = LOW;
      if (cartridge_present[i] == HIGH) {
          value = analogRead(CHUTE_SENS_PIN[i]);
          difference_empty = abs(chute_sens_ambient[i] - value);
          difference_full = abs(chute_sens_full[i] - value);
          if (difference_empty < difference_full) {
              current_chute_scan[i] = HIGH;
              // State = 1st empty chute OR 9 if no chute OR 10 if more than 1 chute
              if (state == 9) {
                    state = i;
                }
                else {
                    state = 10;
                }
          }
      }
    
  }
  return state;
}


void refill_empty(int chute_number) {
  String msg;
  servoX[chute_number].write(170);
  delay(500);
  servoX[chute_number].write(5);
//  digitalWrite(COMMS_LED_PIN, LOW);
//  digitalWrite(COMMS_TX_LED_PIN, LOW);
//  // "u" for unload
//  msg = "mu";
//  msg += chute_number;
//  msg += "$";
//  Serial.println(msg);
//  digitalWrite(COMMS_LED_PIN, HIGH);
//  digitalWrite(COMMS_TX_LED_PIN, HIGH);
  delay(500);
}


void assign_pins(){
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

   for (int i=0; i<8; i++){
      servoX[i].attach(SERVO_PINS[i]);
      servoX[i].write(15);
    }
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  assign_pins();
  startup_check();
  Serial.println();
  setup_cartridges();
  update_LEDs();
  delay(2000);
  setup_ambient_levels();
  last_chute_refilled = 8;
  communicate_status("s");
  button_press(true);
}


void loop() {
  button_state = digitalRead(BUTTON_PIN);
  if (button_state == LOW) {
    time_switch_released = time_button_pressed(millis());
    if (time_switch_released > SHORT_PRESS_DURATION) {
      // soft restart
      long_press();
    }
    else {
      // pause, reevaluate state, communicate status, and pause
      short_press();
    }
  }

  // Check for errors

  if ((scan_for_detector_chute() == LOW) && !armed) {
    armed = true;
  }
  if ((scan_for_detector_chute() == HIGH) && armed) {
    armed = false;
    delay(500);
    first_empty_chute = scan_for_empty_chute();
//    Serial.println(first_empty_chute);
    
    // if no empty chute for cartridge present then gripper failed to pick up tile
    // or tile failed to slide down chute. communicate error
    if (first_empty_chute == 9) {
        communicate_status("j");
    }
    
    if (first_empty_chute < 9) {
      refill_empty(first_empty_chute);
      last_chute_refilled = first_empty_chute;
      
      // scan again after preset time and confirm the desired chute is refilled
      // if not, cartridge could be empty. communicate error
      delay(RESCAN_DELAY);
      first_empty_chute = scan_for_empty_chute();
      if (first_empty_chute != 9) {
          communicate_status("e");
      }
    }
  }
  delay(20);
}


