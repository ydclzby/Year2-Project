#include <Wire.h>
#include <MPU6050_tockn.h>
#include <AutoPID.h>
#include <AccelStepper.h>
#include <Ticker.h>

#define dirPin 12
#define stepPin 14
#define dirPin2 27
#define stepPin2 26

#define kp 500
#define ki 0
#define kd 15

double AngleY;
double set_point=0;
double control_output;


AccelStepper stepper(AccelStepper::DRIVER, stepPin, dirPin);
AccelStepper stepper2(AccelStepper::DRIVER, stepPin2, dirPin2);
MPU6050 mpu6050(Wire);
AutoPID controller(&AngleY, &set_point, &control_output, -10000, 10000, kp,ki,kd);
Ticker timer;


void stepperRun()
{
    stepper.runSpeed();
    stepper2.runSpeed();
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

    timer.attach_ms(1,stepperRun);

}


void loop()
{
    mpu6050.update();
    AngleY = mpu6050.getAngleY();
    //Serial.println(AngleY);
    //controller.run();
    control_output = stabilityPDControl(0.005,AngleY,set_point,kp,kd);
    Serial.println(control_output);
    // controller.run();
    // if(controller.atSetPoint(1))
    // {
    //   control_output=0;
    // }

    stepper.setSpeed(-control_output);
    stepper2.setSpeed(-control_output);
    // stepper.runSpeed();
    // stepper2.runSpeed();
    
}