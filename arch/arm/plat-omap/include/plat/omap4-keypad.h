#ifndef ARCH_ARM_PLAT_OMAP4_KEYPAD_H
#define ARCH_ARM_PLAT_OMAP4_KEYPAD_H

#include <linux/platform_data/omap4-keypad.h>

extern int omap4_keyboard_init(struct omap4_keypad_platform_data *,
				struct omap_board_data *);
#endif
