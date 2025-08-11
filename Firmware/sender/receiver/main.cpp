#include <Wire.h>
#include <SPI.h>
#include <mcp2515.h>
#include <Adafruit_SSD1306.h>
// Konstanten
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define EXPECTED_SENDER_ID 0x01      // Erwartete Senderkennung
#define CAN_ID_DATA 0x036             // CAN-ID für Daten
#define CAN_ID_ACK 0x037              // CAN-ID für ACK
#define DATA_PACKET_SIZE 8
#define TIMEOUT_MS 150               // 150 milliSekunden für Timeout
#define DELAY_LIMIT_MS 30           // Verzögerungslimit für den Zeitstempel in Millisekunden
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
struct can_frame canMsg;
struct can_frame ackMsg;  // Für das Senden der Rückmeldung
MCP2515 mcp2515(10);
uint8_t lastSequenceNumber = 0xFF; // Startwert für erste Nachricht
uint8_t lastTimestamp = 0xFF;      // Letzter Zeitstempel
unsigned long lastMessageTime = 0;
unsigned long lastRealTime = 0; // Letzte echte empfangene Zeit
int lastY = SCREEN_HEIGHT / 2;  // Y-Position des letzten Punktes für den Graphen
int xPos = 0;                   // X-Position für die Plot-Darstellung
bool dataReceived = false;       // Flag für empfangene Daten
bool firstMessageFlag = true;    // Flag für die erste Nachricht
bool isErrorDisplayed = false;   // Flag für angezeigte Fehlermeldung
bool isWaitingDisplayed = true;  // Flag für angezeigte Warte-Meldung
// CRC-Berechnung über die letzten 4 Bytes (data[4] bis data[7])
uint8_t calculateCRC(uint8_t* data, size_t length) {
    uint8_t crc = 0;
    for (size_t i = 0; i < length; i++) {
        crc ^= data[length - 1 - i];
    }
    return crc;
}
void sendAcknowledgement(uint8_t ack) {
    ackMsg.can_id = CAN_ID_ACK;
    ackMsg.can_dlc = 1;
    ackMsg.data[0] = ack;
    mcp2515.sendMessage(&ackMsg);
    Serial.print("ACK gesendet: ");
    Serial.println(ack == '1' ? "1" : "0");
}
void showError(const char* message) {
    Serial.println(message);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.print(message);
    display.display();
    // Sende eine negative Rückmeldung (ACK = '0')
    sendAcknowledgement('0');
    // Zurücksetzen der Sequenznummer und des Zeitstempels
    lastSequenceNumber = 0xFF;
    lastTimestamp = 0xFF;
    firstMessageFlag = true;
    // Reset EKG-Diagramm Position
    xPos = 0;
    lastY = SCREEN_HEIGHT / 2;
    // Setze das Fehlerstatus-Flag
    isErrorDisplayed = true;
}
void plotEKG(int value) {
    // Definieren Sie den erwarteten Bereich Ihrer EKG-Werte
    const int minValue = 300; // Beispielwert, anpassen basierend auf Ihrem Sensor
    const int maxValue = 700; // Beispielwert, anpassen basierend auf Ihrem Sensor
    // Skalierung der EKG-Werte auf die Höhe des Displays, invertiert
    int y = map(value, minValue, maxValue, SCREEN_HEIGHT - 1, 0);
    y = constrain(y, 0, SCREEN_HEIGHT - 1); // Sicherstellen, dass y innerhalb des Displays liegt
    // Zeichne eine Linie vom letzten Punkt zum aktuellen Punkt
    display.drawLine(xPos, lastY, xPos + 1, y, SSD1306_WHITE);
    // Aktualisiere die X-Position für den nächsten Punkt
    xPos++;
    if (xPos >= SCREEN_WIDTH) {
        xPos = 0;
        display.clearDisplay();
        display.drawLine(xPos, lastY, xPos + 1, y, SSD1306_WHITE); // Zeichne den ersten Punkt nach dem Clear
    }
    // Aktualisiere das Display bei jedem neuen Punkt für eine flüssigere Darstellung
    display.display();
    lastY = y;  // Update der letzten Y-Position
}
void processMessage() {
    uint8_t sequenceNumber = canMsg.data[0] & 0x0F;
    uint8_t senderId = canMsg.data[1];
    uint8_t receivedCrc = canMsg.data[2];
    uint8_t timestamp = canMsg.data[3];
    // EKG-Wert auslesen (2 Bytes, da der Sender 2 Bytes sendet)
    int ekgValue = 0;
    memcpy(&ekgValue, &canMsg.data[4], sizeof(ekgValue));
    // CRC berechnen über data[4] bis data[7]
    uint8_t calculatedCrc = calculateCRC(&canMsg.data[4], 4);
    // Debugging: Empfangene Daten anzeigen
    Serial.print("EKG-Wert empfangen: ");
    Serial.println(ekgValue);
    // Überprüfung auf Verfälschung
    if (senderId != EXPECTED_SENDER_ID) {
        showError("Ungültige Sender-ID!");
        return;
    }
    if (calculatedCrc != receivedCrc) {
        showError("CRC Fehler - Verfaelschung!");
        return;
    }
    // Sicherheitsüberprüfungen mit Zeitstempel und Sequenznummer
    if (firstMessageFlag) {
        // Erste Nachricht akzeptieren ohne Überprüfung
        lastSequenceNumber = sequenceNumber;
        lastTimestamp = timestamp;
        lastRealTime = millis();
        dataReceived = true;
        sendAcknowledgement('1');
        firstMessageFlag = false; // Setzt den Flag nach erster Nachricht
    }
    else {
        // Unbeabsichtigte Wiederholung: Wenn der Zeitstempel gleich dem vorherigen oder die Sequenznummer gleich der vorherigen ist
        if (timestamp == lastTimestamp || sequenceNumber == lastSequenceNumber) {
            showError("Wiederholung!");
            return;
        }
        // Falsche Reihenfolge: Wenn der neue Zeitstempel kleiner ist als der alte (ohne Überlauf) oder wenn die Sequenznummer außerhalb der erwarteten Reihenfolge ist
        else if ((timestamp < lastTimestamp && (lastTimestamp - timestamp) < 128) ||
                 (sequenceNumber != (lastSequenceNumber + 1) % 16)) {
            if (sequenceNumber == (lastSequenceNumber - 1 + 16) % 16) {
                showError("Falsche Rf!");
            } else {
                showError("Verlust!");
            }
            return;
        }
        // Unzulässige Verzögerung: Wenn der Zeitunterschied in realen Millisekunden zu groß ist
        else if (millis() - lastRealTime > DELAY_LIMIT_MS) {
            showError("Verzögerung!");
            return;
        }
        // Falls keine Fehler auftreten, normale Verarbeitung:
        else {
            lastSequenceNumber = sequenceNumber; // Update der letzten Sequenznummer
            lastTimestamp = timestamp;           // Update des letzten Zeitstempels
            lastRealTime = millis();             // Update des letzten echten Zeitpunkts
            dataReceived = true;
            sendAcknowledgement('1');
        }
    }
    // Überprüfen, ob eine Fehlermeldung angezeigt wurde und eine neue, gültige Nachricht empfangen wurde
    if (isErrorDisplayed) {
        display.clearDisplay();          // Entferne die Fehlermeldung sofort
        display.display();
        xPos = 0;                        // Reset der X-Position
        lastY = SCREEN_HEIGHT / 2;       // Reset der Y-Position
        isErrorDisplayed = false;        // Setze das Fehlerstatus-Flag zurück
    }
    // Überprüfen, ob die Warte-Meldung angezeigt wurde und eine neue, gültige Nachricht empfangen wurde
    if (isWaitingDisplayed) {
        display.clearDisplay();          // Entferne die Warte-Meldung sofort
        display.display();
        xPos = 0;                        // Reset der X-Position
        lastY = SCREEN_HEIGHT / 2;       // Reset der Y-Position
        isWaitingDisplayed = false;      // Setze das Warte-Flag zurück
    }
    // EKG-Diagramm aktualisieren
    plotEKG(ekgValue);
}
void setup() {
    Serial.begin(115200);
    SPI.begin();
    mcp2515.reset();
    mcp2515.setBitrate(CAN_1000KBPS, MCP_16MHZ);
    mcp2515.setNormalMode();
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;);
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    Serial.println(F("Empfänger bereit..."));
    // Zu Beginn "Warte auf Daten..." anzeigen
    display.setCursor(0, 0);
    display.print("Warte auf Daten...");
    display.display();
}
void loop() {
    unsigned long currentTime = millis();
    // Überprüfung, ob die Zeiterwartung überschritten wurde
    if (lastMessageTime != 0 && (currentTime - lastMessageTime) > TIMEOUT_MS) {
        showError("Timeout!");
        // Reset der Sequenznummer und Flags nach Timeout
        lastSequenceNumber = 0xFF;
        lastTimestamp = 0xFF;
        firstMessageFlag = true;
        // Reset EKG-Diagramm Position
        xPos = 0;
        lastY = SCREEN_HEIGHT / 2;
    }
    // Nachrichtenverarbeitung
    if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
        if (canMsg.can_id == CAN_ID_DATA && canMsg.can_dlc == DATA_PACKET_SIZE) {
            lastMessageTime = currentTime; // Aktualisiere den Zeitstempel bei erfolgreichem Empfang
            processMessage();
        }
    }
}
