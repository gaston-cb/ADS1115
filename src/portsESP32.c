/*
 * portsESP32.c
 *
 *  Created on: Jun 7, 2022
 *      Author: root
 */
#include <stdint.h>
#include <stddef.h>
#include <freertos/FreeRTOS.h>
#include "driver/gpio.h"
#include "driver/i2c.h"
#define ERROR_I2C_OK 0x00
#define ERROR_I2C_FAIL 0xFF


/**
 *
 * @param I2C_address
 * @param buffer_data
 * @param length_buffer
 * @return error code: Ox01 -> success
 * 					 : OxFF -> error
 */

uint8_t  I2CWriteToSlave(uint8_t I2C_address, uint8_t *buffer_data,size_t length_buffer){
	uint8_t  error_code = ERROR_I2C_FAIL;
	esp_err_t error_i2c ; //= NULL ;
	i2c_config_t conf = {
			.mode = I2C_MODE_MASTER,
			.sda_io_num = 21,
			.sda_pullup_en = GPIO_PULLUP_ENABLE,
			.scl_io_num = 22,
			.scl_pullup_en = GPIO_PULLUP_ENABLE,
			.master.clk_speed = 100000, //
	};
	error_i2c = i2c_param_config(I2C_NUM_0,&conf) ;
	if (error_i2c == ESP_OK){
		error_code = ERROR_I2C_OK ;

	}else {
		error_code = ERROR_I2C_FAIL ;
		return error_code ;
	}
	error_i2c = i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 10, 10, 0) ;

	if (error_i2c == ESP_OK){
		error_code = ERROR_I2C_OK ;
	}else if (error_i2c == ESP_ERR_INVALID_ARG){
		error_code = ERROR_I2C_FAIL ;
		printf ("INVALID_ARG ") ;
		return error_code ;

	}else if (error_i2c == ESP_FAIL) {
		printf ("ESP_FAIL ") ;
		error_code = ERROR_I2C_OK ;
		return error_code ;


	}

	error_i2c = i2c_master_write_to_device(I2C_NUM_0,I2C_address, buffer_data, length_buffer, (TickType_t )1000/portTICK_RATE_MS) ;
	if (error_i2c == ESP_OK){
		error_code = ERROR_I2C_OK ;

	}else if (error_i2c ==ESP_ERR_INVALID_ARG){
		error_code = ERROR_I2C_FAIL ;
		return error_code ;

	}else if (error_i2c ==ESP_FAIL){
		error_code = ERROR_I2C_FAIL ;
		return error_code ;


	}else if (error_i2c ==ESP_ERR_TIMEOUT){
		error_code = ERROR_I2C_FAIL ;
		return error_code ;

	}else if (error_i2c ==ESP_ERR_INVALID_STATE){
		error_code = ERROR_I2C_FAIL ;
		return error_code ;

	}else error_code = ERROR_I2C_FAIL ;

	i2c_driver_delete(I2C_NUM_0) ;
	return error_code  ;
}

/**
 *
 * @param I2C_address
 * @param buffer_data
 * @param length_buffer
 */

uint8_t I2CReadToSlave(uint8_t I2C_address, uint8_t *buffer_data,size_t length_buffer){
	uint8_t  error_code = ERROR_I2C_FAIL;
	esp_err_t error_i2c ; //= NULL ;
	i2c_config_t conf = {
				.mode = I2C_MODE_MASTER,
				.sda_io_num = 21,
				.sda_pullup_en = GPIO_PULLUP_ENABLE,
				.scl_io_num = 22,
				.scl_pullup_en = GPIO_PULLUP_ENABLE,
				.master.clk_speed = 400000, //
	};
	error_i2c = i2c_param_config(I2C_NUM_0,&conf) ;
	if (error_i2c == ESP_OK){
		error_code  = ERROR_I2C_OK ;
	}else {
		error_code= ERROR_I2C_FAIL ;
		return error_i2c ;
	}
	error_i2c = i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0) ;
	if (error_i2c == ESP_OK){
		error_code  = ERROR_I2C_FAIL ;
	}else if (error_i2c == ESP_ERR_INVALID_ARG){
		error_code = ERROR_I2C_FAIL ;
		return error_code ;
	}else if (error_i2c == ESP_FAIL) {
		error_code = ERROR_I2C_FAIL ;
		return error_code ;
	}
	error_i2c = i2c_master_read_from_device(I2C_NUM_0,0b01001000, buffer_data, length_buffer, (TickType_t )1000/portTICK_RATE_MS) ;
	if (error_i2c == ESP_OK){
		error_code = ERROR_I2C_OK ;
	}else if (error_i2c ==ESP_ERR_INVALID_ARG){
		error_code = ERROR_I2C_FAIL ;
		return error_code ;
	}else if (error_i2c ==ESP_FAIL){
		error_code = ERROR_I2C_FAIL ;
		return error_code ;
	}else if (error_i2c ==ESP_ERR_TIMEOUT){
		error_code = ERROR_I2C_FAIL ;
		return error_code ;
	}else if (error_i2c ==ESP_ERR_INVALID_STATE){
		error_code = ERROR_I2C_FAIL ;
		return error_code ;
	}else error_code = ERROR_I2C_FAIL ;
	i2c_driver_delete(I2C_NUM_0) ;
	printf("driver_read_end:%d",error_code) ;
	return error_code ;
}











