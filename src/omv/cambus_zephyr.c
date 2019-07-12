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

int cambus_readb2(uint8_t slv_addr, uint16_t reg_addr, uint8_t *reg_data)
{
	int ret;
	u8_t addr_buffer[2];

	assert(g_bus_dev);

	addr_buffer[1] = reg_addr & 0xFF;
	addr_buffer[0] = reg_addr >> 8;
	ret = i2c_write_read(g_bus_dev, slv_addr, addr_buffer, 2, reg_data, 1);

	return ret;
}

int cambus_writeb2(uint8_t slv_addr, uint16_t reg_addr, uint8_t reg_data)
{
	int ret;
	uint8_t data[3];

	assert(g_bus_dev);
	data[1] = reg_addr & 0xFF;
	data[0] = reg_addr >> 8;
	data[2] = reg_data;
	ret = i2c_write(g_bus_dev, (const u8_t *)data, 3, slv_addr);

	return ret;
}


int cambus_readw2(uint8_t slv_addr, uint16_t reg_addr, uint16_t *reg_data)
{
	int ret;
	u8_t addr_buffer[2];

	assert(g_bus_dev);

	addr_buffer[1] = reg_addr & 0xFF;
	addr_buffer[0] = reg_addr >> 8;

	ret = i2c_write_read(g_bus_dev, slv_addr, addr_buffer, 2, (u8_t *)reg_data, 2);
	*reg_data = ((*reg_data >> 8U) & 0xFF) | ((*reg_data & 0xFFU) << 8U);

	return ret;
}

int cambus_writew2(uint8_t slv_addr, uint16_t reg_addr, uint16_t reg_data)
{
	int ret, data_width = 2;
	uint8_t data[4];

	assert(g_bus_dev);
	data[1] = reg_addr & 0xFF;
	data[0] = reg_addr >> 8;
	while (data_width--) {
		data[data_width + 2] = (uint8_t)reg_data;
		reg_data >>= 8;
	}
	ret = i2c_write(g_bus_dev, (const u8_t *)data, 4, slv_addr);
    
	return ret;
}

int cambus_readl2(uint8_t slv_addr, uint16_t reg_addr, uint32_t *reg_data)
{
	int ret, data_width = 4, i = 0;
	uint8_t data[4];
	u8_t addr_buffer[2];

	assert(g_bus_dev);
	addr_buffer[1] = reg_addr & 0xFF;
	addr_buffer[0] = reg_addr >> 8;
	ret = i2c_write_read(g_bus_dev, slv_addr, addr_buffer, 2, (u8_t *)data, 4);
	if (ret)
		return ret;
	while (data_width--)
		((uint8_t *)reg_data)[i++] = data[data_width];

	return ret;
}

int cambus_writel2(uint8_t slv_addr, uint16_t reg_addr, uint32_t reg_data)
{
	int ret, data_width = 4;
	uint8_t data[6];

	assert(g_bus_dev);
	data[1] = reg_addr & 0xFF;
	data[0] = reg_addr >> 8;
	while (data_width--) {
		data[data_width + 2] = (uint8_t)reg_data;
		reg_data >>= 8;
	}
	ret = i2c_write(g_bus_dev, (const u8_t *)data, 6, slv_addr);

	return ret;
}
