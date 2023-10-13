#pragma once

#include <string>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "esphome/core/component.h"
#include "esphome/core/log.h"

#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"

#include "esp_netif.h"
#include "esp_netif_types.h"

#include "esp_openthread.h"
#include "esp_openthread_netif_glue.h"
#include "esp_ot_config.h"
#include "esp_vfs_eventfd.h"
#include "driver/uart.h"
#include "nvs_flash.h"
#include "openthread/logging.h"
#include "openthread/thread.h"

#if CONFIG_OPENTHREAD_FTD
#include "openthread/dataset_ftd.h"
#endif

namespace esphome {
namespace openthread {

class OpenthreadComponent : public Component {
 public:
  OpenthreadComponent();
  void setup() override;
  void loop() override;
  float get_setup_priority() const override;
  bool can_proceed() override;
  bool is_connected();
  std::string get_use_address() const;
  void set_network_key(otNetworkKey network_key) { network_key_ = network_key; };
  void set_network_key(std::string network_key);
  void set_channel(uint8_t channel) { channel_ = channel; }
  otNetworkKey get_network_key() { return network_key_; }
  uint8_t get_channel() { return channel_; }
  void dump_config() override;

 protected:
  otNetworkKey network_key_;
  uint8_t channel_;
  char use_address_[OT_IP6_ADDRESS_STRING_SIZE];

};
extern OpenthreadComponent *global_ot_component;

}  // namespace openthread
}  // namespace esphome
