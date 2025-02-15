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

#include "plat_hook.h"
#include <stdio.h>
#include <string.h>
#include <logging/log.h>
#include "sensor.h"
#include "pmbus.h"
#include "libutil.h"
#include "plat_i2c.h"
#include "power_status.h"
#include "common_i2c_mux.h"
#include "i2c-mux-tca9543a.h"
#include "i2c-mux-pi4msd5v9542.h"
#include "plat_sensor_table.h"
#include "i2c-mux-pca954x.h"
#include "plat_class.h"
#include "plat_dev.h"
#include "nvme.h"
#include "plat_ipmi.h"

LOG_MODULE_REGISTER(plat_hook);

#define PEX_SWITCH_INIT_RETRY_COUNT 3

struct k_mutex xdpe15284_mutex;

/**************************************************************************************************
 * INIT ARGS
**************************************************************************************************/
adc_asd_init_arg adc_asd_init_args[] = {
	[0] = { .is_init = false },
};

adm1272_init_arg adm1272_init_args[] = {
	[0] = { .is_init = false, .is_need_set_pwr_cfg = true, .pwr_monitor_cfg.value = 0x3F3F },
	[1] = { .is_init = false, .is_need_set_pwr_cfg = true, .pwr_monitor_cfg.value = 0x3F3F },
};

ltc4286_init_arg ltc4286_init_args[] = {
	[0] = { .is_init = false, .r_sense_mohm = 0.3, .mfr_config_1 = { 0x5572 } },
	[1] = { .is_init = false, .r_sense_mohm = 0.3, .mfr_config_1 = { 0x5572 } }
};

ina233_init_arg ina233_init_args[] = {
	[0] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	[1] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	[2] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	[3] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	[4] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	[5] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	[6] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	[7] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	[8] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	[9] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	[10] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	[11] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
};

pex89000_init_arg pex_sensor_init_args[] = {
	[0] = { .idx = 0, .is_init = false },
	[1] = { .idx = 1, .is_init = false },
};

ina233_init_arg accl_ina233_init_args[] = {
	// ACCL 1
	[0] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	[1] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	[2] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	// ACCL 2
	[3] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	[4] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	[5] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	// ACCL 3
	[6] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	[7] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	[8] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	// ACCL 4
	[9] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	[10] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	[11] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	// ACCL 5
	[12] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	[13] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	[14] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	// ACCL 6
	[15] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	[16] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	[17] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	// ACCL 7
	[18] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	[19] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	[20] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	// ACCL 8
	[21] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	[22] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	[23] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	// ACCL 9
	[24] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	[25] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	[26] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	// ACCL 10
	[27] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	[28] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	[29] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	// ACCL 11
	[30] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	[31] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	[32] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	// ACCL 12
	[33] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	[34] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
	[35] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001, .mfr_config_init = true,
	.mfr_config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b0100,
	},
	},
};

sq52205_init_arg sq52205_init_args[] = {
	[0] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001,
	.config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b000,
		.reset_bit = 0b0,
	},
	},
	[1] = { .is_init = false, .current_lsb = 0.001, .r_shunt = 0.001,
	.config = {
		.operating_mode =0b111,
		.shunt_volt_time = 0b100,
		.bus_volt_time = 0b100,
		.aver_mode = 0b111, //set 1024 average times
		.rsvd = 0b000,
		.reset_bit = 0b0,
	},
	},
};

/**************************************************************************************************
 *  PRE-HOOK/POST-HOOK ARGS
 **************************************************************************************************/
mux_config tca9543_configs[] = {
	[0] = { .target_addr = 0x70, .channel = TCA9543A_CHANNEL_1 },
	[1] = { .target_addr = 0x71, .channel = TCA9543A_CHANNEL_1 },
};

pwr_monitor_pre_proc_arg pwr_monitor_pre_proc_args[] = {
	[0] = { { .target_addr = 0x70, .channel = TCA9543A_CHANNEL_1 }, .card_id = 0 },
	[1] = { { .target_addr = 0x70, .channel = TCA9543A_CHANNEL_1 }, .card_id = 1 },
	[2] = { { .target_addr = 0x70, .channel = TCA9543A_CHANNEL_1 }, .card_id = 2 },
	[3] = { { .target_addr = 0x70, .channel = TCA9543A_CHANNEL_1 }, .card_id = 3 },
	[4] = { { .target_addr = 0x70, .channel = TCA9543A_CHANNEL_1 }, .card_id = 4 },
	[5] = { { .target_addr = 0x70, .channel = TCA9543A_CHANNEL_1 }, .card_id = 5 },
	[6] = { { .target_addr = 0x70, .channel = TCA9543A_CHANNEL_0 }, .card_id = 6 },
	[7] = { { .target_addr = 0x70, .channel = TCA9543A_CHANNEL_0 }, .card_id = 7 },
	[8] = { { .target_addr = 0x70, .channel = TCA9543A_CHANNEL_0 }, .card_id = 8 },
	[9] = { { .target_addr = 0x70, .channel = TCA9543A_CHANNEL_0 }, .card_id = 9 },
	[10] = { { .target_addr = 0x70, .channel = TCA9543A_CHANNEL_0 }, .card_id = 10 },
	[11] = { { .target_addr = 0x70, .channel = TCA9543A_CHANNEL_0 }, .card_id = 11 },
};

