#pragma once

#include "esp_openthread_types.h"

#define CONFIG_OPENTHREAD_NETWORK_POLLPERIOD_TIME 3000


#define ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG()              \
    {                                                      \
        .radio_mode = RADIO_MODE_NATIVE,                   \
    }

#define ESP_OPENTHREAD_DEFAULT_PORT_CONFIG()    \
    {                                           \
        .storage_partition_name = "nvs",        \
        .netif_queue_size = 10,                 \
        .task_queue_size = 10,                  \
    }

#define ESP_OPENTHREAD_DEFAULT_HOST_CONFIG()                \
    {                                                       \
        .host_connection_mode = HOST_CONNECTION_MODE_NONE,  \
    }
