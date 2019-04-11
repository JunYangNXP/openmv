/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * SCCB (I2C like) driver.
 *
 */
#include <stdbool.h>
#include STM32_HAL_H
#include <systick.h>
#include "omv_boardconfig.h"
#include "cambus.h"

struct camera_id_map {
	uint8_t slave_addr;
	uint8_t chip_id;
	uint16_t id_reg;
};

struct camera_id_map g_camera_array[] = {
	{
		MT9M114_I2C_ADDR, MT9M114_CHIP_ID, MT9M114_CHIP_ID_REG
	}
};

struct device *g_bus_dev;

uint8_t cambus_get_chip(uint8_t slv_id)
{
	int i;

	for (i = 0; i < sizeof(g_camera_array) / sizeof(struct camera_id_map); i++) {
		if (g_camera_array[i].slave_addr == slv_id)
			return g_camera_array[i].chip_id;
	}
	return 0;
}

int cambus_init(void)
{
	g_bus_dev = device_get_binding(CONFIG_I2C_1_NAME);

	assert(g_bus_dev);
	return 0;
}

int cambus_scan(void)
{
	uint8_t chip_id, i;
	int ret;

	assert(g_bus_dev);
	for (i = 0; i < sizeof(g_camera_array) / sizeof(struct camera_id_map); i++) {
		ret = i2c_write_read(g_bus_dev, g_camera_array[i].slave_addr,
			&g_camera_array[i].id_reg,
			sizeof(g_camera_array[i].id_reg),
			&chip_id, 1);
		if (!ret && chip_id == g_camera_array[i].chip_id)
			return g_camera_array[i].slave_addr;
	}

	return 0;
}

int cambus_readb(uint8_t slv_addr, uint8_t reg_addr, uint8_t *reg_data)
{
	int ret;

	assert(g_bus_dev);
	ret = i2c_write_read(g_bus_dev, slv_addr, &reg_addr, 1, reg_data, 1);

	return ret;
}

int cambus_writeb(uint8_t slv_addr, uint8_t reg_addr, uint8_t reg_data)
{
	int ret;

	assert(g_bus_dev);
	ret = i2c_reg_write_byte(g_bus_dev, slv_addr, reg_addr, reg_data);

	return ret;
}

int cambus_readw(uint8_t slv_addr, uint8_t reg_addr, uint16_t *reg_data)
{
	int ret;
	
	assert(g_bus_dev);
	ret = i2c_write_read(g_bus_dev, slv_addr, &reg_addr, 1, reg_data, 2);

	return ret;
}

int cambus_writew(uint8_t slv_addr, uint8_t reg_addr, uint16_t reg_data)
{
	int ret;

	assert(g_bus_dev);
	ret = i2c_burst_write(g_bus_dev, slv_addr, reg_addr, (const u8_t *)&reg_data, 2);
   
	return ret;
}

int cambus_readw2(uint8_t slv_addr, uint16_t reg_addr, uint16_t *reg_data)
{
	int ret;

	assert(g_bus_dev);
	ret = i2c_burst_read16(g_bus_dev, slv_addr, reg_addr, (u8_t *)reg_data, 2);

	return ret;
}

int cambus_writew2(uint8_t slv_addr, uint16_t reg_addr, uint16_t reg_data)
{
	int ret;

	assert(g_bus_dev);
	ret = i2c_burst_write16(g_bus_dev, slv_addr, reg_addr, (const u8_t *)&reg_data, 2);
    
	return ret;
}
