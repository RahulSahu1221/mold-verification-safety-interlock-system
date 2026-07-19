#include <LiquidCrystal.h>

// ---- Pin assignments ----
const int flPin = 2, frPin = 3, rcPin = 4;   // sensors: Front-Left, Front-Right, Rear-Centre
const int greenLED = 8, redLED = 9, buzzer = 10, relay = 7;
const int startSwitchPin = 6;

LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);   // same wiring as before — now driving a 20x4 display

const int debounceDelay = 50;
int lastDisplayedState = -1;
bool prevSwitchState = false;
bool restartRequired = false;

// ---- Event log: circular buffer holding the last 5 events in RAM ----
const int LOG_SIZE = 5;
String eventLog[LOG_SIZE];   // each entry stored as "mm:ss  description"
int logCount = 0;            // how many entries written so far (caps at LOG_SIZE)
int logHead = 0;             // index where the NEXT event will be written

bool flOk, frOk, rcOk;       // current debounced readings, global so logEvent() can read them

bool readStable(int pin) {
  bool firstRead = digitalRead(pin);
  delay(debounceDelay);
  bool secondRead = digitalRead(pin);
  if (firstRead == secondRead) return firstRead;
  return false;
}

// ---- Elapsed time since power-on, as mm:ss ----
// Note: no RTC module used, so this is "time since simulation/power started", not real clock time.
String timeStamp() {
  unsigned long totalSec = millis() / 1000;
  int mm = (totalSec / 60) % 60;
  int ss = totalSec % 60;
  char buf[6];
  sprintf(buf, "%02d:%02d", mm, ss);
  return String(buf);
}

// ---- Stores one event in the circular buffer AND streams it out over Serial ----
void logEvent(String description) {
  String entry = timeStamp() + "  " + description;
  eventLog[logHead] = entry;
  logHead = (logHead + 1) % LOG_SIZE;
  if (logCount < LOG_SIZE) logCount++;
  Serial.println(entry);   // full history visible in Serial Monitor / Proteus virtual terminal
}

// ---- Names of currently-failing sensors, for the fault log line ----
String faultSensorNames() {
  String result = "";
  if (!flOk) result += "Ft Left ";
  if (!frOk) result += "Ft Rgt ";
  if (!rcOk) result += "Rear Cen ";
  result += "Fault";
  return result;
}

// ---- Nth most recent log entry (0 = latest); "" if it doesn't exist yet ----
String getRecentLog(int n) {
  if (n >= logCount) return "";
  int idx = (logHead - 1 - n + LOG_SIZE) % LOG_SIZE;
  return eventLog[idx];
}

void setup() {
  pinMode(flPin, INPUT); pinMode(frPin, INPUT); pinMode(rcPin, INPUT);
  pinMode(startSwitchPin, INPUT);
  pinMode(greenLED, OUTPUT); pinMode(redLED, OUTPUT);
  pinMode(buzzer, OUTPUT); pinMode(relay, OUTPUT);

  Serial.begin(9600);     // full event history streams here
  lcd.begin(20, 4);       // 20x4 display, same wiring as the old 16x2
  lcd.print("Mold Interlock");
  delay(1500);
  lcd.clear();
}

void loop() {
  flOk = readStable(flPin);
  frOk = readStable(frPin);
  rcOk = readStable(rcPin);
  bool allSeated   = flOk && frOk && rcOk;
  bool switchState = readStable(startSwitchPin);
  bool switchJustTurnedOn = switchState && !prevSwitchState;

  int currentState;

  if (!allSeated) {
    restartRequired = true;
    currentState = 1;
    digitalWrite(greenLED, LOW); digitalWrite(redLED, HIGH);
    digitalWrite(buzzer, HIGH);  digitalWrite(relay, LOW);

  } else if (restartRequired) {
    if (switchJustTurnedOn) restartRequired = false;
    if (restartRequired) {
      currentState = 3;
      digitalWrite(greenLED, LOW); digitalWrite(redLED, LOW);
      digitalWrite(buzzer, LOW);   digitalWrite(relay, LOW);
    } else {
      currentState = 0;
      digitalWrite(greenLED, HIGH); digitalWrite(redLED, LOW);
      digitalWrite(buzzer, LOW);    digitalWrite(relay, HIGH);
    }

  } else if (switchState) {
    currentState = 0;
    digitalWrite(greenLED, HIGH); digitalWrite(redLED, LOW);
    digitalWrite(buzzer, LOW);    digitalWrite(relay, HIGH);

  } else {
    currentState = 2;
    digitalWrite(greenLED, LOW); digitalWrite(redLED, LOW);
    digitalWrite(buzzer, LOW);   digitalWrite(relay, LOW);
  }

  // ---- On every state CHANGE: log the event(s), then redraw the display ----
  if (currentState != lastDisplayedState) {

    if (currentState == 1) {
      logEvent(faultSensorNames());
      if (lastDisplayedState == 0) logEvent("Mach. Stopped");   // a running machine faulting also stops it

    } else if (currentState == 3 && lastDisplayedState == 1) {
      logEvent("Fault Cleared");

    } else if (currentState == 0) {
      logEvent("Mach. Started");

    } else if (currentState == 2 && lastDisplayedState == 0) {
      logEvent("Mach. Stopped");   // manual stop, no fault involved
    }

    lcd.clear();
    lcd.setCursor(0, 0);

    if (currentState == 0) {
      lcd.print("Mold Seated OK");
      lcd.setCursor(0, 1);
      lcd.print("System Running");

    } else if (currentState == 1) {
      lcd.print("FAULT");
      lcd.setCursor(0, 1);
      if (!flOk) lcd.print("Front-Left ");
      if (!frOk) lcd.print("Front-Right ");
      if (!rcOk) lcd.print("Rear-Centre ");

    } else if (currentState == 2) {
      lcd.print("Mold Seated OK");
      lcd.setCursor(0, 1);
      lcd.print("Waiting: Start?");

    } else if (currentState == 3) {
      lcd.print("Fault Cleared");
      lcd.setCursor(0, 1);
      lcd.print("Toggle Reset");
    }

    // Bottom 2 rows: recent event history
    lcd.setCursor(0, 2);
    lcd.print(getRecentLog(0));   // most recent event
    lcd.setCursor(0, 3);
    lcd.print(getRecentLog(1));   // second-most-recent event

    lastDisplayedState = currentState;
  }

  prevSwitchState = switchState;
  delay(200);
}