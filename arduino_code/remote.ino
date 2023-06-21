#include <Wire.h>
#include <MPU6050_tockn.h>
#include <AccelStepper.h>
#include <WiFi.h>
#include <Ticker.h>
#include <WebServer.h>

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

double AngleY;
double AngleZ;
double set_point = 0;
double control_output;

float rcm, lcm;
float cm_bak = 0;
int min_count = 0;
int max_count = 0;

AccelStepper stepper(AccelStepper::DRIVER, stepPin, dirPin);
AccelStepper stepper2(AccelStepper::DRIVER, stepPin2, dirPin2);
MPU6050 mpu6050(Wire);
Ticker timer;
Ticker timer2;

const char* ssid = "ydcl";
const char* password = "12345678";
//const char* serverURL = "http:172.20.10.2:3000/command"

WebServer server(80);

void handleRoot(){
    String HTML = "<html><head><style>\   
.btn {background-color: inherit;padding: 14px 28px;font-size: 16px;}\   
.btn:hover {background: #eee;}\   
</style></head>\   
<body>\  
<h1>EEErover Controller</h1>\
<br>Rock type: <span id=\"state\">Unknown</span>\   
</body>\   
<script>\   
var xhttp = new XMLHttpRequest();\   
xhttp.onreadystatechange = function() {if (this.readyState == 4 && this.status == 200) {document.getElementById(\"state\").innerHTML = this.responseText;}};\   
function Forward() {xhttp.open(\"GET\", \"/f\"); xhttp.send();}\   
function Left() {xhttp.open(\"GET\", \"/l\"); xhttp.send();}\   
function Right() {xhttp.open(\"GET\", \"/r\"); xhttp.send();}\   
function Stop() {xhttp.open(\"GET\", \"/S\"); xhttp.send();}\  
var current_key = 'None';\
function key_handler(event) {\
if (event.type == 'keyup') {\
Stop(); current_key = '';\
} else if (event.type == 'keydown') {\
if (event.code == current_key) return;\
current_key = event.code;\
switch (current_key) {\
case 'KeyW':\
Forward(); break;\
case 'KeyA':\
Left(); break;\
case 'KeyD':\
Right(); break;}}}\
document.addEventListener('keydown', key_handler);\
document.addEventListener('keyup', key_handler);\ 
</script></html>";
    server.send(200,"text/html", HTML);
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
}

void Stop()
{
  set_point = 0;
  control_output += stabilityPDControl(0.005, AngleY, set_point, kp, kd);
  control_output = constrain(control_output, -10000, 10000);
  Serial.println(control_output);
  stepper.setSpeed(-control_output);
  stepper2.setSpeed(-control_output);
}

void Forward()
{
  set_point = 2;
  control_output += stabilityPDControl(0.005, AngleY, set_point, kp, kd);
  control_output = constrain(control_output, -10000, 10000);
  Serial.println(control_output);
  stepper.setSpeed(-control_output);
  stepper2.setSpeed(-control_output);
}

void Left()
{
  set_point = 0;
  control_output += stabilityPDControl(0.005, AngleY, set_point, kp, kd);
  control_output = constrain(control_output, -10000, 10000);
  Serial.println(control_output);
  stepper.setSpeed(-control_output + 500);
  stepper2.setSpeed(-control_output - 500);
}

void Right()
{
  set_point = 0;
  control_output += stabilityPDControl(0.005, AngleY, set_point, kp, kd);
  control_output = constrain(control_output, -10000, 10000);
  Serial.println(control_output);
  stepper.setSpeed(-control_output - 500);
  stepper2.setSpeed(-control_output + 500);
}

float stabilityPDControl(float DT, float input, float setPoint, float Kp, float Kd)
{
  static float PID_errorOld = 0.0;
  static float setPointOld = 0.0;
  float error;
  float output;

  error = setPoint - input;

  float Kd_setPoint = constrain((setPoint - setPointOld), -8, 8);
  output = Kp * error + (Kd * Kd_setPoint - Kd * (input - PID_errorOld)) / DT;

  PID_errorOld = input;
  setPointOld = setPoint;

  return output;
}






void setup()
{
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin2, OUTPUT);
  pinMode(dirPin2, OUTPUT);
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

  timer.attach_ms(0.05, stepperRun);
  timer2.attach_ms(5, mpu_reading);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,password);

  while (WiFi.status() != WL_CONNECTED){
      delya(500);
      Serial.print(".");
  }

  Serial.print("\nIP address:");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/f", Forward);
  server.on("/S", Stop);
  server.on("/l", Left);
  server.on("/r", Right);
  server.onNotFound([](){server.send(200,"text/html;charset=utf-8","404 not found!");});
  server.begin();

}


void loop()
{
  Server.handleClient();
}


