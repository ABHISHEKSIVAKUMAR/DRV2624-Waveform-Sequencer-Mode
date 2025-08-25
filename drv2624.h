#ifndef _DRV2624_H_
#define _DRV2624_H_

#pragma once

#ifndef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#endif
#include "esp_log.h"
#include "driver/i2c_master.h"

#ifdef __cplusplus
extern "C"
{
#endif

    esp_err_t drv2624_calibrate(i2c_master_dev_handle_t *drv2624_handle);

    esp_err_t drv2624_get_calibration_results(i2c_master_dev_handle_t *drv2624_handle);

    esp_err_t drv2624_initialize(i2c_master_dev_handle_t *drv2624_handle, uint8_t res_comp, uint8_t feedback_gain, uint8_t backemf_gain);

#ifdef __cplusplus
}
#endif

#endif