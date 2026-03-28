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


bool imuAvailable false;


void setup() {
  Serial.begin(9600);


  pinMode(trigPin, OUTPUT )
  pinMode(echoPin, INPUT )
  pinMode(buzzPin, OUTPUT)
  pinMode(joyButton, INPUT_PULLUP)

  lcd.begin(20, 4);
  lcd.print("Hello, World!");
  IMU.begin()
    if (!IMU.begin()) {
      Serial.println("Failed to initialize IMU!");
      imuAvailable = false;
      IMU.end()
      while (1);
    else{
      imuAvailable = true;
    }
}
}

void loop() {
  lcd.setCursor(0, 1);
  lcd.print(millis() / 1000);
  delay(1000);

  // off (low) then sends plus (high) tehn stops sending pulse (low)
  digitalWrite(trigPin, LOW);
  delayMicroseconds(10);// 10 microseconds as sensors needs that
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH); // reads pulse

  int distance = duration * 0.034 / 2; //calulaes distance based on spped of sound and half cause only need distance 

  Serial.print("distance: ");//printing distance
  serial.print(distance);
  serial.print("cm");

  
  float gx, gy, gz; //gyroscope X,Y,Z
if (imuAvailable == true) {
  if (IMU.gyroscopeAvailable()) {
      IMU.readGyroscope(gx, gy, gz);

      Serial.print(gx);
      Serial.print('\t');
      Serial.print(gy);
      Serial.print('\t');
      Serial.println(gz);
  }
  float ax, ay, az; //acclenrometer X,Y,Z

  if (IMU.accelerationAvailable()) {
      IMU.readAcceleration(ax, ay, az);

      Serial.print(ax);
      Serial.print('\t');
      Serial.print(ay);
      Serial.print('\t');
      Serial.println(az);
}

}

// Done with the IMU readings
//IMU.end();
  



}
