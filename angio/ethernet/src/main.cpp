#include <Arduino.h>
#include <ETH.h>
#include <WiFi.h>
#include <driver/spi_master.h>

const int PIN_ETH_MISO = 10;
const int PIN_ETH_MOSI = 11;
const int PIN_ETH_SCK = 12;
const int PIN_ETH_CS = 13;
const int PIN_ETH_RESET = 8;
const int PIN_ETH_INT = 9;

esp_err_t init_spi(spi_host_device_t spi_host, int miso, int mosi, int sck,
                   int cs, uint32_t baudrate, spi_device_handle_t &handle) {
  spi_bus_config_t buscfg = {};
  buscfg.miso_io_num = miso, buscfg.mosi_io_num = mosi;
  buscfg.sclk_io_num = sck;
  buscfg.quadwp_io_num = -1;
  buscfg.quadhd_io_num = -1;

  spi_device_interface_config_t devcfg = {};
  devcfg.command_bits = 1;
  devcfg.address_bits = 7;
  devcfg.clock_speed_hz = baudrate;
  devcfg.spics_io_num = cs;
  devcfg.queue_size = 2;

  // Select DMA channel based on the hardware SPI host
  int dma_chan = SPI_DMA_CH_AUTO;

  esp_err_t ret = spi_bus_initialize(spi_host, &buscfg, dma_chan);
  if (ret != ESP_OK) {
    return ret;
  }

  ret = spi_bus_add_device(spi_host, &devcfg, &handle);
  if (ret != ESP_OK) {
    return ret;
  }
  return ESP_OK;
}

esp_err_t init_ethernet() {
  spi_device_handle_t spi_handle;
  esp_err_t ret = init_spi(SPI3_HOST, PIN_ETH_MISO, PIN_ETH_MOSI, PIN_ETH_SCK,
                           PIN_ETH_CS, 20000000, spi_handle);
  if (ESP_OK != ret) {
    ESP_LOGE("network", "Error initing SPI bus %x", ret);
    return ret;
  }

  ETH.begin(spi_handle, -1, PIN_ETH_RESET, PIN_ETH_INT, ETH_PHY_DM9051);

  WiFi.onEvent([](arduino_event_id_t event, arduino_event_info_t info) {
    switch (event) {
    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.println("Ethernet connected");
      Serial.printf("IP Address: %s\r\n", ETH.localIP().toString().c_str());
      break;

    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("Ethernet disconnected");
      break;
    }
  });

  return ESP_OK;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Angio ethernet example");

  esp_err_t ret = init_ethernet();
  if (ESP_OK != ret) {
    // error
  }
}

void loop() {
  static int i = 0;
  delay(1000);
}
