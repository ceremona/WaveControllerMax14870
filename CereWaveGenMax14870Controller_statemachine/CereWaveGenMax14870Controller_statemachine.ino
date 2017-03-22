/* Sine Wave Modulated PWM Generator 
 * C.  Davis  4/21/16
 *
 * Thanks to R. Wozniak, Ira, MichaelM and Jp3141 at PJRC Forums for help in optimizations
 * for more information, see the thread located here:
 * http://forum.pjrc.com/threads/26608-Frequency-Locked-Sine-Wave-Generator-Teensy-3-1
 *
 * This program only runs on Teensy 3.1 which has a true Digital to Analog Converter
 * Compile for 96 MHz  <Tools/CPU Speed: "96MHZ (overclock)">
 * Good for 0 to 180 Hz
 */

const int SOUND = 23;
const int DIR1 = 11;
const int DIR2 = 12;
//const int F2 = 4;
//const int F3 = 5;
//const int F4 = 6;
const int MAG1 = 9;
const int MAG2 = 10;
const int POT = 14;
const int STBY = 2;
//const int AUDIO = A6;
int potval=0,val1,val2;

String functions[] = {"trianglewave", "sinewave", "bowedwave", "cavedwave"};

int state=0;
float clock = 95.99899F;    // Clock Calibration Factor for my teensy 3.1 at 96 MHz <Best>
 //float twopi = 6.2831853F;
 //float sineFactor;
int steps = 1000;            // number of phase angle steps for sinewave generation
                             // a smaller number will here allow to go to higher frequency
                             // but will result in a steppier sine curve.
const float delays[]={.4,3,.8,.4,.9,.5,2,.6,1,.4};
const float amps[]={};
const float onoff[] = {0,4095};
int i=0,j=1,k=0;
float clock_steps = (clock * (float)steps * 0.5f);                        
float amplitude;                        
float freq, phaseshift;
float phasestep = 2*PI/((float)steps);
float period;
float sineAmplitude[2000]; //Initialize array 1.25 * steps. 
                            //This is allows both for 2PI radians of sine,
                            //and 2PI radians of cosine shifted + PI/2
float btriangleAmplitude[2000];  //Bowed triangle
float ctriangleAmplitude[2000];  //Caved triangle
float triangleAmplitude[2000];


void fillSine() {
  for (int i=0; i <= 999; i++){
    freq = (float)i*phasestep;
    sineAmplitude[i]=sin(freq);
    sineAmplitude[1000+i]=sineAmplitude[i];
    Serial.printf("%4d: %3f\n",i,sineAmplitude[i]);
  }
}

void fillBTriangle() {                           /* More like a bowed triangle */
  for (int i=0; i <= 999; i++){
    freq = i*phasestep;
    //Serial.println(phase);
    btriangleAmplitude[i]=sqrt(abs((i%500) -250))/15.81F;
    btriangleAmplitude[1000+i]=btriangleAmplitude[i];
    Serial.printf("%4d: %3f\n",i,btriangleAmplitude[i]);
  }
}
void fillCTriangle() {                           
  for (int i=0; i <= 511; i++){
    freq = i*phasestep;
    //Serial.println(phase);
    //ctriangleAmplitude[i]=sq(abs((i%500)));
    ctriangleAmplitude[i]=(float)(sq(i-256)%65537)/65536;
    ctriangleAmplitude[512+i]=ctriangleAmplitude[i];
    Serial.printf("%4d: %3f\n",i,ctriangleAmplitude[i]);
  }
}

void fillTriangle() {                         /* triangle wave*/
  for (int i=0; i <= 999; i++) {
    freq = i*phasestep;
    //Serial.println(phase);
    triangleAmplitude[i]=(i%1000/1000.0F);
    triangleAmplitude[1000+i]=triangleAmplitude[i];
    Serial.printf("%4d: %3f\n",i,triangleAmplitude[i]);
  }
}

