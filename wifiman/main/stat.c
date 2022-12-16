/*
 * stat.c
 *
 *  Created on: Dec 2, 2022
 *      Author: lth
 */

#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"

#include "stat.h"

static const char *TAG = "stat";

void stat_task(void *pvParameters) {
    ESP_LOGD(TAG, "Firing up stat task");

    uint32_t total_run;
    double percentage;
    uint32_t pi;
    uint32_t pd;
    TaskStatus_t task_array[MAX_TASKS];
    UBaseType_t number_of_tasks;

    for (;;) {

        vTaskDelay(pdMS_TO_TICKS(10000)); // 10 secs between each run

        //number_of_tasks = uxTaskGetNumberOfTasks();
        number_of_tasks = uxTaskGetSystemState(task_array, MAX_TASKS, &total_run);

        ESP_LOGI(TAG, "Uptime %lu s (ticks: %lu)", (uint32_t )(xTaskGetTickCount() / 1000), total_run);
        ESP_LOGI(TAG, "+--------------+------+-----+------------+-------+------+");
        ESP_LOGI(TAG, "| Name         | No   | Pri | Task       | %%     | HW   |");
        ESP_LOGI(TAG, "+--------------+------+-----+------------+-------+------+");
        for (int task = 0; task < number_of_tasks; ++task) {

            percentage = 100 * (double) ((double) task_array[task].ulRunTimeCounter / (double) total_run);
            pi = percentage;
            pd = (double) ((percentage - pi) * 100);

            ESP_LOGI(TAG, "| %-12s | %4u | %3u | %10lu | %2lu.%02lu | %4lu |", task_array[task].pcTaskName, task_array[task].xTaskNumber, task_array[task].uxCurrentPriority,
                    task_array[task].ulRunTimeCounter, pi, pd, task_array[task].usStackHighWaterMark);

        }
        ESP_LOGI(TAG, "+--------------+------+-----+------------+-------+------+");

    }
}

void stat_init(uint32_t delay) {
    ESP_LOGD(TAG, "initializing stats");

    BaseType_t res = xTaskCreate(stat_task, "stat", 4 * 1024, NULL, 2, NULL);
    if (res != pdPASS) {
        ESP_LOGE(TAG, "Unable to create stat task - returned %d", res);
    }

}
