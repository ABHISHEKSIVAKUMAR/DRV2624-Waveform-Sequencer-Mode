#include "drv2624.h"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"

void app_main(void)
{
    // Initialize I2C bus
    i2c_master_bus_config_t wearable_i2c_bus_conf = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .flags.enable_internal_pullup = true,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = GPIO_NUM_1,
        .sda_io_num = GPIO_NUM_0};
    static i2c_master_bus_handle_t wearable_i2c_bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&wearable_i2c_bus_conf, &wearable_i2c_bus_handle));

    // Initialize slave and add it to the I2C bus
    i2c_device_config_t drv2624_conf = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x5A,
        .flags.disable_ack_check = 0,
        .scl_speed_hz = 400000,
        .scl_wait_us = 100,
    };
    static i2c_master_dev_handle_t drv2624_handle;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(wearable_i2c_bus_handle, &drv2624_conf, &drv2624_handle));

    uint8_t write_buffer[2], read_buffer;

#ifdef CONFIG_DRV2624_RUN_CALIBRATION
    // Calibrate DRV2624 and get the values
    if (drv2624_calibrate(&drv2624_handle) != ESP_OK)
    {
        write_buffer[0] = 0x01;
        if (i2c_master_transmit_receive(drv2624_handle, &write_buffer[0], 1, &read_buffer, sizeof(read_buffer), 100) != ESP_OK)
            return;
        ESP_LOGI("DRV2624", "Failed to calibrate Reg 0x01 value: 0x%x", read_buffer);
    }

    // Wait for calibration to be done
    vTaskDelay(pdMS_TO_TICKS(3000));

    // Display results for further use
    if (drv2624_get_calibration_results(&drv2624_handle) != ESP_OK)
        return;
#else
    // Initialize DRV2624 with known gain and component values
    if (drv2624_initialize(&drv2624_handle, CONFIG_VIBRATION_MOTOR_RES_COMP, CONFIG_FEEDBACK_GAIN, CONFIG_BACKEMF_GAIN) != ESP_OK)
    {
        write_buffer[0] = 0x01;
        if (i2c_master_transmit_receive(drv2624_handle, &write_buffer[0], 1, &read_buffer, sizeof(read_buffer), 100) != ESP_OK)
            return;
        ESP_LOGI("DRV2624", "Failed to initialize Reg 0x01 value: 0x%x", read_buffer);
    }

#ifdef CONFIG_RTP_WAVEFORM_MODE
    // Set the device to work in the RTP mode and set the GO bit functionality
    write_buffer[0] = 0x07;
    write_buffer[1] = 0b01001000;
    if (i2c_master_transmit(drv2624_handle, write_buffer, sizeof(write_buffer), 100) != ESP_OK)
        return;

    ESP_LOGI("DRV2624", "Device is in RTP mode, Executing basic functionality.");

    // Set GO bit to 1
    write_buffer[0] = 0x0c;
    write_buffer[1] = 0x01;
    if (i2c_master_transmit(drv2624_handle, write_buffer, sizeof(write_buffer), 100) != ESP_OK)
        return;

    // Wait for 2 seconds
    vTaskDelay(pdMS_TO_TICKS(2000));

    // Set new amplitude in RTP register
    write_buffer[0] = 0x0e;
    write_buffer[1] = 0x20;
    if (i2c_master_transmit(drv2624_handle, write_buffer, sizeof(write_buffer), 100) != ESP_OK)
        return;

    // Wait for 2 seconds
    vTaskDelay(pdMS_TO_TICKS(2000));

    // Set amplitude no. 2 in RTP register
    write_buffer[0] = 0x0e;
    write_buffer[1] = 0x08;
    if (i2c_master_transmit(drv2624_handle, write_buffer, sizeof(write_buffer), 100) != ESP_OK)
        return;

    // Wait for 2 seconds
    vTaskDelay(pdMS_TO_TICKS(2000));

    // Set GO bit to 0 - Stop vibration
    write_buffer[0] = 0x0c;
    write_buffer[1] = 0x01;
    if (i2c_master_transmit(drv2624_handle, write_buffer, sizeof(write_buffer), 100) != ESP_OK)
        return;
