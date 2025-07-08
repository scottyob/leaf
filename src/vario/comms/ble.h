#pragma once

#include <NimBLEDevice.h>
#include "TinyGPSPlus.h"
#include "comms/message_types.h"
#include "etl/message_router.h"

// FreeRTOS Task for handling Bluetooth Operations
class BLE : public etl::message_router<BLE, GpsMessage, FanetPacket> {
 public:
  static BLE& get();

  /// @brief Sets up the Bluetooth instance.  Allocates memory, but does not start advertising
  void setup();

  /// @brief Starts advertising
  void start();

  /// @brief Stops advertising, does not free up resources
  void stop();

  /// @brief ends the service, tears down bluetooth resources
  void end();

  // On Receive handlers for the message router.
  void on_receive(const GpsMessage& msg);
  void on_receive(const FanetPacket& msg);

  void on_receive_unknown(const etl::imessage& msg) {}

 private:
  BLE()
      : pServer(nullptr),
        pService(nullptr),
        pCharacteristic(nullptr),
        pAdvertising(nullptr),
        started(false),
        message_router(0) {}

  bool started;  // Bluetooth advertising started

  NimBLEServer* pServer;
  NimBLEService* pService;
  NimBLECharacteristic* pCharacteristic;
  NimBLEAdvertising* pAdvertising;

  // Queue for handling messages into this task from either buffer or periodic wakeup events.
  QueueHandle_t xQueue;
  // Timer for periodically requesting an update be sent for our Baro updates
  TimerHandle_t xTimer;
  // Task to handle Bluetooth IO
  TaskHandle_t xTask;

  // FreeRTOS Task handler callbacks
  static void bleTask(void*);
  static void timerCallback(TimerHandle_t timer);

  void sendVarioUpdate();
  void sendGpsUpdate(TinyGPSPlus& gps);
  void sendFanetUpdate(FanetPacket& packetMsg);

  /**
   * Add's the checksum and postfix characters to a NMEA string. It may contain an existing checksum
   * that will be overwritten When the capacity is not enough, the result is undefined Note: Must
   * start with the prefix character $ (for performance reasons)
   * @param nmea example '$PFEC,GPint,RMC05'
   * @return             '$PFEC,GPint,RMC05*2D\r\n'
   */
  void addChecksumToNMEA(etl::istring& nmea);

  SemaphoreHandle_t gpsMutex;
  unsigned long lastGpsGgaMs = 0;
  unsigned long lastGpsGprmcMs = 0;
};
