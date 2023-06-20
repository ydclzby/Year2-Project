#include <Wire.h>
#include <MPU6050_tockn.h>
#include <AutoPID.h>
#include <AccelStepper.h>
#include <Ticker.h>

#define dirPin 12
#define stepPin 14
#define dirPin2 27
#define stepPin2 26

#define lTrigPin 5
#define lEchoPin 18
#define rTrigPin 15
#define rEchoPin 2

#define kp 10
#define ki 0
#define kd 4

#define ULTRA_TOR 10


double AngleY;
double AngleZ;
double set_point=0;
double control_output;

float rcm,lcm;
float cm_bak = 0;
int min_count = 0;
int max_count = 0;



AccelStepper stepper(AccelStepper::DRIVER, stepPin, dirPin);
AccelStepper stepper2(AccelStepper::DRIVER, stepPin2, dirPin2);
MPU6050 mpu6050(Wire);
//AutoPID controller(&control_output, &spd_set_point, &spd_control_output, -3, 5, kp_sp,ki_sp,kd_sp);
Ticker timer;
Ticker timer2;


void ultra_read_l()
{
  digitalWrite(lTrigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(lTrigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(lTrigPin, LOW);
  lcm = pulseIn(lEchoPin, HIGH) / 58.0;
  lcm = (int(lcm * 100.0)) / 100.0;
  Serial.print("Left Distance:");
  Serial.print(lcm);
  Serial.println("cm  ");
}

void ultra_read_r()
{
   digitalWrite(rTrigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(rTrigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(rTrigPin, LOW);
  rcm = pulseIn(rEchoPin, HIGH) / 58.0;
  rcm = (int(rcm * 100.0)) / 100.0;
  Serial.print("Right Distance:");
  Serial.print(rcm);
  Serial.println("cm");
}

void stepperRun()
{
  stepper.runSpeed();
  stepper2.runSpeed();
}

void mpu_reading()
{
  mpu6050.update();
  AngleY = mpu6050.getAngleY();
  AngleZ = mpu6050.getAngleZ();
  ultra_read_r();
  ultra_read_l();
}

void Stop(){
  set_point = 0;
  control_output += stabilityPDControl(0.005,AngleY,set_point,kp,kd);
  control_output = constrain(control_output, -10000, 10000);
  Serial.println(control_output);
  stepper.setSpeed(-control_output);
  stepper2.setSpeed(-control_output); 
}

void Forward(){
  set_point = 2;
  control_output += stabilityPDControl(0.005,AngleY,set_point,kp,kd);
  control_output = constrain(control_output, -10000, 10000);
  Serial.println(control_output);
  stepper.setSpeed(-control_output);
  stepper2.setSpeed(-control_output); 
}

void Left(){
  set_point = 0;
  control_output += stabilityPDControl(0.005,AngleY,set_point,kp,kd);
  control_output = constrain(control_output, -10000, 10000);
  Serial.println(control_output);
  stepper.setSpeed(-control_output+500);
  stepper2.setSpeed(-control_output-500); 
}


void Right(){
  set_point = 0;
  control_output += stabilityPDControl(0.005,AngleY,set_point,kp,kd);
  control_output = constrain(control_output, -10000, 10000);
  Serial.println(control_output);
  stepper.setSpeed(-control_output-500);
  stepper2.setSpeed(-control_output+500); 
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
  pinMode(lTrigPin, OUTPUT);
  pinMode(lEchoPin, INPUT);
  pinMode(rTrigPin, OUTPUT);
  pinMode(rEchoPin, INPUT);

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
  Stop();
  //Forward();

}

bool haveRightTurned = false;

void wallFollower()
{
  Forward();
  if(!rightHasWall() && !haveRightTurned)
  {
    haveRightTurned = true;
    Right();
  }
  if(rightHasWall())
  {
    haveRightTurned = false;
  }

}

bool leftHasWall()
{
  if(lcm < ULTRA_TOR)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool rightHasWall()
{
  if(rcm < ULTRA_TOR)
  {
    return true;
  }
  else
  {
    return false;
  }
}