vr_page_cfg xdpe15284_page[] = {
	[0] = { .vr_page = PMBUS_PAGE_0 },
	[1] = { .vr_page = PMBUS_PAGE_1 },
};

mux_config pca9548_configs[] = {
	[0] = { .bus = I2C_BUS8, .target_addr = 0x74, .channel = PCA9548A_CHANNEL_5 },
	[1] = { .bus = I2C_BUS8, .target_addr = 0x74, .channel = PCA9548A_CHANNEL_4 },
	[2] = { .bus = I2C_BUS8, .target_addr = 0x74, .channel = PCA9548A_CHANNEL_3 },
	[3] = { .bus = I2C_BUS8, .target_addr = 0x74, .channel = PCA9548A_CHANNEL_2 },
	[4] = { .bus = I2C_BUS8, .target_addr = 0x74, .channel = PCA9548A_CHANNEL_1 },
	[5] = { .bus = I2C_BUS8, .target_addr = 0x74, .channel = PCA9548A_CHANNEL_0 },
	[6] = { .bus = I2C_BUS7, .target_addr = 0x70, .channel = PCA9548A_CHANNEL_5 },
	[7] = { .bus = I2C_BUS7, .target_addr = 0x70, .channel = PCA9548A_CHANNEL_4 },
	[8] = { .bus = I2C_BUS7, .target_addr = 0x70, .channel = PCA9548A_CHANNEL_3 },
	[9] = { .bus = I2C_BUS7, .target_addr = 0x70, .channel = PCA9548A_CHANNEL_2 },
	[10] = { .bus = I2C_BUS7, .target_addr = 0x70, .channel = PCA9548A_CHANNEL_1 },
	[11] = { .bus = I2C_BUS7, .target_addr = 0x70, .channel = PCA9548A_CHANNEL_0 },
};

mux_config pca9546_configs[] = {
	[0] = { .target_addr = 0x72, .channel = PCA9546A_CHANNEL_0 },
	[1] = { .target_addr = 0x72, .channel = PCA9546A_CHANNEL_1 },
	[2] = { .target_addr = 0x72, .channel = PCA9546A_CHANNEL_2 },
	[3] = { .target_addr = 0x72, .channel = PCA9546A_CHANNEL_3 },
};

uint8_t plat_monitor_table_arg[] = { PCIE_CARD_1, PCIE_CARD_2,	PCIE_CARD_3,  PCIE_CARD_4,
				     PCIE_CARD_5, PCIE_CARD_6,	PCIE_CARD_7,  PCIE_CARD_8,
				     PCIE_CARD_9, PCIE_CARD_10, PCIE_CARD_11, PCIE_CARD_12 };

/**************************************************************************************************
 *  PRE-HOOK/POST-HOOK FUNC
 **************************************************************************************************/
bool pre_ina233_read(sensor_cfg *cfg, void *args)
{
	CHECK_NULL_ARG_WITH_RETURN(cfg, false);
	CHECK_NULL_ARG_WITH_RETURN(args, false);

	// Select Channel
	bool ret = true;
	int mutex_status = 0;
	pwr_monitor_pre_proc_arg *pre_args = (pwr_monitor_pre_proc_arg *)args;
	mux_config mux_cfg = pre_args->mux_configs;
	mux_cfg.bus = cfg->port;

	if (asic_card_info[pre_args->card_id].card_status == ASIC_CARD_NOT_PRESENT) {
		cfg->is_enable_polling = false;
		return false;
	}

	struct k_mutex *mutex = get_i2c_mux_mutex(mux_cfg.bus);
	mutex_status = k_mutex_lock(mutex, K_MSEC(MUTEX_LOCK_INTERVAL_MS));
	if (mutex_status != 0) {
		LOG_ERR("Mutex lock fail, status: %d", mutex_status);
		return false;
	}

	ret = set_mux_channel(mux_cfg, MUTEX_LOCK_ENABLE);
	if (ret == false) {
		LOG_ERR("ina233 switch mux fail");
		k_mutex_unlock(mutex);
	}

	return ret;
}

