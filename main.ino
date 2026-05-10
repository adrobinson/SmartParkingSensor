#include <Arduino_LSM9DS1.h>
#include <math.h>
#include <LiquidCrystal_74HC595.h>

class ultraSonic{
  private:
    String name;
    int trigPin;
    int echoPin;
    long duration;
    int lastDistance = 0;
    int distance;
    unsigned long lastTime = 0;
    float velocity;
    int failCount = 0;
  public:
    ultraSonic(String name, int trig, int echo){
      this->name = name;
      trigPin = trig;
      echoPin = echo;
      pinMode(trigPin, OUTPUT);
      pinMode(echoPin, INPUT);
    }
    
    void begin() {
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
      if (duration == 0) {
        distance = 999;
      } else {
        distance = duration * 0.034 / 2;
      }
      //delay(60);
    }

    void calcVelocity(){
      if (duration == 0) return;
      if (distance > 100) return;
      unsigned long now = millis();
      float dt = (now - lastTime) / 1000.0;
      lastTime = now;
      velocity = (float)(distance - lastDistance) / dt;
      lastDistance = distance;
    }
    bool isFailing() {
        if (distance == 999) {
            failCount++;
        } else {
            failCount = 0;
        }
        return failCount >= 3;
    }
    void print(){
      Serial.print(name + ": ");
      Serial.println(duration == 0 ? "Out of range" : String(distance) + "cm");
    }

    int   getDistance() { return distance; }
    float getVelocity() { return velocity; }
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

    void safe(int distance){
      if (distance >= 101){ digitalWrite(LEDPin, HIGH); }
      else { digitalWrite(LEDPin, LOW); }
    }
    void warning(int distance){
      if (distance <= 100 && distance >= 26){ digitalWrite(LEDPin, HIGH); }
      else { digitalWrite(LEDPin, LOW); }
    }
    void close(int distance){
      if (distance <= 25){ digitalWrite(LEDPin, HIGH); }
      else { digitalWrite(LEDPin, LOW); }
    }
    void danger(int distance){
      if (distance <= 5){ digitalWrite(LEDPin, HIGH); }
      else { digitalWrite(LEDPin, LOW); }
    }
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
    bool crashDetected;

  public:
    IMUClass(){
      alpha = 0.98;
      pitch = 0;
      roll  = 0;
      lastTime = 0;
      imuAvailable = false;
      crashDetected = false;
    }

    void begin() {
        if (!IMU.begin()) {
            Serial.println("Failed to initialize IMU!");
            imuAvailable = false;
            while (1);
        }
        lastTime = millis();  // start dt from now, not from boot
        imuAvailable = true;
    }

    void read() {
      if (IMU.accelerationAvailable()) IMU.readAcceleration(ax, ay, az);
      if (IMU.gyroscopeAvailable())    IMU.readGyroscope(gx, gy, gz);
    }
    void printCrashValues(){
    float magnitude = sqrt(ax*ax + ay*ay + az*az);
    Serial.print("Mag: ");   Serial.print(magnitude);
    Serial.print("  Roll: "); Serial.print(roll);
    Serial.print("  Pitch: "); Serial.println(pitch);
}

    void process(){
        unsigned long now = millis();
        float dt = (now - lastTime) / 1000.0;
        lastTime = now;
        accel_roll  = atan2(ay, az) * 180.0 / PI;
        accel_pitch = atan2(-ax, sqrt(ay*ay + az*az)) * 180.0 / PI;
        pitch = alpha * (pitch + gy * dt) + (1 - alpha) * accel_pitch;  // gy not gx
        roll  = alpha * (roll  + gx * dt) + (1 - alpha) * accel_roll;   // gx not gy
    }

    void printFiltered(){
      Serial.print("Pitch: "); Serial.println(pitch);
      Serial.print("Roll: ");  Serial.println(roll);
    }

    void printRaw(){
        Serial.print("AccelPitch: "); Serial.println(accel_pitch);
        Serial.print("AccelRoll: ");  Serial.println(accel_roll);
    }

    bool crash(){
      float magnitude = sqrt(ax*ax + ay*ay + az*az);
      if (magnitude > 4 || roll > 100 || pitch > 100){
        crashDetected = true;
        Serial.println("crash");
      } else {
        Serial.println("nocrash");
      }
      return crashDetected;
    }

    void resetCrash(){
      crashDetected = false;
    }

    bool isCrashed() { return crashDetected; }
    float getPitch()  { return pitch; }
    float getRoll()   { return roll;  }
};


class Button {
  private:
    int buttonPin;
  public:
    Button(int Pin) {
      buttonPin = Pin;
      pinMode(buttonPin, INPUT_PULLUP);
    }

