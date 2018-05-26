// Arduino MEGA 2560
// Victor Samper
// sampervictor@gmail.com
// 10 April 2018
// V. 1.1.0

// Include the Servo library 
#include <Servo.h> 


// Declare pins
// servo pins for SERVO0 - SERVO7
int SERVO_PINS[] = { 2, 3, 4, 5, 6, 7, 8, 9
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
int READY_LED_PIN = 47;
int FAIL_LED_PIN = 48;
int ERROR_LED_PIN = 49;

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
int SENSOR_TRIGGER_UPPER_DELTA = 20;
int SENSOR_TRIGGER_LOWER_DELTA = 10;
unsigned long RESET_PRESET_TIME = 3000;
boolean ENABLE_SERIAL_MONITOR = false;


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
int indicatorX[8];
unsigned long time_switch_released;
int tx = LOW;
int rx = LOW;
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


unsigned long button_press() {
	int timing[6][4] = {
		{100, true, false, false},
		{200, false, true, false},
		{300, false, false, true},
		{500, false, false, false},
		{700, false, false, true},
		{900, false, false, false}
	};

	int time_start = millis();
	int timing_index = 0;
	int button_state = digitalRead(BUTTON_PIN);
	while (button_state == HIGH) {
		digitalWrite(READY_LED_PIN, timing[timing_index][1]);
		digitalWrite(FAIL_LED_PIN, timing[timing_index][2]);
		digitalWrite(ERROR_LED_PIN, timing[timing_index][3]);
		if (time_passed(time_start) > timing[timing_index][0]) {
			timing_index++;
		}
		if (timing_index == 6) {
			timing_index = 0;
		}
		button_state = digitalRead(BUTTON_PIN);
		delay(10);
	}
	while (button_state == LOW) {
		button_state = digitalRead(BUTTON_PIN);
		delay(10);
	}
	return time_passed(time_start);
}

void setup_ambient_levels() {
		unsigned long start_time = millis();
		unsigned long PERIOD = 500;
		int SAMPLE_TIME = 2000;
		int number_measurements = 0;
		unsigned long running_total;
		unsigned long measurement_time;
		int LED_state = HIGH;

		// Communicate mode to Main Controller
		digitalWrite(ERROR_PIN, LOW); 
		digitalWrite(READY_PIN, LOW); 

		// Flash READY/FAIL/ERROR LEDs to indicate waiting for button press
		// Send (TRUE) to button_press() to select this mode
		button_press(true);

		// indicate that measuring the ambient without chute by flashing the LED
		// cycle through the 9 measurements, recording signal for 2 seconds each time, then averaging and storing
		running_total = 0;
		for (int i = 0; i < 9; i++) {
			measurement_time = millis();
			while (time_passed(measurement_time) < sample_time) {
				running_total += analogRead(CHUTE_SENS_PIN[i]);
				number_measurements++;
				if (flash_state(start_time, period) == 0) {
					LED_state = LOW;
				}
				else
				{
					LED_state = HIGH;
				}
				digitalWrite(AMBIENT_WITHOUT_CHUTE_PIN, LED_state);
				delay(50);
			}
			chute_sens_ambient[i] = running_total / number_measurements;
		}

		// indicate that ambient without chute measurement is complete with solid LED
		digitalWrite(AMBIENT_WITHOUT_CHUTE_PIN, LOW);

		// wait for button press before advancing
		button_press();

		// indicate that measuring the ambient with chute by flashing the LED
		// cycle through the 9 measurements, recording signal for 2 seconds each time, then averaging and storing
		running_total = 0;
		for (int i = 0; i < 9; i++) {
			measurement_time = millis();
			while (time_passed(measurement_time) < sample_time) {
				running_total += analogRead(CHUTE_SENS_PIN[i]);
				number_measurements++;
				digitalWrite(AMBIENT_WITH_CHUTE_PIN, flash_state(start_time, period));
				delay(50);
			}
			chute_sens_empty[i] = running_total / number_measurements;
		}
		// indicate that ambient with chute measurement is complete with solid LED
		digitalWrite(AMBIENT_WITH_CHUTE_PIN, LOW);

		// wait for button press before advancing
		button_press();

		// indicate that measuring the reflected tile level by flashing the LED
		// cycle through the 8 measurements, recording signal for 2 seconds each time, then averaging and storing
		for (int i = 0; i < 9; i++) {
			measurement_time = millis();
			while (time_passed(measurement_time) < sample_time) {
				running_total += analogRead(CHUTE_SENS_PIN[i]);
				number_measurements++;
				digitalWrite(TILE_REFLECTION_PIN, flash_state(start_time, period));
				delay(50);
			}
			chute_sens_full[i] = running_total / number_measurements;
		}

		// indicate that reflected tile level measurement is complete with solid LED
		digitalWrite(TILE_REFLECTION_PIN, LOW);

		// wait for button press before advancing
		button_press();
		digitalWrite(READY_PIN, LOW);

		// exit setup mode
		digitalWrite(ERROR_PIN, LOW); // control line to TT
		digitalWrite(READY_PIN, HIGH); // control line to TT
	}

}


void communicate_status() {

}


void button_pressed() {

}


void short_press() {

}


void long_press() {

}


void chute_detected() {

}


unsigned long time_passed() {

}


void update_cartridge_status() {

}


void setup() {
	// put your setup code here, to run once:

}

void loop() {
	// put your main code here, to run repeatedly:

}
