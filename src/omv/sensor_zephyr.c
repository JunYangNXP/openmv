/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Sensor abstraction layer.
 *
 */
#include <stdlib.h>
#include <string.h>
#include "mp.h"
#include "irq.h"
#include "cambus.h"
#include "ov9650.h"
#include "ov2640.h"
#include "ov7725.h"
#include "mt9v034.h"
#include "lepton.h"
#include "sensor.h"
#include "systick.h"
#include "framebuffer.h"
#include "omv_boardconfig.h"

sensor_t g_sensor;

extern int mt9m114_init(sensor_t *sensor);

struct img_sensor_map {
	uint8_t  chip_id;
	int  (*sensor_init)(sensor_t *sensor);
};

struct img_sensor_map g_img_sensor_map[] = {
	{
		MT9M114_CHIP_ID,
		mt9m114_init,
	}
};

extern void img_sensor_power(int on);
extern uint8_t cambus_get_chip(uint8_t slv_id);
extern void camera_data_bus_init(void);

const int resolution[][2] = {
	{0,    0   },
	// C/SIF Resolutions
	{88,   72  },    /* QQCIF     */
	{176,  144 },    /* QCIF      */
	{352,  288 },    /* CIF       */
	{88,   60  },    /* QQSIF     */
	{176,  120 },    /* QSIF      */
	{352,  240 },    /* SIF       */
	// VGA Resolutions
	{40,   30  },    /* QQQQVGA   */
	{80,   60  },    /* QQQVGA    */
	{160,  120 },    /* QQVGA     */
	{320,  240 },    /* QVGA      */
	{640,  480 },    /* VGA       */
	{60,   40  },    /* HQQQVGA   */
	{120,  80  },    /* HQQVGA    */
	{240,  160 },    /* HQVGA     */
	// FFT Resolutions
	{64,   32  },    /* 64x32     */
	{64,   64  },    /* 64x64     */
	{128,  64  },    /* 128x64    */
	{128,  128 },    /* 128x64    */
	// Other
	{128,  160 },    /* LCD       */
	{128,  160 },    /* QQVGA2    */
	{720,  480 },    /* WVGA      */
	{752,  480 },    /* WVGA2     */
	{800,  600 },    /* SVGA      */
	{1280, 1024},    /* SXGA      */
	{1600, 1200},    /* UXGA      */
};

int sensor_init(void)
{
#if 0
	img_sensor_power(1);
	cambus_init();
	g_sensor.slv_addr = cambus_scan();
	printk("g_sensor.slv_addr:%d\r\n", g_sensor.slv_addr);
#endif
	return 0;
}

void sensor_init0(void)
{
	return;
}

int sensor_reset(void)
{
	int i;

	g_sensor.chip_id = cambus_get_chip(g_sensor.slv_addr);
	for (i = 0; i < sizeof(g_img_sensor_map) / sizeof(struct img_sensor_map); i++) {
		if (g_sensor.chip_id == g_img_sensor_map[i].chip_id) {
			g_img_sensor_map[i].sensor_init(&g_sensor);
			camera_data_bus_init();
			g_sensor.reset(&g_sensor);
			return 0;
		}
	}
	return -1;
}

int sensor_get_id(void)
{
	return 0;
}

int sensor_sleep(int enable)
{
	return 0;
}

int sensor_shutdown(int enable)
{
	return 0;
}

int sensor_read_reg(uint8_t reg_addr)
{
	return 0;
}

int sensor_write_reg(uint8_t reg_addr, uint16_t reg_data)
{
	return 0;
}

int sensor_set_pixformat(pixformat_t pixformat)
{
	return 0;
}

int sensor_set_framesize(framesize_t framesize)
{
	return 0;
}

int sensor_set_framerate(framerate_t framerate)
{
	return 0;
}

int sensor_set_windowing(int x, int y, int w, int h)
{
	return 0;
}

int sensor_set_contrast(int level)
{
	return 0;
}

int sensor_set_brightness(int level)
{
	return 0;
}

int sensor_set_saturation(int level)
{
	return 0;
}

int sensor_set_gainceiling(gainceiling_t gainceiling)
{
	return 0;
}

int sensor_set_quality(int qs)
{
	return 0;
}

int sensor_set_colorbar(int enable)
{
	return 0;
}

int sensor_set_auto_gain(int enable, float gain_db, float gain_db_ceiling)
{
	return 0;
}

int sensor_get_gain_db(float *gain_db)
{
	return 0;
}

int sensor_set_auto_exposure(int enable, int exposure_us)
{
	return 0;
}

int sensor_get_exposure_us(int *get_exposure_us)
{
	return 0;
}

int sensor_set_auto_whitebal(int enable, float r_gain_db, float g_gain_db, float b_gain_db)
{
	return 0;
}

int sensor_get_rgb_gain_db(float *r_gain_db, float *g_gain_db, float *b_gain_db)
{
	return 0;
}

int sensor_set_hmirror(int enable)
{
	return 0;
}

int sensor_set_vflip(int enable)
{
	return 0;
}

int sensor_set_special_effect(sde_t sde)
{
	return 0;
}

int sensor_set_lens_correction(int enable, int radi, int coef)
{
	return 0;
}

int sensor_set_vsync_output(GPIO_TypeDef *gpio, uint32_t pin)
{
	return 0;
}

extern int camera_snapshot(void *pixel_data);
int sensor_snapshot(sensor_t *sensor, image_t *image, streaming_cb_t streaming_cb)
{
	int ret;

	printk("sensor_snapshot image:%p, pixels:%p\r\n", image, image->pixels);
	ret = camera_snapshot(image->pixels);
	return ret;
}
