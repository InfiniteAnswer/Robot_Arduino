#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11); // RX, TX

unsigned long time;
int incomingByte = 0;
char character;
String a;
int r = 0;


int RX_PIN = 0;
int TX_PIN = 1;
int MAG_FAIL_PIN = 2;
int MAG_READY_PIN = 3;
int MAG_ERROR_PIN = 4;
int GRI_BIT1_PIN = 6;
int GRI_BIT0_PIN = 5;
int GRI_FAIL_PIN = 7;
int GRI_UNUSED_PIN = 8;
int MAG_UNUSED_PIN = 9;

int rx;
int tx;
int my_rx;
int my_tx;
int mag_fail;
int gri_bit1;
int gri_bit0;
int gri_fail;
String msg;

void setup() {
  delay(1000);
  Serial.begin(38400);
  mySerial.begin(38400);

//  pinMode(RX_PIN, INPUT);
//  pinMode(TX_PIN, OUTPUT);
  pinMode(MAG_FAIL_PIN, INPUT);
  pinMode(MAG_READY_PIN, INPUT);
  pinMode(MAG_ERROR_PIN, INPUT);
  pinMode(GRI_BIT1_PIN, OUTPUT);
  pinMode(GRI_BIT0_PIN, OUTPUT);
  pinMode(GRI_FAIL_PIN, INPUT);
  pinMode(GRI_UNUSED_PIN, INPUT);
  pinMode(MAG_UNUSED_PIN, INPUT);

  time = millis();
}


void loop() {
    mag_fail = digitalRead(MAG_FAIL_PIN);
    gri_fail = digitalRead(GRI_FAIL_PIN);
    
    // Error or fail?
    if (mag_fail) {
        msg = "mf0$";
        Serial.println(msg);
    }
    if (gri_fail) {
        msg = "gf0$";
        Serial.println(msg);
    }
    
    // Gripper command?
    if (Serial.available()) {
        msg = Serial.readStringUntil('\n');  //[:-2];
        if (msg == "mgw$") {
            gri_bit0 = HIGH;
            gri_bit1 = HIGH;
        }
        if (msg == "mgo$") {
            gri_bit0 = LOW;
            gri_bit1 = HIGH;
        }
        if (msg == "mgc$") {
            gri_bit0 = LOW;
            gri_bit1 = LOW;
        }
        digitalWrite(GRI_BIT0_PIN, gri_bit0);
        digitalWrite(GRI_BIT1_PIN, gri_bit1);
    }
    
    // Mag message?
    if (mySerial.available()) {
        msg = mySerial.readStringUntil('\n');
        Serial.println(msg);
    }
}

