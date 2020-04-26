#include <TimerOne.h>
#include <avr/pgmspace.h>
#define DcoResetPin 9
#define ControlVoltage A1
#define chargeVoltagePin 3
#define ControlSourcePin 4
#define maxFreqSet 7902.13
#define maxAnOutVolt 4.736
#define MAFLENGTH 20 //Moving Average Filter sample length
int CV[MAFLENGTH]; //CV buffer 
float freq = 261.63;
const float PROGMEM notes[] = {8.18, 8.66, 9.18, 9.72, 10.3, 10.91, 11.56, 12.25, 12.98, 13.75, 14.57, 15.43, 16.35, 17.32, 18.35, 19.45, 20.6, 21.83, 23.12, 24.5, 25.96, 27.5, 29.14, 30.87, 32.7, 34.65, 36.71, 38.89, 41.2, 43.65, 46.25, 49, 51.91, 55, 58.27, 61.74, 65.41, 69.3, 73.42, 77.78, 82.41, 87.31, 92.5, 98, 103.83, 110, 116.54, 123.47, 130.81, 138.59, 146.83, 155.56, 164.81, 174.61, 185, 196, 207.65, 220, 233.08, 246.94, 261.63, 277.18, 293.66, 311.13, 329.63, 349.23, 369.99, 392, 415.3, 440, 466.16, 493.88, 523.25, 554.37, 587.33, 622.25, 659.26, 698.46, 739.99, 783.99, 830.61, 880, 932.33, 987.77, 1046.5, 1108.73, 1174.66, 1244.51, 1318.51, 1396.91, 1479.98, 1567.98, 1661.22, 1760, 1864.66, 1975.53, 2093, 2217.46, 2349.32, 2489.02, 2637.02, 2793.83, 2959.96, 3135.96, 3322.44, 3520, 3729.31, 3951.05, 4186.01, 4434.92, 4698.64, 4978.03, 5274.04,5587.65, 5919.91, 6271.93, 6644.88, 7040, 7458.62, 7902.13};
const float PROGMEM calcPeriod[] = {122249.388,115473.44,108932.46,102880.66,97087.38,91659.03,86505.19,81632.65,77041.60,72727.27,68634.18,64808.81,61162.08,57736.72,54495.91,51413.88,48543.69,45808.52,43252.60,40816.33,38520.80,36363.64,34317.09,32393.91,30581.04,28860.03,27240.53,25713.55,24271.84,22909.51,21621.62,20408.16,19264.11,18181.82,17161.49,16196.95,15288.18,14430.01,13620.27,12856.78,12134.45,11453.44,10810.81,10204.08,9631.13,9090.91,8580.74,8099.13,7644.68,7215.53,6810.60,6428.39,6067.59,5727.05,5405.41,5102.04,4815.8,4545.45,4290.31,4049.57,3822.19,3607.76,3405.30,3214.09,3033.70,2863.44,2702.78,2551.02,2407.90,2272.73,2145.19,2024.78,1911.13,1803.85,1702.62,1607.07,1516.85,1431.72,1351.37,1275.53,1203.93,1136.36,1072.58,1012.38,955.57,901.93,851.31,803.53,758.43,715.87,675.68,637.76,601.97,568.18,536.29,506.19,477.78,450.97,425.66,401.76,379.22,357.93,337.84,318.88,300.98,284.09,268.15,253.10,238.89,225.48,212.83,200.88,189.61,178.97,168.92,159.44,150.49,142.05,134.07,126.55};
const int PROGMEM chargePWM[] = {0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,3,3,3,3,3,3,4,4,4,4,4,5,5,5,6,6,6,7,7,8,8,8,9,9,10,11,11,12,13,13,14,15,16,17,18,19,20,21,23,24,25,27,28,30,32,34,36,38,40,43,45,48,51,54,57,60,64,68,72,76,80,85,9096,101,107,114,120,128,135,143,152,161,170,180,191,202,214,227,241,255};
              
byte commandByte=0;
byte noteByte=0;
byte velocityByte=0;
int note;
int noteLast;
int i=0;
byte noteOn = 144;
byte noteOff = 128;
int controlSource;
int controlSourceLast;

  
void setup()
{
  pinMode(DcoResetPin, OUTPUT);
  pinMode(chargeVoltagePin, OUTPUT);
  pinMode(ControlSourcePin, INPUT);
  Timer1.initialize(calcPeriod[0]);
  Timer1.pwm(DcoResetPin, 50);
  
  Serial.begin(31250); //MIDI baud rate
}

void loop() {
controlSource = digitalRead(ControlSourcePin);
if(controlSource == 1){ //switch to MIDI
  if(Serial.available()>2){
    commandByte = Serial.read();
    if(commandByte!=248){
      noteByte = Serial.read();
      velocityByte = Serial.read(); //only reads the byte, currently has no effect on amplitude of the oscillator
        if(commandByte == noteOn){
          Timer1.setPeriod(pgm_read_float(&calcPeriod[noteByte]));
          Timer1.restart();
          analogWrite(chargeVoltagePin, pgm_read_byte(&chargePWM[noteByte]));
        }
        if(commandByte == noteOff){
        } 
    }
  }
}
else if(controlSource == 0){ //switch to CV input
  note = 0;
  for(int i=0; i<MAFLENGTH; i++){
    CV[i] = analogRead(ControlVoltage);
    note +=CV[i];
  }
  note = note / MAFLENGTH;
  note = map(note, 0,1023, 0,120);
  if(noteLast != note){
    Timer1.setPeriod(pgm_read_float(&calcPeriod[note]));
    analogWrite(chargeVoltagePin, pgm_read_byte(&chargePWM[note]));
  }

}

noteLast = note;
controlSourceLast = controlSource;
}
