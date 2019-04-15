/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * main function.
 *
 */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "py/mpconfig.h"
#include "py/qstr.h"
#include "py/nlr.h"
#include "py/lexer.h"
#include "py/parse.h"
#include "py/compile.h"
#include "py/runtime.h"
#include "py/obj.h"
#include "py/objmodule.h"
#include "py/objstr.h"
#include "py/gc.h"
#include "py/stackctrl.h"
#include "sdcard.h"

#include "extmod/vfs.h"
#include "extmod/vfs_fat.h"
#include "lib/utils/pyexec.h"

#include "irq.h"
#include "led.h"
#include "spi.h"
#include "i2c.h"
#include "uart.h"

#include "./sensor.h"
#include "usbdbg.h"
#include "sdram.h"
#include "fb_alloc.h"
#include "ff_wrapper.h"

#include "py_sensor.h"
#include "py_image.h"
#include "py_lcd.h"
#include "py_fir.h"

#include "framebuffer.h"

#include "ini.h"

extern void mp_hal_stdout_tx_strn(const char *str, mp_uint_t len);
extern status_t sdcard_init(void);
extern bool sdcard_is_present(void);

void __fatal_error(const char *msg)
{
	volatile uint delay;

	for (delay = 0; delay < 10000000; delay++)
		;
	mp_hal_stdout_tx_strn("\nFATAL ERROR:\n", 14);
	mp_hal_stdout_tx_strn(msg, strlen(msg));
	while(1)
		;
}

static void flash_error(int n)
{
	return;
}

fs_user_mount_t *g_vfs_fat;

FRESULT exec_boot_script(const char *path, bool selftest, bool interruptible)
{
	nlr_buf_t nlr;
	bool interrupted = false;
#ifdef CONFIG_FAT_FILESYSTEM_ELM
	FRESULT f_res = f_stat(path, NULL);
#else
	FRESULT f_res = f_stat(&g_vfs_fat->fatfs, path, NULL);
#endif

	if (f_res == FR_OK) {
		if (nlr_push(&nlr) == 0) {
			// Enable IDE interrupts if allowed
			if (interruptible) {
				usbdbg_set_irq_enabled(true);
				usbdbg_set_script_running(true);
			}
			// Parse, compile and execute the main script.
			pyexec_file(path);
			nlr_pop();
		} else {
			interrupted = true;
		}
	}

	// Disable IDE interrupts
	usbdbg_set_irq_enabled(false);
	usbdbg_set_script_running(false);

	if (interrupted) {
		if (selftest) {
			// Get the exception message. TODO: might be a hack.
			mp_obj_str_t *str = mp_obj_exception_get_value((mp_obj_t)nlr.ret_val);
			// If any of the self-tests fail log the exception message
			// and loop forever. Note: IDE exceptions will not be caught.
			__fatal_error((const char*) str->data);
		} else {
			mp_obj_print_exception(&mp_plat_print, (mp_obj_t)nlr.ret_val);
			if (nlr_push(&nlr) == 0) {
				flash_error(3);
				nlr_pop();
			}// If this gets interrupted again ignore it.
		}
	}

	if (selftest && f_res == FR_OK) {
		// Remove self tests script and flush cache
#ifdef CONFIG_FAT_FILESYSTEM_ELM
		f_unlink(path);
#else
		f_unlink(&g_vfs_fat->fatfs, path);
#endif

		// Set flag for SWD debugger.
		// Note: main.py does not use the frame buffer.
#ifndef OMV_MPY_ONLY
		MAIN_FB()->bpp = 0xDEAD;
#endif
	}

	return f_res;
}

#include "usb_app.h"
#include "diskio.h"
#include "genhdr/mpversion.h"

extern int pyexec_str(vstr_t *str);
int omv_main(void)
{
	int mp_info_log = 0;

	sensor_init0();
	fb_alloc_init0();
	file_buffer_init0();
	py_lcd_init0();
	usbdbg_init();
	sdcard_init();
	sensor_init();

	if (sdcard_is_present()) {
		FRESULT res;

		g_vfs_fat = m_new_obj_maybe(fs_user_mount_t);
		assert(g_vfs_fat);
		g_vfs_fat->flags = FSUSER_FREE_OBJ;
		sdcard_init_vfs(g_vfs_fat, 1);
#ifdef CONFIG_FAT_FILESYSTEM_ELM
		res = f_mount(&g_vfs_fat->fatfs, "/", 1);
#else
		res = f_mount(&g_vfs_fat->fatfs);
#endif
		if (res != FR_OK) {
			printk("OMV SD mount failed\r\n");
			return false;
		}

		printk("OMV SD mounted\r\n");
	}

	exec_boot_script("/boot.py", false, false);

	// Init USB device to default setting if it was not already configured
	if (!(pyb_usb_flags & PYB_USB_FLAG_USB_MODE_CALLED)) {
		pyb_usb_dev_init(USBD_VID, USBD_PID_CDC_MSC, USBD_MODE_CDC_MSC);
	}

	exec_boot_script("/selftest.py", true, false);
	exec_boot_script("/main.py", false, true);

	usbdbg_init();

	usbddbg_open();
pyexec_from_host:

	usbdbg_set_irq_enabled(true);
	printk("MicroPython " MICROPY_GIT_TAG " on " MICROPY_BUILD_DATE "; " MICROPY_HW_BOARD_NAME " with " MICROPY_HW_MCU_NAME "\r\n");
	printk("Waiting for PY script from host IDE\r\n");

	while (1) {
		if (usbdbg_script_ready())
			break;
	}
	if (!mp_info_log)
		mp_hal_stdout_tx_str("MicroPython " MICROPY_GIT_TAG " on " MICROPY_BUILD_DATE "; " MICROPY_HW_BOARD_NAME " with " MICROPY_HW_MCU_NAME "\r\n");

	mp_info_log = 1;
	do {
		nlr_buf_t nlr;

		if (nlr_push(&nlr) == 0) {
			vstr_t * vstr_dbg = usbdbg_get_script();

			printk("PY script from host IDE:\r\n%s\r\n", vstr_dbg->buf);
			usbdbg_set_irq_enabled(true);

			pyexec_str(usbdbg_get_script());
			nlr_pop();
		} else {
			mp_obj_print_exception(&mp_plat_print, (mp_obj_t)nlr.ret_val);
		}
	} while (0);
	goto pyexec_from_host;
	return -1;
}
