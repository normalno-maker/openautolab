#include <TM1637Display.h>
#include <Servo.h>
#include <HX711.h>
HX711 scale;
unsigned long int t0; // here time of start will be stored
unsigned long int secs; // here number of seconds on display will be stored
const uint8_t motorplus =3; //positive pole of pump motor
const uint8_t motorminus=12; //negative pole of pump motor
const uint8_t scaleclk=11; //pins of
const uint8_t scaledat=10; //scale
const uint8_t displayclk=9; //pins of
const uint8_t displaydio=8; //display
const uint8_t valve1=7; //pins
const uint8_t valve2=6; //of
const uint8_t valve3=5; //valves
const uint8_t valve4=4;
const uint8_t buzzer=13; //buzzer pin
const uint8_t servo=2; //servo pin
const uint8_t button=1; //temporary button
uint8_t container; //pin corresponding to current valve
unsigned long int airpump=10000UL; //number of milliseconds pumping without liquid tolerated   
bool error=false; //if something sent wrong machine beeps
TM1637Display display = TM1637Display(displayclk, displaydio);
Servo mixer;
void agitation(float a, float b, float c, float d) {
unsigned long int init=1000.0 * a; //duration of one unit of first agitation, 1sec
unsigned long int intvl=1000.0 * b; //agitation every unit of time, 1sec
unsigned long int agit=1000.0 * c; //substequent agitations duration unit, 1sec
unsigned long int devt=60000.0 * d; //overall development time unit, 1min
t0=millis();
while ((millis()-t0)<devt) {
  if (((millis()-t0)%intvl)<agit || (millis()-t0)<init) {
    if (((millis()-t0)%2000UL)<1000UL) mixer.write(180);
    else mixer.write(0);
  }
  secs=(t0+devt-millis())/1000UL;
  display.showNumberDecEx((secs/60UL)*100UL+(secs%60UL), 0b01000000, false);
}
}

void beep() {
  t0=millis();
  while (1) {
    if((millis()-t0)%1000UL<500UL) digitalWrite(buzzer,HIGH);
    else digitalWrite(buzzer,LOW);
    if(digitalRead(button)==LOW || millis()-t0>5000) {digitalWrite(buzzer,LOW); break;}
    }
}

void intank(int tank) {
  switch(tank) {
    case 1:
     container=valve1;
     break;
    case 2:
     container=valve2;
     break;
    case 3:
     container=valve3;
     break;
    case 4:
     container=valve4;
     break;
  }
  display.clear();
  int i=0;
  float measurements[10];
  scale.tare();
  digitalWrite(container,LOW);
  digitalWrite(motorminus,LOW);
  t0=millis();
  while (1) {
    delay(100);
    measurements[i]=scale.get_units();
    display.showNumberDecEx((int) measurements[i], 0b00000000, false);
    if(measurements[i]>300) {error=false; break;}
    i=(i+1)%10;
    float maximum=measurements[0];
    float minimum=measurements[0];
    for(int j=0;j<10;j++) {
      if(measurements[j]<minimum) minimum=measurements[j];
      if(measurements[j]>maximum) maximum=measurements[j];
    }
    if(maximum-minimum<3.0 && millis()-t0>airpump) {error=true; break;}
  }
  digitalWrite(motorminus,HIGH);
//  delay(500);
  digitalWrite(container,HIGH);
//  delay(500);
}

void outtank(int tank) {
  switch(tank) {
    case 1:
     container=valve1;
     break;
    case 2:
     container=valve2;
     break;
    case 3:
     container=valve3;
     break;
    case 4:
     container=valve4;
     break;
  }
  display.clear();
  int i=0;
  float measurements[10];
  scale.tare();
  digitalWrite(container,LOW);
  digitalWrite(motorplus,LOW);
  t0=millis();
  while (1) {
    delay(100);
    measurements[i]=scale.get_units();
    display.showNumberDecEx((int) measurements[i], 0b00000000, false);
    i=(i+1)%10;
    float maximum=measurements[0];
    float minimum=measurements[0];
    for(int j=0;j<10;j++) {
      if(measurements[j]<minimum) minimum=measurements[j];
      if(measurements[j]>maximum) maximum=measurements[j];
    }
    if(maximum-minimum<3.0 && millis()-t0>airpump) break;
  }
  digitalWrite(motorplus,HIGH);
//  delay(500);
  digitalWrite(container,HIGH);
//  delay(500);
}

void stage(int in, int out, float a, float b, float c, float d) {
  intank(in);
  if(error) beep();
  agitation(a,b,c,d);
  outtank(out);
}

void setup() {
pinMode(motorplus,OUTPUT);
pinMode(motorminus,OUTPUT);
pinMode(valve1,OUTPUT);
pinMode(valve2,OUTPUT);
pinMode(valve3,OUTPUT);
pinMode(valve4,OUTPUT);
pinMode(buzzer,OUTPUT);
pinMode(button,INPUT_PULLUP);
digitalWrite(valve1,HIGH);
digitalWrite(valve2,HIGH);
digitalWrite(valve3,HIGH);
digitalWrite(valve4,HIGH);
digitalWrite(motorplus,HIGH);
digitalWrite(motorminus,HIGH);
mixer.attach(servo);
display.setBrightness(7);
display.clear();
scale.begin(scaledat,scaleclk);
scale.set_scale(1850);
//        init int agit devt
//stage(1,1,30, 60, 10, 9); //develop in lqn
stage(1,4,60, 30, 5, 11); //develop in rodinal 1+50
stage(3,4,30, 60, 10, 1); //in-between wash
stage(2,2,30, 30, 5,  6);  //fix
stage(3,4, 5, 30, 5,  5);   //wash1
stage(3,4, 5, 30, 5,  5);   //wash2
stage(3,4, 5, 30, 5,  5);   //wash3
//stage(3,1,5,30,5,0.25);
//stage(1,4,5,30,5,0.25);
//stage(3,2,5,30,5,0.25);
//stage(2,4,5,30,5,0.25);
beep();
}
void loop() {
}

