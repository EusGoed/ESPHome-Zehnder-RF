#ifndef __COMPONENT_ZEHNDER_H__
#define __COMPONENT_ZEHNDER_H__

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/spi/spi.h"
#include "esphome/components/fan/fan_state.h"
#include "esphome/components/nrf905/nRF905.h"
// #include "esphome/core/preferences.h"

namespace esphome {
namespace zehnder {

#define FAN_FRAMESIZE 16        // Each frame consists of 16 bytes
#define FAN_TX_FRAMES 4         // Retransmit every transmitted frame 4 times
#define FAN_TX_RETRIES 10       // Retry transmission 10 times if no reply is received
#define FAN_TTL 250             // 0xFA, default time-to-live for a frame
#define FAN_REPLY_TIMEOUT 1000  // Wait 500ms for receiving a reply when doing a network scan

/* Fan device types */
enum {
  FAN_TYPE_BROADCAST = 0x00,       // Broadcast to all devices
  FAN_TYPE_MAIN_UNIT = 0x01,       // Fans
  FAN_TYPE_REMOTE_CONTROL = 0x03,  // Remote controls
  FAN_TYPE_CO2_SENSOR = 0x18
};  // CO2 sensors

/* Fan commands */
enum {
  FAN_FRAME_SETVOLTAGE = 0x01,  // Set speed (voltage / percentage)
  FAN_FRAME_SETSPEED = 0x02,    // Set speed (preset)
  FAN_FRAME_SETTIMER = 0x03,    // Set speed with timer
  FAN_NETWORK_JOIN_REQUEST = 0x04,
  FAN_FRAME_SETSPEED_REPLY = 0x05,
  FAN_NETWORK_JOIN_OPEN = 0x06,
  FAN_TYPE_FAN_SETTINGS = 0x07,  // Current settings, sent by fan in reply to 0x01, 0x02, 0x10
  FAN_FRAME_0B = 0x0B,
  FAN_NETWORK_JOIN_ACK = 0x0C,
  // FAN_NETWORK_JOIN_FINISH = 0x0D,
  FAN_TYPE_QUERY_NETWORK = 0x0D,
  FAN_TYPE_QUERY_DEVICE = 0x10,
  FAN_FRAME_SETVOLTAGE_REPLY = 0x1D
};

/* Fan speed presets */
enum {
  FAN_SPEED_AUTO = 0x00,    // Off:      0% or  0.0 volt
  FAN_SPEED_LOW = 0x01,     // Low:     30% or  3.0 volt
  FAN_SPEED_MEDIUM = 0x02,  // Medium:  50% or  5.0 volt
  FAN_SPEED_HIGH = 0x03,    // High:    90% or  9.0 volt
  FAN_SPEED_MAX = 0x04
};  // Max:    100% or 10.0 volt

#define NETWORK_LINK_ID 0xA55A5AA5
#define NETWORK_DEFAULT_ID 0xE7E7E7E7
#define FAN_JOIN_DEFAULT_TIMEOUT 10000

typedef enum { ResultOk, ResultBusy, ResultFailure } Result;

// struct ZehnderRFConfig {
//   uint32_t nrf905_tx_address;  // nRF905 Tx address
//   uint32_t fan_network_id;     // Fan (Zehnder/BUVA) network ID
//   uint8_t fan_my_device_type;  // Fan (Zehnder/BUVA) device type
//   uint8_t fan_my_device_id;    // Fan (Zehnder/BUVA) device ID
//   uint8_t fan_main_unit_type;  // Fan (Zehnder/BUVA) main unit type
//   uint8_t fan_main_unit_id;    // Fan (Zehnder/BUVA) main unit ID
// } PACKED;

typedef enum {
  StateStartup,
  StateStartDiscovery,
  StateDiscoveryWaitForLinkRequest,
  StateDiscoveryWaitForJoinResponse,
  StateDiscoveryJoinComplete,

  StateIdle,
  StateWaitQueryResponse,
  StateWaitSetSpeedResponse,
  StateWaitSetSpeedConfirm,

  StateNrOf  // Keep last
} State;

class ZehnderRF : public fan::FanState {
 public:
  ZehnderRF();

  void setup() override;

  void dump_config() override;
  void loop() override;

  int get_speed_count() { return 4; }

  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_rf(nrf905::nRF905 *const pRf) { rf_ = pRf; }

  void set_update_interval(const uint32_t interval) { interval_ = interval; }

 protected:
  void queryDevice(void);
  void setSpeed(const uint8_t speed, const uint8_t timer = 0);

  uint8_t createDeviceID(void);
  void discoveryStart(const uint8_t deviceId);

  Result startTransmit(const uint8_t *const pData, const int8_t rxRetries = -1,
                       const std::function<void(void)> callback = NULL);
  void rfComplete(void);
  void rfHandler(void);
  void rfHandleReceived(const uint8_t *const pData, const uint8_t dataLength);

  State state_{StateStartup};

  bool next_update_{true};
  nrf905::nRF905 *rf_;
  uint32_t interval_;

  uint8_t _txFrame[FAN_FRAMESIZE];

  // ESPPreferenceObject pref_;
  // ZehnderRFConfig config_;
};

}  // namespace zehnder
}  // namespace esphome

#endif /* __COMPONENT_ZEHNDER_H__ */