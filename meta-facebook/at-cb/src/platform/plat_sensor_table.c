/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "plat_sensor_table.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <logging/log.h>
#include "pmbus.h"
#include "ast_adc.h"
#include "sensor.h"
#include "hal_gpio.h"
#include "plat_i2c.h"
#include "pex89000.h"
#include "plat_gpio.h"
#include "plat_hook.h"
#include "plat_class.h"
#include "plat_dev.h"
#include "sq52205.h"
#include "nvme.h"

LOG_MODULE_REGISTER(plat_sensor_table);

struct k_mutex i2c_2_tca9543_mutex;
struct k_mutex i2c_3_tca9543_mutex;
struct k_mutex i2c_4_pi4msd5v9542_mutex;
struct k_mutex i2c_7_accl_mutex;
struct k_mutex i2c_8_accl_mutex;

sensor_cfg plat_sensor_config[] = {
	/* number,                  type,       port,      address,      offset,
	   access check arg0, arg1, sample_count, cache, cache_status, mux_ADDRess, mux_offset,
	   pre_sensor_read_fn, pre_sensor_read_args, post_sensor_read_fn, post_sensor_read_fn  */

	/** Temperature **/
	{ SENSOR_NUM_TEMP_OUTLET_1, sensor_dev_tmp75, I2C_BUS2, TMP75_IN_ADDR, TMP75_TEMP_OFFSET,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0,
	  SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, NULL },
	{ SENSOR_NUM_TEMP_OUTLET_2, sensor_dev_tmp75, I2C_BUS2, TMP75_OUT_ADDR, TMP75_TEMP_OFFSET,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0,
	  SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, NULL },
	{ SENSOR_NUM_TEMP_FIO_INLET, sensor_dev_lm75bd118, I2C_BUS10, FIO_THERMAL_ADDR,
	  LM75BD118_TEMP_OFFSET, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, NULL },

	/** ADC **/
	{ SENSOR_NUM_VOL_P3V3_AUX, sensor_dev_ast_adc, ADC_PORT3, NONE, NONE, stby_access, 2, 1,
	  SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS,
	  NULL, NULL, NULL, NULL, &adc_asd_init_args[0] },
	{ SENSOR_NUM_VOL_P1V8_PEX, sensor_dev_ast_adc, ADC_PORT4, NONE, NONE, is_dc_access, 1, 1,
	  SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS,
	  NULL, NULL, NULL, NULL, &adc_asd_init_args[0] },
	{ SENSOR_NUM_VOL_P1V2_AUX, sensor_dev_ast_adc, ADC_PORT5, NONE, NONE, stby_access, 1, 1,
	  SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS,
	  NULL, NULL, NULL, NULL, &adc_asd_init_args[0] },
	{ SENSOR_NUM_VOL_P5V_AUX, sensor_dev_ast_adc, ADC_PORT7, NONE, NONE, stby_access, 711, 200,
	  SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS,
	  NULL, NULL, NULL, NULL, &adc_asd_init_args[0] },
	{ SENSOR_NUM_VOL_P1V8_VDD_1, sensor_dev_ast_adc, ADC_PORT10, NONE, NONE, is_dc_access, 1, 1,
	  SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS,
	  NULL, NULL, NULL, NULL, &adc_asd_init_args[0] },
	{ SENSOR_NUM_VOL_P1V8_VDD_2, sensor_dev_ast_adc, ADC_PORT11, NONE, NONE, is_dc_access, 1, 1,
	  SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS,
	  NULL, NULL, NULL, NULL, &adc_asd_init_args[0] },
	{ SENSOR_NUM_VOL_P1V25_VDD_1, sensor_dev_ast_adc, ADC_PORT12, NONE, NONE, is_dc_access, 1,
	  1, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS,
	  NULL, NULL, NULL, NULL, &adc_asd_init_args[0] },
	{ SENSOR_NUM_VOL_P1V25_VDD_2, sensor_dev_ast_adc, ADC_PORT13, NONE, NONE, is_dc_access, 1,
	  1, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS,
	  NULL, NULL, NULL, NULL, &adc_asd_init_args[0] },

	/** VR **/
	{ SENSOR_NUM_TEMP_P0V8_VDD_1, sensor_dev_xdpe15284, I2C_BUS1, XDPE15284D_ADDR,
	  PMBUS_READ_TEMPERATURE_1, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_xdpe15284_read, &xdpe15284_page[0],
	  post_xdpe15284_read, NULL, NULL },
	{ SENSOR_NUM_VOL_P0V8_VDD_1, sensor_dev_xdpe15284, I2C_BUS1, XDPE15284D_ADDR,
	  PMBUS_READ_VOUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_xdpe15284_read, &xdpe15284_page[0],
	  post_xdpe15284_read, NULL, NULL },
	{ SENSOR_NUM_CUR_P0V8_VDD_1, sensor_dev_xdpe15284, I2C_BUS1, XDPE15284D_ADDR,
	  PMBUS_READ_IOUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_xdpe15284_read, &xdpe15284_page[0],
	  post_xdpe15284_read, NULL, NULL },
	{ SENSOR_NUM_PWR_P0V8_VDD_1, sensor_dev_xdpe15284, I2C_BUS1, XDPE15284D_ADDR,
	  PMBUS_READ_POUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_xdpe15284_read, &xdpe15284_page[0],
	  post_xdpe15284_read, NULL, NULL },

	{ SENSOR_NUM_TEMP_P0V8_VDD_2, sensor_dev_xdpe15284, I2C_BUS1, XDPE15284D_ADDR,
	  PMBUS_READ_TEMPERATURE_1, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_xdpe15284_read, &xdpe15284_page[1],
	  post_xdpe15284_read, NULL, NULL },
	{ SENSOR_NUM_VOL_P0V8_VDD_2, sensor_dev_xdpe15284, I2C_BUS1, XDPE15284D_ADDR,
	  PMBUS_READ_VOUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_xdpe15284_read, &xdpe15284_page[1],
	  post_xdpe15284_read, NULL, NULL },
	{ SENSOR_NUM_CUR_P0V8_VDD_2, sensor_dev_xdpe15284, I2C_BUS1, XDPE15284D_ADDR,
	  PMBUS_READ_IOUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_xdpe15284_read, &xdpe15284_page[1],
	  post_xdpe15284_read, NULL, NULL },
	{ SENSOR_NUM_PWR_P0V8_VDD_2, sensor_dev_xdpe15284, I2C_BUS1, XDPE15284D_ADDR,
	  PMBUS_READ_POUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_xdpe15284_read, &xdpe15284_page[1],
	  post_xdpe15284_read, NULL, NULL },

	/** PEX temp **/
	{ SENSOR_NUM_TEMP_PEX_0, sensor_dev_pex89000, I2C_BUS2, PEX89144_I2CS_ADDR, PEX_TEMP,
	  is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0,
	  SENSOR_INIT_STATUS, pre_pex89000_read, &tca9543_configs[0], post_pex89000_read,
	  &tca9543_configs[0], &pex_sensor_init_args[0] },
	{ SENSOR_NUM_TEMP_PEX_1, sensor_dev_pex89000, I2C_BUS3, PEX89144_I2CS_ADDR, PEX_TEMP,
	  is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0,
	  SENSOR_INIT_STATUS, pre_pex89000_read, &tca9543_configs[1], post_pex89000_read,
	  &tca9543_configs[1], &pex_sensor_init_args[1] },

	/** SQ52205 **/
	{ SENSOR_NUM_VOL_P12V_1_M_AUX, sensor_dev_sq52205, I2C_BUS2, SQ52205_P1V25_1_ADDR,
	  SQ52205_READ_VOL_OFFSET, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &sq52205_init_args[0] },
	{ SENSOR_NUM_CUR_P12V_1_M_AUX, sensor_dev_sq52205, I2C_BUS2, SQ52205_P1V25_1_ADDR,
	  SQ52205_READ_CUR_OFFSET, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &sq52205_init_args[0] },
	{ SENSOR_NUM_PWR_P12V_1_M_AUX, sensor_dev_sq52205, I2C_BUS2, SQ52205_P1V25_1_ADDR,
	  SQ52205_READ_PWR_OFFSET, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &sq52205_init_args[0] },
	{ SENSOR_NUM_VOL_P12V_2_M_AUX, sensor_dev_sq52205, I2C_BUS2, SQ52205_P1V25_2_ADDR,
	  SQ52205_READ_VOL_OFFSET, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &sq52205_init_args[1] },
	{ SENSOR_NUM_CUR_P12V_2_M_AUX, sensor_dev_sq52205, I2C_BUS2, SQ52205_P1V25_2_ADDR,
	  SQ52205_READ_CUR_OFFSET, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &sq52205_init_args[1] },
	{ SENSOR_NUM_PWR_P12V_2_M_AUX, sensor_dev_sq52205, I2C_BUS2, SQ52205_P1V25_2_ADDR,
	  SQ52205_READ_PWR_OFFSET, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &sq52205_init_args[1] },

	/** INA233 12V 1 **/
	{ SENSOR_NUM_VOL_P12V_ACCL_1, sensor_dev_ina233, I2C_BUS4, INA233_12V_1_7_ADDR,
	  PMBUS_READ_VOUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[0], post_ina233_read, &pwr_monitor_pre_proc_args[0],
	  &ina233_init_args[0] },
	{ SENSOR_NUM_CUR_P12V_ACCL_1, sensor_dev_ina233, I2C_BUS4, INA233_12V_1_7_ADDR,
	  PMBUS_READ_IOUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[0], post_ina233_read, &pwr_monitor_pre_proc_args[0],
	  &ina233_init_args[0] },
	{ SENSOR_NUM_PWR_P12V_ACCL_1, sensor_dev_ina233, I2C_BUS4, INA233_12V_1_7_ADDR,
	  PMBUS_READ_POUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[0], post_ina233_read, &pwr_monitor_pre_proc_args[0],
	  &ina233_init_args[0] },

	/** INA233 12V 2 **/
	{ SENSOR_NUM_VOL_P12V_ACCL_2, sensor_dev_ina233, I2C_BUS4, INA233_12V_2_8_ADDR,
	  PMBUS_READ_VOUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[1], post_ina233_read, &pwr_monitor_pre_proc_args[1],
	  &ina233_init_args[1] },
	{ SENSOR_NUM_CUR_P12V_ACCL_2, sensor_dev_ina233, I2C_BUS4, INA233_12V_2_8_ADDR,
	  PMBUS_READ_IOUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[1], post_ina233_read, &pwr_monitor_pre_proc_args[1],
	  &ina233_init_args[1] },
	{ SENSOR_NUM_PWR_P12V_ACCL_2, sensor_dev_ina233, I2C_BUS4, INA233_12V_2_8_ADDR,
	  PMBUS_READ_POUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[1], post_ina233_read, &pwr_monitor_pre_proc_args[1],
	  &ina233_init_args[1] },

	/** INA233 12V 3 **/
	{ SENSOR_NUM_VOL_P12V_ACCL_3, sensor_dev_ina233, I2C_BUS4, INA233_12V_3_9_ADDR,
	  PMBUS_READ_VOUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[2], post_ina233_read, &pwr_monitor_pre_proc_args[2],
	  &ina233_init_args[2] },
	{ SENSOR_NUM_CUR_P12V_ACCL_3, sensor_dev_ina233, I2C_BUS4, INA233_12V_3_9_ADDR,
	  PMBUS_READ_IOUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[2], post_ina233_read, &pwr_monitor_pre_proc_args[2],
	  &ina233_init_args[2] },
	{ SENSOR_NUM_PWR_P12V_ACCL_3, sensor_dev_ina233, I2C_BUS4, INA233_12V_3_9_ADDR,
	  PMBUS_READ_POUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[2], post_ina233_read, &pwr_monitor_pre_proc_args[2],
	  &ina233_init_args[2] },

	/** INA233 12V 4 **/
	{ SENSOR_NUM_VOL_P12V_ACCL_4, sensor_dev_ina233, I2C_BUS4, INA233_12V_4_10_ADDR,
	  PMBUS_READ_VOUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[3], post_ina233_read, &pwr_monitor_pre_proc_args[3],
	  &ina233_init_args[3] },
	{ SENSOR_NUM_CUR_P12V_ACCL_4, sensor_dev_ina233, I2C_BUS4, INA233_12V_4_10_ADDR,
	  PMBUS_READ_IOUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[3], post_ina233_read, &pwr_monitor_pre_proc_args[3],
	  &ina233_init_args[3] },
	{ SENSOR_NUM_PWR_P12V_ACCL_4, sensor_dev_ina233, I2C_BUS4, INA233_12V_4_10_ADDR,
	  PMBUS_READ_POUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[3], post_ina233_read, &pwr_monitor_pre_proc_args[3],
	  &ina233_init_args[3] },

	/** INA233 12V 5 **/
	{ SENSOR_NUM_VOL_P12V_ACCL_5, sensor_dev_ina233, I2C_BUS4, INA233_12V_5_11_ADDR,
	  PMBUS_READ_VOUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[4], post_ina233_read, &pwr_monitor_pre_proc_args[4],
	  &ina233_init_args[4] },
	{ SENSOR_NUM_CUR_P12V_ACCL_5, sensor_dev_ina233, I2C_BUS4, INA233_12V_5_11_ADDR,
	  PMBUS_READ_IOUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[4], post_ina233_read, &pwr_monitor_pre_proc_args[4],
	  &ina233_init_args[4] },
	{ SENSOR_NUM_PWR_P12V_ACCL_5, sensor_dev_ina233, I2C_BUS4, INA233_12V_5_11_ADDR,
	  PMBUS_READ_POUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[4], post_ina233_read, &pwr_monitor_pre_proc_args[4],
	  &ina233_init_args[4] },

	/** INA233 12V 6 **/
	{ SENSOR_NUM_VOL_P12V_ACCL_6, sensor_dev_ina233, I2C_BUS4, INA233_12V_12_ADDR,
	  PMBUS_READ_VOUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[5], post_ina233_read, &pwr_monitor_pre_proc_args[5],
	  &ina233_init_args[5] },
	{ SENSOR_NUM_CUR_P12V_ACCL_6, sensor_dev_ina233, I2C_BUS4, INA233_12V_12_ADDR,
	  PMBUS_READ_IOUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[5], post_ina233_read, &pwr_monitor_pre_proc_args[5],
	  &ina233_init_args[5] },
	{ SENSOR_NUM_PWR_P12V_ACCL_6, sensor_dev_ina233, I2C_BUS4, INA233_12V_12_ADDR,
	  PMBUS_READ_POUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[5], post_ina233_read, &pwr_monitor_pre_proc_args[5],
	  &ina233_init_args[5] },

	/** INA233 12V 7 **/
	{ SENSOR_NUM_VOL_P12V_ACCL_7, sensor_dev_ina233, I2C_BUS4, INA233_12V_1_7_ADDR,
	  PMBUS_READ_VOUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[6], post_ina233_read, &pwr_monitor_pre_proc_args[6],
	  &ina233_init_args[6] },
	{ SENSOR_NUM_CUR_P12V_ACCL_7, sensor_dev_ina233, I2C_BUS4, INA233_12V_1_7_ADDR,
	  PMBUS_READ_IOUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[6], post_ina233_read, &pwr_monitor_pre_proc_args[6],
	  &ina233_init_args[6] },
	{ SENSOR_NUM_PWR_P12V_ACCL_7, sensor_dev_ina233, I2C_BUS4, INA233_12V_1_7_ADDR,
	  PMBUS_READ_POUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[6], post_ina233_read, &pwr_monitor_pre_proc_args[6],
	  &ina233_init_args[6] },

	/** INA233 12V 8 **/
	{ SENSOR_NUM_VOL_P12V_ACCL_8, sensor_dev_ina233, I2C_BUS4, INA233_12V_2_8_ADDR,
	  PMBUS_READ_VOUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[7], post_ina233_read, &pwr_monitor_pre_proc_args[7],
	  &ina233_init_args[7] },
	{ SENSOR_NUM_CUR_P12V_ACCL_8, sensor_dev_ina233, I2C_BUS4, INA233_12V_2_8_ADDR,
	  PMBUS_READ_IOUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[7], post_ina233_read, &pwr_monitor_pre_proc_args[7],
	  &ina233_init_args[7] },
	{ SENSOR_NUM_PWR_P12V_ACCL_8, sensor_dev_ina233, I2C_BUS4, INA233_12V_2_8_ADDR,
	  PMBUS_READ_POUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[7], post_ina233_read, &pwr_monitor_pre_proc_args[7],
	  &ina233_init_args[7] },

	/** INA233 12V 9 **/
	{ SENSOR_NUM_VOL_P12V_ACCL_9, sensor_dev_ina233, I2C_BUS4, INA233_12V_3_9_ADDR,
	  PMBUS_READ_VOUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[8], post_ina233_read, &pwr_monitor_pre_proc_args[8],
	  &ina233_init_args[8] },
	{ SENSOR_NUM_CUR_P12V_ACCL_9, sensor_dev_ina233, I2C_BUS4, INA233_12V_3_9_ADDR,
	  PMBUS_READ_IOUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[8], post_ina233_read, &pwr_monitor_pre_proc_args[8],
	  &ina233_init_args[8] },
	{ SENSOR_NUM_PWR_P12V_ACCL_9, sensor_dev_ina233, I2C_BUS4, INA233_12V_3_9_ADDR,
	  PMBUS_READ_POUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[8], post_ina233_read, &pwr_monitor_pre_proc_args[8],
	  &ina233_init_args[8] },

	/** INA233 12V 10 **/
	{ SENSOR_NUM_VOL_P12V_ACCL_10, sensor_dev_ina233, I2C_BUS4, INA233_12V_4_10_ADDR,
	  PMBUS_READ_VOUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[9], post_ina233_read, &pwr_monitor_pre_proc_args[9],
	  &ina233_init_args[9] },
	{ SENSOR_NUM_CUR_P12V_ACCL_10, sensor_dev_ina233, I2C_BUS4, INA233_12V_4_10_ADDR,
	  PMBUS_READ_IOUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[9], post_ina233_read, &pwr_monitor_pre_proc_args[9],
	  &ina233_init_args[9] },
	{ SENSOR_NUM_PWR_P12V_ACCL_10, sensor_dev_ina233, I2C_BUS4, INA233_12V_4_10_ADDR,
	  PMBUS_READ_POUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[9], post_ina233_read, &pwr_monitor_pre_proc_args[9],
	  &ina233_init_args[9] },

	/** INA233 12V 11 **/
	{ SENSOR_NUM_VOL_P12V_ACCL_11, sensor_dev_ina233, I2C_BUS4, INA233_12V_5_11_ADDR,
	  PMBUS_READ_VOUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[10], post_ina233_read, &pwr_monitor_pre_proc_args[10],
	  &ina233_init_args[10] },
	{ SENSOR_NUM_CUR_P12V_ACCL_11, sensor_dev_ina233, I2C_BUS4, INA233_12V_5_11_ADDR,
	  PMBUS_READ_IOUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[10], post_ina233_read, &pwr_monitor_pre_proc_args[10],
	  &ina233_init_args[10] },
	{ SENSOR_NUM_PWR_P12V_ACCL_11, sensor_dev_ina233, I2C_BUS4, INA233_12V_5_11_ADDR,
	  PMBUS_READ_POUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[10], post_ina233_read, &pwr_monitor_pre_proc_args[10],
	  &ina233_init_args[10] },

	/** INA233 12V 12 **/
	{ SENSOR_NUM_VOL_P12V_ACCL_12, sensor_dev_ina233, I2C_BUS4, INA233_12V_12_ADDR,
	  PMBUS_READ_VOUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[11], post_ina233_read, &pwr_monitor_pre_proc_args[11],
	  &ina233_init_args[11] },
	{ SENSOR_NUM_CUR_P12V_ACCL_12, sensor_dev_ina233, I2C_BUS4, INA233_12V_12_ADDR,
	  PMBUS_READ_IOUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[11], post_ina233_read, &pwr_monitor_pre_proc_args[11],
	  &ina233_init_args[11] },
	{ SENSOR_NUM_PWR_P12V_ACCL_12, sensor_dev_ina233, I2C_BUS4, INA233_12V_12_ADDR,
	  PMBUS_READ_POUT, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_ina233_read,
	  &pwr_monitor_pre_proc_args[11], post_ina233_read, &pwr_monitor_pre_proc_args[11],
	  &ina233_init_args[11] },
};

