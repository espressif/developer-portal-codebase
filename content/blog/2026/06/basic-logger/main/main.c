#include <stdio.h>
#include <stdlib.h>  
#include <esp_log.h>
#include <esp_err.h>
#include <driver/gpio.h>  // For GPIO_PULLUP_ENABLE
#include "i2c_bus.h"
#include "aht20.h"
#include <freertos/FreeRTOS.h>
#include "esp_littlefs.h"
#include <sys/stat.h>   // for struct stat and stat()
#include <unistd.h>     // for unlink()


// Define TAG for logging
#define TAG "ambient_sensor"

// Define actual GPIO pins (REPLACE with your values!)
#define AMBIENT_SENSOR_I2C_PORT 0
#define AMBIENT_SENSOR_SDA_IO GPIO_NUM_22
#define AMBIENT_SENSOR_SCL_IO GPIO_NUM_28
#define AMBIENT_SENSOR_I2C_FREQ_HZ 100000


#define PARTITION_LABEL "storage"


esp_err_t ambient_sensor_init(aht20_dev_handle_t *aht20_handle, i2c_bus_handle_t *i2c_bus_handle) {
    if (aht20_handle == NULL || i2c_bus_handle == NULL) {
        ESP_LOGE(TAG, "Invalid arguments: aht20_handle or i2c_bus_handle is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    // Configure I2C bus
    const i2c_config_t i2c_cfg = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = AMBIENT_SENSOR_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = AMBIENT_SENSOR_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = AMBIENT_SENSOR_I2C_FREQ_HZ,
    };

    *i2c_bus_handle = i2c_bus_create(AMBIENT_SENSOR_I2C_PORT, &i2c_cfg);
    if (*i2c_bus_handle == NULL) {
        ESP_LOGE(TAG, "Failed to create I2C bus");
        return ESP_ERR_INVALID_STATE;
    }

    // Configure AHT20 sensor
    aht20_i2c_config_t aht20_cfg = {
        .bus_inst = *i2c_bus_handle,
        .i2c_addr = AHT20_ADDRRES_0,
    };

    esp_err_t ret = aht20_new_sensor(&aht20_cfg, aht20_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create AHT20 sensor: %s", esp_err_to_name(ret));
        i2c_bus_delete(*i2c_bus_handle);
        *i2c_bus_handle = NULL;  // Avoid dangling pointer
        return ret;
    }

    ESP_LOGI(TAG, "Ambient sensor created (I2C port=%d, SDA=%d, SCL=%d, freq=%d Hz)",
             AMBIENT_SENSOR_I2C_PORT,
             AMBIENT_SENSOR_SDA_IO,
             AMBIENT_SENSOR_SCL_IO,
             AMBIENT_SENSOR_I2C_FREQ_HZ);

    return ESP_OK;
}


void little_fs_init(){

    ESP_LOGI(TAG, "Initializing LittleFS");

    esp_vfs_littlefs_conf_t conf = {
        .base_path = "/littlefs",
        .partition_label = PARTITION_LABEL,
        .format_if_mount_failed = true,
        .dont_mount = false,
    };
    esp_err_t ret = esp_vfs_littlefs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find LittleFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize LittleFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    size_t total = 0, used = 0;
    ret = esp_littlefs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get LittleFS partition information (%s)", esp_err_to_name(ret));
        esp_littlefs_format(conf.partition_label);
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
}


// void test_file_operations(){

//     ESP_LOGI(TAG, "Opening file");
//     FILE *f = fopen("/littlefs/hello.txt", "w");
//     if (f == NULL) {
//         ESP_LOGE(TAG, "Failed to open file for writing");
//         return;
//     }
//     fprintf(f, "Hello World!\n");
//     fclose(f);
//     ESP_LOGI(TAG, "File written");

//     // Check if destination file exists before renaming
//     struct stat st;
//     if (stat("/littlefs/foo.txt", &st) == 0) {
//         // Delete it if it exists
//         unlink("/littlefs/foo.txt");
//     }

//     // Rename original file
//     ESP_LOGI(TAG, "Renaming file");
//     if (rename("/littlefs/hello.txt", "/littlefs/foo.txt") != 0) {
//         ESP_LOGE(TAG, "Rename failed");
//         return;
//     }

//     // Open renamed file for reading
//     ESP_LOGI(TAG, "Reading file");
//     f = fopen("/littlefs/foo.txt", "r");
//     if (f == NULL) {
//         ESP_LOGE(TAG, "Failed to open file for reading");
//         return;
//     }

//     char line[128] = {0};
//     fgets(line, sizeof(line), f);
//     fclose(f);
//     // strip newline
//     char* pos = strpbrk(line, "\r\n");
//     if (pos) {
//         *pos = '\0';
//     }
//     ESP_LOGI(TAG, "Read from file: '%s'", line);

//     ESP_LOGI(TAG, "Reading from flashed filesystem example.txt");
//     f = fopen("/littlefs/example.txt", "r");
//     if (f == NULL) {
//         ESP_LOGE(TAG, "Failed to open file for reading");
//         return;
//     }
//     fgets(line, sizeof(line), f);
//     fclose(f);
//     // strip newline
//     pos = strpbrk(line, "\r\n");
//     if (pos) {
//         *pos = '\0';
//     }
//     ESP_LOGI(TAG, "Read from file: '%s'", line);

// }

void app_main(void) {
    ESP_LOGI(TAG,"Get started\n");

    // Allocate handles (no need to malloc aht20_dev_handle_t directly)
    aht20_dev_handle_t aht20_handle = NULL;  // aht20_new_sensor likely allocates this
    i2c_bus_handle_t i2c_bus_handle = NULL;

    little_fs_init();
    test_file_operations();

    ESP_ERROR_CHECK(ambient_sensor_init(&aht20_handle, &i2c_bus_handle));
    

    uint32_t temperature_raw, humidity_raw;
    float temperature, humidity;

    while(1){
        aht20_read_temperature_humidity(aht20_handle, &temperature_raw, &temperature, &humidity_raw, &humidity);
        ESP_LOGI(TAG, "%-20s: %2.2f %%", "humidity is", humidity);
        ESP_LOGI(TAG, "%-20s: %2.2f degC", "temperature is", temperature);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }


    // All done, unmount partition and disable LittleFS
    esp_vfs_littlefs_unregister(PARTITION_LABEL);
    ESP_LOGI(TAG, "LittleFS unmounted");

}