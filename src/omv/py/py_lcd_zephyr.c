/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * LCD Python module.
 *
 */
#include <mp.h>
#include <objstr.h>
#include <spi.h>
#include <systick.h>
#include "imlib.h"
#include "fb_alloc.h"
#include "ff_wrapper.h"
#include "py_assert.h"
#include "py_helper.h"
#include "py_image.h"

mp_int_t g_py_lcd_width;
mp_int_t g_py_lcd_height;

static mp_obj_t py_lcd_init(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
	return mp_const_none;
}

static mp_obj_t py_lcd_width(void)
{
	return mp_obj_new_int(g_py_lcd_width);
}

static mp_obj_t py_lcd_height(void)
{
	return mp_obj_new_int(g_py_lcd_height);
}

static mp_obj_t py_lcd_type(void)
{
	return mp_const_none;
}

static mp_obj_t py_lcd_set_backlight(mp_obj_t state_obj)
{
	return mp_const_none;
}

static mp_obj_t py_lcd_get_backlight(void)
{
	return mp_const_none;
}

static mp_obj_t py_lcd_display(uint n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
	return mp_const_none;
}

static mp_obj_t py_lcd_clear(void)
{
	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_lcd_init_obj, 0, py_lcd_init);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_lcd_width_obj, py_lcd_width);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_lcd_height_obj, py_lcd_height);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_lcd_type_obj, py_lcd_type);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_lcd_set_backlight_obj, py_lcd_set_backlight);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_lcd_get_backlight_obj, py_lcd_get_backlight);
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(py_lcd_display_obj, 1, py_lcd_display);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_lcd_clear_obj, py_lcd_clear);

static const mp_map_elem_t globals_dict_table[] = {
	{
		MP_OBJ_NEW_QSTR(MP_QSTR___name__),
		MP_OBJ_NEW_QSTR(MP_QSTR_lcd)
	},
	{
		MP_OBJ_NEW_QSTR(MP_QSTR_init),
		(mp_obj_t)&py_lcd_init_obj
	},
	{
		MP_OBJ_NEW_QSTR(MP_QSTR_width),
		(mp_obj_t)&py_lcd_width_obj
	},
	{
		MP_OBJ_NEW_QSTR(MP_QSTR_height),
		(mp_obj_t)&py_lcd_height_obj
	},
	{
		MP_OBJ_NEW_QSTR(MP_QSTR_type),
		(mp_obj_t)&py_lcd_type_obj
	},
	{
		MP_OBJ_NEW_QSTR(MP_QSTR_set_backlight),
		(mp_obj_t)&py_lcd_set_backlight_obj
	},
	{
		MP_OBJ_NEW_QSTR(MP_QSTR_get_backlight),
		(mp_obj_t)&py_lcd_get_backlight_obj
	},
	{
		MP_OBJ_NEW_QSTR(MP_QSTR_display),
		(mp_obj_t)&py_lcd_display_obj
	},
	{
		MP_OBJ_NEW_QSTR(MP_QSTR_clear),
		(mp_obj_t)&py_lcd_clear_obj
	},
	{
		NULL,
		NULL
	},
};
STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t lcd_module = {
	.base = { &mp_type_module },
	.globals = (mp_obj_t)&globals_dict,
};

void py_lcd_init0(void)
{
	return;
}
