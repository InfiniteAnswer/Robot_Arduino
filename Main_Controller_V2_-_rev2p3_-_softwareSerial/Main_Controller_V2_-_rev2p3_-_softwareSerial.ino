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
int GRI_FAIL_PIN = 8;
int GRI_UNUSED_PIN = 7;
int MAG_UNUSED_PIN = 9;

int NUMBER_REPEATS = 10;

int rx;
int tx;
int my_rx;
int my_tx;
int mag_fail;
int gri_bit1;
int gri_bit0;
int gri_fail;
String msg;
boolean gri_msg_sent = false;
boolean mag_msg_sent = false;

void setup() {
  delay(1000);
  Serial.begin(38400);
  Serial.setTimeout(50);
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
    
//     Error or fail?
    if (mag_fail && !mag_msg_sent) {      
        msg = "mf0$";
        Serial.println(msg);
        mag_msg_sent = true;
    }
    
    if (gri_fail && !gri_msg_sent) {
        delay(50); // attempt to avoid false alarms from spikes on lines
        for (int i=0; i < NUMBER_REPEATS; i++){
          if (digitalRead(GRI_FAIL_PIN) == LOW) {
            gri_fail = LOW;
          }
          delay(10);
        }
        if (gri_fail) {
          msg = "gf0$";
          Serial.println(msg);
          gri_msg_sent = true;
        }
    }

    if (!mag_fail) {
      mag_msg_sent = false;
    }

    if (!gri_fail) {
      gri_msg_sent = false;
    }
    
    // Gripper command?
    msg = "";
    while (Serial.available()) {
        // msg = Serial.readStringUntil('\n');  //[:-2];
        msg += Serial.readString();
    }
    if (msg.length() != 0) {
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
//    Serial.print(millis());
//    Serial.print(',');
//    Serial.print(mag_fail);
//    Serial.print(',');
//    Serial.println(gri_fail);
//    Serial.println();
      delay(50);
}

