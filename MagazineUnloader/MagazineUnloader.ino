void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}


/*
 * Concept for the program is:
 * 1. each chute has its own LED + LDR
 * 2. there is one stand-alone chute detector LED + LDR
 * 3. each LED flashes to eliminate ambient interference
 * 4. LED flash frequency does not eliminate LED-LED x-talk
 * 5. maximum of 16 LED + LDR chutes
 * 6. total of max 17 LED + LDRs
 * 7. Question... does every LDR go to an An-IP or can it go to Di-IP?
 */


// Include the Servo library 
#include <Servo.h> 
// Declare the Servo pin 
int servoPin = 3; 

int LEDpins[] = {52, 48};
int LDRpins[] = {0, 2};
int servoLEDIndicatorPin = 44;
int LEDstates[] = {0, 0};
int LDRvals[] = {0, 0};
int averageAmbient[] = {0, 0};
long LEDpulseTime[] = {7e3, 9e3};
long averagingLDRTime = 250e3;
int prvLDRVal[] = {0, 0};
int currLDRVal[] = {0, 0};
long pulses[] = {0, 0};
int freq[] = {0, 0};
int LEDS = 2;
long times[] = {0, 0};
int noise = 40; // 300 for 200Hz+ 200 for 5-20mm standoff or higher freq (100Hz), 100 for longer distance
long startAveragingTimes[] = {0, 0}; // time when averaging starts 
long ambient[] = {0, 0};

int machineState = 0;
int chuteState = 0;
int servoAngle1 = 0;
int servoAngle2 = 180;
int paletteStopTime = 200; //ms
Servo Servo1; 
int trustMeasurement = 0;

void setup() {
    Serial.begin(115200);  

    // Create a servo object and attach to pin
    Servo1.attach(servoPin); 

    pinMode(servoLEDIndicatorPin, OUTPUT);
    digitalWrite(servoLEDIndicatorPin, 0);
    
    for (int i=0; i<LEDS; i++){
      pinMode(LEDpins[i], OUTPUT);
      digitalWrite(LEDpins[i], 0);   
    }

    // zero LDRs for ambient light level
    int averagingTime = 5000; // averaging time in ms
    int samples = 1000; // number of samples in the averaging time window
    int sampleDelay = averagingTime / samples;

//    for (int l=0; l<LEDS; l++){
//      for (int i = 0; i<1000; i++){
//        ambient[l] += analogRead(LDRpins[l]);
//        delay(sampleDelay);            
//      } 
//      String msg = "Completed ambient evaluation for LED: " + String(l);
//      Serial.println(msg);   
//      Serial.print("1000x ambient = ");
//      Serial.println(ambient[l]);
//      Serial.print("calculated ambient = "); 
//      averageAmbient[l] = ambient[l] / samples;
//      Serial.println(averageAmbient[l]);
//      Serial.println();
//    }
    for (int l=0; l<LEDS; l++){
      times[l] = micros();
      startAveragingTimes[l] = micros();
    }
    Serial.println("Collecting 900 flashes");
    int sampleNumber = 900;
    int stabilisationNumber = 50000;
    int samples0[sampleNumber];
    int samples1[sampleNumber];
    int counter = 0;
    for (int i = -3000; i<900; i++){
      ledControl();
      if (i>=0){
        samples0[counter] = analogRead(LDRpins[0]);
        samples1[counter] = analogRead(LDRpins[1]);
        counter ++;
      }
      delay(1);
    }
    //average lower 300 samples
    Serial.println("Sorting 900 samples");
    int swaps = 0;
    int temp = 0;
    for (int outeri = 0; outeri<sampleNumber; outeri++){
      for (int i = 0; i<(sampleNumber-1); i++){
        if (samples0[i+1]>samples0[i]){
          temp = samples0[i];
          samples0[i] = samples0[i+1];
          samples0[i+1] = temp;
          swaps += 1;
        }
        if (samples1[i+1]>samples1[i]){
          temp = samples1[i];
          samples1[i] = samples1[i+1];
          samples1[i+1] = temp;
          swaps += 1;
        } 
      }
      if (swaps==0) break;
      swaps=0;
    }
    for (int i=0;i<100;i++){
      Serial.print("Sample: ");
      Serial.println(samples0[i]);
      Serial.println(samples1[i]);
    }
    Serial.println("Averaging lower 300 samples");
    long sum0=0;
    long sum1=0;
    for (int i=0; i<(100); i++){
      sum0 += samples0[i];
      sum1 += samples1[i];  
    }
    Serial.println(sum1);
    averageAmbient[0] = sum0/(100);
    averageAmbient[1] = sum1/(100);
    String msg = "Average LOW ambient evaluation for LED0: " + String(averageAmbient[0]);
    Serial.println(msg); 
    msg = "Average LOW ambient evaluation for LED1: " + String(averageAmbient[1]);
    Serial.println(msg); 

    for (int l=0; l<LEDS; l++){
      times[l] = micros();
      startAveragingTimes[l] = micros();
    }
}