int magState=LOW;
long previousMillis=0;
/*
void setup() {
  pinMode(DIR1, OUTPUT);
  pinMode(MAG1, OUTPUT);
  pinMode(DIR1,OUTPUT);
  pinMode(MAG2,OUTPUT);
//  pinMode(PWMA,OUTPUT);
//  pinMode(STBY,OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(POT, INPUT);
  analogWriteResolution(8);
  Serial.begin(9600);
  delay(1000);
  Serial.println();
  Serial.println("\n--Fill Sin--\n");
  fillSine();
  Serial.println("\n--Fill Bowed Triangle--\n");
  fillBTriangle();
  Serial.println("\n--Fill Curved Triangle--\n");
  fillCTriangle();
  Serial.println("\n--Fill Triangle--\n");
  fillTriangle();
  Serial.println("\n----------------End Wavetables-----------------\n");
  //pinMode(A0,INPUT);
 // analogWriteFrequency(MAG1,30000);
//  analogWriteFrequency(10,20000);
}
*/
float sinewave(int i) {
  amplitude = 2048.0F*(sineAmplitude[i])+2047.0F;
  //Serial.println(amplitude);
  return amplitude;
}
class ElectroMagnet
{
  int updateInterval;      // interval between updates
  unsigned long lastUpdate; // last update of position
  int magState;
  int directionPin;
  int i;
  int steps;
  unsigned long previousMillis;
  int myMotor;
  float strength;
  int dirState;
  int amp;
  int carrierfreq;

public: 
  ElectroMagnet(int stepDuration, int motorPin, int PWMCarrierFreq, int direction)
  {  
    updateInterval = stepDuration;
   // carrierfreq = PWMCarrierFreq;
    magState = LOW; 
    previousMillis = 0;
    i = 0;
    steps = 1000;
    //dirState = FORWARD;
    amp = 0;
    myMotor = motorPin;
    directionPin = direction;
    pinMode(motorPin, OUTPUT); 
    pinMode(directionPin, OUTPUT);
    dirState = 0;
    analogWriteFrequency(motorPin, PWMCarrierFreq);
  }

/*  
void Initialize(int motorPin, PWMCarrierFreq, direction) {
    myMotor = motorPin;
    directionPin = direction;
    pinMode(motorPin, OUTPUT); 
    pinMode(directionPin, OUTPUT);
    dirState = FORWARD;
    analogWriteFrequency(motorPin, PWMCarrierFreq);
}
*/
void Down()
  {
    //servo.detach();
  }
  
void Update()
  {
    long currentMillis = micros();
    long interval = currentMillis - previousMillis;  
    if (i >= steps) i = 0;
    amp = sinewave(i);
    if(amp==0) Serial.printf("%4d: %4d, %i \n",i ,amp, dirState);
    analogWrite(myMotor,0);
    if (i==500) {                  /*  State machine to make the motor alternate directions */ 
      //digitalWrite(LED_BUILTIN,HIGH);
      if(dirState==0) {           /*  once the pwm finishes it's cycle */
        digitalWrite(myMotor,1);  //reverse motor direction
          dirState=1;
      } 
      else if(dirState==1)  
      {
        digitalWrite(directionPin,0);
        dirState =0;
        return;
      }
    }
    analogWrite(myMotor,amp);   //set "speed"
    if(interval > updateInterval)   //step intervals which set waveform frequency
    {   // time to update
      //lastUpdate = micros();
      previousMillis = currentMillis;
//      Serial.println(i);
      if(magState == LOW) {
        magState = HIGH;
      } else {
        magState = LOW;
      }
      i++;
    }
  }
};
 
ElectroMagnet ElectroMagnet1(700,9,1000,0);  // step duration, motor pin number, pwmcarrierfreq, direction
ElectroMagnet ElectroMagnet2(700,10,2000,0);
 
void setup() 
{ 
  ElectroMagnet ElectroMagnet1(700,9,1000,0);  // step duration, motor pin number, pwmcarrierfreq, direction
  ElectroMagnet ElectroMagnet2(700,10,2000,0);
  analogWriteResolution(8);
  pinMode(LED_BUILTIN,OUTPUT);
  Serial.begin(9600);
  delay(500);
  Serial.println();
  Serial.println("\n--Fill Sin--\n");
  fillSine();
  Serial.println("\n--Fill Bowed Triangle--\n");
  fillBTriangle();
  Serial.println("\n--Fill Curved Triangle--\n");
  fillCTriangle();
  Serial.println("\n--Fill Triangle--\n");
  fillTriangle();
  Serial.println("\n----------------End Wavetables-----------------\n");
  delay(20);
//  ElectroMagnet1.Initialize(3,4000);  //pin, pwmfrequency
//  ElectroMagnet2.Initialize(4,1000);
     //ElectroMagnet1.Update(); 
} 
 
void loop() 
{ 
 // if(digitalRead(2) == HIGH) {
    ElectroMagnet1.Update(); 
    ElectroMagnet2.Update();
 // }
//  led1.Update();
//  led2.Update();
//  led3.Update();
}

/*
void loop() {
//  digitalWrite(STBY,HIGH);
//  period = (float)analogRead(POT)/1023.0f;      //get analog value from pots scale to between 0 and 1
    period=.22;
    elapsedMicros timePassed;
    long currentMillis = micros();
    long interval = currentMillis - previousMillis; 
    Serial.println(interval);
    //Serial.printf("%4d: %4d\n", i,amp1);
      if (i >= steps) i = 0; // j=random(0,2);k=random(0,2); 
      int amp1 = sinewave(i);
      //if (amp1 == 4095 || amp1 == 0) Serial.printf("%4d: %4d\n", i,amp1);
    if(timePassed >= 16000*period) //step intervals which set waveform frequency
    {
      Serial.println(interval);
      previousMillis = currentMillis;   
      if(magState == LOW) {
        magState = HIGH;
      } else {
        magState = LOW;
      }
      //if(amp1>2048) digitalWrite(LED_BUILTIN,HIGH); else digitalWrite(LED_BUILTIN,LOW);
      if (i==750) {                      //  State machine to make the motor alternate directions  
        if(state==0) {                   //  once the pwm finishes it's cycle 
          digitalWrite(DIR1,HIGH);
          digitalWrite(LED_BUILTIN,HIGH);
          state=1;
        }  
        else if(state==1) {
          digitalWrite(DIR1,LOW);
          digitalWrite(LED_BUILTIN,LOW);
          state=0;
        }
      }
      //Serial.printf("%4d: %4d\n", i,amp1);
      analogWrite(MAG1, amp1);
      //magState = HIGH;  // Set off state
      i++;
    }
}
*/

float trianglewave(int i) {
  amplitude = 4095.0F*(triangleAmplitude[i]);
  //Serial.println(amplitude);
  return amplitude;
}

float bowedwave(int i) {
  amplitude = 4095.0F*(btriangleAmplitude[i]);
  //Serial.println(amplitude);
  return amplitude;
}

float cavedwave(int i) {
  amplitude = 4095.0F*(ctriangleAmplitude[i]);
  //Serial.println(amplitude);
  return amplitude;
}

