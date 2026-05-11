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


class LCDDisplay {
  private:
    LiquidCrystal_74HC595 lcd;
  public:
    LCDDisplay() : lcd(11, 13, 12, 1, 3, 4, 5, 6, 7) {}
    void begin() { lcd.begin(16, 2); }

    void crashScreen(){
      lcd.setCursor(0, 0);
      lcd.print("CRASH!          ");
      lcd.setCursor(0, 1);
      lcd.print("Hold RST btn");
    }

    void printRowF(int distance, float velocity) {
        lcd.setCursor(0, 0);
        if (distance == 999) {
            lcd.print("Out of range    ");
        } else {
            String row = "F:" + String(distance) + "cm  ";
            row += (velocity < -10) ? "SLOW DOWN" : "OK       ";
            lcd.print(row);
        }
    }

    void printRowB(int distance, float velocity) {
        lcd.setCursor(0, 1);
        if (distance == 999) {
            lcd.print("Out of range    ");
        } else {
            String row = "B:" + String(distance) + "cm  ";
            row += (velocity < -10) ? "SLOW DOWN" : "OK       ";
            lcd.print(row);
        }
    }

    void clear() { lcd.clear(); }

    void printSensorError() {
      lcd.setCursor(0, 0);
      lcd.print("Add Sensors     ");
      lcd.setCursor(0, 1);
      lcd.print("                ");
    }
};


class IMUClass{
  private:
    float ax, ay, az;
    float accel_pitch, accel_roll;
    float pitch, roll;
    bool imuAvailable;
    bool crashDetected;

  public:
    IMUClass(){
      pitch = 0;
      roll  = 0;
      imuAvailable = false;
      crashDetected = false;
    }

    void begin() {
        if (!IMU.begin()) {
            Serial.println("Failed to initialize IMU!");
            imuAvailable = false;
            while (1);
        }
        imuAvailable = true;
    }
    float getMagnitude() {
        return sqrt(ax*ax + ay*ay + az*az);
    }
    void read() {
        while (!IMU.accelerationAvailable());
        IMU.readAcceleration(ax, ay, az);
    }
    void process(){
        accel_roll  = atan2(ay, az) * 180.0 / PI;
        accel_pitch = atan2(-ax, sqrt(ay*ay + az*az)) * 180.0 / PI;
        pitch = accel_pitch;
        roll  = accel_roll;
    }

    void printCrashValues(){
        float magnitude = sqrt(ax*ax + ay*ay + az*az);
        Serial.print("Mag: ");    Serial.print(magnitude);
        Serial.print("  Roll: "); Serial.print(roll);
        Serial.print("  Pitch: "); Serial.println(pitch);
    }

    void printFiltered(){
      Serial.print("Pitch: "); Serial.println(pitch);
      Serial.print("Roll: ");  Serial.println(roll);
    }

    bool crash(){
        float magnitude = sqrt(ax*ax + ay*ay + az*az);
        if (magnitude > 4 || fabs(roll) > 100 || fabs(pitch) > 100){
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


class Buzzer {
  private:
    int buzPin;
    unsigned long lastBuzzTime = 0;
    unsigned long buzzDuration = 0;
    bool buzzing = false;
  public:
    Buzzer(int Pin){
      buzPin = Pin;
      pinMode(buzPin, OUTPUT);
    }

    void update() {
      if (buzzing && millis() - lastBuzzTime >= buzzDuration) {
        digitalWrite(buzPin, LOW);
        buzzing = false;
      }
    }

    void buzz(int distance) {
        if (buzzing) return;
        if (distance <= 5) {
            digitalWrite(buzPin, HIGH);
            lastBuzzTime = millis(); buzzDuration = 20; buzzing = true;
        } else if (distance <= 25) {
            digitalWrite(buzPin, HIGH);
            lastBuzzTime = millis(); buzzDuration = 100; buzzing = true;
        } else if (distance <= 100) {
            digitalWrite(buzPin, HIGH);
            lastBuzzTime = millis(); buzzDuration = 500; buzzing = true;
        }
    }

    void off(){ buzzing = false; digitalWrite(buzPin, LOW); }
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
unsigned long lastLoopTime = 0;
void setup() {
  Serial.begin(9600);
  lcd.begin();
  imu.begin();
  sensor1.begin();
  sensor2.begin();
}

void loop() {
  buzzer.update();

  imu.read();
  imu.process();

  if (imu.isCrashed()) {
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

  if (imu.crash()) return;

  unsigned long currentTime = millis();
  if (currentTime - lastLoopTime < 500) return;
  lastLoopTime = currentTime;
  
  sensor1.read();
  delay(60);
  sensor2.read();
  sensor1.calcVelocity();
  sensor2.calcVelocity();
  imu.printCrashValues();

  if (sensor1.isFailing() || sensor2.isFailing()) {
    lcd.printSensorError();
  } else {
    lcd.printRowF(sensor1.getDistance(), sensor1.getVelocity());
    lcd.printRowB(sensor2.getDistance(), sensor2.getVelocity());
  }

  greenLedF.safe   (sensor1.getDistance());
  yellowLedF.warning(sensor1.getDistance());
  redLedF.close    (sensor1.getDistance());

  greenLedB.safe   (sensor2.getDistance());
  yellowLedB.warning(sensor2.getDistance());
  redLedB.close    (sensor2.getDistance());

  int closest = min(sensor1.getDistance(), sensor2.getDistance());
  //buzzer.warnBuz  (closest);
  //buzzer.closeBuz (closest);
  //buzzer.dangerBuz(closest);
  buzzer.buzz(closest);

  sensor1.print();
  sensor2.print();
  imu.printFiltered();


  Serial.print("DATA,");
  Serial.print(millis());                 Serial.print(",");
  Serial.print(sensor1.getDistance());    Serial.print(",");
  Serial.print(sensor2.getDistance());    Serial.print(",");
  Serial.print(imu.getRoll());            Serial.print(",");
  Serial.print(imu.getPitch());           Serial.print(",");
  Serial.print(imu.getMagnitude());       Serial.print(",");
  Serial.println(imu.isCrashed() ? 1 : 0);


 // delay(500);
}
