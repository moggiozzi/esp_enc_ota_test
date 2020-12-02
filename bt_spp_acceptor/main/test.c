#include "test.h"

#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "errno.h"

typedef struct fw_update_info_TAG {
	bool image_header_was_checked;
	const esp_partition_t *configured;
	const esp_partition_t *running;
    esp_ota_handle_t update_handle;
    const esp_partition_t *update_partition;
    uint32_t binary_file_length;
    uint32_t state;
}fw_update_info_t;

fw_update_info_t fw_update_info;

void fw_update_printBootInfo(void) {
	fw_update_info.configured = esp_ota_get_boot_partition();
	fw_update_info.running = esp_ota_get_running_partition();

	ESP_LOGW(__func__, "Boot 0x%08x, Running 0x%08x",
			fw_update_info.configured->address, fw_update_info.running->address);

	esp_app_desc_t running_app_info;
	if (esp_ota_get_partition_description(fw_update_info.running, &running_app_info) == ESP_OK) {
		ESP_LOGI(__func__, "Running fw version: %s", running_app_info.version);
	}

	const esp_partition_t* last_invalid_app = esp_ota_get_last_invalid_partition();
	esp_app_desc_t invalid_app_info;
	if (esp_ota_get_partition_description(last_invalid_app, &invalid_app_info) == ESP_OK) {
		ESP_LOGI(__func__, "Last invalid fw version: %s", invalid_app_info.version);
	}
}

void task_payload1(void *arg);
void task_ota(void *arg);
void run_test(void) {

	memset(&fw_update_info, 0, sizeof(fw_update_info));
	fw_update_printBootInfo();

	//run payload1
	//xTaskCreate(task_payload1, "task_payload1", 4*1024, NULL, 10, NULL);

	//vTaskDelay(pdMS_TO_TICKS(30000));

	// run otatest
	xTaskCreate(task_ota, "task_ota", 4*1024, NULL, 9, NULL);
}

void task_ota(void *arg) {
	esp_err_t err;
	// mark valid
	ESP_LOGI(__func__, "Try esp_ota_mark_app_valid_cancel_rollback()...");
	err = esp_ota_mark_app_valid_cancel_rollback();
	ESP_LOGI(__func__, "esp_ota_mark_app_valid_cancel_rollback ret %d", err);
	vTaskDelay(pdMS_TO_TICKS(1000));

	// get next slot
	fw_update_info.update_partition = esp_ota_get_next_update_partition(NULL);
	if (fw_update_info.update_partition != NULL) {
		ESP_LOGI(__func__, "next slot addr 0x%x, size 0x%x, enc %d",
				fw_update_info.update_partition->address,
				fw_update_info.update_partition->size,
				fw_update_info.update_partition->encrypted);
	} else {
		ESP_LOGI(__func__, "esp_ota_get_next_update_partition() failed");
	}
	vTaskDelay(pdMS_TO_TICKS(1000));

	while(1) {
		// change slot
		ESP_LOGI(__func__, "Try esp_ota_set_boot_partition()...");
		err = esp_ota_set_boot_partition(fw_update_info.update_partition);
		if (err == ESP_OK) {
			ESP_LOGI(__func__, "Please restart! Boot at 0x%x", fw_update_info.update_partition->address);
		} else {
			ESP_LOGE(__func__, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
		}
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

#define SQRT2           1.414213562f
#define NUM_SAMPLES (40)
const float cosine_table[NUM_SAMPLES] = {
   (float) 1,          (float) 0.987688341,(float) 0.951056516,(float) 0.891006524,(float) 0.809016994,
   (float) 0.707106781,(float) 0.587785252,(float) 0.4539905,  (float) 0.309016994,(float) 0.156434465,
   (float) 6.12574E-17,(float)-0.156434465,(float)-0.309016994,(float)-0.4539905,  (float)-0.587785252,
   (float)-0.707106781,(float)-0.809016994,(float)-0.891006524,(float)-0.951056516,(float)-0.987688341,
   (float)-1,          (float)-0.987688341,(float)-0.951056516,(float)-0.891006524,(float)-0.809016994,
   (float)-0.707106781,(float)-0.587785252,(float)-0.4539905,  (float)-0.309016994,(float)-0.156434465,
   (float)-1.83772E-16,(float) 0.156434465,(float) 0.309016994,(float) 0.4539905,  (float) 0.587785252,
   (float) 0.707106781,(float) 0.809016994,(float) 0.891006524,(float) 0.951056516,(float) 0.987688341
};

const float sine_table[NUM_SAMPLES] = {
   (float) 0,          (float) 0.156434465,(float) 0.309016994,(float) 0.4539905,  (float) 0.587785252,
   (float) 0.707106781,(float) 0.809016994,(float) 0.891006524,(float) 0.951056516,(float) 0.987688341,
   (float) 1,          (float) 0.987688341,(float) 0.951056516,(float) 0.891006524,(float) 0.809016994,
   (float) 0.707106781,(float) 0.587785252,(float) 0.4539905,  (float) 0.309016994,(float) 0.156434465,
   (float) 1.22515E-16,(float)-0.156434465,(float)-0.309016994,(float)-0.4539905,  (float)-0.587785252,
   (float)-0.707106781,(float)-0.809016994,(float)-0.891006524,(float)-0.951056516,(float)-0.987688341,
   (float)-1,          (float)-0.987688341,(float)-0.951056516,(float)-0.891006524,(float)-0.809016994,
   (float)-0.707106781,(float)-0.587785252,(float)-0.4539905,  (float)-0.309016994,(float)-0.156434465
};

struct xy_comp {
  float x;
  float y;
};

float calc_effective_val_using_xy_components(struct xy_comp *xy) {
  return sqrtf(xy->x * xy->x + xy->y * xy->y);
}

void calc_dft(float *p_channel_data,  int_fast8_t n, struct xy_comp *dft_n) {
  uint_fast8_t i;
  uint_fast8_t idx_tbl;
  float *p_history_buffer = p_channel_data;
  float *p_history_sample = p_history_buffer + 0 - 1;

  dft_n->x = 0.0f;
  dft_n->y = 0.0f;
  for (i = 0, idx_tbl = 0; i < NUM_SAMPLES; ++i, --p_history_sample, idx_tbl += n) {
    if (p_history_sample < p_history_buffer) p_history_sample += NUM_SAMPLES;
    if (idx_tbl >= NUM_SAMPLES) idx_tbl -= NUM_SAMPLES;

    dft_n->x += *p_history_sample * sine_table[idx_tbl];
    dft_n->y += *p_history_sample * cosine_table[idx_tbl];

  }
  dft_n->x *= -SQRT2/*2.0f*/ / NUM_SAMPLES;
  dft_n->y *= SQRT2/*2.0f*/ / NUM_SAMPLES;
}

void task_payload1(void *arg) {
	ESP_LOGI(__func__, "");
	while(1) {
		float *data = malloc(NUM_SAMPLES * sizeof(float));
		memcpy(data, sine_table, sizeof(sine_table));
		struct xy_comp xy;
		calc_dft(data, NUM_SAMPLES, &xy);
		calc_effective_val_using_xy_components(&xy);
		free(data);
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}
