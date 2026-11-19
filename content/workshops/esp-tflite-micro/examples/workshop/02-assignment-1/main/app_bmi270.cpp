/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "app_bmi270.h"

static const char *TAG = "IMU_BMI270";

app_bmi270_handle_t* app_bmi270_init(void)
{
    app_bmi270_handle_t* handle = (app_bmi270_handle_t*)calloc(1, sizeof(app_bmi270_handle_t));
    ESP_RETURN_ON_FALSE(handle, nullptr, TAG, "Failed to allocate memory for imu_bmi270_handle_t");

    // Allocate memory for gyro data
    handle->gyr_data = (float (*)[3])calloc(CONFIG_NUM_SAMPLES, 3 * sizeof(float));
    ESP_RETURN_ON_FALSE(handle->gyr_data, nullptr, TAG, "Failed to allocate memory for gyro data");

    // Configure SDO pin
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << CONFIG_IMU_SDO_PIN);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SDO pin configuration failed: %s", esp_err_to_name(ret));
    }
    gpio_set_level(static_cast<gpio_num_t>(CONFIG_IMU_SDO_PIN), 0);

    // Initialize I2C bus
    i2c_config_t i2c_bus_conf = {};
    i2c_bus_conf.mode = I2C_MODE_MASTER;
    i2c_bus_conf.sda_io_num = CONFIG_IMU_I2C_SDA;
    i2c_bus_conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_bus_conf.scl_io_num = CONFIG_IMU_I2C_SCL;
    i2c_bus_conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_bus_conf.master.clk_speed = 200 * 1000;
    handle->i2c_bus = i2c_bus_create(I2C_NUM_0, &i2c_bus_conf);
    ESP_RETURN_ON_FALSE(handle->i2c_bus, nullptr, TAG, "Failed to create I2C bus");

    // Initialize BMI270
    ret = bmi270_sensor_create(handle->i2c_bus, &handle->bmi_handle, bmi270_config_file, 0);
    ESP_RETURN_ON_FALSE(ret == ESP_OK && handle->bmi_handle, nullptr, TAG, "Failed to create BMI270 sensor");

    // Configure BMI270
    struct bmi2_sens_config config[2];
    config[BMI2_ACCEL].type = BMI2_ACCEL;
    config[BMI2_GYRO].type = BMI2_GYRO;

    int8_t rslt =  bmi2_get_sensor_config(config, 2, handle->bmi_handle);
    bmi2_error_codes_print_result(rslt);

    rslt = bmi2_map_data_int(BMI2_DRDY_INT, BMI2_INT1, handle->bmi_handle);
    bmi2_error_codes_print_result(rslt);

    if (rslt == BMI2_OK) {
        // Acc
        config[0].cfg.acc.odr         = BMI2_ACC_ODR_200HZ;
        config[0].cfg.acc.range       = BMI2_ACC_RANGE_2G;
        config[0].cfg.acc.bwp         = BMI2_ACC_NORMAL_AVG4;
        config[0].cfg.acc.filter_perf = BMI2_PERF_OPT_MODE;

        // Gyro
        config[1].cfg.gyr.odr         = BMI2_GYR_ODR_200HZ;
        config[1].cfg.gyr.range       = BMI2_GYR_RANGE_2000;
        config[1].cfg.gyr.bwp         = BMI2_GYR_NORMAL_MODE;
        config[1].cfg.gyr.noise_perf  = BMI2_POWER_OPT_MODE;
        config[1].cfg.gyr.filter_perf = BMI2_PERF_OPT_MODE;

        rslt = bmi2_set_sensor_config(config, 2, handle->bmi_handle);
        bmi2_error_codes_print_result(rslt);

        if (rslt == BMI2_OK) {
            /* Accel and Gyro must be enabled after setting configurations */
            uint8_t sens_list[2] = { BMI2_ACCEL, BMI2_GYRO };
            rslt = bmi2_sensor_enable(sens_list, 2, handle->bmi_handle);
            bmi2_error_codes_print_result(rslt);
        }
    }

    if (rslt != BMI2_OK) {
        ESP_LOGE(TAG, "Failed to configure BMI270");
        bmi270_sensor_del(&handle->bmi_handle);
        i2c_bus_delete(&handle->i2c_bus);
        free(handle);
        return nullptr;
    }

    return handle;
}

float imu_lsb_to_dps(int16_t val, float full_scale_dps, uint8_t bit_width)
{
    const float half_scale = (float)((1u << bit_width) / 2u);
    return (full_scale_dps / half_scale) * (float)val;
}

esp_err_t app_bmi270_collect(app_bmi270_handle_t* handle, float (*imu_data)[3], size_t num_samples)
{
    if (handle == nullptr || handle->bmi_handle == nullptr) {
        return ESP_ERR_INVALID_ARG;
    }

    struct bmi2_sens_data sensor_data;

    for (int i = 0; i < num_samples; i++) {
        int8_t rslt = bmi2_get_sensor_data(&sensor_data, handle->bmi_handle);
        if (rslt == BMI2_OK && (sensor_data.status & BMI2_DRDY_GYR)) {
            float gx = imu_lsb_to_dps(sensor_data.gyr.x, 2000.0f, handle->bmi_handle->resolution);
            float gy = imu_lsb_to_dps(sensor_data.gyr.y, 2000.0f, handle->bmi_handle->resolution);
            float gz = imu_lsb_to_dps(sensor_data.gyr.z, 2000.0f, handle->bmi_handle->resolution);
            imu_data[i][0] = gx;
            imu_data[i][1] = gy;
            imu_data[i][2] = gz;
        } else {
            return ESP_ERR_INVALID_STATE;
        }

        vTaskDelay(pdMS_TO_TICKS(CONFIG_SAMPLE_INTERVAL_MS));
    }

    return ESP_OK;
}

esp_err_t app_bmi270_collect_once(app_bmi270_handle_t* handle, float imu_data[3])
{
    if (handle == nullptr || handle->bmi_handle == nullptr) {
        return ESP_ERR_INVALID_ARG;
    }

    struct bmi2_sens_data sensor_data;
    int8_t rslt = bmi2_get_sensor_data(&sensor_data, handle->bmi_handle);
    if (rslt != BMI2_OK || !(sensor_data.status & BMI2_DRDY_GYR)) {
        return ESP_ERR_INVALID_STATE;
    }
    float gx = imu_lsb_to_dps(sensor_data.gyr.x, 2000.0f, handle->bmi_handle->resolution);
    float gy = imu_lsb_to_dps(sensor_data.gyr.y, 2000.0f, handle->bmi_handle->resolution);
    float gz = imu_lsb_to_dps(sensor_data.gyr.z, 2000.0f, handle->bmi_handle->resolution);
    imu_data[0] = gx;
    imu_data[1] = gy;
    imu_data[2] = gz;
    return ESP_OK;
}
