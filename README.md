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

## Hardware

### Sender
![Sender Schaltplan](Hardware/Sender-Schaltplan.png)
![Sender PCB Layout](Hardware/Sender-PCB-Layout.png)
![Senderplatine](Hardware/Senderplatine.jpg)

### Empfänger
![Empfänger Schaltplan](Hardware/Empfänger-Schaltplan.png)
![Empfänger PCB Layout](Hardware/Empfänger-PCB-Layout.png)
![Empfängerplatine](Hardware/Empfängerplatine.jpg)

### Gesamtsystem
![Gesamtsystem](Hardware/Gesamtsystem%20mit%20Verbindung%20durch%20Sub-D-Kabel.jpg)

---

## Firmware
Der Quellcode für Sender und Empfänger befindet sich im Ordner `Firmware/`.  
Enthalten sind alle Arduino-Sketches in C++.

---

## Lizenz
Dieses Projekt steht unter der [MIT-Lizenz](LICENSE).
