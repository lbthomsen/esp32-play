
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include "stat.h"
#include "wifiman.h"

static const char *TAG = "MAIN";

void app_main(void)
{

	wifiman_init();
	//stat_init();

    while (true) {
        ESP_LOGD(TAG, "Ping");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
