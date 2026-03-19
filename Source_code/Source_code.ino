#include <MeMCore.h>
#include <math.h>

/* EEPROM was used to save and load colour calibration data, ensuring the mBot remembers it settings 
even when powered off, thus eliminating the need for recalibration before each run */
#include <EEPROM.h>

// Pin definitions for sensors and LEDs
#define inputA A0
#define inputB A1
#define IR A2 
#define LDR A3

// Constants for wait times
#define RGBWait 200
#define LDRWait 10

// Arrays for colour calibration and detection
float colour[] = {0,0,0}; // Holds the current colour reading
float whiteArray[] = {0,0,0}; // Array to store white values
float blackArray[] = {0,0,0}; // Array to store black values
float colDiff[] = {0,0,0}; // Difference between white and black readings

// Pre-calibrated RGB values for colour detection
float caliColour[6][3] = {
  {250, 96, 60},    // Red
  {190, 224, 152},  // Green
  {250, 160, 60},   // Orange
  {250, 220, 210},  // Pink
  {185, 210, 230},  // Light Blue
  {250, 240, 250}   // White
};

// Variables for IR readings
double ambientIR = 0.0;
double reflectedIR = 0.0;

// Initialize sensors and motors
MeUltrasonicSensor ultrasonic(PORT_1);   
MeLineFollower lineFinder(PORT_2); 
MeBuzzer buzzer;
MeDCMotor motorLeft(M1);   
MeDCMotor motorRight(M2);  

// Constants for motor speeds and wall detection
const int BASE_SPEED = 220;    
const int CORRECTION_SPEED = 170;  
const int TOO_CLOSE_ULTRASONIC = 8;   
const int TOO_FAR_ULTRASONIC = 13;
const int TOO_CLOSE_IR = 570; 

const int SAFE_DISTANCE_IR = 600;
const int SAFE_DISTANCE_ULTRASONIC = 11;

// Function to get the average reading from the LDR sensor
int AvgReading(int times) {
  int total = 0;

  // Read the LDR multiple times for a stable value
  for (int i = 0; i < times; i++) {
    total += analogRead(LDR);
    delay(LDRWait);
  }
  return total / times;
}

// Function to turn on the IR emitter only
void shineIR() {
  analogWrite(inputA, 0);
  analogWrite(inputB, 0);
}

// Function to turn on red LED
void shineRed() {
  analogWrite(inputA, 255);
  analogWrite(inputB, 255);
}

// Fucntion to turn on green LED
void shineGreen() {
  analogWrite(inputA, 255);
  analogWrite(inputB, 0);
}

// Fucntion to turn on Blue LED
void shineBlue() {
  analogWrite(inputA, 0);
  analogWrite(inputB, 255);
}

// Celebratory tune
void celebrate() {
  buzzer.tone(392, 200);
  buzzer.tone(523, 200);
  buzzer.tone(659, 200);
  buzzer.tone(784, 200);
  buzzer.tone(659, 150);
  buzzer.tone(784, 400);
  buzzer.noTone();
}

// Controls which LED to shine based on state
void decoder(int state) {
  if (state == 0) shineRed();
  else if (state == 1) shineGreen();
  else if (state == 2) shineBlue();
  else if (state == 3) shineIR();
}

void setup() {
  Serial.begin(9600); // Initialize serial communication
  pinMode(IR, INPUT); // Set IR pin as input

  // Load calibration data from EEPROM
  loadCali();

  // Enter calibration mode if 'c' is received within the first three seconds after the robot is on
  delay(3000);
  if (Serial.read() == 'c') {
    LDRCalibrate();
  }
}

// Checks if the line follower sensor detects a black line
bool checkLineDetected() {
  return lineFinder.readSensors() == S1_IN_S2_IN; // Check if both sensors detect black
}

// Calibrate the LDR sensor for white and black surface 
void LDRCalibrate() {
  delay(5000); // Delay to allow positioning

  // Calibrate for white
  for (int i = 0; i < 3; i++) {
    decoder(i);
    delay(RGBWait);
    whiteArray[i] = AvgReading(17);
    shineIR();
  }

  // Calibrate for black
  delay(5000);
  for (int i = 0; i < 3; i++) {
    decoder(i);
    delay(RGBWait);
    blackArray[i] = AvgReading(17);
    shineIR();
  }
  saveCali(); // Save calibration data
}

// Save calibration data to EEPROM
void saveCali() {
  for (int i = 0; i < 3; i++) {
    EEPROM.put(i * sizeof(float), whiteArray[i]);
    EEPROM.put((i + 3) * sizeof(float), blackArray[i]);
  }
}

// Load calibration data from EEPROM
void loadCali() {
  for (int i = 0; i < 3; i++) {
    EEPROM.get(i * sizeof(float), whiteArray[i]);
    EEPROM.get((i + 3) * sizeof(float), blackArray[i]);
  }
}

