/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Camera bus driver.
 *
 */
#ifndef __CAMBUS_H__
#define __CAMBUS_H__
#include <stdint.h>
#include "i2c.h"
#define MT9M114_I2C_ADDR 0x48
#define MT9M114_CHIP_ID 0x24
#define MT9M114_CHIP_ID_REG 0

int cambus_init(void);
int cambus_scan(void);
int cambus_readb(uint8_t slv_addr, uint8_t reg_addr,  uint8_t *reg_data);
int cambus_writeb(uint8_t slv_addr, uint8_t reg_addr, uint8_t reg_data);
int cambus_readw(uint8_t slv_addr, uint8_t reg_addr,  uint16_t *reg_data);
int cambus_writew(uint8_t slv_addr, uint8_t reg_addr, uint16_t reg_data);
int cambus_readb2(uint8_t slv_addr, uint16_t reg_addr,  uint8_t *reg_data);
int cambus_writeb2(uint8_t slv_addr, uint16_t reg_addr, uint8_t reg_data);
int cambus_readw2(uint8_t slv_addr, uint16_t reg_addr,  uint16_t *reg_data);
int cambus_writew2(uint8_t slv_addr, uint16_t reg_addr, uint16_t reg_data);
int cambus_readl2(uint8_t slv_addr, uint16_t reg_addr,  uint32_t *reg_data);
int cambus_writel2(uint8_t slv_addr, uint16_t reg_addr, uint32_t reg_data);
#endif // __CAMBUS_H__
