# arduino-druckanzeige
Druckanzeige mit BMP280 und LCD

## Beschreibung
Dieses Projekt verwendet einen **Arduino** und **BMP280-Sensoren**, um den Luftdruck zu messen und ihn auf einem **I2C LCD-Display** darzustellen. Es werden vier Sensoren über einen **TCA9548A I2C-Multiplexer** angesteuert. Die Druckwerte werden gelättet und auf einem Balken angezeigt.

## Funktionen:
- Anzeige des Drucks von vier Sensoren.
- Sanfte Kalibirierung der Druckwerte.
- Druckdarstellung als Balken und numerischer Wert (in mbar).

## Verkabelung
- **Sensoren**: BMP280 über I2C-Multiplexer TCA9548A.
- **LCD**: I2C (20x4) Display.

## Software
Die Software liest regelmäßig die Werte der BMP280-Sensoren und berechnet einen gleitenden Mittelwert, um Ausreißer zu vermeiden. Die Kalibrierung erfolgt sanft, sodass das System die Min/Max-Werte über die Zeit dynamisch anpasst.

## Lizenz
Dieses Projekt ist unter der **MIT-Lizenz** lizenziert.