bool post_ina233_read(sensor_cfg *cfg, void *args, int *reading)
{
	CHECK_NULL_ARG_WITH_RETURN(cfg, false);
	ARG_UNUSED(reading);
	ARG_UNUSED(args);

	int unlock_status = 0;
	uint8_t bus = cfg->port;

	struct k_mutex *mutex = get_i2c_mux_mutex(bus);
	if (mutex->lock_count != 0) {
		unlock_status = k_mutex_unlock(mutex);
	}

	if (unlock_status != 0) {
		LOG_ERR("Mutex unlock fail, status: %d", unlock_status);
		return false;
	}

	return true;
}

bool pre_xdpe15284_read(sensor_cfg *cfg, void *args)
{
	CHECK_NULL_ARG_WITH_RETURN(cfg, false);
	CHECK_NULL_ARG_WITH_RETURN(args, false);

	bool ret = false;
	int retry = 3;
	int mutex_status = 0;
	I2C_MSG msg = { 0 };
	vr_page_cfg *xdpe15284_vr_page = (vr_page_cfg *)args;

	mutex_status = k_mutex_lock(&xdpe15284_mutex, K_MSEC(MUTEX_LOCK_INTERVAL_MS));
	if (mutex_status != 0) {
		LOG_ERR("Mutex lock fail, status: %d", mutex_status);
		return false;
	}

	msg.bus = cfg->port;
	msg.target_addr = cfg->target_addr;
	msg.tx_len = 2;
	msg.data[0] = PMBUS_PAGE;
	msg.data[1] = xdpe15284_vr_page->vr_page;

	ret = i2c_master_write(&msg, retry);
	if (ret != 0) {
		LOG_ERR("Set xdpe15284 page fail, ret: %d", ret);
		return false;
	}

	return true;
}

bool post_xdpe15284_read(sensor_cfg *cfg, void *args, int *reading)
{
	CHECK_NULL_ARG_WITH_RETURN(cfg, false);
	ARG_UNUSED(reading);
	ARG_UNUSED(args);

	int unlock_status = k_mutex_unlock(&xdpe15284_mutex);
	if (unlock_status != 0) {
		LOG_ERR("Mutex unlock fail, status: %d, sensor num: 0x%x", unlock_status, cfg->num);
		return false;
	}

	return true;
}

bool pre_pex89000_read(sensor_cfg *cfg, void *args)
{
	CHECK_NULL_ARG_WITH_RETURN(cfg, false);
	CHECK_NULL_ARG_WITH_RETURN(args, false);

	/* Can not access i2c mux and PEX89000 when DC off */
	if (is_acb_power_good() == false) {
		return false;
	}

	pex89000_init_arg *pex_init_arg = (pex89000_init_arg *)cfg->init_args;

	bool ret = true;
	int mutex_status = 0;
	static uint8_t check_init_count = 0;
	mux_config *pre_args = (mux_config *)args;
	pre_args->bus = cfg->port;

	struct k_mutex *mutex = get_i2c_mux_mutex(pre_args->bus);
	mutex_status = k_mutex_lock(mutex, K_MSEC(MUTEX_LOCK_INTERVAL_MS));
	if (mutex_status != 0) {
		LOG_ERR("Mutex lock fail, status: %d", mutex_status);
		return false;
	}

	ret = set_mux_channel(*pre_args, MUTEX_LOCK_ENABLE);
	if (ret == false) {
		LOG_ERR("pex switch mux fail");
		k_mutex_unlock(mutex);
		return false;
	}

	if (pex_init_arg->is_init == false) {
		if (check_init_count >= PEX_SWITCH_INIT_RETRY_COUNT) {
			post_pex89000_read(cfg, cfg->post_sensor_read_args, NULL);
			return false;
		}

		check_init_count += 1;
		ret = init_drive_type_delayed(cfg);
		if (ret == false) {
			LOG_ERR("pex initial fail");
			post_pex89000_read(cfg, cfg->post_sensor_read_args, NULL);
			return ret;
		}
	}

	return ret;
}

bool post_pex89000_read(sensor_cfg *cfg, void *args, int *reading)
{
	CHECK_NULL_ARG_WITH_RETURN(cfg, false);
	ARG_UNUSED(reading);
	ARG_UNUSED(args);

	int unlock_status = 0;
	uint8_t bus = cfg->port;

	struct k_mutex *mutex = get_i2c_mux_mutex(bus);
	if (mutex->lock_count != 0) {
		unlock_status = k_mutex_unlock(mutex);
	}

	if (unlock_status != 0) {
		LOG_ERR("Mutex unlock fail, status: %d", unlock_status);
		return false;
	}

	return true;
}