sensor_cfg plat_accl1_sensor_config[] = {
	/** Nvme Temperature **/
	{ SENSOR_NUM_TEMP_ACCL_FREYA_1, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_1_ADDR,
	  NVME_TEMP_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[0],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_TEMP_ACCL_FREYA_2, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_2_ADDR,
	  NVME_TEMP_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[0],
	  NULL, NULL, NULL, &pca9546_configs[0] },

	/** Nvme Voltage **/
	{ SENSOR_NUM_VOL_ACCL_FREYA_1_1, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_1_ADDR,
	  NVME_VOLTAGE_RAIL_1_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[0],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_1_2, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_1_ADDR,
	  NVME_VOLTAGE_RAIL_2_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[0],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_2_1, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_2_ADDR,
	  NVME_VOLTAGE_RAIL_1_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[0],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_2_2, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_2_ADDR,
	  NVME_VOLTAGE_RAIL_2_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[0],
	  NULL, NULL, NULL, &pca9546_configs[0] },

	/** INA233 Voltage **/
	{ SENSOR_NUM_VOL_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS8, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[0], &pca9546_configs[3] },
	{ SENSOR_NUM_VOL_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[1], &pca9546_configs[3] },
	{ SENSOR_NUM_VOL_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[2], &pca9546_configs[3] },

	/** INA233 Current **/
	{ SENSOR_NUM_CUR_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS8, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[0], &pca9546_configs[3] },
	{ SENSOR_NUM_CUR_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[1], &pca9546_configs[3] },
	{ SENSOR_NUM_CUR_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[2], &pca9546_configs[3] },

	/** INA233 Power **/
	{ SENSOR_NUM_PWR_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS8, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[0], &pca9546_configs[3] },
	{ SENSOR_NUM_PWR_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[1], &pca9546_configs[3] },
	{ SENSOR_NUM_PWR_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[2], &pca9546_configs[3] },
};

