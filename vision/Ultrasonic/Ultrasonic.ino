#include <Ticker.h> // Include the Ticker library for timer functionality

Ticker timer; // Create a Ticker object called "timer"
Ticker timer2; // Create a second Ticker object called "timer2"

int pos = 0; // Initialize a variable called "pos" to 0
const int lTrigPin = 15; // Define the left trigger pin as pin 15
const int lEchoPin = 2; // Define the left echo pin as pin 2
const int rTrigPin = 5; // Define the right trigger pin as pin 5
const int rEchoPin = 18; // Define the right echo pin as pin 18

float rcm, lcm; // Declare variables rcm and lcm of type float
float cm_bak = 0; // Initialize a variable called cm_bak to 0
int min_count = 0; // Initialize a variable called min_count to 0
int max_count = 0; // Initialize a variable called max_count to 0

// Function to read the distance using the left ultrasonic sensor
void ultra_read_l()
{
  digitalWrite(lTrigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(lTrigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(lTrigPin, LOW);
  lcm = pulseIn(lEchoPin, HIGH) / 58.0; // Measure the pulse width and convert it to distance in centimeters
  lcm = (int(lcm * 100.0)) / 100.0; // Round the distance to two decimal places
}

// Function to read the distance using the right ultrasonic sensor
void ultra_read_r()
{
  digitalWrite(rTrigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(rTrigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(rTrigPin, LOW);
  rcm = pulseIn(rEchoPin, HIGH) / 58.0; // Measure the pulse width and convert it to distance in centimeters
  rcm = (int(rcm * 100.0)) / 100.0; // Round the distance to two decimal places
}

void setup() {
  Serial.begin(9600); // Initialize the serial communication at a baud rate of 9600
  pinMode(lTrigPin, OUTPUT); // Set the left trigger pin as an output
  pinMode(lEchoPin, INPUT); // Set the left echo pin as an input
  pinMode(rTrigPin, OUTPUT); // Set the right trigger pin as an output
  pinMode(rEchoPin, INPUT); // Set the right echo pin as an input

  timer.attach_ms(5, ultra_read_l); // Attach the ultra_read_l function to the timer with a 5 ms interval
  timer2.attach_ms(5, ultra_read_r); // Attach the ultra_read_r function to the timer2 with a 5 ms interval
}

void loop() {
  Serial.print("Left Distance:");
  Serial.print(lcm); // Print the left distance in centimeters
  Serial.print("cm  ");
  Serial.print("Right Distance:");
  Serial.print(rcm); // Print the right distance in centimeters
  Serial.print("cm");
  Serial.println(); // Print a newline character to separate each reading
  delay(50); // Delay for 50 milliseconds before taking the next reading
}
