/*
 *   teamd.h - Network team device daemon
 *   Copyright (C) 2011 Jiri Pirko <jpirko@redhat.com>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _TEAMD_H_
#define _TEAMD_H_

#include <stdbool.h>
#include <libdaemon/dlog.h>
#include <sys/types.h>
#include <sys/time.h>
#include <jansson.h>
#include <team.h>
#include <private/list.h>

#define teamd_log_err(args...) daemon_log(LOG_ERR, ##args)
#define teamd_log_warn(args...) daemon_log(LOG_WARNING, ##args)
#define teamd_log_info(args...) daemon_log(LOG_INFO, ##args)
#define teamd_log_dbg(args...) daemon_log(LOG_DEBUG, ##args)

enum teamd_command {
	DAEMON_CMD_RUN,
	DAEMON_CMD_KILL,
	DAEMON_CMD_VERSION,
	DAEMON_CMD_HELP,
	DAEMON_CMD_CHECK
};

struct teamd_runner;

struct teamd_loop_callback {
	struct list_item list;
	void (*func)(void *func_priv);
	void *func_priv;
	int fd;
	bool is_period;
};

struct teamd_context {
	enum teamd_command	cmd;
	bool			daemonize;
	bool			debug;
	bool			force_recreate;
	char *			config_file;
	char *			config_text;
	json_t *		config_json;
	char *			pid_file;
	char *			team_devname;
	char *			argv0;
	struct team_handle *	th;
	const struct teamd_runner *	runner;
	void *			runner_priv;
	uint32_t		ifindex;
	uint32_t		hwaddr_len;
	struct {
		struct list_item		callback_list;
		int				ctrl_pipe_r;
		int				ctrl_pipe_w;
		int				err;
		struct teamd_loop_callback *	daemon_lcb;
		struct teamd_loop_callback *	libteam_event_lcb;
	} run_loop;
};

struct teamd_runner {
	const char *name;
	const char *team_mode_name;
	size_t priv_size;
	int (*init)(struct teamd_context *ctx);
	void (*fini)(struct teamd_context *ctx);
};

/* Runner structures */
const struct teamd_runner teamd_runner_dummy;
const struct teamd_runner teamd_runner_roundrobin;
const struct teamd_runner teamd_runner_activebackup;

int teamd_loop_callback_fd_add(struct teamd_context *ctx,
			       struct teamd_loop_callback **plcb, int fd,
			       void (*func)(void *func_priv), void *func_priv);
int teamd_loop_callback_period_add(struct teamd_context *ctx,
				   struct teamd_loop_callback **plcb,
				   time_t sec, long nsec,
				   void (*func)(void *func_priv),
				   void *func_priv);
void teamd_loop_callback_del(struct teamd_context *ctx,
			     struct teamd_loop_callback *lcb);

/* Various helpers */
char *dev_name(const struct teamd_context *ctx, uint32_t ifindex);
char *dev_name_dup(const struct teamd_context *ctx, uint32_t ifindex);

#endif /* _TEAMD_H_ */
