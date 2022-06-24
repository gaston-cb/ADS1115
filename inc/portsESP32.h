/*
 * portsESP32.h
 *
 *  Created on: 23 jun. 2022
 *      Author: gaston
 */

#ifndef COMPONENTS_ADS1115_INC_PORTSESP32_H_
#define COMPONENTS_ADS1115_INC_PORTSESP32_H_
#include <stdint.h>

#define ERROR_I2C_OK 0x00
#define ERROR_I2C_FAIL 0xFF

uint8_t  I2CWriteToSlave(uint8_t I2C_address, uint8_t *buffer_data,size_t length_buffer) ;
uint8_t  I2CReadToSlave(uint8_t I2C_address, uint8_t *buffer_data,size_t length_buffer) ;


#endif /* COMPONENTS_ADS1115_INC_PORTSESP32_H_ */
