# Firmware – EKG CAN-Bus System

## Aufbau
Dieses Projekt enthält die Firmware für:
- **Sender** (`Firmware/sender/main.cpp`):  
  Liest EKG-Daten vom AD8232-Sensor über A0, sendet sie per MCP2515 (CAN, 1 Mbit/s) an den Empfänger und erwartet ein ACK/NACK.
- **Empfänger** (`Firmware/receiver/main.cpp`):  
  Empfängt CAN-Daten, prüft Sequenznummer, Zeitstempel und Checksumme (XOR), zeigt die EKG-Kurve live auf SSD1306 OLED an und sendet ACK/NACK zurück.

## Bibliotheken
Für beide Sketche werden folgende Bibliotheken benötigt:
- **MCP2515** (CAN via SPI)
- **Adafruit SSD1306** (OLED-Display, I²C)
- **Adafruit GFX** (Grafikbibliothek für OLED)
- **Wire** (I²C, Standardbibliothek in Arduino)
- **SPI** (Standardbibliothek in Arduino)

## Hardware-Voraussetzungen
- **Sender:**
  - ATmega328P (Arduino Uno kompatibel, Standalone oder Board)
  - AD8232 EKG-Sensor
  - MCP2515 CAN-Bus-Modul mit TJA1050
  - Spannungsversorgung 5V (L7805-Regler von 9V-Batterie)
- **Empfänger:**
  - ATmega328P (Arduino Uno kompatibel, Standalone oder Board)
  - MCP2515 CAN-Bus-Modul mit TJA1050
  - SSD1306 OLED-Display (I²C, 128×64)
  - Spannungsversorgung 5V (L7805-Regler von 9V-Batterie)

## Kompilieren & Hochladen
1. **Arduino IDE öffnen**
2. Board auswählen:  
   `Werkzeuge → Board → Arduino Uno` (oder kompatibles Board)
3. COM-Port auswählen:
   `Werkzeuge → Port → [dein Arduino-Port]`
4. Benötigte Bibliotheken installieren:
   - `Sketch → Bibliothek einbinden → Bibliotheken verwalten`
   - Nach „MCP2515“, „Adafruit SSD1306“, „Adafruit GFX“ suchen und installieren
5. Passenden Sketch öffnen:
   - Sender: `Firmware/sender/main.cpp`
   - Empfänger: `Firmware/receiver/main.cpp`
6. Hochladen:
   - Klick auf „→“ (Upload-Symbol) in der Arduino IDE

## Lizenz
Dieses Projekt steht unter der [MIT-Lizenz](../LICENSE).
