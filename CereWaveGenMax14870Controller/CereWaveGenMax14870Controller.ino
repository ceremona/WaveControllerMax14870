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
 * 
 * For the Max14870 motor controller/driver
 */

const int SOUND = 23;
const int DIR1 = 11;
const int DIR2 = 12;
//const int F2 = 4;
//const int F3 = 5;
//const int F4 = 6;
const int PWM1 = 9;
const int PWM2 = 10;
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

void setup() {
  pinMode(DIR1,OUTPUT);
  pinMode(PWM1,OUTPUT);
  pinMode(DIR2,OUTPUT);
  pinMode(PWM2,OUTPUT);
//  pinMode(PWMA,OUTPUT);
//  pinMode(STBY,OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(POT, INPUT);
  analogWriteResolution(12);
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
  analogWriteFrequency(PWM1,50000);
//  analogWriteFrequency(10,20000);
}


void loop() {
//  digitalWrite(STBY,HIGH);
//  period = (float)analogRead(POT)/1023.0f;      //get analog value from pots scale to between 0 and 1
  period=.22;
  i = i + 1;
  
  if (i==750) {                      /*  State machine to make the motor alternate directions */ 
    if(state==0) {                   /*  once the pwm finishes it's cycle */
      digitalWrite(DIR1,HIGH);
      state=1;
    }  
    else if(state==1) {
      digitalWrite(DIR1,LOW);
      state=0;
    }
  }

  if (i >= steps) {i = 0; /* j=random(0,2);k=random(0,2); */ }
  int amp1 = sinewave(i);
//  int amp2 = bowedwave(i);
  if(amp1>2048) digitalWrite(LED_BUILTIN,HIGH); else digitalWrite(LED_BUILTIN,LOW);

  if (amp1 == 4095 || amp1 == 0) Serial.printf("%4d: %4d\n", i,amp1);

    analogWrite(PWM1,amp1);
    delayMicroseconds(16000*period);
}

void fuzzballs() {
}

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

float sinewave(int i) {
  amplitude = 2048.0F*(sineAmplitude[i])+2047.0F;
  //Serial.println(amplitude);
  return amplitude;
}
