#include "nRF905.h"
#include "esphome/core/log.h"

namespace esphome {
namespace nrf905 {

static const char *TAG = "nRF905";

nRF905::nRF905(void) {}

void nRF905::setup() {
  ESP_LOGD(TAG, "Starting nRF905 initialization");

  // SPI Setup
  this->spi_setup();  // Use SPIDevice's implementation

  // GPIO pin setups
  if (this->_gpio_pin_am != NULL) {
    ESP_LOGD(TAG, "Setting up AM pin");
    this->_gpio_pin_am->setup();
  } else {
    ESP_LOGW(TAG, "AM pin is not configured.");
  }

  if (this->_gpio_pin_cd != NULL) {
    ESP_LOGD(TAG, "Setting up CD pin");
    this->_gpio_pin_cd->setup();
  } else {
    ESP_LOGW(TAG, "CD pin is not configured.");
  }

  if (this->_gpio_pin_ce != NULL) {
    ESP_LOGD(TAG, "Setting up CE pin");
    this->_gpio_pin_ce->setup();
  } else {
    ESP_LOGE(TAG, "CE pin is not configured. Initialization aborted.");
    return;
  }

  if (this->_gpio_pin_dr != NULL) {
    ESP_LOGD(TAG, "Setting up DR pin");
    this->_gpio_pin_dr->setup();
  } else {
    ESP_LOGW(TAG, "DR pin is not configured.");
  }

  if (this->_gpio_pin_pwr != NULL) {
    ESP_LOGD(TAG, "Setting up PWR pin");
    this->_gpio_pin_pwr->setup();
  } else {
    ESP_LOGE(TAG, "PWR pin is not configured. Initialization aborted.");
    return;
  }

  if (this->_gpio_pin_txen != NULL) {
    ESP_LOGD(TAG, "Setting up TXEN pin");
    this->_gpio_pin_txen->setup();
  } else {
    ESP_LOGE(TAG, "TXEN pin is not configured. Initialization aborted.");
    return;
  }

  // Initialize the device in PowerDown mode
  this->setMode(PowerDown);

  // Read and validate configuration registers
  this->readConfigRegisters();

  // Set up default configurations
  this->_config.band = true;  // Use the 868 MHz band
  this->_config.channel = 118;
  this->_config.crc_enable = true;
  this->_config.crc_bits = 16;
  this->_config.tx_power = 10;  // Max power
  this->_config.rx_power = PowerNormal;
  this->_config.rx_address = 0x89816EA9;  // Zehnder Network Link ID
  this->_config.rx_address_width = 4;
  this->_config.rx_payload_width = 16;
  this->_config.tx_address_width = 4;
  this->_config.tx_payload_width = 16;
  this->_config.xtal_frequency = 16000000;
  this->_config.clkOutFrequency = ClkOut500000;
  this->_config.clkOutEnable = false;

  // Write configurations
  this->writeConfigRegisters();
  this->writeTxAddress(0x89816EA9);

  // Set mode to idle
  this->setMode(Idle);

  ESP_LOGD(TAG, "nRF905 Setup complete");
}

void nRF905::dump_config() {
  ESP_LOGCONFIG(TAG, "Dumping nRF905 configuration:");

  LOG_PIN("  CS Pin:", this->cs_);
  if (this->_gpio_pin_am != NULL) {
    LOG_PIN("  AM Pin:", this->_gpio_pin_am);
  } else {
    ESP_LOGW(TAG, "AM pin not configured.");
  }
  if (this->_gpio_pin_dr != NULL) {
    LOG_PIN("  DR Pin:", this->_gpio_pin_dr);
  } else {
    ESP_LOGW(TAG, "DR pin not configured.");
  }
  if (this->_gpio_pin_cd != NULL) {
    LOG_PIN("  CD Pin:", this->_gpio_pin_cd);
  } else {
    ESP_LOGW(TAG, "CD pin not configured.");
  }
  LOG_PIN("  CE Pin:", this->_gpio_pin_ce);
  LOG_PIN("  PWR Pin:", this->_gpio_pin_pwr);
  LOG_PIN("  TXEN Pin:", this->_gpio_pin_txen);
}

void nRF905::setMode(const Mode mode) {
  ESP_LOGD(TAG, "Setting mode: %d", mode);

  // Set power
  if (this->_gpio_pin_pwr != NULL) {
    this->_gpio_pin_pwr->digital_write(mode != PowerDown);
  }

  // Set CE
  if (this->_gpio_pin_ce != NULL) {
    this->_gpio_pin_ce->digital_write(mode == Receive || mode == Transmit);
  }

  // Enable TX
  if (this->_gpio_pin_txen != NULL) {
    this->_gpio_pin_txen->digital_write(mode == Transmit);
  }

  this->_mode = mode;
}

}  // namespace nrf905
}  // namespace esphome