#else
    // Get the device out of standby mode and put in waveform sequencer mode and set the trigger to be the GO bit
    write_buffer[0] = 0x07;
    write_buffer[1] = 0b01000001;
    if (i2c_master_transmit(drv2624_handle, write_buffer, sizeof(write_buffer), 100) != ESP_OK)
        return;

    // Set the first waveform to be waveform 3 (change to 1, later)
    write_buffer[0] = 0x0f;
    write_buffer[1] = 0x03;
    if (i2c_master_transmit(drv2624_handle, write_buffer, sizeof(write_buffer), 100) != ESP_OK)
        return;

    // Optionally, make sequence terminate by writing 0 to the next index in sequence

    // Set upper and lower bits to 0, and write 0x00 to the revision byte
    write_buffer[0] = 0xfd;
    write_buffer[1] = 0x00;
    if (i2c_master_transmit(drv2624_handle, write_buffer, sizeof(write_buffer), 100) != ESP_OK)
        return;

    write_buffer[0] = 0xfe;
    if (i2c_master_transmit(drv2624_handle, write_buffer, sizeof(write_buffer), 100) != ESP_OK)
        return;

    write_buffer[0] = 0xff;
    if (i2c_master_transmit(drv2624_handle, write_buffer, sizeof(write_buffer), 100) != ESP_OK)
        return;

    // Write the upper, lower byte and effect details corresponding to waveform 3 (later, 1)
    write_buffer[0] = 0xfe;
    write_buffer[1] = 0x07;
    if (i2c_master_transmit(drv2624_handle, write_buffer, sizeof(write_buffer), 100) != ESP_OK)
        return;

    write_buffer[0] = 0xff;
    write_buffer[1] = 0x01;
    if (i2c_master_transmit(drv2624_handle, write_buffer, sizeof(write_buffer), 100) != ESP_OK)
        return;

    write_buffer[1] = 0x7e;
    if (i2c_master_transmit(drv2624_handle, write_buffer, sizeof(write_buffer), 100) != ESP_OK)
        return;

    write_buffer[1] = 0b00001000;
    if (i2c_master_transmit(drv2624_handle, write_buffer, sizeof(write_buffer), 100) != ESP_OK)
        return;

    // Write the waveform voltage and time pairs beginning with the starting byte of the waveform
    write_buffer[0] = 0xfd;
    write_buffer[1] = 0x01;
    if (i2c_master_transmit(drv2624_handle, write_buffer, sizeof(write_buffer), 100) != ESP_OK)
        return;

    write_buffer[0] = 0xfe;
    write_buffer[1] = 0x7e;
    if (i2c_master_transmit(drv2624_handle, write_buffer, sizeof(write_buffer), 100) != ESP_OK)
        return;

    // Voltage 1
    write_buffer[0] = 0xff;
    write_buffer[1] = 63;
    if (i2c_master_transmit(drv2624_handle, write_buffer, sizeof(write_buffer), 100) != ESP_OK)
        return;

    // Time 1
    write_buffer[1] = 200;
    if (i2c_master_transmit(drv2624_handle, write_buffer, sizeof(write_buffer), 100) != ESP_OK)
        return;

    // Voltage 2
    write_buffer[1] = 32;
    if (i2c_master_transmit(drv2624_handle, write_buffer, sizeof(write_buffer), 100) != ESP_OK)
        return;

    // Time 2
    write_buffer[1] = 100;
    if (i2c_master_transmit(drv2624_handle, write_buffer, sizeof(write_buffer), 100) != ESP_OK)
        return;

    // Voltage 3
    write_buffer[1] = 63;
    if (i2c_master_transmit(drv2624_handle, write_buffer, sizeof(write_buffer), 100) != ESP_OK)
        return;

    // Time 3
    write_buffer[1] = 200;
    if (i2c_master_transmit(drv2624_handle, write_buffer, sizeof(write_buffer), 100) != ESP_OK)
        return;

    // Voltage 4
    write_buffer[1] = 0;
    if (i2c_master_transmit(drv2624_handle, write_buffer, sizeof(write_buffer), 100) != ESP_OK)
        return;

    // Time 4
    write_buffer[1] = 20;
    if (i2c_master_transmit(drv2624_handle, write_buffer, sizeof(write_buffer), 100) != ESP_OK)
        return;

    ESP_LOGI("DRV2624", "Waveform sequencer configured successfully. Executing basic functionality in 2 seconds.");
    vTaskDelay(pdMS_TO_TICKS(2000));

    // Set go bit to 1
    write_buffer[0] = 0x0c;
    write_buffer[1] = 0x01;
    if (i2c_master_transmit(drv2624_handle, write_buffer, sizeof(write_buffer), 100) != ESP_OK)
        return;

vTaskDelay(pdMS_TO_TICKS(3000));

//Second Vibration Pattern only two voltage and time pair
 // Get the device out of standby mode and put in waveform sequencer mode and set the trigger to be the GO bit
    write_buffer[0] = 0x0a;
    write_buffer[1] = 0b00100100;
    if (i2c_master_transmit(drv2624_handle, write_buffer, sizeof(write_buffer), 100) != ESP_OK)
        return;
    
    // Set the waveform (change to 1, later)
    write_buffer[0] = 0x0c;
    write_buffer[1] = 0x02;
    if (i2c_master_transmit(drv2624_handle, write_buffer, sizeof(write_buffer), 100) != ESP_OK)
        return;
    
    // Set upper and lower bits to 0, and write 0x00 to the revision byte
    write_buffer[0] = 0xfd;
    write_buffer[1] = 0x00;
    if (i2c_master_transmit(drv2624_handle, write_buffer, sizeof(write_buffer), 100) != ESP_OK)
        return;

    write_buffer[0] = 0xfe;
    if (i2c_master_transmit(drv2624_handle, write_buffer, sizeof(write_buffer), 100) != ESP_OK)
        return;

    write_buffer[0] = 0xff;
    if (i2c_master_transmit(drv2624_handle, write_buffer, sizeof(write_buffer), 100) != ESP_OK)
        return;    
    
    write_buffer[0] = 0xfe;
    write_buffer[1] = 0x0a;
    if (i2c_master_transmit(drv2624_handle, write_buffer, sizeof(write_buffer), 100) != ESP_OK)
        return;

    write_buffer[0] = 0xff;
    if (i2c_master_transmit(drv2624_handle, write_buffer, sizeof(write_buffer), 100) != ESP_OK)
        return;    

#endif

#endif
}