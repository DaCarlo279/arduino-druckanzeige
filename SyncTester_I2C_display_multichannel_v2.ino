#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <LiquidCrystal_I2C.h>

// LCD-Display (Adresse ggf. anpassen, z. B. 0x27 oder 0x3F)
LiquidCrystal_I2C lcd(0x25, 20, 4);

// BMP280 Ã¼ber I2C-Multiplexer (TCA9548A)
Adafruit_BMP280 bmp;
const int NUM_SENSORS = 4;
const int HISTORY_SIZE = 5;
const byte TCA_ADDR = 0x70;

// Ringpuffer fÃ¼r geglÃ¤ttete Druckwerte
float pressureHistory[NUM_SENSORS][HISTORY_SIZE] = {0};
int historyIndex[NUM_SENSORS] = {0};

// Dynamische Kalibrierung (sanft gleitend)
float calibMin[NUM_SENSORS] = {9999, 9999, 9999, 9999};
float calibMax[NUM_SENSORS] = {0, 0, 0, 0};
const float calibrationRate = 0.01; // 1% Anpassung

// Multiplexer-Kanal wÃ¤hlen
void tcaSelect(uint8_t channel) {
  if (channel > 7) return;
  Wire.beginTransmission(TCA_ADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

// BMP280-Druck lesen (in mbar)
float readPressureFromSensor(uint8_t channel) {
  tcaSelect(channel);
  if (!bmp.begin(0x76)) {
    return -1;
  }
  return bmp.readPressure() / 100.0F; // Pa â†’ mbar
}

// Gleitenden Mittelwert verwalten
void addToHistory(uint8_t sensor, float value) {
  pressureHistory[sensor][historyIndex[sensor]] = value;
  historyIndex[sensor] = (historyIndex[sensor] + 1) % HISTORY_SIZE;
}

float getAveragePressure(uint8_t sensor) {
  float sum = 0;
  for (int i = 0; i < HISTORY_SIZE; i++) {
    sum += pressureHistory[sensor][i];
  }
  return sum / HISTORY_SIZE;
}

// Balken erzeugen (12 Zeichen + Druckwert)
String createBar(float pressure, uint8_t sensor) {
  float pMin = calibMin[sensor];
  float pMax = calibMax[sensor];

  if (pMax - pMin < 5) {
    pMin -= 2;
    pMax += 2;
  }

  int length = map(pressure, pMin, pMax, 0, 12);
  length = constrain(length, 0, 12);

  String bar = "";
  for (int i = 0; i < 12; i++) {
    bar += (i < length) ? "#" : " ";
  }
  return bar;
}

void setup() {
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Druckanzeige Init");
  delay(1000);

  // Historie initial fÃ¼llen
  for (uint8_t i = 0; i < NUM_SENSORS; i++) {
    float p = readPressureFromSensor(i);
    if (p <= 0) p = 1013.0; // Default-Wert, falls Sensorfehler
    for (int j = 0; j < HISTORY_SIZE; j++) {
      pressureHistory[i][j] = p;
    }
    calibMin[i] = p;
    calibMax[i] = p;
  }
}

void loop() {
  for (uint8_t i = 0; i < NUM_SENSORS; i++) {
    float pressure = readPressureFromSensor(i);
    lcd.setCursor(0, i);

    if (pressure > 0) {
      addToHistory(i, pressure);
      float avg = getAveragePressure(i);

      // Sanfte Kalibrierung
      if (avg < calibMin[i]) {
        calibMin[i] = calibMin[i] * (1.0 - calibrationRate) + avg * calibrationRate;
      } else {
        calibMin[i] = calibMin[i] * (1.0 - calibrationRate / 10.0) + avg * (calibrationRate / 10.0);
      }

      if (avg > calibMax[i]) {
        calibMax[i] = calibMax[i] * (1.0 - calibrationRate) + avg * calibrationRate;
      } else {
        calibMax[i] = calibMax[i] * (1.0 - calibrationRate / 10.0) + avg * (calibrationRate / 10.0);
      }

      // Balken + Druckwert anzeigen
      String bar = createBar(avg, i);
      char buf[6];
      snprintf(buf, sizeof(buf), "%4.0f", avg);
      lcd.print(bar);
      lcd.print(" ");
      lcd.print(buf);
    } else {
      lcd.print("Sensor ");
      lcd.print(i);
      lcd.print(": Fehler");
    }
  }

  delay(500);
}
