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

#if defined(__linux__) && \
    defined(MS_BIND) && \
    defined(MS_REC) && \
    defined(HAVE_CLONE)

#define CLONE_STACK_SIZE	(128*1024)

static void stress_bind_mount_child_handler(int dummy)
{
	(void)dummy;

	_exit(0);
}

/*
 *  stress_bind_mount_child()
 *	aggressively perform bind mounts, this can force out of memory
 *	situations
 */
static int stress_bind_mount_child(void *parg)
{
	const args_t *args = ((pthread_args_t *)parg)->args;
	uint64_t *counter = args->counter;

	if (stress_sighandler(args->name, SIGALRM,
	    stress_bind_mount_child_handler, NULL) < 0) {
		pr_fail_err("sighandler SIGALRM");
		return EXIT_FAILURE;
	}
	if (stress_sighandler(args->name, SIGSEGV,
	    stress_bind_mount_child_handler, NULL) < 0) {
		pr_fail_err("sighandler SIGSEGV");
		return EXIT_FAILURE;
	}
	(void)setpgid(0, g_pgrp);
	stress_parent_died_alarm();

	do {
		int rc;

		rc = mount("/", "/", "", MS_BIND | MS_REC, 0);
		if (rc < 0) {
			if (errno != ENOSPC)
				pr_fail_err("mount");
			break;
		}
		/*
		 *  The following fails with -EBUSY, but try it anyhow
	`	 *  just to make the kernel work harder
		 */
		(void)umount("/");
		inc_counter(args);
	} while (g_keep_stressing_flag &&
		 (!args->max_ops || *counter < args->max_ops));

	return 0;
}

/*
 *  stress_bind_mount()
 *      stress bind mounting
 */
static int stress_bind_mount(const args_t *args)
{
	int pid = 0, status;
	pthread_args_t pargs = { args, NULL };
	const ssize_t stack_offset =
		stress_get_stack_direction() *
		(CLONE_STACK_SIZE - 64);

	do {
		int ret;
		char stack[CLONE_STACK_SIZE];
		char *stack_top = stack + stack_offset;

		memset(stack, 0, sizeof stack);

		pid = clone(stress_bind_mount_child,
			align_stack(stack_top),
			CLONE_NEWUSER | CLONE_NEWNS | CLONE_VM | SIGCHLD,
			(void *)&pargs, 0);
		if (pid < 0) {
			int rc = exit_status(errno);

			pr_fail_err("clone");
			return rc;
		}
		ret = waitpid(pid, &status, 0);
		(void)ret;
	} while (keep_stressing());

	return EXIT_SUCCESS;
}
#else
static int stress_bind_mount(const args_t *args)
{
	return stress_not_implemented(args);
}
#endif

stressor_info_t stress_bind_mount_info = {
	.stressor = stress_bind_mount
};