    bool press() {
      if (digitalRead(buttonPin) == LOW) {
        return true;
      } else {
        return false;
      }
    }
};


class LCDDisplay {
  private:
    LiquidCrystal_74HC595 lcd;
  public:
    LCDDisplay() : lcd(11, 13, 12, 1, 3, 4, 5, 6, 7) {}
    void begin() { lcd.begin(16, 2); }

    void printDistanceF(int distance) {
      lcd.setCursor(0, 0);
      lcd.print("D");
      lcd.print(distance);
      lcd.print("cm  ");
    }

    void printDistanceB(int distance){
      lcd.setCursor(0, 1);
      lcd.print("D:");
      lcd.print(distance);
      lcd.print("cm  ");
    }

    void crashScreen(){
      lcd.setCursor(0, 0);
      lcd.print("CRASH!          ");
      lcd.setCursor(0, 1);
      lcd.print("Hold RST btn");
    }

    void printApproachF(float velocity) {
      lcd.setCursor(7, 0);
      if (velocity < -10) { lcd.print("SLOW DOWN"); }
      else { lcd.print("OK       "); }
    }

    void printApproachB(float velocity) {
      lcd.setCursor(7, 1);
      if (velocity < -10) { lcd.print("SLOW DOWN"); }
      else { lcd.print("OK       "); }
    }

    void clear() { lcd.clear(); }
   

    void printSensorError() {
      lcd.setCursor(0, 0);
      lcd.print("Add Sensors     ");
      lcd.setCursor(0, 1);
      lcd.print("                ");
    }
};



class Buzzer {
  private:
    int buzPin;
  public:
    Buzzer(int Pin){
      buzPin = Pin;
      pinMode(buzPin, OUTPUT);
    }

    void warnBuz(int distance){
      if (distance <= 100 && distance >= 26){
        digitalWrite(buzPin, HIGH); delay(500); digitalWrite(buzPin, LOW);
      } else { digitalWrite(buzPin, LOW); }
    }

    void closeBuz(int distance){
      if (distance <= 25 && distance >= 6){
        digitalWrite(buzPin, HIGH); delay(100); digitalWrite(buzPin, LOW);
      } else { digitalWrite(buzPin, LOW); }
    }

    void dangerBuz(int distance){
      if (distance <= 5){
        digitalWrite(buzPin, HIGH); delay(20); digitalWrite(buzPin, LOW);
      } else { digitalWrite(buzPin, LOW); }
    }

    void off(){ digitalWrite(buzPin, LOW); }
};

ultraSonic sensor1("Dist1", 9, 7);
ultraSonic sensor2("Dist2", 5, 6);
IMUClass   imu;
LCDDisplay lcd;

LED greenLedF ("green",  4);
LED yellowLedF("yellow", 3);
LED redLedF   ("red",    2);

LED greenLedB ("green",  A3);
LED yellowLedB("yellow", A1);
LED redLedB   ("red",    A2);

Button crashReset(8);
Buzzer buzzer(A0);

void setup() {
  Serial.begin(9600);
  imu.begin();
  lcd.begin();
  sensor1.begin();  // re-assert after lcd.begin()
  sensor2.begin();  // this is the critical one 
}

void loop() {
  imu.read();
  imu.process();

  if (imu.crash()) {
    greenLedF.off();  yellowLedF.off();  redLedF.off();
    greenLedB.off();  yellowLedB.off();  redLedB.off();
    buzzer.off();
    lcd.crashScreen();

    if (crashReset.press()) {
      imu.resetCrash();
      lcd.clear();
    }
    return;
  }

  sensor1.read();
  delay(60);
  sensor2.read();
  sensor1.calcVelocity();
  sensor2.calcVelocity();
  imu.printCrashValues();

  // check if sensors are reading
if (sensor1.isFailing() || sensor2.isFailing()) {
    lcd.printSensorError();
} else {
    lcd.printDistanceF(sensor1.getDistance());
    lcd.printApproachF(sensor1.getVelocity());
    lcd.printDistanceB(sensor2.getDistance());
    lcd.printApproachB(sensor2.getVelocity());
}

  greenLedF.safe   (sensor1.getDistance());
  yellowLedF.warning(sensor1.getDistance());
  redLedF.close    (sensor1.getDistance());

  greenLedB.safe   (sensor2.getDistance());
  yellowLedB.warning(sensor2.getDistance());
  redLedB.close    (sensor2.getDistance());

  int closest = min(sensor1.getDistance(), sensor2.getDistance());
  buzzer.warnBuz  (closest);
  buzzer.closeBuz (closest);
  buzzer.dangerBuz(closest);

  sensor1.print();
  sensor2.print();
  imu.printFiltered();

  delay(500);
}
