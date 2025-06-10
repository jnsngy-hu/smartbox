#include <Keypad.h>
#include <Servo.h>

// mi melyik pinen van
#define RED_PIN 10
#define GREEN_PIN 9
#define SERVO_PIN 11
#define TRIG_PIN 13
#define ECHO_PIN 12
Servo szervo;

char password[] = {'5', '5', '1', '1'};
int passwordLength = 4;
int position = 0;
bool waitingForPassword = false;
bool isOpen = false;
bool servoMoved = false;

unsigned long redStartTime = 0;
bool redTimerRunning = false;

const int threshold = 6;

const byte ROWS = 3;
const byte COLS = 3;

char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'}
};

byte rowPins[ROWS] = {8, 7, 6};
byte colPins[COLS] = {5, 4, 3};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup() {
  Serial.begin(9600);
  // HCSR04
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  // LED
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  szervo.attach(SERVO_PIN);
  // szervó alapállapot
  szervo.write(180);
}
void loop() {
  int distance = measureDistance();
  // ha nyitva és a távolság érzékelési zónán belül van
  if (!isOpen && distance < threshold) {
    if (!redTimerRunning) {
      redStartTime = millis();
      redTimerRunning = true;
      Serial.println("tárgy érzékelve");
    }

    unsigned long elapsed = millis() - redStartTime;

    // LED villogás
    if (!servoMoved) {
      if (elapsed < 7000) {
        if ((elapsed / 500) % 2 == 0) setColor(255, 0);
        else setColor(0, 255);
      } else if (elapsed < 10000) {
        if ((elapsed / 150) % 2 == 0) setColor(255, 0);
        else setColor(0, 0);
      }
    }

    // 10 mp után záródik
    if (!servoMoved && elapsed >= 10000) {
      szervo.write(95);
      servoMoved = true;
      waitingForPassword = true;
      setColor(255, 0);
    }

  } else if (!isOpen && distance >= threshold) {
    // ha közben eltűnt az akadály
    if (!servoMoved) {
      redTimerRunning = false;
      setColor(0, 255);
    }
  }

  // PIN nyitás
  if (waitingForPassword) {
    char key = keypad.getKey();
    if (key) {
      Serial.print("K: "); Serial.println(key);

      if (key == password[position]) {
        position++;
        if (position >= passwordLength) {
          Serial.println("PIN OK");
          szervo.write(180);
          isOpen = true;
          waitingForPassword = false;
          redTimerRunning = false;
          servoMoved = false;
          setColor(0, 255);
          position = 0;
        }
      } else {
        Serial.println("PIN NOK");
        position = 0;
      }
    }
  }
  // ha nyitva és nincs semmi benne
  if (isOpen && distance >= threshold) {
    isOpen = false;
    redTimerRunning = false;
    servoMoved = false;
    waitingForPassword = false;
    position = 0;
    setColor(0, 255);
  }

  delay(100);
}

int measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return 999;
  return duration * 0.034 / 2;
}

void setColor(int r, int g) {
  analogWrite(RED_PIN, r);
  analogWrite(GREEN_PIN, g);
}
