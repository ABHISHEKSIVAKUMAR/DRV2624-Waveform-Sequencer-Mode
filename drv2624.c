#include "drv2624.h"

esp_err_t drv2624_calibrate(i2c_master_dev_handle_t* drv2624_handle)
{
    uint8_t write_buffer[2] = { 0x07, 0b01000100 };
    // wake up drv2624 from standby
    esp_err_t ret = i2c_master_transmit(*drv2624_handle, write_buffer, sizeof(write_buffer), 100);
    if (ret != ESP_OK)
        return ret;

    // set mode to auto calibration
    write_buffer[1] = 0b01000011;
    ret = i2c_master_transmit(*drv2624_handle, write_buffer, sizeof(write_buffer), 100);
    if (ret != ESP_OK)
        return ret;

    // Set actuator type = ERM
    write_buffer[0] = 0x08;
    write_buffer[1] = 0b00001000;
    ret = i2c_master_transmit(*drv2624_handle, write_buffer, sizeof(write_buffer), 100);
    if (ret != ESP_OK)
        return ret;

    // write the actuator rated voltage from the formula in datasheet - 3.3v for now
    write_buffer[0] = 0x1f;
    write_buffer[1] = 0b10010111;
    ret = i2c_master_transmit(*drv2624_handle, write_buffer, sizeof(write_buffer), 100);
    if (ret != ESP_OK)
        return ret;

    // write the max voltage that can be given to the actuator (register mistake in application note 0x02 instead of 0x20)
    write_buffer[0] = 0x20;
    write_buffer[1] = 0b10111001;
    ret = i2c_master_transmit(*drv2624_handle, write_buffer, sizeof(write_buffer), 100);
    if (ret != ESP_OK)
        return ret;

    // Write the drive time. Not sure how to find it, leave at default

    // write to go bit in 0x0c to start auto calibration
    write_buffer[0] = 0x0c;
    write_buffer[1] = 0b00000001;
    ret = i2c_master_transmit(*drv2624_handle, write_buffer, sizeof(write_buffer), 100);
    if (ret != ESP_OK)
        return ret;

    return ESP_OK;
}

esp_err_t drv2624_get_calibration_results(i2c_master_dev_handle_t* drv2624_handle)
{
    uint8_t write_buffer = 0x01, read_buffer = 0x00;
    esp_err_t ret = i2c_master_transmit_receive(*drv2624_handle, &write_buffer, 1, &read_buffer, sizeof(read_buffer), 100);
    if (ret != ESP_OK)
        return ret;
    ESP_LOGI("ADI", "Register 0x01 data: %x", read_buffer);

    write_buffer = 0x21;
    ret = i2c_master_transmit_receive(*drv2624_handle, &write_buffer, sizeof(write_buffer), &read_buffer, sizeof(read_buffer), 100);
    if (ret != ESP_OK)
        return ret;
    ESP_LOGI("ADI", "Auto Calibration Resistance Calibration: %d", read_buffer);

    write_buffer = 0x22;
    ret = i2c_master_transmit_receive(*drv2624_handle, &write_buffer, sizeof(write_buffer), &read_buffer, sizeof(read_buffer), 100);
    if (ret != ESP_OK)
        return ret;
    ESP_LOGI("ADI", "Auto Calibration Feedback Gain: %d", read_buffer);

    write_buffer = 0x22;
    ret = i2c_master_transmit_receive(*drv2624_handle, &write_buffer, sizeof(write_buffer), &read_buffer, sizeof(read_buffer), 100);
    if (ret != ESP_OK)
        return ret;
    ESP_LOGI("ADI", "Auto Calibration Back EMF Gain: %d", read_buffer & 0x03);

    return ESP_OK;
}

esp_err_t drv2624_initialize(i2c_master_dev_handle_t* drv2624_handle, uint8_t res_comp, uint8_t feedback_gain, uint8_t backemf_gain)
{
    uint8_t write_buffer[2] = { 0x07, 0b01000101 };
    // wake up drv2624 from standby
    esp_err_t ret = i2c_master_transmit(*drv2624_handle, write_buffer, sizeof(write_buffer), 100);
    if (ret != ESP_OK)
        return ret;

    // Set actuator type = ERM
    write_buffer[0] = 0x08;
    write_buffer[1] = 0b00001000;
    ret = i2c_master_transmit(*drv2624_handle, write_buffer, sizeof(write_buffer), 100);
    if (ret != ESP_OK)
        return ret;

    // write the actuator rated voltage from the formula in datasheet - 3.3v for now
    write_buffer[0] = 0x1f;
    write_buffer[1] = 0b10010111;
    ret = i2c_master_transmit(*drv2624_handle, write_buffer, sizeof(write_buffer), 100);
    if (ret != ESP_OK)
        return ret;

    // write the max voltage that can be given to the actuator (register mistake in application note 0x02 instead of 0x20)
    write_buffer[0] = 0x20;
    write_buffer[1] = 0b10111001;
    ret = i2c_master_transmit(*drv2624_handle, write_buffer, sizeof(write_buffer), 100);
    if (ret != ESP_OK)
        return ret;

    // Write the drive time. Not sure how to find it, leave at default

    write_buffer[0] = 0x21;
    write_buffer[1] = res_comp;
    ret = i2c_master_transmit(*drv2624_handle, write_buffer, sizeof(write_buffer), 100);
    if (ret != ESP_OK)
        return ret;

    write_buffer[0] = 0x22;
    write_buffer[1] = feedback_gain;
    ret = i2c_master_transmit(*drv2624_handle, write_buffer, sizeof(write_buffer), 100);
    if (ret != ESP_OK)
        return ret;

    write_buffer[0] = 0x23;
    write_buffer[1] = 0b0011100 | backemf_gain;
    return i2c_master_transmit(*drv2624_handle, write_buffer, sizeof(write_buffer), 100);
}