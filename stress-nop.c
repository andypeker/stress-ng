/*
 * Copyright (C) 2013-2018 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * This code is a complete clean re-write of the stress tool by
 * Colin Ian King <colin.king@canonical.com> and attempts to be
 * backwardly compatible with the stress tool by Amos Waterland
 * <apw@rossby.metr.ou.edu> but has more stress tests and more
 * functionality.
 *
 */
#include "stress-ng.h"

#if defined(HAVE_ASM_NOP)

#define NOP	__asm__ __volatile__("nop;")
#define NOP8	NOP;  NOP;  NOP;  NOP;  NOP;  NOP;  NOP;  NOP;
#define NOP64	NOP8; NOP8; NOP8; NOP8; NOP8; NOP8; NOP8; NOP8;

/*
 *  stress_nop()
 *	stress that does lots of not a lot
 */
static int stress_nop(const args_t *args)
{
	do {
		register int i = 256;

		while (i--) {
			NOP64
			NOP64
			NOP64
			NOP64
		}
		inc_counter(args);
	} while (keep_stressing());

	return EXIT_SUCCESS;
}
#else
static int stress_nop(const args_t *args)
{
	return stress_not_implemented(args);
}
#endif

stressor_info_t stress_nop_info = {
	.stressor = stress_nop
};
