#include <Arduino_LSM9DS1.h>
#include <LiquidCrystal_74HC595.h>

#define DS 11
#define SHCP 13
#define STCP 12
#define RS 1
#define E 2
#define D4 3
#define D5 4
#define D6 5
#define D7 6

LiquidCrystal_74HC595 lcd(DS, SHCP, STCP, RS, E, D4, D5, D6, D7);

int trigPin =6;
int echoPin =7;

int buzzPin = 9;

int joyButton = 8;
int joyY = A0;
int joyX = A1;



#include <math.h>


float pitch = 0;
float roll = 0;
float alpha = 0.98;
float angle = 0;
float dt = 0.01; // time for equations


bool imuAvailable = false;


void setup() {
  Serial.begin(9600);//serail input speed


  pinMode(trigPin, OUTPUT );//if its outptuing somthing set output vicevera for input
  pinMode(echoPin, INPUT );
  pinMode(buzzPin, OUTPUT);
  pinMode(joyButton, INPUT_PULLUP);// pullup resister pult in function to pull the line high to prevent noise

  lcd.begin(20, 4);
  lcd.print("Hello, World!");
  //IMU intilistion eror handling
  IMU.begin();
    if (!IMU.begin()) {
      Serial.println("Failed to initialize IMU!");
      imuAvailable = false;
      IMU.end();
      while (1);
    }else{
      imuAvailable = true;
    }

}

//void loop() {
  
  /*

  // for LCD
  lcd.setCursor(0, 1); //sets location o teh LCD im usign it to set point where to add stuff
  lcd.print(millis() / 1000); //adding timer at the  position (0,1) as set
  delay(1000);
  // this is teh ultra sonic sensor section
  // off (low) then sends plus (high) tehn stops sending pulse (low)
  digitalWrite(trigPin, LOW);
  delayMicroseconds(10);// 10 microseconds as sensors needs that
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH); // reads pulse
  int distance = duration * 0.034 / 2; //calulaes distance based on spped of sound and half cause only need distance 

  //Serial.print("distance: ");//printing distance
  //Serial.print(distance);
  //Serial.print("cm");
*/
  
void loop() {
  float ax, ay, az;//varabyles for string XYZ
  float gx, gy, gz;
// reading IMUs gyro XYZ and accell XYZ
  if (IMU.accelerationAvailable()) IMU.readAcceleration(ax, ay, az);
  if (IMU.gyroscopeAvailable()) IMU.readGyroscope(gx, gy, gz);
//getting angles
  float accel_pitch = atan2(ay, az) * 180 / PI;
  float accel_roll  = atan2(ax, az) * 180 / PI;
//complementory filter
  pitch = alpha * (pitch + gx * dt) + (1 - alpha) * accel_pitch;
  roll  = alpha * (roll  + gy * dt) + (1 - alpha) * accel_roll;

  Serial.print(accel_pitch); Serial.print(",");
  Serial.print(accel_roll);  Serial.print(",");
  Serial.print(pitch);       Serial.print(",");
  Serial.println(roll);

  delay(100);
}

// Done with the IMU readings
//IMU.end();
  



