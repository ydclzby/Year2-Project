#include <Wire.h>
#include <MPU6050_tockn.h>
#include <AutoPID.h>
#include <AccelStepper.h>
#include <Ticker.h>
#include <math.h>
#include <ctype.h>
#include <HardwareSerial.h>

#define dirPin 12
#define stepPin 14
#define dirPin2 27
#define stepPin2 26

#define lTrigPin 5
#define lEchoPin 18
#define rTrigPin 15
#define rEchoPin 2

#define UART_BAUDRATE 115200
#define RXD2 16 // FPGA side : (ARDUNIO_IO[9])
#define TXD2 17 // FPGA side : (ARDUNIO_IO[8])

HardwareSerial Fpga(2);

unsigned long start = 0;
unsigned long period = 500;

static bool newLine = true; // Flag to indicate whether to print a new line
static int counter = 0;


#define kp 10
#define ki 0
#define kd 4




double AngleY;
double AngleZ;
double set_point=0;
double control_output;
double offset;
bool redSeen;
bool blueSeen;
bool yellowSeen;
bool wallFront;


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

// void core1_task(void* Parameters){
//   wallFollower();
// }


int byte2int(byte* buf, int size){
  int val = 0;
  for (int i=(size-1); i>=0; i--){
    val += buf[i] << (8*i);
  }
  return val;
}

int b;

void fpga_read(){
  // if (Fpga.available() >= 4) {
  //   byte buf[4];
  //   Fpga.readBytes(buf, 4);
    
  //   // Print "Received data: " when newline is printed
  //   if (newLine) {
  //     Serial.print("Received data: ");
  //     newLine = false;
  //   }
  //   int a = byte2int(buf, 4);
  //   // Check the received value and print corresponding message

  //   if(b == 1){
  //     redSeen = a;
  //     b = 0;
  //   }
    
  //   if(b == 2){
  //     blueSeen = a;
  //     b = 0;
  //   }

  //   if(b == 3){
  //     yellowSeen = a;
  //     b = 0;
  //   }

  //   if(b == 4){
  //     wallFront = a;
  //     b = 0;
  //   }

  //   if(a == 0x00524242){
  //     b = 1;
  //     Serial.print("red ");
  //   } 
  //   else if(a == 0x00424242){
  //     b = 2;
  //     Serial.print("blue ");
  //   }
  //   else if(a == 0x00594242){
  //     b = 3;
  //     Serial.print("yellow ");
  //   }
  //   else if(a == 0x0057574d){
  //     b = 4;
  //     Serial.print("white ");
  //   }
    
  //   else {
  //     Serial.print(a, HEX);
  //     Serial.print(" ");
  //   } 
    
  //   // counter of 8 to make the received data occur one time in a line and then print 8 messages
  //   counter++;
  //   if (counter == 8) {
  //     Serial.println();
  //     counter = 0;
  //     newLine = true;
  //   }
  // }
  static bool newLine = true; // Flag to indicate whether to print a new line
  static int counter = 0; // Counter for printing values in a line
  
  if (Fpga.available() >= 4) {
    byte buf[4];
    Fpga.readBytes(buf, 4);

    if (newLine) {
      Serial.print("Received data: ");
      newLine = false;
    }
    int a = byte2int(buf, 4);
    
    if(a == 0x00524242){
      Serial.print("red ");
    } 
    else if(a == 0x00424242){
      Serial.print("blue ");
    }
    else if(a == 0x00594242){
      Serial.print("yellow ");
    }
    else if(a == 0x0057574d){
      Serial.print("white ");
    }
    else {
      Serial.print(a, HEX);
      Serial.print(" ");
    } 

    counter++;
    if (counter == 8) {
      Serial.println();
      counter = 0;
      newLine = true;
    }
  }
}


void ultra_read_l()
{
  digitalWrite(lTrigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(lTrigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(lTrigPin, LOW);
  lcm = pulseIn(lEchoPin, HIGH) / 58.0;
  lcm = (int(lcm * 100.0)) / 100.0;
  // Serial.print("Left Distance:");
  // Serial.print(lcm);
  // Serial.println("cm  ");
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
  // Serial.print("Right Distance:");
  // Serial.print(rcm);
  // Serial.println("cm");
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
  // ultra_read_r();
  // ultra_read_l();
}

void Stop(){
  //Serial.println("S");
  set_point = 2;
  offset = 0;
  control_output += stabilityPDControl(0.005,AngleY,set_point,kp,kd);
  control_output = constrain(control_output, -10000, 10000);
  //Serial.println(control_output);
  stepper.setSpeed(-control_output+offset);
  stepper2.setSpeed(-control_output-offset); 
}

void Forward(){
  Serial.println("F");
  set_point = -0.5;
  offset = 0;
  control_output += stabilityPDControl(0.005,AngleY,set_point,kp,kd);
  control_output = constrain(control_output, -10000, 10000);
  //Serial.println(control_output);
  stepper.setSpeed(-control_output+offset);
  stepper2.setSpeed(-control_output-offset); 
}

void Left(){
  Serial.println("L");
  set_point = 2;
  offset = 500;
  control_output += stabilityPDControl(0.005,AngleY,set_point,kp,kd);
  control_output = constrain(control_output, -10000, 10000);
  //Serial.println(control_output);
  stepper.setSpeed(-control_output-offset);
  stepper2.setSpeed(-control_output+offset); 
}


void Right(){
  Serial.println("R");
  set_point = 2;
  offset = -500;
  control_output += stabilityPDControl(0.005,AngleY,set_point,kp,kd);
  control_output = constrain(control_output, -10000, 10000);
  //Serial.println(control_output);
  stepper.setSpeed(-control_output-offset);
  stepper2.setSpeed(-control_output+offset); 
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

  Fpga.begin (115200, SERIAL_8N1, RXD2, TXD2);
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

  while (!Serial);

  Serial.println("ESP32 UART Receiver");
}

void loop()
{
  ultra_read_r();
  ultra_read_l();
  fpga_read();
//  Serial.println(wallFront);
  wallFollower();
  //Left();
  //delay(1000);

}


#define ULTRA_TOR 10
bool haveRightTurned = false;

void wallFollower()
{
  Stop();
  /*if(front has Wall)
  {
    
  }*/ 
  // if(wallFront){
  //   Left();
  // }
  if(!rightHasWall())
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
