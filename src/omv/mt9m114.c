/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * MT9M114 driver.
 *
 */
#include STM32_HAL_H
#include "cambus.h"
#include "mt9m114.h"
#include "systick.h"
#include "framebuffer.h"
#include "sensor.h"
#include "omv_boardconfig.h"
#include <stdint.h>

typedef struct _mt9m114_reg
{
	uint16_t reg;
	uint8_t size;
	uint32_t value;
} mt9m114_reg_t;

static const mt9m114_reg_t mt9m114_480_272[] = {
	{MT9M114_VAR_CAM_SENSOR_CFG_Y_ADDR_START, 2, 0x00D4},     /* cam_sensor_cfg_y_addr_start = 212 */
	{MT9M114_VAR_CAM_SENSOR_CFG_X_ADDR_START, 2, 0x00A4},     /* cam_sensor_cfg_x_addr_start = 164 */
	{MT9M114_VAR_CAM_SENSOR_CFG_Y_ADDR_END, 2, 0x02FB},       /* cam_sensor_cfg_y_addr_end = 763 */
	{MT9M114_VAR_CAM_SENSOR_CFG_X_ADDR_END, 2, 0x046B},       /* cam_sensor_cfg_x_addr_end = 1131 */
	{MT9M114_VAR_CAM_SENSOR_CFG_CPIPE_LAST_ROW, 2, 0x0223},   /* cam_sensor_cfg_cpipe_last_row = 547 */
	{MT9M114_VAR_CAM_CROP_WINDOW_WIDTH, 2, 0x03C0},           /* cam_crop_window_width = 960 */
	{MT9M114_VAR_CAM_CROP_WINDOW_HEIGHT, 2, 0x0220},          /* cam_crop_window_height = 544 */
	{MT9M114_VAR_CAM_OUTPUT_WIDTH, 2, 0x01E0},                /* cam_output_width = 480 */
	{MT9M114_VAR_CAM_OUTPUT_HEIGHT, 2, 0x0110},               /* cam_output_height = 272 */
	{MT9M114_VAR_CAM_STAT_AWB_CLIP_WINDOW_XEND, 2, 0x01DF},   /* cam_stat_awb_clip_window_xend = 479 */
	{MT9M114_VAR_CAM_STAT_AWB_CLIP_WINDOW_YEND, 2, 0x010F},   /* cam_stat_awb_clip_window_yend = 271 */
	{MT9M114_VAR_CAM_STAT_AE_INITIAL_WINDOW_XEND, 2, 0x005F}, /* cam_stat_ae_initial_window_xend = 95 */
	{MT9M114_VAR_CAM_STAT_AE_INITIAL_WINDOW_YEND, 2, 0x0035}, /* cam_stat_ae_initial_window_yend = 53 */
};

