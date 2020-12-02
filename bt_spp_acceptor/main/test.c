#include "test.h"

#include <string.h>
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



void run_test(void) {
	esp_err_t err;
	memset(&fw_update_info, 0, sizeof(fw_update_info));
	fw_update_printBootInfo();

	// mark valid
	ESP_LOGI(__func__, "Try esp_ota_mark_app_valid_cancel_rollback()...");
	err = esp_ota_mark_app_valid_cancel_rollback();
	ESP_LOGI(__func__, "esp_ota_mark_app_valid_cancel_rollback ret %d", err);

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

	// change slot
	ESP_LOGI(__func__, "Try esp_ota_set_boot_partition()...");
	err = esp_ota_set_boot_partition(fw_update_info.update_partition);
	if (err == ESP_OK) {
		ESP_LOGI(__func__, "Please restart! Boot at 0x%x", fw_update_info.update_partition->address);
	} else {
		ESP_LOGE(__func__, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
	}
}
