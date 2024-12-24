#include "WebDebug.h"
#ifdef WEB_DEBUG

#include <WebSocketsServer.h>

#include "WiFi.h"
#include "display.h"
#include "fonts.h"
#include "baro.h"
#include "gps.h"

// Websocket webserver
WebSocketsServer webSocket = WebSocketsServer(80);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload,
                    size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED: {
            IPAddress ip = webSocket.remoteIP(num);
            Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num,
                          ip[0], ip[1], ip[2], ip[3], payload);

            // send message to client
            webSocket.sendTXT(num, "Connected");
        } break;
        case WStype_TEXT:
            Serial.printf("[%u] get Text: %s\n", num, payload);

            // send message to client
            // webSocket.sendTXT(num, "message here");

            // send data to all connected clients
            // webSocket.broadcastTXT("message here");
            break;
        case WStype_BIN:
            Serial.printf("[%u] get binary length: %u\n", num, length);
            // hexdump(payload, length);

            // send message to client
            // webSocket.sendBIN(num, payload, length);
            break;
        case WStype_ERROR:
        case WStype_FRAGMENT_TEXT_START:
        case WStype_FRAGMENT_BIN_START:
        case WStype_FRAGMENT:
        case WStype_FRAGMENT_FIN:
            break;
    }
}

void webdebug_setup() {
    // Setup the display
    u8g2.begin();
    u8g2.clearBuffer();
    auto const PADDING = 15;
    auto y = 20;

    // Write the initial setup message
    u8g2.firstPage();
    do {
        u8g2.setFont(leaf_6x12);
        u8g2.setCursor(0, y);
        u8g2.print("Web Debugging");
        y += PADDING;
        u8g2.setCursor(0, y);
        u8g2.print("Enabling Wifi");
    } while (u8g2.nextPage());

    // Assume that WiFi has been previously setup
    // on this device.  Wait for WiFi to connect
    WiFi.begin();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }

    // Wait for a TCP connection
    u8g2.firstPage();
    do {
        u8g2.setFont(leaf_6x12);
        u8g2.setCursor(0, y);
        u8g2.print("Connected");
        y += PADDING;
        u8g2.setCursor(0, y);
        u8g2.print(WiFi.localIP());
        y += PADDING;
        u8g2.setCursor(0, y + PADDING);
        u8g2.print("Server Running");
    } while (u8g2.nextPage());

    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
}

void webdebug_update() {
    webSocket.loop();

    // Stream all of our telemetry out to all connected clients
    webSocket.broadcastTXT((String)"bcf:" + baro.climbRateFiltered);
    webSocket.broadcastTXT((String)"bp:" + baro.pressure);
    webSocket.broadcastTXT((String)"ba:" + baro.alt);
    webSocket.broadcastTXT((String)"ga:" + gps.altitude.value());
}

#endif  // WEB_DEBUG