sensor_cfg plat_accl2_sensor_config[] = {
	/** Nvme Temperature **/
	{ SENSOR_NUM_TEMP_ACCL_FREYA_1, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_1_ADDR,
	  NVME_TEMP_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[1],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_TEMP_ACCL_FREYA_2, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_2_ADDR,
	  NVME_TEMP_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[1],
	  NULL, NULL, NULL, &pca9546_configs[0] },

	/** Nvme Voltage **/
	{ SENSOR_NUM_VOL_ACCL_FREYA_1_1, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_1_ADDR,
	  NVME_VOLTAGE_RAIL_1_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[1],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_1_2, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_1_ADDR,
	  NVME_VOLTAGE_RAIL_2_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[1],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_2_1, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_2_ADDR,
	  NVME_VOLTAGE_RAIL_1_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[1],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_2_2, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_2_ADDR,
	  NVME_VOLTAGE_RAIL_2_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[1],
	  NULL, NULL, NULL, &pca9546_configs[0] },

	/** INA233 Voltage **/
	{ SENSOR_NUM_VOL_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS8, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[3], &pca9546_configs[3] },
	{ SENSOR_NUM_VOL_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[4], &pca9546_configs[3] },
	{ SENSOR_NUM_VOL_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[5], &pca9546_configs[3] },

	/** INA233 Current **/
	{ SENSOR_NUM_CUR_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS8, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[3], &pca9546_configs[3] },
	{ SENSOR_NUM_CUR_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[4], &pca9546_configs[3] },
	{ SENSOR_NUM_CUR_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[5], &pca9546_configs[3] },

	/** INA233 Power **/
	{ SENSOR_NUM_PWR_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS8, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[3], &pca9546_configs[3] },
	{ SENSOR_NUM_PWR_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[4], &pca9546_configs[3] },
	{ SENSOR_NUM_PWR_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[5], &pca9546_configs[3] },
};

sensor_cfg plat_accl3_sensor_config[] = {
	/** Nvme Temperature **/
	{ SENSOR_NUM_TEMP_ACCL_FREYA_1, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_1_ADDR,
	  NVME_TEMP_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[2],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_TEMP_ACCL_FREYA_2, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_2_ADDR,
	  NVME_TEMP_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[2],
	  NULL, NULL, NULL, &pca9546_configs[0] },

	/** Nvme Voltage **/
	{ SENSOR_NUM_VOL_ACCL_FREYA_1_1, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_1_ADDR,
	  NVME_VOLTAGE_RAIL_1_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[2],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_1_2, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_1_ADDR,
	  NVME_VOLTAGE_RAIL_2_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[2],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_2_1, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_2_ADDR,
	  NVME_VOLTAGE_RAIL_1_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[2],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_2_2, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_2_ADDR,
	  NVME_VOLTAGE_RAIL_2_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[2],
	  NULL, NULL, NULL, &pca9546_configs[0] },

	/** INA233 Voltage **/
	{ SENSOR_NUM_VOL_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS8, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[6], &pca9546_configs[3] },
	{ SENSOR_NUM_VOL_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[7], &pca9546_configs[3] },
	{ SENSOR_NUM_VOL_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[8], &pca9546_configs[3] },

	/** INA233 Current **/
	{ SENSOR_NUM_CUR_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS8, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[6], &pca9546_configs[3] },
	{ SENSOR_NUM_CUR_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[7], &pca9546_configs[3] },
	{ SENSOR_NUM_CUR_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[8], &pca9546_configs[3] },

	/** INA233 Power **/
	{ SENSOR_NUM_PWR_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS8, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[6], &pca9546_configs[3] },
	{ SENSOR_NUM_PWR_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[7], &pca9546_configs[3] },
	{ SENSOR_NUM_PWR_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[8], &pca9546_configs[3] },
};