bool pre_accl_mux_switch(uint8_t sensor_num, void *arg)
{
	CHECK_NULL_ARG_WITH_RETURN(arg, false);

	bool ret = false;
	mux_config accl_mux = { 0 };
	mux_config channel_mux = { 0 };
	uint8_t *card_id = (uint8_t *)arg;

	if (get_accl_mux_config(*card_id, &accl_mux) != true) {
		return false;
	}

	if (get_mux_channel_config(*card_id, sensor_num, &channel_mux) != true) {
		return false;
	}

	int mutex_status = 0;
	struct k_mutex *mutex = get_i2c_mux_mutex(accl_mux.bus);

	mutex_status = k_mutex_lock(mutex, K_MSEC(MUTEX_LOCK_INTERVAL_MS));
	if (mutex_status != 0) {
		LOG_ERR("Mutex lock fail, status: %d", mutex_status);
		return false;
	}

	ret = set_mux_channel(accl_mux, MUTEX_LOCK_ENABLE);
	if (ret == false) {
		LOG_ERR("ACCL switch mux fail");
		k_mutex_unlock(mutex);
		return false;
	}

	ret = set_mux_channel(channel_mux, MUTEX_LOCK_ENABLE);
	if (ret == false) {
		LOG_ERR("ACCL switch mux fail");
		k_mutex_unlock(mutex);
		return false;
	}

	return true;
}

bool post_accl_mux_switch(uint8_t sensor_num, void *arg)
{
	CHECK_NULL_ARG_WITH_RETURN(arg, false);

	mux_config accl_mux = { 0 };
	uint8_t *card_id = (uint8_t *)arg;

	if (get_accl_mux_config(*card_id, &accl_mux) != true) {
		return false;
	}

	int unlock_status = 0;
	struct k_mutex *mutex = get_i2c_mux_mutex(accl_mux.bus);
	if (mutex->lock_count != 0) {
		unlock_status = k_mutex_unlock(mutex);
	}

	if (unlock_status != 0) {
		LOG_ERR("Mutex unlock fail, status: %d", unlock_status);
		return false;
	}

	return true;
}

bool pre_accl_nvme_read(sensor_cfg *cfg, void *args)
{
	CHECK_NULL_ARG_WITH_RETURN(cfg, false);
	CHECK_NULL_ARG_WITH_RETURN(args, false);

	int ret = 0;
	uint8_t nvme_temp = 0;
	uint8_t drive_ready_bit = 0;
	uint8_t nvme_status[FREYA_STATUS_BLOCK_LENGTH] = { 0 };
	freya_info *accl_freya = (freya_info *)args;

	ret = read_nvme_info(cfg->port, cfg->target_addr, FREYA_STATUS_BLOCK_OFFSET,
			     FREYA_STATUS_BLOCK_LENGTH, nvme_status);
	if (ret != 0) {
		LOG_ERR("ACCL pre-read get freya status fail, sensor num: 0x%x", cfg->num);
		return false;
	}

	nvme_temp = nvme_status[NVME_TEMPERATURE_INDEX];
	drive_ready_bit = (nvme_status[FREYA_READY_STATUS_OFFSET] & FREYA_READY_STATUS_BIT);
	if ((nvme_temp == 0) || (drive_ready_bit != 0)) {
		/* Freya not ready */
		cfg->cache_status = SENSOR_POLLING_DISABLE;

		switch (cfg->target_addr) {
		case ACCL_FREYA_1_ADDR:
			accl_freya->freya1_fw_info.is_freya_ready = FREYA_NOT_READY;
			break;
		case ACCL_FREYA_2_ADDR:
			accl_freya->freya2_fw_info.is_freya_ready = FREYA_NOT_READY;
			break;
		default:
			break;
		}
		return true;
	} else {
		if (cfg->cache_status == SENSOR_POLLING_DISABLE) {
			cfg->cache_status = SENSOR_INIT_STATUS;
		}
	}

	switch (cfg->target_addr) {
	case ACCL_FREYA_1_ADDR:
		if (accl_freya->is_cache_freya1_info != true) {
			ret = get_freya_fw_info(cfg->port, cfg->target_addr,
						&accl_freya->freya1_fw_info);
			if ((ret == 0) || (ret == FREYA_NOT_SUPPORT_MODULE_IDENTIFIER_RET_CODE)) {
				accl_freya->is_cache_freya1_info = true;
				return true;
			}
		}
		break;
	case ACCL_FREYA_2_ADDR:
		if (accl_freya->is_cache_freya2_info != true) {
			ret = get_freya_fw_info(cfg->port, cfg->target_addr,
						&accl_freya->freya2_fw_info);
			if ((ret == 0) || (ret == FREYA_NOT_SUPPORT_MODULE_IDENTIFIER_RET_CODE)) {
				accl_freya->is_cache_freya2_info = true;
				return true;
			}
		}
		break;
	default:
		LOG_ERR("Invalid sensor address: 0x%x, sensor num: 0x%x", cfg->target_addr,
			cfg->num);
		return false;
	}

	if (ret != 0) {
		if (ret != FREYA_NOT_READY_RET_CODE) {
			LOG_ERR("Get freya info fail, sensor num: 0x%x", cfg->num);
		}
		return false;
	}

	return true;
}