static const mt9m114_reg_t mt9m114_init_cfg[] = {
	{MT9M114_REG_LOGICAL_ADDRESS_ACCESS, 2u, 0x1000},
	/* PLL Fout = (Fin * 2 * m) / ((n + 1) * (p + 1)) */
	{MT9M114_VAR_CAM_SYSCTL_PLL_ENABLE, 1u, 0x01},        /*  cam_sysctl_pll_enable = 1 */
	{MT9M114_VAR_CAM_SYSCTL_PLL_DIVIDER_M_N, 2u, 0x0120}, /*  cam_sysctl_pll_divider_m_n = 288 */
	{MT9M114_VAR_CAM_SYSCTL_PLL_DIVIDER_P, 2u, 0x0700},   /*  cam_sysctl_pll_divider_p = 1792 */
	{MT9M114_VAR_CAM_SENSOR_CFG_PIXCLK, 4u, 0x2DC6C00},   /*  cam_sensor_cfg_pixclk = 48000000 */
	{0x316A, 2, 0x8270}, /*  auto txlo_row for hot pixel and linear full well optimization */
	{0x316C, 2, 0x8270}, /*  auto txlo for hot pixel and linear full well optimization */
	{0x3ED0, 2, 0x2305}, /*  eclipse setting, ecl range=1, ecl value=2, ivln=3 */
	{0x3ED2, 2, 0x77CF}, /*  TX_hi=12 */
	{0x316E, 2, 0x8202}, /*  auto ecl , threshold 2x, ecl=0 at high gain, ecl=2 for low gain */
	{0x3180, 2, 0x87FF}, /*  enable delta dark */
	{0x30D4, 2, 0x6080}, /*  disable column correction due to AE oscillation problem */
	{0xA802, 2, 0x0008}, /*  RESERVED_AE_TRACK_02 */
	{0x3E14, 2, 0xFF39}, /*  Enabling pixout clamping to VAA during ADC streaming to solve column band issue */
	{MT9M114_VAR_CAM_SENSOR_CFG_ROW_SPEED, 2u, 0x0001},           /*  cam_sensor_cfg_row_speed = 1 */
	{MT9M114_VAR_CAM_SENSOR_CFG_FINE_INTEG_TIME_MIN, 2u, 0x00DB}, /*  cam_sensor_cfg_fine_integ_time_min = 219 */
	{MT9M114_VAR_CAM_SENSOR_CFG_FINE_INTEG_TIME_MAX, 2u, 0x07C2}, /*  cam_sensor_cfg_fine_integ_time_max = 1986 */
	{MT9M114_VAR_CAM_SENSOR_CFG_FRAME_LENGTH_LINES, 2u, 0x02FE},  /*  cam_sensor_cfg_frame_length_lines = 766 */
	{MT9M114_VAR_CAM_SENSOR_CFG_LINE_LENGTH_PCK, 2u, 0x0845},     /*  cam_sensor_cfg_line_length_pck = 2117 */
	{MT9M114_VAR_CAM_SENSOR_CFG_FINE_CORRECTION, 2u, 0x0060},     /*  cam_sensor_cfg_fine_correction = 96 */
	{MT9M114_VAR_CAM_SENSOR_CFG_REG_0_DATA, 2u, 0x0020},          /*  cam_sensor_cfg_reg_0_data = 32 */
	{MT9M114_VAR_CAM_SENSOR_CONTROL_READ_MODE, 2u, 0x0000},       /*  cam_sensor_control_read_mode = 0 */
	{MT9M114_VAR_CAM_CROP_WINDOW_XOFFSET, 2u, 0x0000},            /*  cam_crop_window_xoffset = 0 */
	{MT9M114_VAR_CAM_CROP_WINDOW_YOFFSET, 2u, 0x0000},            /*  cam_crop_window_yoffset = 0 */
	{MT9M114_VAR_CAM_CROP_CROPMODE, 1u, 0x03},                    /*  cam_crop_cropmode = 3 */
	{MT9M114_VAR_CAM_AET_AEMODE, 1u, 0x00},                       /*  cam_aet_aemode = 0 */
	{MT9M114_VAR_CAM_AET_MAX_FRAME_RATE, 2u, 0x1D9A},             /*  cam_aet_max_frame_rate = 7578 */
	{MT9M114_VAR_CAM_AET_MIN_FRAME_RATE, 2u, 0x1D9A},             /*  cam_aet_min_frame_rate = 7578 */
	{MT9M114_VAR_CAM_STAT_AWB_CLIP_WINDOW_XSTART, 2u, 0x0000},    /*  cam_stat_awb_clip_window_xstart = 0 */
	{MT9M114_VAR_CAM_STAT_AWB_CLIP_WINDOW_YSTART, 2u, 0x0000},    /*  cam_stat_awb_clip_window_ystart = 0 */
	{MT9M114_VAR_CAM_STAT_AE_INITIAL_WINDOW_XSTART, 2u, 0x0000},  /*  cam_stat_ae_initial_window_xstart = 0 */
	{MT9M114_VAR_CAM_STAT_AE_INITIAL_WINDOW_YSTART, 2u, 0x0000},  /*  cam_stat_ae_initial_window_ystart = 0 */
	{MT9M114_REG_PAD_SLEW, 2u, 0x0777},                           /*  Pad slew rate */
	{MT9M114_VAR_CAM_OUTPUT_FORMAT_YUV, 2u, 0x0038},              /*  Must set cam_output_format_yuv_clip for CSI */
};

status_t mt9m114_modify_reg(uint8_t i2c_addr,
			uint32_t reg, uint8_t data_width,
			uint32_t clrMask, uint32_t value)
{
	status_t status;
	uint32_t regval;

	if (data_width != 1 && data_width != 2 && data_width != 4)
		return kStatus_InvalidArgument;

	if (data_width == 1)
		status = cambus_readb2(i2c_addr, reg, (uint8_t *)&regval);
	else if (data_width == 2)
		status = cambus_readw2(i2c_addr, reg, (uint16_t *)&regval);
	else
		status = cambus_readl2(i2c_addr, reg, (uint32_t *)&regval);

	if (kStatus_Success != status)
		return status;

	regval = (regval & ~(clrMask)) | (value & clrMask);

	if (data_width == 1)
		return cambus_writeb2(i2c_addr, reg, (uint8_t)regval);
	else if (data_width == 2)
		return cambus_writew2(i2c_addr, reg, (uint16_t)regval);
	else
		return cambus_writel2(i2c_addr, reg, (uint32_t)regval);
}