sensor_cfg plat_accl4_sensor_config[] = {
	/** Nvme Temperature **/
	{ SENSOR_NUM_TEMP_ACCL_FREYA_1, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_1_ADDR,
	  NVME_TEMP_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[3],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_TEMP_ACCL_FREYA_2, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_2_ADDR,
	  NVME_TEMP_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[3],
	  NULL, NULL, NULL, &pca9546_configs[0] },

	/** Nvme Voltage **/
	{ SENSOR_NUM_VOL_ACCL_FREYA_1_1, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_1_ADDR,
	  NVME_VOLTAGE_RAIL_1_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[3],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_1_2, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_1_ADDR,
	  NVME_VOLTAGE_RAIL_2_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[3],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_2_1, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_2_ADDR,
	  NVME_VOLTAGE_RAIL_1_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[3],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_2_2, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_2_ADDR,
	  NVME_VOLTAGE_RAIL_2_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[3],
	  NULL, NULL, NULL, &pca9546_configs[0] },

	/** INA233 Voltage **/
	{ SENSOR_NUM_VOL_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS8, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[9], &pca9546_configs[3] },
	{ SENSOR_NUM_VOL_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[10], &pca9546_configs[3] },
	{ SENSOR_NUM_VOL_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[11], &pca9546_configs[3] },

	/** INA233 Current **/
	{ SENSOR_NUM_CUR_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS8, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[9], &pca9546_configs[3] },
	{ SENSOR_NUM_CUR_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[10], &pca9546_configs[3] },
	{ SENSOR_NUM_CUR_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[11], &pca9546_configs[3] },

	/** INA233 Power **/
	{ SENSOR_NUM_PWR_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS8, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[9], &pca9546_configs[3] },
	{ SENSOR_NUM_PWR_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[10], &pca9546_configs[3] },
	{ SENSOR_NUM_PWR_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[11], &pca9546_configs[3] },
};

sensor_cfg plat_accl5_sensor_config[] = {
	/** Nvme Temperature **/
	{ SENSOR_NUM_TEMP_ACCL_FREYA_1, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_1_ADDR,
	  NVME_TEMP_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[4],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_TEMP_ACCL_FREYA_2, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_2_ADDR,
	  NVME_TEMP_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[4],
	  NULL, NULL, NULL, &pca9546_configs[0] },

	/** Nvme Voltage **/
	{ SENSOR_NUM_VOL_ACCL_FREYA_1_1, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_1_ADDR,
	  NVME_VOLTAGE_RAIL_1_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[4],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_1_2, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_1_ADDR,
	  NVME_VOLTAGE_RAIL_2_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[4],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_2_1, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_2_ADDR,
	  NVME_VOLTAGE_RAIL_1_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[4],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_2_2, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_2_ADDR,
	  NVME_VOLTAGE_RAIL_2_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[4],
	  NULL, NULL, NULL, &pca9546_configs[0] },

	/** INA233 Voltage **/
	{ SENSOR_NUM_VOL_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS8, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[12], &pca9546_configs[3] },
	{ SENSOR_NUM_VOL_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[13], &pca9546_configs[3] },
	{ SENSOR_NUM_VOL_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[14], &pca9546_configs[3] },

	/** INA233 Current **/
	{ SENSOR_NUM_CUR_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS8, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[12], &pca9546_configs[3] },
	{ SENSOR_NUM_CUR_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[13], &pca9546_configs[3] },
	{ SENSOR_NUM_CUR_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[14], &pca9546_configs[3] },

	/** INA233 Power **/
	{ SENSOR_NUM_PWR_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS8, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[12], &pca9546_configs[3] },
	{ SENSOR_NUM_PWR_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[13], &pca9546_configs[3] },
	{ SENSOR_NUM_PWR_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[14], &pca9546_configs[3] },
};

sensor_cfg plat_accl6_sensor_config[] = {
	/** Nvme Temperature **/
	{ SENSOR_NUM_TEMP_ACCL_FREYA_1, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_1_ADDR,
	  NVME_TEMP_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[5],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_TEMP_ACCL_FREYA_2, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_2_ADDR,
	  NVME_TEMP_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[5],
	  NULL, NULL, NULL, &pca9546_configs[0] },

	/** Nvme Voltage **/
	{ SENSOR_NUM_VOL_ACCL_FREYA_1_1, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_1_ADDR,
	  NVME_VOLTAGE_RAIL_1_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[5],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_1_2, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_1_ADDR,
	  NVME_VOLTAGE_RAIL_2_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[5],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_2_1, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_2_ADDR,
	  NVME_VOLTAGE_RAIL_1_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[5],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_2_2, sensor_dev_nvme, I2C_BUS8, ACCL_FREYA_2_ADDR,
	  NVME_VOLTAGE_RAIL_2_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[5],
	  NULL, NULL, NULL, &pca9546_configs[0] },

	/** INA233 Voltage **/
	{ SENSOR_NUM_VOL_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS8, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[15], &pca9546_configs[3] },
	{ SENSOR_NUM_VOL_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[16], &pca9546_configs[3] },
	{ SENSOR_NUM_VOL_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[17], &pca9546_configs[3] },

	/** INA233 Current **/
	{ SENSOR_NUM_CUR_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS8, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[15], &pca9546_configs[3] },
	{ SENSOR_NUM_CUR_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[16], &pca9546_configs[3] },
	{ SENSOR_NUM_CUR_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[17], &pca9546_configs[3] },

	/** INA233 Power **/
	{ SENSOR_NUM_PWR_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS8, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[15], &pca9546_configs[3] },
	{ SENSOR_NUM_PWR_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[16], &pca9546_configs[3] },
	{ SENSOR_NUM_PWR_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS8, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[17], &pca9546_configs[3] },
};

sensor_cfg plat_accl7_sensor_config[] = {
	/** Nvme Temperature **/
	{ SENSOR_NUM_TEMP_ACCL_FREYA_1, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_1_ADDR,
	  NVME_TEMP_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[6],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_TEMP_ACCL_FREYA_2, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_2_ADDR,
	  NVME_TEMP_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[6],
	  NULL, NULL, NULL, &pca9546_configs[0] },

	/** Nvme Voltage **/
	{ SENSOR_NUM_VOL_ACCL_FREYA_1_1, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_1_ADDR,
	  NVME_VOLTAGE_RAIL_1_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[6],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_1_2, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_1_ADDR,
	  NVME_VOLTAGE_RAIL_2_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[6],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_2_1, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_2_ADDR,
	  NVME_VOLTAGE_RAIL_1_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[6],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_2_2, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_2_ADDR,
	  NVME_VOLTAGE_RAIL_2_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[6],
	  NULL, NULL, NULL, &pca9546_configs[0] },

	/** INA233 Voltage **/
	{ SENSOR_NUM_VOL_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS7, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[18], &pca9546_configs[3] },
	{ SENSOR_NUM_VOL_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[19], &pca9546_configs[3] },
	{ SENSOR_NUM_VOL_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[20], &pca9546_configs[3] },

	/** INA233 Current **/
	{ SENSOR_NUM_CUR_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS7, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[18], &pca9546_configs[3] },
	{ SENSOR_NUM_CUR_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[19], &pca9546_configs[3] },
	{ SENSOR_NUM_CUR_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[20], &pca9546_configs[3] },

	/** INA233 Power **/
	{ SENSOR_NUM_PWR_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS7, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[18], &pca9546_configs[3] },
	{ SENSOR_NUM_PWR_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[19], &pca9546_configs[3] },
	{ SENSOR_NUM_PWR_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[20], &pca9546_configs[3] },
};

sensor_cfg plat_accl8_sensor_config[] = {
	/** Nvme Temperature **/
	{ SENSOR_NUM_TEMP_ACCL_FREYA_1, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_1_ADDR,
	  NVME_TEMP_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[7],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_TEMP_ACCL_FREYA_2, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_2_ADDR,
	  NVME_TEMP_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[7],
	  NULL, NULL, NULL, &pca9546_configs[0] },

	/** Nvme Voltage **/
	{ SENSOR_NUM_VOL_ACCL_FREYA_1_1, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_1_ADDR,
	  NVME_VOLTAGE_RAIL_1_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[7],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_1_2, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_1_ADDR,
	  NVME_VOLTAGE_RAIL_2_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[7],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_2_1, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_2_ADDR,
	  NVME_VOLTAGE_RAIL_1_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[7],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_2_2, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_2_ADDR,
	  NVME_VOLTAGE_RAIL_2_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[7],
	  NULL, NULL, NULL, &pca9546_configs[0] },

	/** INA233 Voltage **/
	{ SENSOR_NUM_VOL_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS7, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[21], &pca9546_configs[3] },
	{ SENSOR_NUM_VOL_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[22], &pca9546_configs[3] },
	{ SENSOR_NUM_VOL_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[23], &pca9546_configs[3] },

	/** INA233 Current **/
	{ SENSOR_NUM_CUR_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS7, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[21], &pca9546_configs[3] },
	{ SENSOR_NUM_CUR_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[22], &pca9546_configs[3] },
	{ SENSOR_NUM_CUR_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[23], &pca9546_configs[3] },

	/** INA233 Power **/
	{ SENSOR_NUM_PWR_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS7, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[21], &pca9546_configs[3] },
	{ SENSOR_NUM_PWR_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[22], &pca9546_configs[3] },
	{ SENSOR_NUM_PWR_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[23], &pca9546_configs[3] },
};

