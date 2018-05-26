// Include the Servo library 
#include <Servo.h> 

// Declare pins
int FAIL_RETURN_PIN = 12;
int SERVO_PIN = 10;
int BIT_0_PIN = 9;
int BIT_1_PIN = 8;
int FAIL_SWITCH_PIN = 4;
int LED_WIDE_PIN = 7;
int LED_OPEN_PIN = 6;
int LED_CLOSED_PIN = 5;
int LED_FAIL_PIN = 2;
int LED_GOOD_PIN = 3;
int SIG_0_PIN = A0;
int SIG_1_PIN = A1;

// Declare constants
int servo_wide = 80;
int servo_open = 100;
int servo_closed = 125;
int upper_threshold_SIG_0 = 100;
int lower_threshold_SIG_0 = 100;
int upper_threshold_SIG_1 = 100;
int lower_threshold_SIG_1 = 100;
unsigned long RESET_PRESET_TIME = 3000;
boolean ENABLE_SERIAL_MONITOR = false;

// Declare variables
int servo_angle = servo_closed;
long average_SIG_0 = 0;
long average_SIG_1 = 0;
int LED_wide = LOW;
int LED_open = LOW;
int LED_closed = LOW;
int LED_good = LOW;
int LED_fail = LOW;
int bit_0 = LOW;
int bit_1 = LOW;
int bit_0_old = LOW;
int bit_1_old = LOW;
int sw_state = HIGH;
int SIG_0;
int SIG_1;
int temp_SIG_0;
int temp_SIG_1;
boolean triggered_0 = false;
boolean triggered_1 = false;
boolean switch_pressed = false;
unsigned long time_switch_pressed;
unsigned long time_switch_released;
Servo Servo1; 


void update_LEDs(){
  digitalWrite(LED_WIDE_PIN, LED_wide);
  digitalWrite(LED_OPEN_PIN, LED_open);
  digitalWrite(LED_CLOSED_PIN, LED_closed);
  digitalWrite(LED_FAIL_PIN, LED_fail);
  digitalWrite(LED_GOOD_PIN, LED_good);
}


void setup() {  
// open the serial port at 9600 bps:
  if (ENABLE_SERIAL_MONITOR) Serial.begin(9600);  
  
  Servo1.attach(SERVO_PIN);
  pinMode(BIT_0_PIN, INPUT);
  pinMode(BIT_1_PIN, INPUT);
  pinMode(FAIL_SWITCH_PIN, INPUT);
  pinMode(FAIL_RETURN_PIN, OUTPUT);
  pinMode(LED_GOOD_PIN, OUTPUT);
  pinMode(LED_FAIL_PIN, OUTPUT);
  pinMode(LED_WIDE_PIN, OUTPUT);
  pinMode(LED_OPEN_PIN, OUTPUT);
  pinMode(LED_CLOSED_PIN, OUTPUT);
  digitalWrite(FAIL_RETURN_PIN, LOW);

// Cycle through 3 flashing cycles of good-fail LEDs
  for (int i=0; i < 3; i++){
    LED_fail = LOW;
    LED_good = HIGH;
    update_LEDs();
    delay(500);
    LED_fail = HIGH;
    LED_good = LOW;
    update_LEDs();
    delay(500); 
  }
  LED_fail = LOW;
  
// Cycle through 3 wide-open-close cylcles
  for (int i=0; i < 3; i++){
    LED_wide = HIGH;
    LED_open = LOW;
    LED_closed = LOW;
    update_LEDs();
    Servo1.write(servo_wide);
    delay(500);
    LED_wide = LOW;
    LED_open = HIGH;
    LED_closed = LOW;
    update_LEDs();
    Servo1.write(servo_open);
    delay(500);
    LED_wide = LOW;
    LED_open = LOW;
    LED_closed = HIGH;
    update_LEDs();
    Servo1.write(servo_closed);
    delay(500);
  }


// Set analag DAQ speed to high


// Perform averaging of SIG_0 over 2 seconds
  int sample_interval = 50;
  int sample_time = 2000;
  int fail_LED_flash_rate = 2; // Hz
  int fail_LED_flash_rate_intervals = 1000 / fail_LED_flash_rate / sample_interval;
  int number_samples = sample_time / sample_interval;
  average_SIG_0 = 0;
  for (int i=0; i < number_samples; i++){
    average_SIG_0 += analogRead(SIG_0_PIN);
    if (i % fail_LED_flash_rate_intervals == 0){
      LED_fail = HIGH;
      LED_good = LOW;
      update_LEDs();
    }
    else if (i % (fail_LED_flash_rate_intervals / 2) == 0){
      LED_fail = LOW;
      LED_good = HIGH;
      update_LEDs();
    }
    delay(sample_interval);
  }
  average_SIG_0 /= number_samples;

// Perform averaging of SIG_1 over 2 seconds
  sample_interval = 50;
  sample_time = 2000;
  fail_LED_flash_rate = 4; // Hz
  fail_LED_flash_rate_intervals = 1000 / fail_LED_flash_rate / sample_interval;
  number_samples = sample_time / sample_interval;
  average_SIG_1 = 0;
  for (int i=0; i < number_samples; i++){
    average_SIG_1 += analogRead(SIG_1_PIN);
    if (i % fail_LED_flash_rate_intervals == 0){
      LED_fail = HIGH;
      LED_good = LOW;
      update_LEDs();
    }
    else if (i % (fail_LED_flash_rate_intervals / 2) == 0){
      LED_fail = LOW;
      LED_good = HIGH;
      update_LEDs();
    }
    delay(sample_interval);
  }
  average_SIG_1 /= number_samples;

// Set LED good to indicate that setup is completed successfully
//  update_LEDs(LOW, LOW, HIGH, LOW, HIGH);
  LED_fail = LOW;
  LED_good = HIGH;
  digitalWrite(FAIL_RETURN_PIN, LOW);
  update_LEDs();  
  switch_pressed = false;
}