static status_t mt9m114_soft_reset(sensor_t *sensor)
{
	mt9m114_modify_reg(sensor->slv_addr, MT9M114_REG_RESET_AND_MISC_CONTROL, 2u, 0x01, 0x01);
	k_sleep(1);
	mt9m114_modify_reg(sensor->slv_addr, MT9M114_REG_RESET_AND_MISC_CONTROL, 2u, 0x01, 0x00);
	k_sleep(45);

	return kStatus_Success;
}

static void mt9m114_reg_check(sensor_t *sensor, const mt9m114_reg_t regs[], uint32_t num)
{
	int i;
	status_t status = kStatus_Success;

	for (i = 0; i < num; i++) {
		uint8_t readb = 0;
		uint16_t readw = 0;
		uint32_t readl = 0;

		if (regs[i].size == 1)
			status = cambus_readb2(sensor->slv_addr, regs[i].reg, &readb);
		else if (regs[i].size == 2)
			status = cambus_readw2(sensor->slv_addr, regs[i].reg, &readw);
		else if (regs[i].size == 4)
			status = cambus_readl2(sensor->slv_addr, regs[i].reg, &readl);
		else {
			printk("mt9m114_reg_check error1 regs[%d].reg:0x%04x, regs[%d].size:%d, regs[%d].value:0x%08x\r\n",
				i, regs[i].reg, i, regs[i].size, i, (unsigned int)regs[i].value);
			return;
		}

		if (kStatus_Success != status) {
			printk("mt9m114_reg_check error2 regs[%d].reg:0x%04x, regs[%d].size:%d, regs[%d].value:0x%08x\r\n",
				i, regs[i].reg, i, regs[i].size, i, (unsigned int)regs[i].value);
			break;
		} else if (regs[i].size == 1) {
			if (readb != regs[i].value)
				printk("mt9m114_reg_check error3 regs[%d].reg:0x%04x, regs[%d].size:%d, regs[%d].value:0x%08x, 0x%02x\r\n",
				i, regs[i].reg, i, regs[i].size, i, (unsigned int)regs[i].value, readb);
		} else if (regs[i].size == 2) {
			if (readw != regs[i].value)
				printk("mt9m114_reg_check error4 regs[%d].reg:0x%04x, regs[%d].size:%d, regs[%d].value:0x%08x, 0x%04x\r\n",
				i, regs[i].reg, i, regs[i].size, i, (unsigned int)regs[i].value, readw);
		} else if (regs[i].size == 4) {
			if (readl != regs[i].value)
				printk("mt9m114_reg_check error5 regs[%d].reg:0x%04x, regs[%d].size:%d, regs[%d].value:0x%08x, 0x%08x\r\n",
				i, regs[i].reg, i, regs[i].size, i, (unsigned int)regs[i].value, (unsigned int)readl);
		} else
			printk("mt9m114_reg_check error6 regs[%d].reg:0x%04x, regs[%d].size:%d, regs[%d].value:0x%08x\r\n",
				i, regs[i].reg, i, regs[i].size, i, (unsigned int)regs[i].value);
	}

}
static status_t mt9m114_multi_write(sensor_t *sensor, const mt9m114_reg_t regs[], uint32_t num)
{
	status_t status = kStatus_Success;
	int i;

	for (i = 0; i < num; i++) {
		if (regs[i].size == 1)
			status = cambus_writeb2(sensor->slv_addr, regs[i].reg, (uint8_t)regs[i].value);
		else if (regs[i].size == 2)
			status = cambus_writew2(sensor->slv_addr, regs[i].reg, (uint16_t)regs[i].value);
		else if (regs[i].size == 4)
			status = cambus_writel2(sensor->slv_addr, regs[i].reg, (uint32_t)regs[i].value);
		else {
			printk("mt9m114_multi_write size error regs[%d].reg:0x%04x, regs[%d].size:%d, regs[%d].value:0x%08x\r\n",
				i, regs[i].reg, i, regs[i].size, i, (unsigned int)regs[i].value);
			return kStatus_InvalidArgument;
		}

		if (kStatus_Success != status) {
			printk("mt9m114_multi_write error regs[%d].reg:0x%04x, regs[%d].size:%d, regs[%d].value:0x%08x\r\n",
				i, regs[i].reg, i, regs[i].size, i, (unsigned int)regs[i].value);
			break;
		}
	}

	mt9m114_reg_check(sensor, regs, num);

	return status;
}

