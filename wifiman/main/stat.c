/*
 * stat.c
 *
 *  Created on: Dec 2, 2022
 *      Author: lth
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include "stat.h"

static const char *TAG = "STAT";

//TaskHandle_t stat_task_handle = 0;

void stat_task(void *pvParameters) {
    ESP_LOGI(TAG, "Firing up stat task");

    uint32_t total_run = 1;
    TaskStatus_t task_array[16]; // We allocate enough 

    for (;;) {

        vTaskDelay(pdMS_TO_TICKS(10000)); // 10 secs between each run

        UBaseType_t number_of_tasks = uxTaskGetNumberOfTasks();
        //TaskStatus_t *task_array = malloc(sizeof(TaskStatus_t) + number_of_tasks);
        //if (task_array == NULL) {
        //    ESP_LOGE(TAG, "Unable to allocate memory");
        //} else {

            number_of_tasks = uxTaskGetSystemState(task_array, number_of_tasks, &total_run);

            ESP_LOGI(TAG, "Uptime %lu s (ticks: %lu)", (uint32_t)(xTaskGetTickCount() / 1000), total_run);
            ESP_LOGI(TAG, "+--------------+------+-----+------------+-------+------+");
            ESP_LOGI(TAG, "| Name         | No   | Pri | Task       | %%     | HW   |");
            ESP_LOGI(TAG, "+--------------+------+-----+------------+-------+------+");
            for (int task = 0; task < number_of_tasks; ++task) {

                float percentage = 100 * (float) ((float) task_array[task].ulRunTimeCounter / (float) total_run);
                uint32_t pi = percentage;
                uint32_t pd = (float) ((percentage - pi) * 100);

                ESP_LOGI(
                        TAG,
                        "| %-12s | %4u | %3u | %10lu | %2lu.%02lu | %4lu |",
                        task_array[task].pcTaskName,
                        task_array[task].xTaskNumber,
                        task_array[task].uxCurrentPriority,
                        task_array[task].ulRunTimeCounter,
                        pi,
                        pd,
                        task_array[task].usStackHighWaterMark
                        );

            }
            ESP_LOGI(TAG, "+--------------+------+-----+------------+-------+------+");

            //free(task_array);

        //}

    }
}

void stat_init(uint32_t delay) {
    ESP_LOGD(TAG, "initializing stats");

    BaseType_t res = xTaskCreate(stat_task, "stat", 2 * 1024, NULL, 2, NULL);
    if (res != pdPASS) {
        ESP_LOGE(TAG, "Unable to create stat task - returned %d", res);
    }

}
