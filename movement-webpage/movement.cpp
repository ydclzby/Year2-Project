#include <Wire.h>
#include <MPU6050_tockn.h>
#include <AutoPID.h>
#include <AccelStepper.h>
#include <Ticker.h>

#define dirPin 12
#define stepPin 14
#define dirPin2 27
#define stepPin2 26

#define kp 20
#define ki 0
#define kd 13
#define turn_kp 0.1

double AngleY;
double set_point=0;
double control_output;
double control_output1;
double control_output2;
float turn_angle = 600;
float turn_spd;
double GyroZ;
float turn_output;



AccelStepper stepper(AccelStepper::DRIVER, stepPin, dirPin);
AccelStepper stepper2(AccelStepper::DRIVER, stepPin2, dirPin2);
MPU6050 mpu6050(Wire);
AutoPID controller(&AngleY, &set_point, &control_output, -10000, 10000, kp,ki,kd);
Ticker timer;
Ticker timer2;


void stepperRun()
{
  stepper.runSpeed();
  stepper2.runSpeed();
}

void mpu_reading()
{
  mpu6050.update();
  AngleY = mpu6050.getAngleY();
  GyroZ = mpu6050.getGyroZ();
}

float turn_pid(float turn_spd, float Kp, double GyroZ){
  float output = Kp * (turn_spd - GyroZ);
  return output;
}

float stabilityPDControl(float DT, float input, float setPoint, float Kp, float Kd)
{
  static float PID_errorOld = 0.0;
  static float setPointOld = 0.0;
  float error;
  float output; 

  error = setPoint - input;

  float Kd_setPoint = constrain((setPoint - setPointOld), -8, 8);
  output = Kp * error + (Kd * Kd_setPoint - Kd * (input - PID_errorOld))/DT;

  PID_errorOld = input;
  setPointOld = setPoint;

  return output;
}



void setup()
{
  pinMode(stepPin,OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin2, OUTPUT);
  pinMode(dirPin2,OUTPUT);

  Serial.begin(115200);
  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
 

  stepper.setMaxSpeed(10000.0);
  stepper.setAcceleration(1000.0);
  stepper2.setMaxSpeed(10000.0);
  stepper2.setAcceleration(1000.0);

  timer.attach_ms(0.05,stepperRun);
  timer2.attach_ms(5,mpu_reading);

}


void loop()
{
  //mpu6050.update();
  //AngleY = mpu6050.getAngleY();
  //Serial.println(AngleY);
  //controller.run();
  // control_output += stabilityPDControl(0.005,AngleY,set_point,kp,kd);

  // turn_output = turn_pid(turn_spd, turn_kp,GyroZ);
  // float control_output1 = control_output ;
  // float control_output2 = control_output ;
  // control_output1 = constrain(control_output, -10000, 10000);
  // control_output2 = constrain(control_output, -10000, 10000);
  // Serial.println(control_output);
  // // controller.run();
  // // if(controller.atSetPoint(1))
  // // {
  // //   control_output=0;
  // // }
  // stepper.setSpeed(-control_output1);
  // stepper2.setSpeed(-control_output2);
  stopMovement();
  //moveForward();
  //stepper.runSpeed();
//  stepper2.runSpeed();    
}

void moveForward() {
  // Code to move the rover forward
  Serial.println("Moving forward...");
  set_point =-5;
  control_output += stabilityPDControl(0.005,AngleY,set_point,kp,kd);
  control_output = constrain(control_output,-10000,10000);
  stepper.setSpeed(-control_output);
  stepper2.setSpeed(-control_output);
  // Your code here
}

void turnLeft() {
  // Code to turn the rover left
  Serial.println("Turning left...");
  set_point = 0;
  turn_spd = -600;
  control_output += stabilityPDControl(0.005,AngleY,set_point,kp,kd);
  control_output = constrain(control_output,-10000,10000);
  stepper.setSpeed(-control_output+500);
  stepper2.setSpeed(-control_output-500);
  // Your code here
}

void stopMovement() {
  // Code to stop the rover
  
  set_point = 0;
  control_output += stabilityPDControl(0.005,AngleY,set_point,kp,kd);
  control_output = constrain(control_output,-10000,10000);
  stepper.setSpeed(-control_output);
  stepper2.setSpeed(-control_output);
  Serial.println(control_output);
  // Your code here
}

void turnRight() {
  // Code to turn the rover right
  Serial.println("Turning right...");
  set_point=0;
  turn_spd = 600;
  turn_output = turn_pid(turn_spd, turn_kp, GyroZ);
  control_output += stabilityPDControl(0.005,AngleY,set_point,kp,kd);
  control_output = constrain(control_output,-10000,10000);
  stepper.setSpeed(-control_output-turn_output);
  stepper2.setSpeed(-control_output+turn_output);
  // Your code here
}