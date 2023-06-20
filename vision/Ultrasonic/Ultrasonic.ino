#include <Ticker.h>

Ticker timer;
Ticker timer2;

int pos=0;
const int lTrigPin = 15;
const int lEchoPin = 2;
const int rTrigPin = 5;
const int rEchoPin = 18;

float rcm,lcm;
float cm_bak = 0;
int min_count = 0;
int max_count = 0;

void ultra_read_l()
{
  digitalWrite(lTrigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(lTrigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(lTrigPin, LOW);
  lcm = pulseIn(lEchoPin, HIGH) / 58.0;
  lcm = (int(lcm * 100.0)) / 100.0;
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
}

void setup() {
  Serial.begin(9600);
  pinMode(lTrigPin, OUTPUT);
  pinMode(lEchoPin, INPUT);
  pinMode(rTrigPin, OUTPUT);
  pinMode(rEchoPin, INPUT);

  timer.attach_ms(5,ultra_read_l);
  timer2.attach_ms(5,ultra_read_r);
}

void loop() {
  Serial.print("Left Distance:");
  Serial.print(lcm);
  Serial.print("cm  ");
  Serial.print("Right Distance:");
  Serial.print(rcm);
  Serial.print("cm");
  Serial.println();
  delay(50);
}
