# EKG-CAN-Bus-System

Bachelorprojekt: EKG-Monitoring mit ATmega328P, MCP2515 (CAN) und OLED.  
Enthält Firmware (Sender/Empfänger) und Hardware-Screenshots.

## Projektbeschreibung
Dieses System besteht aus:
- **Sender**: Liest EKG-Daten vom AD8232-Sensor und sendet diese über den CAN-Bus.
- **Empfänger**: Empfängt die Daten, prüft sie auf Integrität (Sequenznummer, Zeitstempel, Checksumme) und zeigt sie in Echtzeit auf einem OLED-Display an.

### Merkmale
- Eigenes Schaltungsdesign & PCB-Layout (KiCad)
- CAN-Bus Kommunikation (MCP2515 + TJA1050)
- Sicherheitsmechanismen: Sequenznummer, Zeitstempel, Checksumme, Timeout, ACK/NACK
- Echtzeitdarstellung des EKG-Signals auf SSD1306 OLED

---

## Software-Überblick

### Sender
- Liest EKG-Daten vom AD8232-Sensor über A0.
- Sendet die Daten per MCP2515 (CAN, 1 Mbit/s) an den Empfänger.
- Erwartet eine Rückmeldung (ACK/NACK).

### Empfänger
- Empfängt CAN-Daten.
- Prüft Sequenznummer, Zeitstempel und Prüfsumme (XOR).
- Zeigt die EKG-Kurve live auf SSD1306 OLED an.
- Sendet ACK/NACK zurück.

### Benötigte Bibliotheken
- [`MCP2515`](https://github.com/autowp/arduino-mcp2515) – CAN via SPI
- [`Adafruit SSD1306`](https://github.com/adafruit/Adafruit_SSD1306) – OLED Display
- [`Adafruit GFX`](https://github.com/adafruit/Adafruit-GFX-Library) – Grafikbibliothek für OLED
- `Wire` (I²C, Arduino Standardbibliothek)
- `SPI` (Arduino Standardbibliothek)

---

## Hardware

### Sender
![Sender Schaltplan](Hardware/Sender-Schaltplan.png)
![Sender PCB Layout](Hardware/Sender-PCB-Layout.png)
![Senderplatine](Hardware/Senderplatine.jpg)

### Empfänger
![Empfaenger Schaltplan](Hardware/Empfaenger-Schaltplan.png)
![Empfaenger PCB Layout](Hardware/Empfaenger-PCB-Layout.png)
![Empfaengerplatine](Hardware/Empfaengerplatine.jpg)

### Gesamtsystem
![Gesamtsystem](Hardware/Gesamtsystem%20mit%20Verbindung%20durch%20Sub-D-Kabel.jpg)

---

## Firmware

Der Quellcode für Sender und Empfänger befindet sich im Ordner [`Firmware/`](Firmware/).

Eine detaillierte Beschreibung, benötigte Bibliotheken und eine Schritt-für-Schritt-Anleitung zum Kompilieren findest du in  
[`Firmware/README.md`](Firmware/README.md).

---

## Lizenz
Dieses Projekt steht unter der [MIT-Lizenz](LICENSE).
