#include <Arduino_LSM9DS1.h>
#include <math.h>
#include <LiquidCrystal_74HC595.h>

class ultraSonic{
  private:
    String name;
    int trigPin;
    int echoPin;
    long duration;
    int distance;

  public:

    ultraSonic(String name, int trig,int echo){
      this->name = name;
      trigPin= trig;
      echoPin = echo;
      pinMode(trigPin, OUTPUT);
      pinMode(echoPin, INPUT);

    }

  void read(){
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);
      duration = pulseIn(echoPin, HIGH, 30000);
      distance = duration * 0.034 / 2;
      delay(25);
  }

  void print(){
    Serial.print(name + ": ");
    Serial.println(duration  == 0 ? "Out of range" : String(distance)  + "cm");
  }
   int getDistance() { return distance; }
};


class LED{
  private:
    String name;
    int LEDPin;
  public:
    LED(String name, int Pin){
    this->name = name;
    LEDPin = Pin;
    pinMode(Pin, OUTPUT);
    }
    void on()  { digitalWrite(LEDPin, HIGH); } 
    void off() { digitalWrite(LEDPin, LOW);  }  
};


class IMUClass{

  private:
    float ax, ay, az;
    float gx, gy, gz;
    float accel_pitch, accel_roll;
    float pitch, roll;
    float alpha;
    unsigned long lastTime;
    bool imuAvailable;
  
  public:
  IMUClass(){
      alpha = 0.98;
      pitch = 0;
      roll  = 0;
      lastTime = 0;
      imuAvailable = false;
  }

  void begin() {
    if (!IMU.begin()) {
        Serial.println("Failed to initialize IMU!");
        imuAvailable = false;
        while (1);
      }
      imuAvailable = true;
    }

  void read() {
    if (IMU.accelerationAvailable()) IMU.readAcceleration(ax, ay, az);
    if (IMU.gyroscopeAvailable())    IMU.readGyroscope(gx, gy, gz);
    }
  void process(){
    float dt = (millis() - lastTime) / 1000.0;
    lastTime = millis();
    accel_roll  = atan2(ay, az) * 180.0 / PI;
    accel_pitch = atan2(-ax, sqrt(ay*ay+az*az)) * 180.0 / PI;
//complemntory filter
    pitch = alpha * (pitch + gx * dt) + (1 - alpha) * accel_pitch;
    roll  = alpha * (roll  + gy * dt) + (1 - alpha) * accel_roll;
  }

  void printFiltered(){

    Serial.print(pitch);       Serial.print(",");
    Serial.println(roll);Serial.print(".");
  }
  void printRaw(){
    Serial.print(accel_pitch); Serial.print(",");
     Serial.print(accel_roll);  Serial.print(".");

  }

    float getPitch() { return pitch; }
    float getRoll()  { return roll;  }


};


class LCDDisplay {
  private:
    LiquidCrystal_74HC595 lcd;
  public:
    LCDDisplay() : lcd(11, 13, 12, 1, 3, 4, 5, 6, 7) {}
    void begin() { lcd.begin(20, 4); }
    void print(String msg) { lcd.print(msg); }
    void clear() { lcd.clear(); }
};

class Buzzer {

  private:
  int buzPin;

  public:
    Buzzer(int Pin){
      buzPin = Pin;
      pinMode(buzPin, OUTPUT);
    }
  void noise(){
    digitalWrite(buzPin, HIGH);
    }
  void off(){
    digitalWrite(buzPin, LOW);
  }
    


};

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