static status_t mt9m114_set_state(sensor_t *sensor, uint8_t nextState)
{
	uint16_t value;

	/* Set the desired next state. */
	printk("mt9m114_set_state start!!\r\n");
	cambus_writeb2(sensor->slv_addr, MT9M114_VAR_SYSMGR_NEXT_STATE, nextState);

	/* Check that the FW is ready to accept a new command. */
	while (1) {
		k_sleep(100);
		cambus_readw2(sensor->slv_addr, MT9M114_REG_COMMAND_REGISTER, &value);
		if (!(value & MT9M114_COMMAND_SET_STATE))
			break;
		//printk("error value:0x%04x\r\n", value);
	}
	printk("mt9m114_set_state 11111\r\n");

	/* Issue the Set State command. */
	cambus_writew2(sensor->slv_addr, MT9M114_REG_COMMAND_REGISTER, MT9M114_COMMAND_SET_STATE | MT9M114_COMMAND_OK);

	/* Wait for the FW to complete the command. */
	while (1) {
		k_sleep(100);
		cambus_readw2(sensor->slv_addr, MT9M114_REG_COMMAND_REGISTER, &value);
		if (!(value & MT9M114_COMMAND_SET_STATE))
			break;
	}

	printk("mt9m114_set_state 222\r\n");
	/* Check the 'OK' bit to see if the command was successful. */
	cambus_readw2(sensor->slv_addr, MT9M114_REG_COMMAND_REGISTER, &value);
	printk("mt9m114_set_state 333 value:0x%04x\r\n", value);
	if (!(value & MT9M114_COMMAND_OK))
		return kStatus_Fail;

	return kStatus_Success;
}

static int reset(sensor_t *sensor)
{
	uint16_t outputFormat = 0;
	int ret;

	/* SW reset. */
	printk("mt9m114 reset start!!\r\n");
	ret = mt9m114_soft_reset(sensor);
	printk("mt9m114 reset mt9m114_soft_reset ret:%d\r\n", ret);
	if (ret != kStatus_Success)
		return ret;

	ret = mt9m114_multi_write(sensor, mt9m114_init_cfg, ARRAY_SIZE(mt9m114_init_cfg));
	printk("mt9m114 reset mt9m114_multi_write 111 ret:%d\r\n", ret);
	if (ret != kStatus_Success)
		return ret;
	/* Pixel format. */
	if (PIXFORMAT_RGB565 == sensor->pixformat)
		outputFormat |= ((1U << 8U) | (1U << 1U));

	ret = cambus_writew2(sensor->slv_addr, MT9M114_VAR_CAM_OUTPUT_FORMAT, outputFormat);
	printk("mt9m114 reset cambus_writew 222 ret:%d\r\n", ret);
	if (ret != kStatus_Success)
		return ret;

	/*kCAMERA_InterfaceGatedClock*/
	ret = cambus_writew2(sensor->slv_addr, MT9M114_VAR_CAM_PORT_OUTPUT_CONTROL, 0x8000);
	printk("mt9m114 reset cambus_writew 333 ret:%d\r\n", ret);
	if (ret != kStatus_Success)
		return ret;

	/* Resolution 480 * 272*/
	ret = mt9m114_multi_write(sensor, mt9m114_480_272, ARRAY_SIZE(mt9m114_480_272));
	printk("mt9m114 reset mt9m114_multi_write 444 ret:%d\r\n", ret);
	if (ret != kStatus_Success)
		return ret;

	/* Execute Change-Config command. */
	ret = mt9m114_set_state(sensor, MT9M114_SYS_STATE_ENTER_CONFIG_CHANGE);
	printk("mt9m114 reset mt9m114_set_state 555 ret:%d\r\n", ret);
	if (ret != kStatus_Success)
		return ret;

	ret = mt9m114_set_state(sensor, MT9M114_SYS_STATE_START_STREAMING);
	printk("mt9m114 reset mt9m114_set_state 666 ret:%d\r\n", ret);
	return ret;
}