// Detect the colour beneath the mBot after it detects a black line
void detectColour() {
  shineIR();

  for (int i = 0; i < 3; i++) {
    colDiff[i] = whiteArray[i] - blackArray[i];
    decoder(i);
    delay(RGBWait);
    colour[i] = AvgReading(17); //Reads colour value beneath the mBot 
    delay(RGBWait);
    colour[i] = ((colour[i] - blackArray[i]) / colDiff[i]) * 255;
  }

  int closest = 0;
  long closestDistance = 195075; //represents maximum possible sqaured distance in RGB colour space

  // The loop below calculates the Euclidean distance between the detected colour and pre-calibrated RGB values
  for (int i = 0; i < 6; i++) {
    long distance = square(caliColour[i][0] - colour[0]) + 
                    square(caliColour[i][1] - colour[1]) + 
                    square(caliColour[i][2] - colour[2]);
    if (distance < closestDistance) {
      closestDistance = distance;
      closest = i;
    }
  }

  if (closest == 0) turnLeft(); // Red detected
  else if (closest == 1) turnRight(); // Green detected
  else if (closest == 2) Uturn(); // Orange detected
  else if (closest == 3) doubleLeftTurn(); // Pink detected
  else if (closest == 4) doubleRightTurn(); //Blue detected
  else stopMotors(), celebrate(); // White detected

}

void loop() {
  if (checkLineDetected()) { //Checks for black line 
    stopMotors();
    detectColour();  
  } else {
    moveAndCorrect();
    delay(10);
  }
}

// Corrects the robot's path based on sensor readings
void moveAndCorrect() {
  long ultrasonicDistance = ultrasonic.distanceCm(); 

  shineIR();
  reflectedIR = analogRead(IR);
  ShineRed();
  float dipping = analogRead(IR) - reflectedIR;

  // If the mBot is at a safe distance from the walls on the leeft and right, it'll continue to move forward.
  if (ultrasonicDistance > SAFE_DISTANCE_ULTRASONIC && dipping > SAFE_DISTANCE_IR) {
    moveMotors(BASE_SPEED, BASE_SPEED);
  } else if (ultrasonicDistance < TOO_CLOSE_ULTRASONIC) { // If the mBot is too close to the left walls, it adjusts to the right
    nudgeRight();
  } else if (ultrasonicDistance > TOO_FAR_ULTRASONIC && dipping < TOO_CLOSE_IR) { // If the mBot is too close to the right walls, it adjusts to the left
    nudgeLeft();
  } else {
    moveMotors(BASE_SPEED, BASE_SPEED);
  }
}

void stopMotors() { //Code for stopping both motors 
  motorLeft.stop();  // Stop left motor 
  motorRight.stop(); // Stop right motor 
} 
 
void moveMotors(int leftSpeed, int rightSpeed) { // Code for moving both motors with the given speed respectively 
  motorLeft.run(-leftSpeed);  
  //Due to the position of the left motor a positive value will cause it to move backwards hence the negative sign 
  motorRight.run(rightSpeed);   
} 
 
void turnRight() { // Code for turning right 90deg 
  moveMotors(250, -250); // Left wheel move forward while right wheel move backward hence turning right 
  delay(320);  // Tested time delay needed for a proper 90-degree turn 
  stopMotors(); 
} 
 
void turnLeft() { // Code for turning left 90deg 
  moveMotors(-250, 250); // Left wheel move backward while right wheel move forward hence turning left 
  delay(320);  // Tested time delay needed for a proper 90-degree turn 
  stopMotors();   
} 
 
void Uturn() { 
  moveMotors(250, -250); // Mbot turns right 
  delay(620);  // Tested time delay needed for a 180-degree turn 
  stopMotors(); 
} 
 
void doubleLeftTurn() { 
  turnLeft(); 
  moveMotors(BASE_SPEED, BASE_SPEED); 
  delay(820); // Tested time delay for Mbot to move foward into the open before the next turn 
  stopMotors();
  turnLeft(); 
} 
 
void doubleRightTurn() { 
  turnRight(); 
  moveMotors(BASE_SPEED, BASE_SPEED); 
  delay(820); // Tested time delay for Mbot to move foward into the open before the next turn 
  stopMotors();
  turnRight(); 
} 
 
void nudgeLeft() { // Code for nudging slightly to the left for some short interval 
  moveMotors(CORRECTION_SPEED, BASE_SPEED); 
  delay(5);  // Short nudge 
} 
 
void nudgeRight() { // Code for nudging slightly to the right for some short interval 
  moveMotors(BASE_SPEED, CORRECTION_SPEED);   
  delay(5);  // Short nudge 
}
