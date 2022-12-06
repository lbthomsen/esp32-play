/*
 * FreeRTOS Example
 *
 */

#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_random.h"
#include "esp_chip_info.h"
#include "esp_log.h"

#define NUM_TASKS 10
#define TASK_DELAY 200

typedef struct {
    uint8_t task;
    uint32_t count;
} message_typedef;

static const char *LOG_TAG = "STATS";

TaskHandle_t load_task_handle[NUM_TASKS];
TaskHandle_t receive_task_handle;
TaskHandle_t stat_task_handle;
QueueHandle_t task_queue;

void load_task(void *pvParameters) {

    BaseType_t res;
    uint32_t task_delay = TASK_DELAY + (esp_random() % TASK_DELAY);
    message_typedef message;
    message.task = (uint8_t)(pvParameters);
    message.count = 0;

    ESP_LOGD(LOG_TAG, "Firing up task %d with delay = %lu", message.task, task_delay);

    for (;;) {
        vTaskDelay(task_delay / portTICK_PERIOD_MS);
        ++message.count;
        res = xQueueSendToBack(task_queue, (void* ) &message, (TickType_t ) 0);
        if (res != pdPASS) {
            ESP_LOGE(LOG_TAG, "Unable to send - returned %lu", res);
        }
    }
}

void receive_task(void *pvParameters) {
    ESP_LOGI(LOG_TAG, "Firing up receive task");

    BaseType_t res;
    message_typedef message;

    for (;;) {
        res = xQueueReceive(task_queue, &message, portMAX_DELAY);
        if (res == pdPASS) { // Wait for 10 secs max
            ESP_LOGD(LOG_TAG, "From %2d (%d): %5lu", message.task, uxQueueMessagesWaiting(task_queue), message.count);
        } else {
            ESP_LOGE(LOG_TAG, "Did not receive data but still alive");
        }
    }

}

void stat_task(void *pvParameters) {
    ESP_LOGI(LOG_TAG, "Firing up stat task");

    uint32_t total_run = 1;

    for (;;) {

        vTaskDelay(pdMS_TO_TICKS(10000));

        UBaseType_t number_of_tasks = uxTaskGetNumberOfTasks();
        TaskStatus_t *task_array = malloc(sizeof(TaskStatus_t) + number_of_tasks);
        if (task_array == NULL) {
            ESP_LOGE(LOG_TAG, "Unable to allocate memory");
        } else {

            number_of_tasks = uxTaskGetSystemState(task_array, number_of_tasks, &total_run);

            ESP_LOGI(LOG_TAG, "+------------+------+------------+------------+-------+------+");
            ESP_LOGI(LOG_TAG, "| Name       | No   | Tot        | Task       | %%     | HW   |");
            ESP_LOGI(LOG_TAG, "+------------+------+------------+------------+-------+------+");
            for (int task = 0; task < number_of_tasks; ++task) {

                double percentage = 100 * (double)((double)task_array[task].ulRunTimeCounter / (double)total_run);
                uint32_t pi = percentage;
                uint32_t pd = (double)((percentage - pi) * 100);

                ESP_LOGI(LOG_TAG, "| %-10s | %4lu | %10lu | %10lu | %2lu.%02lu | %4lu |", task_array[task].pcTaskName, task_array[task].xTaskNumber, total_run, task_array[task].ulRunTimeCounter, pi, pd, task_array[task].usStackHighWaterMark);
            }
            ESP_LOGI(LOG_TAG, "+------------+------+------------+------------+-------+------+");

            free(task_array);

        }

    }
}

void app_main(void) {

    ESP_LOGI(LOG_TAG, "Starting");

    BaseType_t res;
    char buf[16];

    task_queue = xQueueCreate(20, sizeof(message_typedef));
    if (task_queue == 0) {
        ESP_LOGE(LOG_TAG, "Unable to create queue");
    }

    res = xTaskCreate(stat_task, "stat", 5 * 1024, NULL, 1, &stat_task_handle);
    if (res != pdPASS) {
            ESP_LOGE(LOG_TAG, "Unable to create stat task - returned %lu", res);
    }

    res = xTaskCreate(receive_task, "rec", 3 * 1024, NULL, 3, &receive_task_handle);
    if (res != pdPASS) {
        ESP_LOGE(LOG_TAG, "Unable to create receive task - returned %lu", res);
    }

    for (uint32_t i = 0; i < NUM_TASKS; ++i) {
        sprintf(buf, "task %lu", i);
        res = xTaskCreate(load_task, buf, 1024, (void*) i, 2, &load_task_handle[i]);
        if (res != pdPASS) {
            ESP_LOGE(LOG_TAG, "Unable to create send task - returned %lu", res);
        }
        vTaskDelay(1);
    }

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(10000));
        ESP_LOGI(LOG_TAG, "Main task ping");
    }

}

/*
 * vim: ts=4 et nowrap autoindent
 */