void loop() {
      getUpdate();
      String msg = "Machine state: " + String(machineState)+ " Chute state: " + String(chuteState);
      msg += " Restart cycle: " + String(trustMeasurement);
      Serial.println(msg);  

      if (trustMeasurement == 1){
        if (machineState == 1) {
          delay(paletteStopTime);
//          Serial.println("Got passed 1 delay");
          if (chuteState == 0){
            digitalWrite(servoLEDIndicatorPin, 1);
            Servo1.write(servoAngle1);
            delay(500);
//            Serial.println("Got passed 2 delays");
            Servo1.write(servoAngle2);
            chuteState = 1;
            trustMeasurement = 4;
            digitalWrite(servoLEDIndicatorPin, 0);
          }
          else trustMeasurement = 0;
        }
    }
}

  // machine can be in potential load state or wait state
  // when in load state, machine decides to cycle servo over 180° and back (or not)
  // wait state occurs when LDR0 is off
  // load state occurs when LDR0 is on
  // if LDR1 is on then servo actuates
  // if LDR1 is off then servo does not actuate
  // servo can only actuate in the load state



// Calculate number of microseconds passed since last time LED toggled ON-OFF or OFF-ON
// If time passed is > than pulseDuration, toggle LED state
void ledControl() {
  for (int l=0; l<LEDS; l++){
    if ((micros()-times[l])>LEDpulseTime[l]){
      times[l] = micros();
      if (LEDstates[l] == 0){
        LEDstates[l] = 1;
      }
      else {
        LEDstates[l] = 0;
      }
    }
  digitalWrite(LEDpins[l], LEDstates[l]);
  }
}

// Reject noise and adjust sensitivity
// Higher the <noise> value, the shorter the detection range
void thresholdInput() {
  for (int l=0; l<LEDS; l++){
    if (LDRvals[l]<(-noise)){
      LDRvals[l] = 0;
      }
    else {
      LDRvals[l] = 1;
      }
  }
}

void countToggles() {
  for (int l=0; l<LEDS; l++){
    if (LDRvals[l] != prvLDRVal[l]){
      pulses[l] += 1;
  //    Serial.println("detected");
      prvLDRVal[l] = LDRvals[l];
    }  
  }
}

void calcAverages() {
  if ((micros()-startAveragingTimes[0])>averagingLDRTime){
    for (int l=0; l<LEDS; l++){
      pulses[l] /= 2;
      freq[l] = pulses[l] * 1e6 / averagingLDRTime;
      String msg = "Completed ambient evaluation for LED: " + String(l);
      Serial.println(msg);
      msg = "Raw pulses counted: " + String(pulses[l]);
      Serial.println(msg);
      Serial.print("frequency = ");
      Serial.println(freq[l]);
      Serial.print("Trust:");
      Serial.println(trustMeasurement);

      // trust measurement?
//      if ((trustMeasurement ==1) || (trustMeasurement == 3)) {
        if ((trustMeasurement !=0) && (trustMeasurement !=4)){
        if (freq[l] >10){
          Serial.println("reflected");
          if (l==0) machineState = 1;
          if (l==1) chuteState = 1;          
         }
        else {
          Serial.println("not reflected");
          if (l==0) {
            machineState = 0;
//            if (trustMeasurement == 4)trustMeasurement=2;
          }
          if (l == 1) chuteState = 0;
        }    
      }
      pulses[l] = 0;
      startAveragingTimes[l] = micros();
    }
    if ((machineState == 0) && (trustMeasurement ==3)) trustMeasurement = 1;
    if (trustMeasurement == 2) trustMeasurement = 1;
    if (trustMeasurement == 0) trustMeasurement = 2;
    if (trustMeasurement == 4) trustMeasurement = 3;
    
  }
}

void getUpdate(){
  ledControl();
  for (int l=0; l<LEDS; l++){
    LDRvals[l] = analogRead(LDRpins[l]) - averageAmbient[l];  
//    Serial.println(LDRvals[l]);
  }
//  Serial.println();
  thresholdInput();

  countToggles();
  calcAverages();
}