void loop() {
// Read values from 2 collision sensors
    SIG_0 = analogRead(SIG_0_PIN);
    SIG_1 = analogRead(SIG_1_PIN);
    for (int i=0; i<10; i++){
        if ((SIG_0 > (upper_threshold_SIG_0 + average_SIG_0)) ||
            (SIG_0 < (average_SIG_0 - lower_threshold_SIG_0))) {
            SIG_0 = analogRead(SIG_0_PIN);
            delay(10);
        }
        if ((SIG_1 > (upper_threshold_SIG_1 + average_SIG_1)) ||
            (SIG_1 < (average_SIG_1 - lower_threshold_SIG_1))) {
            SIG_1 = analogRead(SIG_1_PIN);
            delay(10);
        }  
    }  
    
//  temp_SIG_0 = analogRead(SIG_0_PIN);
//  temp_SIG_1 = analogRead(SIG_1_PIN);
//  if (temp_SIG_0 > SIG_0) {SIG_0 = temp_SIG_0;}
//  if (temp_SIG_1 > SIG_1) {SIG_1 = temp_SIG_1;}
  
// Compare collision sensor values with threshold triggers
  triggered_0 = ((SIG_0 > (upper_threshold_SIG_0 + average_SIG_0)) ||
      (SIG_0 < (average_SIG_0 - lower_threshold_SIG_0)));
  triggered_1 = ((SIG_1 > (upper_threshold_SIG_1 + average_SIG_1)) ||
      (SIG_1 < (average_SIG_1 - lower_threshold_SIG_1))); 

// Check to see if in diagnostics mode
  if (ENABLE_SERIAL_MONITOR){
    Serial.println(SIG_0);
    Serial.println(SIG_1);
    Serial.println(triggered_0);
    Serial.println(triggered_1);
    Serial.println();
    delay(500);
  }

//Evaluated if collision has occurred
    if (triggered_0 || triggered_1){
        // digitalWrite(FAIL_RETURN_PIN, HIGH);
        LED_good = LOW;
        LED_fail = HIGH;
        digitalWrite(FAIL_RETURN_PIN, LED_fail);
        update_LEDs();
    }
    
    if (digitalRead(FAIL_SWITCH_PIN) == LOW){
        time_switch_pressed = millis();
        switch_pressed = true;
        while (digitalRead(FAIL_SWITCH_PIN) == LOW) {
            delay(50);
        }
    time_switch_released = millis();
    }
    
    if (switch_pressed) {
        // Check to see if switch is held for 3 seconds indicating RESET request
        // Check for 50 day (32 bit unsigned long) overflow
        if (time_switch_released < time_switch_pressed){
            time_switch_pressed -= pow(2, 15);
            time_switch_released += pow(2, 15);
        }
        // Compare long press duration with reset preset time
        if ((time_switch_released - time_switch_pressed) > RESET_PRESET_TIME){
            Serial.end();
            setup();
        }
        LED_good = HIGH;
        LED_fail = LOW;
        update_LEDs();
        digitalWrite(FAIL_RETURN_PIN, LOW);
    }

    // Update servo ONLY if commanded to go to new position
    bit_0 = digitalRead(BIT_0_PIN);
    bit_1 = digitalRead(BIT_1_PIN);
    if ((bit_0 != bit_0_old) || (bit_1 != bit_1_old)){
        bit_0_old = bit_0;
        bit_1_old = bit_1;   
        if ((bit_0 == LOW) && (bit_1 == LOW)){
            LED_wide = LOW;
            LED_open = LOW;
            LED_closed = HIGH;
            servo_angle = servo_closed;
        }
        if ((bit_0 == LOW) && (bit_1 == HIGH)){
            LED_wide = LOW;
            LED_open = HIGH;
            LED_closed = LOW;
            servo_angle = servo_open;
        }
        if ((bit_0 == HIGH) && (bit_1 == HIGH)){
            LED_wide = HIGH;
            LED_open = LOW;
            LED_closed = LOW;
            servo_angle = servo_wide;
        }
        update_LEDs();
        Servo1.write(servo_angle);
    }
}
