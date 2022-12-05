/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_random.h"
#include "esp_chip_info.h"
#include "esp_log.h"

#define NUM_TASKS 5
#define TASK_DELAY 1000

static const char *LOG_TAG = "STATS";

TaskHandle_t load_task_handle[NUM_TASKS];
TaskHandle_t receive_task_handle;
QueueHandle_t task_queue;

void load_task(void *pvParameters) {

	uint32_t task_delay = 500 + (esp_random() % 500);
	uint32_t send_value = (uint32_t)pvParameters;

	ESP_LOGI(LOG_TAG, "Firing up task %lu w delay = %lu", send_value, task_delay);

	for (;;) {
		vTaskDelay(task_delay / portTICK_PERIOD_MS);
		ESP_LOGI(LOG_TAG, "Task %lu sending", send_value);
		xQueueSend(task_queue, (void*) &send_value, (TickType_t) 0);
	}
}

void receive_task(void *pvParameters) {
	ESP_LOGI(LOG_TAG, "Firing up receive task");

	uint32_t receive_value;

	for (;;) {
        if (xQueueReceive(task_queue, &receive_value, 1000)) { // Wait for 10 secs max
            ESP_LOGD(LOG_TAG, "Received 0x%06lx", receive_value);

        } else {
            ESP_LOGI(LOG_TAG, "Did not receive data but still alive");
        }
	}

}

void app_main(void)
{

	ESP_LOGI(LOG_TAG, "Starting");

	BaseType_t res;
	char buf[16];

	task_queue = xQueueCreate(10, 4);
	if (task_queue == 0) {
		ESP_LOGE(LOG_TAG, "Unable to create queue");
	}

	res = xTaskCreate(receive_task, "rec", 4096, NULL, 2, &receive_task_handle);
	if (res != pdPASS) {
		ESP_LOGE(LOG_TAG, "Unable to create receive task - returned %lu", res);
	}

	for (uint32_t i = 0; i < NUM_TASKS; ++i) {
		sprintf(buf, "task %lu", i);
		res = xTaskCreate(load_task, buf, 4096, (void *)i, 1, &load_task_handle[i]);
		if (res != pdPASS) {
			ESP_LOGE(LOG_TAG, "Unable to create send task - returned %lu", res);
		}
	}


	for (;;) {
		vTaskDelay(10000 / portTICK_PERIOD_MS);
        ESP_LOGI(LOG_TAG, "Main task ping");
	}

}
