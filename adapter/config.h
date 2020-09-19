#ifndef CONFIG_H
#define OONFIG_H

#include"../common/common.h"

#define MOTOR_CFG_INIT(motor_num, pos_num, com_num, neg_num) \
	[motor_num] = { \
		.pos = { \
			.group = 0, \
			.num = pos_num, \
		}, \
		.com = { \
			.group = 0, \
			.num = com_num, \
		}, \
		.neg = { \
			.group = 0, \
			.num = neg_num, \
		}, \
	}

static motor_cfg_t moter_item_config[max_motor] = {
	MOTOR_CFG_INIT(hour_motor, 26, 14, 24),
	MOTOR_CFG_INIT(minute_motor, 28, 14, 27),
	MOTOR_CFG_INIT(activity_motor, 30, 14, 29),
	MOTOR_CFG_INIT(date_motor, 23, 14, 22),
	MOTOR_CFG_INIT(battery_week_motor, 21, 14, 19),
	MOTOR_CFG_INIT(notify_motor, 18, 14, 16),
};

//pin config here
cfg_t args = {
		.uart_cfg = {
			.tx = {
				.group = 0,
				.num = 1,
			},
		},
		.charge_cfg = {
			.group = 0,
			.num = 1,
		},
		.keya_cfg = {
			.group = 0,
			.num = 3,
		},
		.keym_cfg = {
			.group = 0,
			.num = 4,
		},
		.keyb_cfg = {
			.group = 0,
			.num = 31,
		},
		.vibrator_cfg = {
			.group = 0,
			.num = 20,
		},
		.gsensor_cfg = {
			.clk = {
				.group = 0,
				.num = 9,
			},
			.mosi = {
				.group = 0,
				.num = 10
			},
			.miso = {
				.group = 0,
				.num = 11,
			},
			.cs = {
				.group = 0,
				.num = 12,
			},
			.int1 = {
				.group = 0,
				.num = 25,
			},
			.int2 = {
				.group = 0,
				.num = 15,
			},
		},
		.magnetometer_cfg = {
			.scl = {
				.group = 0,
				.num = 13,
			},
			.sda = {
				.group = 0,
				.num = 17,
			},
			.int1 = {
				.group = 0,
				.num = 0,
			},
		},
		#if 0
		.motor_hour_cfg = {
			.pos = {
				.group = 0,
				.num = 26,
			},
			.com = {
				.group = 0,
				.num = 14,
			},
			.neg = {
				.group = 0,
				.num = 24,
			},
		},
		.motor_minute_cfg = {
			.pos = {
				.group = 0,
				.num = 28,
			},
			.com = {
				.group = 0,
				.num = 14,
			},
			.neg = {
				.group = 0,
				.num = 27,
			},
		},
		.motor_activity_cfg = {
			.pos = {
				.group = 0,
				.num = 30,
			},
			.com = {
				.group = 0,
				.num = 14,
			},
			.neg = {
				.group = 0,
				.num = 29,
			},
		},
		.motor_date_cfg = {
			.pos = {
				.group = 0,
				.num = 23,
			},
			.com = {
				.group = 0,
				.num = 14,
			},
			.neg = {
				.group = 0,
				.num = 22,
			},
		},
		.motor_battery_week_cfg = {
			.pos = {
				.group = 0,
				.num = 21,
			},
			.com = {
				.group = 0,
				.num = 14,
			},
			.neg = {
				.group = 0,
				.num = 19,
			},
		},
		.motor_notify_cfg = {
			.pos = {
				.group = 0,
				.num = 18,
			},
			.com = {
				.group = 0,
				.num = 14,
			},
			.neg = {
				.group = 0,
				.num = 16,
			},
		},
		#else
		.motor_item_cfg = &moter_item_config[0],
		#endif
};
	
#endif