sensor_cfg plat_accl9_sensor_config[] = {
	/** Nvme Temperature **/
	{ SENSOR_NUM_TEMP_ACCL_FREYA_1, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_1_ADDR,
	  NVME_TEMP_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[8],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_TEMP_ACCL_FREYA_2, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_2_ADDR,
	  NVME_TEMP_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[8],
	  NULL, NULL, NULL, &pca9546_configs[0] },

	/** Nvme Voltage **/
	{ SENSOR_NUM_VOL_ACCL_FREYA_1_1, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_1_ADDR,
	  NVME_VOLTAGE_RAIL_1_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[8],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_1_2, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_1_ADDR,
	  NVME_VOLTAGE_RAIL_2_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[8],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_2_1, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_2_ADDR,
	  NVME_VOLTAGE_RAIL_1_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[8],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_2_2, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_2_ADDR,
	  NVME_VOLTAGE_RAIL_2_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[8],
	  NULL, NULL, NULL, &pca9546_configs[0] },

	/** INA233 Voltage **/
	{ SENSOR_NUM_VOL_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS7, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[24], &pca9546_configs[3] },
	{ SENSOR_NUM_VOL_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[25], &pca9546_configs[3] },
	{ SENSOR_NUM_VOL_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[26], &pca9546_configs[3] },

	/** INA233 Current **/
	{ SENSOR_NUM_CUR_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS7, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[24], &pca9546_configs[3] },
	{ SENSOR_NUM_CUR_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[25], &pca9546_configs[3] },
	{ SENSOR_NUM_CUR_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[26], &pca9546_configs[3] },

	/** INA233 Power **/
	{ SENSOR_NUM_PWR_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS7, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[24], &pca9546_configs[3] },
	{ SENSOR_NUM_PWR_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[25], &pca9546_configs[3] },
	{ SENSOR_NUM_PWR_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[26], &pca9546_configs[3] },
};

sensor_cfg plat_accl10_sensor_config[] = {
	/** Nvme Temperature **/
	{ SENSOR_NUM_TEMP_ACCL_FREYA_1, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_1_ADDR,
	  NVME_TEMP_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[9],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_TEMP_ACCL_FREYA_2, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_2_ADDR,
	  NVME_TEMP_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[9],
	  NULL, NULL, NULL, &pca9546_configs[0] },

	/** Nvme Voltage **/
	{ SENSOR_NUM_VOL_ACCL_FREYA_1_1, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_1_ADDR,
	  NVME_VOLTAGE_RAIL_1_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[9],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_1_2, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_1_ADDR,
	  NVME_VOLTAGE_RAIL_2_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[9],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_2_1, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_2_ADDR,
	  NVME_VOLTAGE_RAIL_1_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[9],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_2_2, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_2_ADDR,
	  NVME_VOLTAGE_RAIL_2_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[9],
	  NULL, NULL, NULL, &pca9546_configs[0] },

	/** INA233 Voltage **/
	{ SENSOR_NUM_VOL_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS7, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[27], &pca9546_configs[3] },
	{ SENSOR_NUM_VOL_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[28], &pca9546_configs[3] },
	{ SENSOR_NUM_VOL_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[29], &pca9546_configs[3] },

	/** INA233 Current **/
	{ SENSOR_NUM_CUR_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS7, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[27], &pca9546_configs[3] },
	{ SENSOR_NUM_CUR_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[28], &pca9546_configs[3] },
	{ SENSOR_NUM_CUR_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[29], &pca9546_configs[3] },

	/** INA233 Power **/
	{ SENSOR_NUM_PWR_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS7, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[27], &pca9546_configs[3] },
	{ SENSOR_NUM_PWR_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[28], &pca9546_configs[3] },
	{ SENSOR_NUM_PWR_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[29], &pca9546_configs[3] },
};

sensor_cfg plat_accl11_sensor_config[] = {
	/** Nvme Temperature **/
	{ SENSOR_NUM_TEMP_ACCL_FREYA_1, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_1_ADDR,
	  NVME_TEMP_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[10],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_TEMP_ACCL_FREYA_2, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_2_ADDR,
	  NVME_TEMP_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[10],
	  NULL, NULL, NULL, &pca9546_configs[0] },

	/** Nvme Voltage **/
	{ SENSOR_NUM_VOL_ACCL_FREYA_1_1, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_1_ADDR,
	  NVME_VOLTAGE_RAIL_1_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[10],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_1_2, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_1_ADDR,
	  NVME_VOLTAGE_RAIL_2_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[10],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_2_1, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_2_ADDR,
	  NVME_VOLTAGE_RAIL_1_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[10],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_2_2, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_2_ADDR,
	  NVME_VOLTAGE_RAIL_2_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[10],
	  NULL, NULL, NULL, &pca9546_configs[0] },

	/** INA233 Voltage **/
	{ SENSOR_NUM_VOL_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS7, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[30], &pca9546_configs[3] },
	{ SENSOR_NUM_VOL_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[31], &pca9546_configs[3] },
	{ SENSOR_NUM_VOL_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[32], &pca9546_configs[3] },

	/** INA233 Current **/
	{ SENSOR_NUM_CUR_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS7, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[30], &pca9546_configs[3] },
	{ SENSOR_NUM_CUR_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[31], &pca9546_configs[3] },
	{ SENSOR_NUM_CUR_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[32], &pca9546_configs[3] },

	/** INA233 Power **/
	{ SENSOR_NUM_PWR_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS7, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[30], &pca9546_configs[3] },
	{ SENSOR_NUM_PWR_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[31], &pca9546_configs[3] },
	{ SENSOR_NUM_PWR_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[32], &pca9546_configs[3] },
};

sensor_cfg plat_accl12_sensor_config[] = {
	/** Nvme Temperature **/
	{ SENSOR_NUM_TEMP_ACCL_FREYA_1, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_1_ADDR,
	  NVME_TEMP_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[11],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_TEMP_ACCL_FREYA_2, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_2_ADDR,
	  NVME_TEMP_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[11],
	  NULL, NULL, NULL, &pca9546_configs[0] },

	/** Nvme Voltage **/
	{ SENSOR_NUM_VOL_ACCL_FREYA_1_1, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_1_ADDR,
	  NVME_VOLTAGE_RAIL_1_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[11],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_1_2, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_1_ADDR,
	  NVME_VOLTAGE_RAIL_2_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[11],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_2_1, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_2_ADDR,
	  NVME_VOLTAGE_RAIL_1_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[11],
	  NULL, NULL, NULL, &pca9546_configs[0] },
	{ SENSOR_NUM_VOL_ACCL_FREYA_2_2, sensor_dev_nvme, I2C_BUS7, ACCL_FREYA_2_ADDR,
	  NVME_VOLTAGE_RAIL_2_OFFSET, is_dc_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, pre_accl_nvme_read, &accl_freya_info[11],
	  NULL, NULL, NULL, &pca9546_configs[0] },

	/** INA233 Voltage **/
	{ SENSOR_NUM_VOL_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS7, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[33], &pca9546_configs[3] },
	{ SENSOR_NUM_VOL_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[34], &pca9546_configs[3] },
	{ SENSOR_NUM_VOL_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[35], &pca9546_configs[3] },

	/** INA233 Current **/
	{ SENSOR_NUM_CUR_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS7, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[33], &pca9546_configs[3] },
	{ SENSOR_NUM_CUR_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[34], &pca9546_configs[3] },
	{ SENSOR_NUM_CUR_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[35], &pca9546_configs[3] },

	/** INA233 Power **/
	{ SENSOR_NUM_PWR_ACCL_P12V_EFUSE, sensor_dev_ina233, I2C_BUS7, ACCL_12V_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[33], &pca9546_configs[3] },
	{ SENSOR_NUM_PWR_ACCL_P3V3_1, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_1_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[34], &pca9546_configs[3] },
	{ SENSOR_NUM_PWR_ACCL_P3V3_2, sensor_dev_ina233, I2C_BUS7, ACCL_3V3_2_INA233_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &accl_ina233_init_args[35], &pca9546_configs[3] },
};

