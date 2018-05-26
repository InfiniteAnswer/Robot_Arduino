// Include the Servo library 
#include <Servo.h> 

// Declare the Servo pins 2-13
int servoPins[] = {2, 3, 4, 5, 6, 7, 8, 9};

Servo Servo1; 
Servo allServos[2];
int myArray[2];
//myArray[0] = 2;
//allServos[0] = Servo1;
//allServos[0] = new Servo;
//allServos[1] = new Servo;

void setup() {
  // put your setup code here, to run once:
  myArray[0] = 1;
  allServos[0] = Servo1;
  allServos[0].attach(2); 

}

void loop() {
  // put your main code here, to run repeatedly:
  Servo1.write(100);
  delay(500);
    Servo1.write(25);
    delay(500);
}
