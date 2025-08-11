#include <Wire.h>
#include <SPI.h>
#include <mcp2515.h>
#define REPORTING_PERIOD_MS 10
#define SENDER_ID 0x01 // Eindeutige Senderkennung
#define LED_PIN 9      // Ändere den LED-Pin auf Pin 9
#define TIMEOUT 500    // Timeout für die Rückmeldung in Millisekunden
struct can_frame canMsg;
struct can_frame ackMsg; // Für den Empfang der Rückmeldung
MCP2515 mcp2515(10);
uint8_t sequenceNumber = 0; // Laufende Nummer
uint8_t calculateCRC(uint8_t* data, size_t length) {
    uint8_t crc = 0;
    for (size_t i = 0; i < length; i++) {
        crc ^= data[length - 1 - i]; // CRC über die letzten 4 Bytes der Daten berechnen
    }
    return crc;
}
void setup() {
    Serial.begin(115200);
    SPI.begin();
    mcp2515.reset();
    mcp2515.setBitrate(CAN_1000KBPS, MCP_16MHZ);
    mcp2515.setNormalMode();
    pinMode(A0, INPUT);   // EKG-Daten vom Sensor
    pinMode(LED_PIN, OUTPUT); // LED-Pin als Ausgang
    digitalWrite(LED_PIN, LOW); // LED zu Beginn ausschalten
    Serial.println("Sender bereit...");
}
void loop() {
    // Lies den EKG-Wert vom AD8232-Sensor
    int ekgValue = analogRead(A0);
    // Bereite die CAN-Nachricht vor
    canMsg.can_id  = 0x036;
    canMsg.can_dlc = 8;
    canMsg.data[0] = sequenceNumber & 0x0F; // Laufende Nummer
    canMsg.data[1] = SENDER_ID;             // Senderkennung
    // Zeitstempel berechnen (Millis geteilt durch 10, damit der Wert in einem Byte passt)
    uint8_t timestamp = (millis() / 10) & 0xFF;
    canMsg.data[3] = timestamp; // Zeitstempel im 4. Byte speichern
    // EKG-Wert im Datenpaket übertragen (2 Bytes)
    memcpy(&canMsg.data[4], &ekgValue, sizeof(ekgValue));
    // CRC berechnen und speichern
    uint8_t crc = calculateCRC(&canMsg.data[4], 4);
    canMsg.data[2] = crc; // CRC in das 3. Byte speichern
    // Debugging: EKG-Daten und CRC anzeigen
    Serial.print("EKG-Daten gesendet: ");
    Serial.println(ekgValue);
    Serial.print("Zeitstempel: ");
    Serial.println(timestamp);
    // Zeige die gesendeten CAN-Nachrichtenbytes an
    Serial.print("Sending CAN message bytes: ");
    for (int i = 0; i < 8; i++) {
        Serial.print(canMsg.data[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
    // Sende die Nachricht über CAN
    mcp2515.sendMessage(&canMsg);
    // Warte auf die Rückmeldung vom Empfänger
    bool receivedAck = false;
    unsigned long startTime = millis();
    digitalWrite(LED_PIN, LOW); // LED standardmäßig aus
    while (millis() - startTime < TIMEOUT) {
        if (mcp2515.readMessage(&ackMsg) == MCP2515::ERROR_OK) {
            // Überprüfe, ob es sich um die ACK-Nachricht handelt
            if (ackMsg.can_id == 0x037 && ackMsg.can_dlc == 1) {
                if (ackMsg.data[0] == '1') {
                    Serial.println("Nachricht bestätigt.");
                    receivedAck = true;
                    break;
                } else if (ackMsg.data[0] == '0') {
                    Serial.println("Nachricht abgelehnt.");
                    receivedAck = false;
                    break;
                }
            }
        }
    }
    if (!receivedAck) {
        Serial.println("Fehler: Keine gültige Rückmeldung erhalten.");
        digitalWrite(LED_PIN, HIGH); // LED einschalten bei Fehler
    } else {
        digitalWrite(LED_PIN, LOW);  // LED ausschalten bei Erfolg
    }
    sequenceNumber++; // Erhöhe die Sequenznummer
    delay(REPORTING_PERIOD_MS); // Wartezeit zwischen den Nachrichten
}