static int sleep(sensor_t *sensor, int enable)
{
	return 0;
}

static int read_reg(sensor_t *sensor, uint8_t reg_addr)
{
	uint16_t reg_data;

	if (cambus_readw(sensor->slv_addr, reg_addr, &reg_data) != 0)
		return -1;

	return reg_data;
}

static int write_reg(sensor_t *sensor, uint8_t reg_addr, uint16_t reg_data)
{
	return cambus_writew(sensor->slv_addr, reg_addr, reg_data);
}

static int set_pixformat(sensor_t *sensor, pixformat_t pixformat)
{
	if (pixformat != PIXFORMAT_RGB565)
		return -1;
	return 0;
}

static int set_framesize(sensor_t *sensor, framesize_t framesize)
{
	uint16_t width = resolution[framesize][0];
	uint16_t height = resolution[framesize][1];

	printk("set_framesize width:%d, height:%d\r\n", width, height);
	return 0;
}

static int set_framerate(sensor_t *sensor, framerate_t framerate)
{
	return 0;
}

static int set_contrast(sensor_t *sensor, int level)
{
	return 0;
}

static int set_brightness(sensor_t *sensor, int level)
{
	return 0;
}

static int set_saturation(sensor_t *sensor, int level)
{
	return 0;
}

static int set_gainceiling(sensor_t *sensor, gainceiling_t gainceiling)
{
	return 0;
}

static int set_quality(sensor_t *sensor, int quality)
{
	return 0;
}

static int set_colorbar(sensor_t *sensor, int enable)
{
	return 0;
}

static int set_special_effect(sensor_t *sensor, sde_t sde)
{
	return 0;
}

static int set_auto_gain(sensor_t *sensor, int enable, float gain_db, float gain_db_ceiling)
{
	return 0;
}

static int get_gain_db(sensor_t *sensor, float *gain_db)
{
	*gain_db = 0;
	return 0;
}

static int set_auto_exposure(sensor_t *sensor, int enable, int exposure_us)
{
	return 0;
}

static int get_exposure_us(sensor_t *sensor, int *exposure_us)
{
	return 0;
}

static int set_auto_whitebal(sensor_t *sensor, int enable, float r_gain_db, float g_gain_db, float b_gain_db)
{
	return 0;
}

static int get_rgb_gain_db(sensor_t *sensor, float *r_gain_db, float *g_gain_db, float *b_gain_db)
{
	return 0;
}

static int set_hmirror(sensor_t *sensor, int enable)
{
	return 0;
}

static int set_vflip(sensor_t *sensor, int enable)
{
	return 0;
}

static int set_lens_correction(sensor_t *sensor, int enable, int radi, int coef)
{
	return 0;
}

int mt9m114_init(sensor_t *sensor)
{
	printk("mt9m114_init start chip_id:%d\r\n", sensor->chip_id);
	sensor->gs_bpp              = sizeof(uint8_t);
	sensor->reset               = reset;
	sensor->sleep               = sleep;
	sensor->read_reg            = read_reg;
	sensor->write_reg           = write_reg;
	sensor->set_pixformat       = set_pixformat;
	sensor->set_framesize       = set_framesize;
	sensor->set_framerate       = set_framerate;
	sensor->set_contrast        = set_contrast;
	sensor->set_brightness      = set_brightness;
	sensor->set_saturation      = set_saturation;
	sensor->set_gainceiling     = set_gainceiling;
	sensor->set_quality         = set_quality;
	sensor->set_colorbar        = set_colorbar;
	sensor->set_auto_gain       = set_auto_gain;
	sensor->get_gain_db         = get_gain_db;
	sensor->set_auto_exposure   = set_auto_exposure;
	sensor->get_exposure_us     = get_exposure_us;
	sensor->set_auto_whitebal   = set_auto_whitebal;
	sensor->get_rgb_gain_db     = get_rgb_gain_db;
	sensor->set_hmirror         = set_hmirror;
	sensor->set_vflip           = set_vflip;
	sensor->set_special_effect  = set_special_effect;
	sensor->set_lens_correction = set_lens_correction;

	SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_VSYNC, 0);
	SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_HSYNC, 0);
	SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_PIXCK, 0);
	SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_FSYNC, 0);
	SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_JPEGE, 0);
	sensor->pixformat = PIXFORMAT_RGB565;
	printk("mt9m114_init done\r\n");
	return 0;
}
