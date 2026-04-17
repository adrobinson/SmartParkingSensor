#include <Arduino_LSM9DS1.h>
#include <LiquidCrystal_74HC595.h>
#include <math.h>

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

int trigPin = 9;
int echoPin = 12;
int buzzPin = A3;
int joyButton = 8;
int joyY = A0;
int joyX = A1;

float pitch = 0;
float roll = 0;
float alpha = 0.98;
float dt = 0.01;

bool imuAvailable = false;

void setup() {
  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzPin, OUTPUT);
  pinMode(joyButton, INPUT_PULLUP);

  lcd.begin(20, 4);
  lcd.print("Hello, World!");

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    imuAvailable = false;
    while (1);
  } else {
    imuAvailable = true;
  }
}

void loop() {
  // --- Ultrasonic sensor ---
  lcd.setCursor(0, 1);
  lcd.print(millis() / 1000);
  //delay(1000);

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);        // settle first
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000); // 30ms timeout
  int distance = duration * 0.034 / 2;

  // imu
  float ax, ay, az;
  float gx, gy, gz;

  if (IMU.accelerationAvailable()) IMU.readAcceleration(ax, ay, az); // mapping to x,y,z
  if (IMU.gyroscopeAvailable()) IMU.readGyroscope(gx, gy, gz);

  float accel_pitch = atan2(ay, az) * 180 / PI; // conver to angles
  float accel_roll  = atan2(ax, az) * 180 / PI;
// complemntory filter.
  pitch = alpha * (pitch + gx * dt) + (1 - alpha) * accel_pitch;
  roll  = alpha * (roll  + gy * dt) + (1 - alpha) * accel_roll;

  Serial.print("Dist: "); 
  Serial.print(distance); // prints ultranoice snroce distance estimatons
  Serial.println("cm");

  
  Serial.print(accel_pitch); Serial.print(",");   
  Serial.print(accel_roll);  Serial.print(",");
  Serial.print(pitch);       Serial.print(",");
  Serial.println(roll);

  delay(100);
}
