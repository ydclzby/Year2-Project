
/*
 * ==程序编写2==
 * 【马达驱动方向和死区大小测试】 
 * 
 * 测试结果 6v电压供给下。
 * pwm+为前进，pwm+-为后退
 * 左马达IN1 IN2,OUT1 OUT2 , pwm   36起转。
 * 右马达IN3 IN4，OUT3 OUT4, PWM    39起转。
 * 
 */

#define IN1 26
#define IN2 27
#define IN3 32
#define IN4 33
int pwm = 0;

void motor(int left_EN, int right_EN)
{
  if (left_EN == 0)
  {
    analogWrite(IN1, 0);
    analogWrite(IN2, 0);
  }
  if (left_EN < 0)
  {
    if (left_EN < -255)
      left_EN = -255;
    analogWrite(IN1, 0);
    analogWrite(IN2, 0 - left_EN);
  }
  if (left_EN > 0)
  {
    if (left_EN > 255)
      left_EN = 255;
    analogWrite(IN1, left_EN);
    analogWrite(IN2, 0);
  }
  if (right_EN == 0)
  {
    analogWrite(IN3, 0);
    analogWrite(IN4, 0);
  }
  if (right_EN < 0)
  {
    if (right_EN < -255)
      right_EN = -255;
    analogWrite(IN3, 0);
    analogWrite(IN4, 0 - right_EN);
  }
  if (right_EN > 0)
  {
    if (right_EN > 255)
      right_EN = 255;
    analogWrite(IN3, right_EN);
    analogWrite(IN4, 0);
  }
}

void serial_debug()
{
  if (Serial.available() > 0)
  {
    delay(5);
    char DATA = Serial.read();
    switch (DATA)
    {
    case '1':
      pwm++;
      Serial.print("PWM: ");
      Serial.println(pwm);
      break;
    case '0':
      pwm--;
      Serial.print("PWM: ");
      Serial.println(pwm);
      break;
    }
  }
}

void setup()
{
  Serial.begin(9600);
}

void loop()
{
  serial_debug();
  motor(pwm, pwm);
}