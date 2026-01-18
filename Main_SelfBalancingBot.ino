#include <MPU6050.h>
#include <I2Cdev.h>
#include "Wire.h"
#include "math.h"

unsigned long prevTime = 0;

#define leftMotorPWMPin 6
#define leftMotorDirPin 7
#define rightMotorPWMPin 5
#define rightMotorDirPin 16

#define Kp 45.00
#define Kd 0.5
#define Ki 0
#define sampleTime 0.005
#define targetAngle -2.5

MPU6050 mpu;

int16_t accX, accZ, gyroY;
volatile int motorPower, gyroRate;
volatile float accAngle, gyroAngle, currentAngle, prevAngle=0, error, prevError=0, errorSum=0;
volatile byte count=0;

int minPWM = 60;


void setMotors(int leftMotorSpeed, int rightMotorSpeed){
  if (leftMotorSpeed >=0){
    analogWrite(leftMotorPWMPin, leftMotorSpeed);
    digitalWrite(leftMotorDirPin,LOW);
  }
  else{
    analogWrite(leftMotorPWMPin, 255 + leftMotorSpeed);
    digitalWrite(leftMotorDirPin,HIGH);

  }
  if (rightMotorSpeed >=0){
    analogWrite(rightMotorPWMPin, rightMotorSpeed);
    digitalWrite(rightMotorDirPin,LOW);
  }
  else{
    analogWrite(rightMotorPWMPin,255 + rightMotorSpeed);
    digitalWrite(rightMotorDirPin, HIGH);
  }
}

//void init_PID(){
  //cli();
  //TCCR1A = 0;
  //TCCR1B = 0;

  //OCR1A = 9999;
  //TCCR1B |= (1 << WGM12);
  //TCCR1B |= (1 << CS11);
  //TIMSK1 |= (1 << OCIE1A);
  //sei();  
//}


void setup() {
  // put your setup code here, to run once:
  pinMode(leftMotorPWMPin, OUTPUT);
  pinMode(leftMotorDirPin, OUTPUT);
  pinMode(rightMotorPWMPin, OUTPUT);
  pinMode(rightMotorDirPin, OUTPUT);
  // set the status LED to output mode 
  pinMode(13, OUTPUT);

  Wire.begin(8,9);
  mpu.initialize();

  // ===== ACCELEROMETER OFFSETS =====
  mpu.setXAccelOffset(-1853); //we use this
  mpu.setYAccelOffset( 261);
  mpu.setZAccelOffset( 4506); //we use this

  // ===== GYROSCOPE OFFSETS =====
  mpu.setXGyroOffset(-64);
  mpu.setYGyroOffset( 15); //we use this
  mpu.setZGyroOffset( 52);

  //init_PID();
}

void loop() {
  // put your main code here, to run repeatedly:

  unsigned long currTime = micros();

  if (currTime - prevTime >= 5000){
  prevTime = currTime;
  accX = mpu.getAccelerationX();
  accZ = mpu.getAccelerationZ();
  gyroY = mpu.getRotationY();

  accAngle = atan2(accX, accZ) * RAD_TO_DEG;
  gyroRate = gyroY / 131.0;
  currentAngle = 0.9934 * (prevAngle + gyroRate * sampleTime) + 0.0066 * accAngle;

  error = targetAngle - currentAngle; //change by CHATGPT

  errorSum += error;
  errorSum = constrain(errorSum, -300, 300);

  motorPower = Kp * error + Ki * errorSum * sampleTime - Kd * (currentAngle - prevAngle) / sampleTime;


  motorPower = constrain(motorPower, -255, 255);
  
int minPWM = 60; //chatgpt fix start

if (motorPower > 0 && motorPower < minPWM)
  motorPower = minPWM;
else if (motorPower < 0 && motorPower > -minPWM)
  motorPower = -minPWM; //chatgpt fix end


  setMotors(motorPower, motorPower);
  prevAngle = currentAngle;
}
}

//ISR(TIMER1_COMPA_vect)
//{
  // calculate the angle of inclination
  //accAngle = atan2(accX, accZ)*RAD_TO_DEG;
  //gyroRate = gyroY / 131.0; 
  //gyroAngle = (float)gyroRate*sampleTime;  
  //currentAngle = 0.9934*(prevAngle + gyroAngle) + 0.0066*(accAngle);
  
  //error = currentAngle - targetAngle;
  //errorSum = errorSum + error;  
  //errorSum = constrain(errorSum, -300, 300);
  //calculate output from P, I and D values
  //motorPower = Kp*(error) + Ki*(errorSum)*sampleTime - Kd*(currentAngle-prevAngle)/sampleTime;
  //prevAngle = currentAngle;
  // toggle the led on pin13 every second
  //count++;
  //if(count == 200)  {
    //count = 0;
    //digitalWrite(13, !digitalRead(13));
  //}
//}