sensor_cfg adm1272_sensor_config_table[] = {
	{ SENSOR_NUM_TEMP_HSC_1, sensor_dev_adm1272, I2C_BUS1, ADM1272_1_ADDR,
	  PMBUS_READ_TEMPERATURE_1, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &adm1272_init_args[0] },
	{ SENSOR_NUM_VOL_P51V_STBY_L, sensor_dev_adm1272, I2C_BUS1, ADM1272_1_ADDR, PMBUS_READ_VIN,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0,
	  SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, &adm1272_init_args[0] },
	{ SENSOR_NUM_VOL_P51V_AUX_L, sensor_dev_adm1272, I2C_BUS1, ADM1272_1_ADDR, PMBUS_READ_VOUT,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0,
	  SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, &adm1272_init_args[0] },
	{ SENSOR_NUM_CUR_P51V_STBY_L, sensor_dev_adm1272, I2C_BUS1, ADM1272_1_ADDR, PMBUS_READ_IIN,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0,
	  SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, &adm1272_init_args[0] },
	{ SENSOR_NUM_CUR_P51V_AUX_L, sensor_dev_adm1272, I2C_BUS1, ADM1272_1_ADDR, PMBUS_READ_IOUT,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0,
	  SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, &adm1272_init_args[0] },
	{ SENSOR_NUM_PWR_P51V_STBY_L, sensor_dev_adm1272, I2C_BUS1, ADM1272_1_ADDR, PMBUS_READ_PIN,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0,
	  SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, &adm1272_init_args[0] },
	{ SENSOR_NUM_PWR_P51V_AUX_L, sensor_dev_adm1272, I2C_BUS1, ADM1272_1_ADDR, PMBUS_READ_POUT,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0,
	  SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, &adm1272_init_args[0] },

	{ SENSOR_NUM_TEMP_HSC_2, sensor_dev_adm1272, I2C_BUS1, ADM1272_2_ADDR,
	  PMBUS_READ_TEMPERATURE_1, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &adm1272_init_args[1] },
	{ SENSOR_NUM_VOL_P51V_STBY_R, sensor_dev_adm1272, I2C_BUS1, ADM1272_2_ADDR, PMBUS_READ_VIN,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0,
	  SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, &adm1272_init_args[1] },
	{ SENSOR_NUM_VOL_P51V_AUX_R, sensor_dev_adm1272, I2C_BUS1, ADM1272_2_ADDR, PMBUS_READ_VOUT,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0,
	  SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, &adm1272_init_args[1] },
	{ SENSOR_NUM_CUR_P51V_STBY_R, sensor_dev_adm1272, I2C_BUS1, ADM1272_2_ADDR, PMBUS_READ_IIN,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0,
	  SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, &adm1272_init_args[1] },
	{ SENSOR_NUM_CUR_P51V_AUX_R, sensor_dev_adm1272, I2C_BUS1, ADM1272_2_ADDR, PMBUS_READ_IOUT,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0,
	  SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, &adm1272_init_args[1] },
	{ SENSOR_NUM_PWR_P51V_STBY_R, sensor_dev_adm1272, I2C_BUS1, ADM1272_2_ADDR, PMBUS_READ_PIN,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0,
	  SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, &adm1272_init_args[1] },
	{ SENSOR_NUM_PWR_P51V_AUX_R, sensor_dev_adm1272, I2C_BUS1, ADM1272_2_ADDR, PMBUS_READ_POUT,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0,
	  SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, &adm1272_init_args[1] },
};

sensor_cfg ltc4286_sensor_config_table[] = {
	{ SENSOR_NUM_TEMP_HSC_1, sensor_dev_ltc4286, I2C_BUS1, LTC4286_1_ADDR,
	  PMBUS_READ_TEMPERATURE_1, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &ltc4286_init_args[0] },
	{ SENSOR_NUM_VOL_P51V_STBY_L, sensor_dev_ltc4286, I2C_BUS1, LTC4286_1_ADDR, PMBUS_READ_VIN,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0,
	  SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, &ltc4286_init_args[0] },
	{ SENSOR_NUM_VOL_P51V_AUX_L, sensor_dev_ltc4286, I2C_BUS1, LTC4286_1_ADDR, PMBUS_READ_VOUT,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0,
	  SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, &ltc4286_init_args[0] },
	{ SENSOR_NUM_CUR_P51V_STBY_L, sensor_dev_ltc4286, I2C_BUS1, LTC4286_1_ADDR, PMBUS_READ_IIN,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0,
	  SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, &ltc4286_init_args[0] },
	{ SENSOR_NUM_CUR_P51V_AUX_L, sensor_dev_ltc4286, I2C_BUS1, LTC4286_1_ADDR, PMBUS_READ_IOUT,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0,
	  SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, &ltc4286_init_args[0] },
	{ SENSOR_NUM_PWR_P51V_STBY_L, sensor_dev_ltc4286, I2C_BUS1, LTC4286_1_ADDR, PMBUS_READ_PIN,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0,
	  SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, &ltc4286_init_args[0] },
	{ SENSOR_NUM_PWR_P51V_AUX_L, sensor_dev_ltc4286, I2C_BUS1, LTC4286_1_ADDR, PMBUS_READ_POUT,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0,
	  SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, &ltc4286_init_args[0] },

	{ SENSOR_NUM_TEMP_HSC_2, sensor_dev_ltc4286, I2C_BUS1, LTC4286_2_ADDR,
	  PMBUS_READ_TEMPERATURE_1, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL,
	  &ltc4286_init_args[1] },
	{ SENSOR_NUM_VOL_P51V_STBY_R, sensor_dev_ltc4286, I2C_BUS1, LTC4286_2_ADDR, PMBUS_READ_VIN,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0,
	  SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, &ltc4286_init_args[1] },
	{ SENSOR_NUM_VOL_P51V_AUX_R, sensor_dev_ltc4286, I2C_BUS1, LTC4286_2_ADDR, PMBUS_READ_VOUT,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0,
	  SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, &ltc4286_init_args[1] },
	{ SENSOR_NUM_CUR_P51V_STBY_R, sensor_dev_ltc4286, I2C_BUS1, LTC4286_2_ADDR, PMBUS_READ_IIN,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0,
	  SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, &ltc4286_init_args[1] },
	{ SENSOR_NUM_CUR_P51V_AUX_R, sensor_dev_ltc4286, I2C_BUS1, LTC4286_2_ADDR, PMBUS_READ_IOUT,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0,
	  SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, &ltc4286_init_args[1] },
	{ SENSOR_NUM_PWR_P51V_STBY_R, sensor_dev_ltc4286, I2C_BUS1, LTC4286_2_ADDR, PMBUS_READ_PIN,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0,
	  SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, &ltc4286_init_args[1] },
	{ SENSOR_NUM_PWR_P51V_AUX_R, sensor_dev_ltc4286, I2C_BUS1, LTC4286_2_ADDR, PMBUS_READ_POUT,
	  stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT, ENABLE_SENSOR_POLLING, 0,
	  SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, &ltc4286_init_args[1] },
};

sensor_cfg q50sn120a1_sensor_config_table[] = {
	{ SENSOR_NUM_TEMP_POWER_BRICK_1, sensor_dev_q50sn120a1, I2C_BUS1, POWER_BRICK_1_ADDR,
	  PMBUS_READ_TEMPERATURE_1, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, NULL },
	{ SENSOR_NUM_VOL_P12V_AUX_1, sensor_dev_q50sn120a1, I2C_BUS1, POWER_BRICK_1_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, NULL },
	{ SENSOR_NUM_CUR_P12V_AUX_1, sensor_dev_q50sn120a1, I2C_BUS1, POWER_BRICK_1_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, NULL },
	{ SENSOR_NUM_PWR_P12V_AUX_1, sensor_dev_q50sn120a1, I2C_BUS1, POWER_BRICK_1_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, NULL },

	{ SENSOR_NUM_TEMP_POWER_BRICK_2, sensor_dev_q50sn120a1, I2C_BUS1, POWER_BRICK_2_ADDR,
	  PMBUS_READ_TEMPERATURE_1, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, NULL },
	{ SENSOR_NUM_VOL_P12V_AUX_2, sensor_dev_q50sn120a1, I2C_BUS1, POWER_BRICK_2_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, NULL },
	{ SENSOR_NUM_CUR_P12V_AUX_2, sensor_dev_q50sn120a1, I2C_BUS1, POWER_BRICK_2_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, NULL },
	{ SENSOR_NUM_PWR_P12V_AUX_2, sensor_dev_q50sn120a1, I2C_BUS1, POWER_BRICK_2_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, NULL },
};

