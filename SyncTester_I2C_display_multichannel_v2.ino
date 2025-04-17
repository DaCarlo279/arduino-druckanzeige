#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_BMP280.h>

#define TCAADDR 0x70
#define NUM_SENSORS 4
#define HISTORY_SIZE 5

LiquidCrystal_I2C lcd(0x25, 20, 4);  // Deine Display-Adresse
Adafruit_BMP280 bmp;

float history[NUM_SENSORS][HISTORY_SIZE];
uint8_t historyIndex[NUM_SENSORS] = {0};

const float minPressure = 600.0;
const float maxPressure = 1050.0;

void tcaselect(uint8_t i) {
  if (i > 7) return;
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();
}

void addToHistory(uint8_t sensor, float value) {
  history[sensor][historyIndex[sensor]] = value;
  historyIndex[sensor] = (historyIndex[sensor] + 1) % HISTORY_SIZE;
}

float getAveragePressure(uint8_t sensor) {
  float sum = 0.0;
  for (int i = 0; i < HISTORY_SIZE; i++) {
    sum += history[sensor][i];
  }
  return sum / HISTORY_SIZE;
}

float readPressureFromSensor(uint8_t channel) {
  tcaselect(channel);
  delay(10);

  if (!bmp.begin(0x76)) {
    Serial.print("Sensor ");
    Serial.print(channel);
    Serial.println(" Init fehlgeschlagen!");
    return -1;
  }

  return bmp.readPressure() / 100.0; // Pa → mbar
}

String createBar(float value, uint8_t sensorIndex) {
  int barLength = map(value, minPressure, maxPressure, 0, 16);
  barLength = constrain(barLength, 0, 16);
  String bar = "";
  for (int i = 0; i < barLength; i++) {
    bar += (char)255;
  }
  return bar;
}

void setup() {
  //Serial.begin(115200);
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Druckanzeige Init");
  delay(1000);
  lcd.clear();

  for (int i = 0; i < NUM_SENSORS; i++) {
    for (int j = 0; j < HISTORY_SIZE; j++) {
      history[i][j] = 1013.0;
    }
  }
}

void loop() {
  for (uint8_t i = 0; i < NUM_SENSORS; i++) {
    float pressure = readPressureFromSensor(i);

    /* Debug: Wo schreiben wir hin?
    Serial.print(">> LCD-Zeile ");
    Serial.print(i);
    Serial.print(" | Sensor ");
    Serial.println(i);
    */

    // Zeile löschen
    lcd.setCursor(0, i);
    lcd.print("                    ");
    lcd.setCursor(0, i);

    /*
    Serial.print("Sensor ");
    Serial.print(i);
    Serial.print(" | Rohdruck: ");
    Serial.print(pressure);
    Serial.print(" mbar");
    */

    if (pressure > 0) {
      addToHistory(i, pressure);
      float avg = getAveragePressure(i);

      /*Serial.print(" | Glatt: ");
      Serial.println(avg);
      */

      String bar = createBar(avg, i);

      lcd.setCursor(0, i);
      lcd.print(bar);
      lcd.setCursor (14, i);
      lcd.print (" ");
      lcd.print((int)avg); // nur ganze Zahl, keine Kommas
    } else {
      Serial.println(" | Fehler beim Auslesen!");
      lcd.setCursor(0, i);
      lcd.print("Sensor ");
      lcd.print(i);
      lcd.print(": Fehler");
    }
  }

  delay(100);
}
