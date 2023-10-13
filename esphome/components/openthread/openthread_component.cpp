#include "openthread_component.h"

#include "openthread/srp_client.h"

#if !defined(USE_ESP_IDF)
#error Only supported on ESP_IDF
#endif

namespace esphome {
namespace openthread {

static const char *const TAG = "openthread";
OpenthreadComponent *global_ot_component;
OpenthreadComponent::OpenthreadComponent() { global_ot_component = this; }

float OpenthreadComponent::get_setup_priority() const { return setup_priority::WIFI; }

bool OpenthreadComponent::can_proceed() {
  const otNetifAddress *unicastAddrs = otIp6GetUnicastAddresses(esp_openthread_get_instance());

  for (const otNetifAddress *addr = unicastAddrs; addr; addr = addr->mNext) {
    if (addr->mAddressOrigin > 0) {
      otIp6AddressToString(&addr->mAddress, this->use_address_, OT_IP6_ADDRESS_STRING_SIZE);
      return true;
    }
  }
  return false;
}

static int hex_digit_to_int(char hex) {
  if ('A' <= hex && hex <= 'F') {
    return 10 + hex - 'A';
  }
  if ('a' <= hex && hex <= 'f') {
    return 10 + hex - 'a';
  }
  if ('0' <= hex && hex <= '9') {
    return hex - '0';
  }
  return -1;
}

static size_t hex_string_to_binary(const char *hex_string, uint8_t *buf, size_t buf_size) {
  int num_char = strlen(hex_string);

  if (num_char != buf_size * 2) {
    return 0;
  }
  for (size_t i = 0; i < num_char; i += 2) {
    int digit0 = hex_digit_to_int(hex_string[i]);
    int digit1 = hex_digit_to_int(hex_string[i + 1]);

    if (digit0 < 0 || digit1 < 0) {
      return 0;
    }
    buf[i / 2] = (digit0 << 4) + digit1;
  }

  return buf_size;
}

static esp_netif_t *init_openthread_netif(const esp_openthread_platform_config_t *config) {
  esp_netif_config_t cfg = ESP_NETIF_DEFAULT_OPENTHREAD();
  esp_netif_t *netif = esp_netif_new(&cfg);
  assert(netif != NULL);
  ESP_ERROR_CHECK(esp_netif_attach(netif, esp_openthread_netif_glue_init(config)));

  return netif;
}

static void ot_task_worker(void *aContext) {
  OpenthreadComponent *ot = (OpenthreadComponent *) aContext;

  esp_openthread_platform_config_t config = {
      .radio_config = ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG(),
      .host_config = ESP_OPENTHREAD_DEFAULT_HOST_CONFIG(),
      .port_config = ESP_OPENTHREAD_DEFAULT_PORT_CONFIG(),
  };

  // Initialize the OpenThread stack
  ESP_ERROR_CHECK(esp_openthread_init(&config));

  // The OpenThread log level directly matches ESP log level
  otLoggingSetLevel(CONFIG_LOG_DEFAULT_LEVEL);

  esp_netif_t *openthread_netif;
  // Initialize the esp_netif bindings
  openthread_netif = init_openthread_netif(&config);
  esp_netif_set_default_netif(openthread_netif);

  otInstance *instance = esp_openthread_get_instance();
  otOperationalDataset dataset;

  // if (!otDatasetIsCommissioned(instance)) {
  memset(&dataset, 0, sizeof(otOperationalDataset));

  // set channel & network key for easy join
  dataset.mChannel = ot->get_channel();
  dataset.mComponents.mIsChannelPresent = true;
  memcpy ( dataset.mNetworkKey.m8, ot->get_network_key().m8, sizeof(dataset.mNetworkKey.m8) );
  dataset.mComponents.mIsNetworkKeyPresent = true;

  // Auto activate
  otDatasetSetActive(instance, &dataset);
  // }

  otIp6SetEnabled(instance, true);
  otThreadSetEnabled(instance, true);

  // Run the main loop
  esp_openthread_launch_mainloop();

  // Clean up
  esp_netif_destroy(openthread_netif);
  esp_openthread_netif_glue_deinit();

  esp_vfs_eventfd_unregister();
  vTaskDelete(NULL);
}

void OpenthreadComponent::setup() {
  esp_vfs_eventfd_config_t eventfd_config = {
      .max_fds = 3,
  };

  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_vfs_eventfd_register(&eventfd_config));

  xTaskCreate(ot_task_worker, "qwerty", 4096, this, 5, NULL);
}

void OpenthreadComponent::loop() {}

bool OpenthreadComponent::is_connected() { return true; };

std::string OpenthreadComponent::get_use_address() const {
  std::string string_address(this->use_address_);
  return string_address;
}

void OpenthreadComponent::dump_config() {
  otInstance *instance = esp_openthread_get_instance();
  ESP_LOGCONFIG(TAG, "OPENTHREAD:");
  ESP_LOGCONFIG(TAG, "  name: %s", otThreadGetNetworkName(instance));
  ESP_LOGCONFIG(TAG, "  address: %s", this->use_address_);
}

void OpenthreadComponent::set_network_key(std::string network_key) {
  hex_string_to_binary(network_key.c_str(), this->network_key_.m8, sizeof(this->network_key_.m8));
}

}  // namespace openthread
}  // namespace esphome