sensor_cfg bmr351_sensor_config_table[] = {
	{ SENSOR_NUM_TEMP_POWER_BRICK_1, sensor_dev_bmr351, I2C_BUS1, POWER_BRICK_1_ADDR,
	  PMBUS_READ_TEMPERATURE_1, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, NULL },
	{ SENSOR_NUM_VOL_P12V_AUX_1, sensor_dev_bmr351, I2C_BUS1, POWER_BRICK_1_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, NULL },
	{ SENSOR_NUM_CUR_P12V_AUX_1, sensor_dev_bmr351, I2C_BUS1, POWER_BRICK_1_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, NULL },
	{ SENSOR_NUM_PWR_P12V_AUX_1, sensor_dev_bmr351, I2C_BUS1, POWER_BRICK_1_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, NULL },

	{ SENSOR_NUM_TEMP_POWER_BRICK_2, sensor_dev_bmr351, I2C_BUS1, POWER_BRICK_2_ADDR,
	  PMBUS_READ_TEMPERATURE_1, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, NULL },
	{ SENSOR_NUM_VOL_P12V_AUX_2, sensor_dev_bmr351, I2C_BUS1, POWER_BRICK_2_ADDR,
	  PMBUS_READ_VOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, NULL },
	{ SENSOR_NUM_CUR_P12V_AUX_2, sensor_dev_bmr351, I2C_BUS1, POWER_BRICK_2_ADDR,
	  PMBUS_READ_IOUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, NULL },
	{ SENSOR_NUM_PWR_P12V_AUX_2, sensor_dev_bmr351, I2C_BUS1, POWER_BRICK_2_ADDR,
	  PMBUS_READ_POUT, stby_access, 0, 0, SAMPLE_COUNT_DEFAULT, POLL_TIME_DEFAULT,
	  ENABLE_SENSOR_POLLING, 0, SENSOR_INIT_STATUS, NULL, NULL, NULL, NULL, NULL },
};

const int SENSOR_CONFIG_SIZE = ARRAY_SIZE(plat_sensor_config);
const int ACCL_SENSOR_CONFIG_SIZE = ARRAY_SIZE(plat_accl1_sensor_config);

sensor_monitor_table_info plat_monitor_table[] = {
	{ plat_accl1_sensor_config, ACCL_SENSOR_CONFIG_SIZE, is_pcie_device_access, PCIE_CARD_1,
	  pre_accl_mux_switch, post_accl_mux_switch, (void *)&plat_monitor_table_arg[0],
	  (void *)&plat_monitor_table_arg[0], "ACCL 1 sensor table" },
	{ plat_accl2_sensor_config, ACCL_SENSOR_CONFIG_SIZE, is_pcie_device_access, PCIE_CARD_2,
	  pre_accl_mux_switch, post_accl_mux_switch, (void *)&plat_monitor_table_arg[1],
	  (void *)&plat_monitor_table_arg[1], "ACCL 2 sensor table" },
	{ plat_accl3_sensor_config, ACCL_SENSOR_CONFIG_SIZE, is_pcie_device_access, PCIE_CARD_3,
	  pre_accl_mux_switch, post_accl_mux_switch, (void *)&plat_monitor_table_arg[2],
	  (void *)&plat_monitor_table_arg[2], "ACCL 3 sensor table" },
	{ plat_accl4_sensor_config, ACCL_SENSOR_CONFIG_SIZE, is_pcie_device_access, PCIE_CARD_4,
	  pre_accl_mux_switch, post_accl_mux_switch, (void *)&plat_monitor_table_arg[3],
	  (void *)&plat_monitor_table_arg[3], "ACCL 4 sensor table" },
	{ plat_accl5_sensor_config, ACCL_SENSOR_CONFIG_SIZE, is_pcie_device_access, PCIE_CARD_5,
	  pre_accl_mux_switch, post_accl_mux_switch, (void *)&plat_monitor_table_arg[4],
	  (void *)&plat_monitor_table_arg[4], "ACCL 5 sensor table" },
	{ plat_accl6_sensor_config, ACCL_SENSOR_CONFIG_SIZE, is_pcie_device_access, PCIE_CARD_6,
	  pre_accl_mux_switch, post_accl_mux_switch, (void *)&plat_monitor_table_arg[5],
	  (void *)&plat_monitor_table_arg[5], "ACCL 6 sensor table" },
	{ plat_accl7_sensor_config, ACCL_SENSOR_CONFIG_SIZE, is_pcie_device_access, PCIE_CARD_7,
	  pre_accl_mux_switch, post_accl_mux_switch, (void *)&plat_monitor_table_arg[6],
	  (void *)&plat_monitor_table_arg[6], "ACCL 7 sensor table" },
	{ plat_accl8_sensor_config, ACCL_SENSOR_CONFIG_SIZE, is_pcie_device_access, PCIE_CARD_8,
	  pre_accl_mux_switch, post_accl_mux_switch, (void *)&plat_monitor_table_arg[7],
	  (void *)&plat_monitor_table_arg[7], "ACCL 8 sensor table" },
	{ plat_accl9_sensor_config, ACCL_SENSOR_CONFIG_SIZE, is_pcie_device_access, PCIE_CARD_9,
	  pre_accl_mux_switch, post_accl_mux_switch, (void *)&plat_monitor_table_arg[8],
	  (void *)&plat_monitor_table_arg[8], "ACCL 9 sensor table" },
	{ plat_accl10_sensor_config, ACCL_SENSOR_CONFIG_SIZE, is_pcie_device_access, PCIE_CARD_10,
	  pre_accl_mux_switch, post_accl_mux_switch, (void *)&plat_monitor_table_arg[9],
	  (void *)&plat_monitor_table_arg[9], "ACCL 10 sensor table" },
	{ plat_accl11_sensor_config, ACCL_SENSOR_CONFIG_SIZE, is_pcie_device_access, PCIE_CARD_11,
	  pre_accl_mux_switch, post_accl_mux_switch, (void *)&plat_monitor_table_arg[10],
	  (void *)&plat_monitor_table_arg[10], "ACCL 11 sensor table" },
	{ plat_accl12_sensor_config, ACCL_SENSOR_CONFIG_SIZE, is_pcie_device_access, PCIE_CARD_12,
	  pre_accl_mux_switch, post_accl_mux_switch, (void *)&plat_monitor_table_arg[11],
	  (void *)&plat_monitor_table_arg[11], "ACCL 12 sensor table" },
};

bool accl_card_init_status[] = { false, false, false, false, false, false,
				 false, false, false, false, false, false };

void load_sensor_config(void)
{
	memcpy(sensor_config, plat_sensor_config, sizeof(plat_sensor_config));
	sensor_config_count = ARRAY_SIZE(plat_sensor_config);

	// Fix config table in different system/config
	pal_extend_sensor_config();
}

uint8_t pal_get_extend_sensor_config()
{
	uint8_t extend_sensor_config_size = 0;
	uint8_t hsc_module = get_hsc_module();
	uint8_t power_brick_module = get_pwr_brick_module();

	switch (hsc_module) {
	case HSC_MODULE_ADM1272:
		extend_sensor_config_size += ARRAY_SIZE(adm1272_sensor_config_table);
		break;
	case HSC_MODULE_LTC4286:
		extend_sensor_config_size += ARRAY_SIZE(ltc4286_sensor_config_table);
		break;
	default:
		LOG_ERR("Invalid hsc module: 0x%x", hsc_module);
		break;
	}

	switch (power_brick_module) {
	case POWER_BRICK_Q50SN120A1:
		extend_sensor_config_size += ARRAY_SIZE(q50sn120a1_sensor_config_table);
		break;
	case POWER_BRICK_BMR3512202:
		extend_sensor_config_size += ARRAY_SIZE(bmr351_sensor_config_table);
		break;
	default:
		LOG_ERR("Invalid power brick module: 0x%x", power_brick_module);
		break;
	}

	return extend_sensor_config_size;
}

void pal_extend_sensor_config()
{
	uint8_t index = 0;
	uint8_t sensor_count = 0;
	uint8_t hsc_module = get_hsc_module();
	uint8_t power_brick_module = get_pwr_brick_module();

	switch (hsc_module) {
	case HSC_MODULE_ADM1272:
		sensor_count = ARRAY_SIZE(adm1272_sensor_config_table);
		for (index = 0; index < sensor_count; index++) {
			add_sensor_config(adm1272_sensor_config_table[index]);
		}
		break;
	case HSC_MODULE_LTC4286:
		sensor_count = ARRAY_SIZE(ltc4286_sensor_config_table);
		for (index = 0; index < sensor_count; index++) {
			add_sensor_config(ltc4286_sensor_config_table[index]);
		}
		break;
	default:
		LOG_ERR("Invalid hsc module: 0x%x", hsc_module);
		break;
	}

	switch (power_brick_module) {
	case POWER_BRICK_Q50SN120A1:
		sensor_count = ARRAY_SIZE(q50sn120a1_sensor_config_table);
		for (index = 0; index < sensor_count; index++) {
			add_sensor_config(q50sn120a1_sensor_config_table[index]);
		}
		break;
	case POWER_BRICK_BMR3512202:
		sensor_count = ARRAY_SIZE(bmr351_sensor_config_table);
		for (index = 0; index < sensor_count; index++) {
			add_sensor_config(bmr351_sensor_config_table[index]);
		}
		break;
	default:
		LOG_ERR("Invalid power brick module: 0x%x", power_brick_module);
		break;
	}
}

uint8_t pal_get_monitor_sensor_count()
{
	return ARRAY_SIZE(plat_monitor_table);
}

