#if defined(USE_ESP32) && defined(USE_OPENTHREAD)

#include <cstring>
#include "esphome/core/hal.h"
#include "esphome/core/log.h"
#include "srp_component.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"

#include "esp_netif.h"
#include "esp_netif_types.h"

#include <mdns.h>

#include "esp_openthread.h"
#include "esp_openthread_netif_glue.h"
#include "openthread/logging.h"
#include "openthread/thread.h"
#include "openthread/srp_client.h"
#include "openthread/srp_client_buffers.h"
// #include "common/code_utils.hpp"

// #include "openthread-core-config.h"

namespace esphome {
namespace mdns {

static const char *const TAG = "srp";

static int CopyString(char *aDest, int aSize, const char *aSource) {
  size_t len = strlen(aSource) + 1;
  if (len > aSize)
    return -1;

  memcpy(aDest, aSource, len);
  return len;
}

static int AppendString(char *aDest, int aSize, const char *aSource) {
  size_t sLen = strlen(aSource);
  size_t dLen = strlen(aDest);

  if (sLen + dLen + 1 > aSize)
    return -1;
  memcpy(aDest + dLen, aSource, sLen + 1);
  return sLen + dLen + 1;
}

static int AppendString(char *aDest, int aSize, uint8_t num) {
  size_t dLen = strlen(aDest);

  if (dLen + 2 > aSize)
    return -1;

  aDest[dLen++] = num;
  aDest[dLen] = 0;

  return dLen + 1;
}

void MDNSComponent::setup() {
  this->compile_records_();
  otInstance *instance = esp_openthread_get_instance();

  char *string;
  uint16_t size;
  uint8_t *txtBuffer;
  int len;
  otSrpClientBuffersServiceEntry *entry = nullptr;

  otSrpClientEnableAutoStartMode(instance, NULL, NULL);
  otSrpClientSetHostName(instance, this->hostname_.c_str());
  otSrpClientEnableAutoHostAddress(instance);

  for (const auto &service : this->services_) {
    entry = otSrpClientBuffersAllocateService(instance);
    string = otSrpClientBuffersGetServiceEntryInstanceNameString(entry, &size);
    CopyString(string, size, this->hostname_.c_str());

    string = otSrpClientBuffersGetServiceEntryServiceNameString(entry, &size);
    CopyString(string, size, service.service_type.c_str());
    AppendString(string, size, ".");
    AppendString(string, size, service.proto.c_str());

    entry->mService.mPort = service.port;
    txtBuffer = otSrpClientBuffersGetServiceEntryTxtBuffer(entry, &size);

    for (const auto &record : service.txt_records) {
      AppendString((char *) txtBuffer, size, record.key.length() + record.value.length() + 1);
      AppendString((char *) txtBuffer, size, record.key.c_str());
      AppendString((char *) txtBuffer, size, "=");
      AppendString((char *) txtBuffer, size, record.value.c_str());
      entry->mTxtEntry.mValueLength = strlen((char *) txtBuffer);
    }
    otSrpClientAddService(instance, &entry->mService);
  }
}

void MDNSComponent::on_shutdown() {}

}  // namespace srp
}  // namespace esphome

 #endif  // USE_ESP32 && USE_OPENTHREAD
