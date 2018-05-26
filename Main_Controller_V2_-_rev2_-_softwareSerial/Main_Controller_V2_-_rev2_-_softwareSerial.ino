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
        if (Serial.available() > 0) {
                // read the incoming byte:
//                incomingByte = Serial.read();
character = Serial.read();

                // say what you got:
//                Serial.print("I received: ");
//                Serial.println(incomingByte, DEC);
//a= Serial.readString();// read the incoming data as string
//
//Serial.println(a); 
Serial.print(character);
        }
if ((millis()-time) > 3000){
  time=millis();
  if ((r%3) == 0) {
    digitalWrite(GRI_BIT0_PIN, LOW);
    digitalWrite(GRI_BIT1_PIN, LOW);
  }
  if ((r%3) == 1) {
    digitalWrite(GRI_BIT0_PIN, LOW);
    digitalWrite(GRI_BIT1_PIN, HIGH);
  }
  if ((r%3) == 2) {
    digitalWrite(GRI_BIT0_PIN, HIGH);
    digitalWrite(GRI_BIT1_PIN, HIGH);
  }
  r++;
  if (r == 3) {r=0;}
}               
}