void plat_fill_monitor_sensor_table()
{
	uint8_t plat_monitor_sensor_count = ARRAY_SIZE(plat_monitor_table);
	memcpy(&sensor_monitor_table[1], plat_monitor_table,
	       plat_monitor_sensor_count * sizeof(sensor_monitor_table_info));
}

sensor_cfg *get_accl_sensor_cfg_info(uint8_t card_id, uint8_t *cfg_count)
{
	CHECK_NULL_ARG_WITH_RETURN(cfg_count, NULL);

	sensor_cfg *cfg = NULL;

	uint8_t index = 0;

	for (index = 1; index < sensor_monitor_count; ++index) {
		uint8_t *val = (uint8_t *)sensor_monitor_table[index].priv_data;

		if (*val == card_id) {
			*cfg_count = ACCL_SENSOR_CONFIG_SIZE;
			return sensor_monitor_table[index].monitor_sensor_cfg;
		}
	}

	LOG_ERR("Fail to get ACCL sensor cfg info via card id: 0x%x", card_id);
	return cfg;
}

bool is_acb_power_good()
{
	// BIC can check motherboard dc power status by CPLD power good flag
	bool ret = false;
	uint8_t board_revision = get_board_revision();

	switch (board_revision) {
	case POC_STAGE:
	case EVT1_STAGE:
		ret = get_acb_power_status();
		if (ret != true) {
			LOG_ERR("Get acb power status fail");
			return false;
		}

		return get_acb_power_good_flag();
	case EVT2_STAGE:
	case DVT_STAGE:
	case PVT_STAGE:
	case MP_STAGE:
		return get_acb_power_good_flag();
	default:
		LOG_ERR("Invalid board revision: 0x%x", board_revision);
		return false;
	}
}

bool is_accl_power_good(uint8_t card_id)
{
	int ret = 0;
	int retry = 5;
	I2C_MSG msg = { 0 };
	uint8_t power_good_bit = 0;

	msg.bus = I2C_BUS3;
	msg.target_addr = CPLD_ADDR;
	msg.rx_len = 1;
	msg.tx_len = 1;

	if (card_id <= PCIE_CARD_6) {
		msg.data[0] = CPLD_ACCLB_PWRGD_OFFSET;
		power_good_bit = (1 << (card_id - PCIE_CARD_1));
	} else if (card_id <= PCIE_CARD_12) {
		msg.data[0] = CPLD_ACCLA_PWRGD_OFFSET;
		power_good_bit = (1 << (card_id - PCIE_CARD_7));
	} else {
		LOG_ERR("%s() invalid card id %u ", __func__, card_id);
		return false;
	}

	ret = i2c_master_read(&msg, retry);
	if (ret != 0) {
		LOG_ERR("%s() Fail to read card %u status from cpld", __func__, card_id);
		return false;
	}

	return (msg.data[0] & power_good_bit);
}

bool is_dc_access(uint8_t sensor_num)
{
	return is_acb_power_good();
}

bool is_pcie_device_access(uint8_t card_id)
{
	if (card_id >= ASIC_CARD_COUNT) {
		LOG_ERR("Invalid card id: 0x%x", card_id);
		return false;
	}

	bool ret = false;
	bool is_power_good = false;
	uint8_t index = 0;
	uint8_t cfg_count = 0;
	sensor_cfg *cfg_table = NULL;

	if (asic_card_info[card_id].card_status == ASIC_CARD_PRESENT) {
		cfg_table = get_accl_sensor_cfg_info(card_id, &cfg_count);
		if (cfg_table == NULL) {
			LOG_ERR("Fail to get ACCL sensor cfg index to check pcie device, card id: 0x%x",
				card_id);
			return ret;
		}
		is_power_good = is_accl_power_good(card_id);

		if (is_power_good) {
			if (accl_card_init_status[card_id] == false) {
				for (index = 0; index < cfg_count; ++index) {
					sensor_cfg *cfg = &cfg_table[index];

					if (pre_accl_mux_switch(cfg->num, (void *)&card_id) !=
					    true) {
						LOG_ERR("Fail to pre-switch ACCL mux to check access, card id: 0x%x",
							card_id);
						break;
					}

					if (init_drive_type_delayed(cfg) != true) {
						post_accl_mux_switch(cfg->num, (void *)&card_id);
						break;
					}

					if (post_accl_mux_switch(cfg->num, (void *)&card_id) !=
					    true) {
						LOG_ERR("Fail to post-switch ACCL mux to check access, card id: 0x%x",
							card_id);
					}
				}

				if (index >= cfg_count) {
					accl_card_init_status[card_id] = true;
				}
			}

			for (index = 0; index < cfg_count; ++index) {
				sensor_cfg *cfg = &cfg_table[index];

				switch (cfg->target_addr) {
				case ACCL_FREYA_1_ADDR:
					if (asic_card_info[card_id].asic_1_status !=
					    ASIC_CARD_DEVICE_PRESENT) {
						cfg->cache_status = SENSOR_NOT_PRESENT;
					} else {
						if (cfg->cache_status == SENSOR_NOT_PRESENT) {
							cfg->cache_status = SENSOR_INIT_STATUS;
						}
					}
					break;
				case ACCL_FREYA_2_ADDR:
					if (asic_card_info[card_id].asic_2_status !=
					    ASIC_CARD_DEVICE_PRESENT) {
						cfg->cache_status = SENSOR_NOT_PRESENT;
					} else {
						if (cfg_table[index].cache_status ==
						    SENSOR_NOT_PRESENT) {
							cfg->cache_status = SENSOR_INIT_STATUS;
						}
					}
					break;
				default:
					break;
				}
			}
			return true;
		} else {
			for (index = 0; index < cfg_count; ++index) {
				sensor_cfg *cfg = &cfg_table[index];
				cfg->cache_status = SENSOR_NOT_ACCESSIBLE;
			}

			accl_card_init_status[card_id] = false;
			return false;
		}
	} else {
		return false;
	}
}

struct k_mutex *get_i2c_mux_mutex(uint8_t i2c_bus)
{
	struct k_mutex *mutex = NULL;

	switch (i2c_bus) {
	case I2C_BUS2:
		mutex = &i2c_2_tca9543_mutex;
		break;
	case I2C_BUS3:
		mutex = &i2c_3_tca9543_mutex;
		break;
	case I2C_BUS4:
		mutex = &i2c_4_pi4msd5v9542_mutex;
		break;
	case I2C_BUS7:
		mutex = &i2c_7_accl_mutex;
		break;
	case I2C_BUS8:
		mutex = &i2c_8_accl_mutex;
		break;
	default:
		LOG_ERR("No support for i2c bus %d mutex", i2c_bus);
		break;
	}

	return mutex;
}

int get_accl_bus(uint8_t card_id, uint8_t sensor_number)
{
	if (card_id >= ASIC_CARD_COUNT) {
		LOG_ERR("Invalid accl card id: 0x%x", card_id);
		return -1;
	}

	return pca9548_configs[card_id].bus;
}

sensor_cfg *get_accl_sensor_config(uint8_t card_id, uint8_t sensor_num)
{
	uint8_t i = 0;
	uint8_t cfg_count = 0;
	sensor_cfg *cfg_table = NULL;

	cfg_table = get_accl_sensor_cfg_info(card_id, &cfg_count);
	if (cfg_table == NULL) {
		LOG_ERR("Fail to get ACCL sensor cfg index, card id: 0x%x", card_id);
		return NULL;
	}

	for (i = 0; i < cfg_count; ++i) {
		if (sensor_num == cfg_table[i].num) {
			return &cfg_table[i];
		}
	}

	LOG_ERR("Fail to find sensor num: 0x%x in ACCL: 0x%x sensor config", sensor_num, card_id);
	return NULL;
}

bool get_accl_mux_config(uint8_t card_id, mux_config *accl_mux)
{
	CHECK_NULL_ARG_WITH_RETURN(accl_mux, false);

	if (card_id >= ASIC_CARD_COUNT) {
		LOG_ERR("Invalid accl card id: 0x%x", card_id);
		return false;
	}

	*accl_mux = pca9548_configs[card_id];
	return true;
}

bool get_mux_channel_config(uint8_t card_id, uint8_t sensor_number, mux_config *channel_mux)
{
	CHECK_NULL_ARG_WITH_RETURN(channel_mux, false);

	if (card_id >= ASIC_CARD_COUNT) {
		LOG_ERR("Invalid accl card id: 0x%x", card_id);
		return false;
	}

	bool ret = false;
	uint8_t accl_bus = get_accl_bus(card_id, sensor_number);
	mux_config *mux_cfg = NULL;
	sensor_cfg *cfg = NULL;

	cfg = get_accl_sensor_config(card_id, sensor_number);
	if (cfg == NULL) {
		return ret;
	}

	CHECK_NULL_ARG_WITH_RETURN(cfg->priv_data, false);

	mux_cfg = cfg->priv_data;
	mux_cfg->bus = accl_bus;
	*channel_mux = *mux_cfg;
	return true;
}
