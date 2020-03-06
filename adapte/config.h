#ifndef CONFIG_H
#define OONFIG_H

#include"../common/common.h"

//pin config here
cfg_t cfg = {
	.uart_cfg = {
		.tx = {
			.group = 0,
			.num = 0,
		},
		.rx = {
				.group = 0,
				.num = 0,
		},
	},
};

#endif
