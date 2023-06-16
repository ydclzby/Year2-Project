#include <MPU6050_tockn.h>
#include <Wire.h>
#include <AutoPID.h>

#define dirPin 12
#define stepPin 14
#define dirPin2 27
#define stepPin2 26
#define stepsPerRevolution 200

#define kp 10
#define ki 0
#define kd 0

MPU6050 mpu6050(Wrie);
long timer;
const unsigned long interval = 100;
float desired_RPM;

double AngleY, Balance_Angle=-0.4, control_output;

long unsigned delayTime;

AutoPID controller(&AngleY, &Balance_Angle, &control_output, -100 , 100,kp,ki,kd);

void dir_calculation(){
  if(control_output>=0){
    digitalWrite(dirPin, 0);
    digitalWrite(dirPin2,1);
  }
  else{
    digitalWrite(dirPin, 1);
    digitalWrite(dirPin2, 0);
  }
}

void PID(){
  AngleY = mpu6050.getAngleY();
  controller.run();
  desired_RPM = abs(control_output);
  desired_RPM = constrain(60,100);
}

void stepMotor(){
    digitalWrite(stepPin,1);
    digitalWrite(stepPin2,1);
    delayMicroseconds(delayTime);
    digitalWrite(stepPin,0);
    digitalWrite(stepPin2,0);
    delayMicroseconds(delayTime);
}

void setup()
{
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin2, OUTPUT);
  pinMode(dirPin2, OUTPUT);

  Serial.begin(9600);
  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
  // delayTime = 60000000 / (stepsPerRevolution * desired_RPM);
}

void loop(){
  mpu6050.update();
  PID();
  delayTime = 60000000/(stepsPerRevolution * desired_RPM);
  dir_calculation();
  while(millis()-timer<interval){
    stepMotor();
  }
  timer = millis();
}