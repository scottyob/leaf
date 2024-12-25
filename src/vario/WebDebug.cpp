#include "WebDebug.h"
#ifdef WEB_DEBUG

#include <WebSocketsServer.h>

#include "WiFi.h"
#include "WiFiUdp.h"
#include "baro.h"
#include "display.h"
#include "fonts.h"
#include "gps.h"

// Websocket webserver
// WebSocketsServer webSocket = WebSocketsServer(80);
WiFiUDP udp;
IPAddress broadcast_address(4294967295);

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

    Serial.end();
    Serial.begin(1500000);

}

void webdebug_update() {
    // Craft a UDP packet with our stats
    String packetPayload = (String) "bcf:" + baro.climbRateFiltered + "\n" +
                           "bp:" + baro.pressure + "\n" + "ba:" + baro.alt +
                           "\n" + "ga:" + gps.altitude.value() + "\n";
    
    // If using UDP to stream
    // udp.beginPacket(broadcast_address, 9999);
    // udp.write((const uint8_t*)packetPayload.c_str(), packetPayload.length());
    // udp.endPacket();

    Serial.println(packetPayload);

}

#endif  // WEB_DEBUG