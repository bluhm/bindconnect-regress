/*	$OpenBSD$	*/

/*
 * Copyright (c) 2023 Alexander Bluhm <bluhm@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/resource.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <err.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX(a, b)	((a) > (b) ? (a) : (b))

int fd_base;
unsigned int fd_num = 100;
unsigned int run_time = 10;

static void __dead
usage(void)
{
	fprintf(stderr, "bindstress [-n num] [-t time]\n"
	    "	-n num   number of sockets, default %u\n"
	    "	-t time  run time in seconds, default %u\n",
	    fd_num, run_time);
	exit(2);
}

static inline struct sockaddr *
sintosa(struct sockaddr_in *sin)
{
	return ((struct sockaddr *)(sin));
}

static void *
thread_socket(void *arg)
{
	int *run = arg;

	while (*run) {
		socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	}
	return NULL;
}

static void *
thread_close(void *arg)
{
	int *run = arg;
	int fd;

	while (*run) {
		fd = fd_base + arc4random_uniform(fd_num);
		close(fd);
	}
	return NULL;
}

static void *
thread_bind(void *arg)
{
	int *run = arg;
	int fd;
	struct sockaddr_in sin;

	memset(&sin, 0, sizeof(sin));
	sin.sin_len = sizeof(sin);
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	while (*run) {
		fd = fd_base + arc4random_uniform(fd_num);
		bind(fd, sintosa(&sin), sizeof(sin));
	}
	return NULL;
}

int
main(int argc, char *argv[])
{
	struct rlimit rlim;
	pthread_t tsocket, tbind, tclose;
	const char *errstr;
	int ch, run;

	fd_base = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd_base < 0)
		err(1, "socket fd_base");

	while ((ch = getopt(argc, argv, "t:")) != -1) {
		switch (ch) {
		case 'n':
			fd_num = strtonum(optarg, 1, INT_MAX - fd_base,
			    &errstr);
			if (errstr != NULL)
				errx(1, "num is %s: %s", errstr, optarg);
			break;
		case 't':
			run_time = strtonum(optarg, 0, UINT_MAX, &errstr);
			if (errstr != NULL)
				errx(1, "time is %s: %s", errstr, optarg);
			break;
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;
	if (argc > 0)
		usage();

	if (closefrom(fd_base) < 0)
		err(1, "closefrom %d", fd_base);

	if (getrlimit(RLIMIT_NOFILE, &rlim) < 0)
		err(1, "getrlimit");
	rlim.rlim_max = fd_base + MAX(rlim.rlim_max, fd_num);
	rlim.rlim_cur = fd_base + fd_num;
	if (setrlimit(RLIMIT_NOFILE, &rlim) < 0)
		err(1, "setrlimit %llu", rlim.rlim_cur);

	run = 1;
	errno = pthread_create(&tsocket, NULL, thread_socket, &run);
	if (errno)
		err(1, "pthread_create socket");
	errno = pthread_create(&tclose, NULL, thread_close, &run);
	if (errno)
		err(1, "pthread_create close");
	errno = pthread_create(&tbind, NULL, thread_bind, &run);
	if (errno)
		err(1, "pthread_create bind");

	if (run_time > 0) {
		if (sleep(run_time) < 0)
			err(1, "sleep %u", run_time);
	}

	run = 0;
	errno = pthread_join(tsocket, NULL);
	if (errno)
		err(1, "pthread_join socket");
	errno = pthread_join(tclose, NULL);
	if (errno)
		err(1, "pthread_join close");
	errno = pthread_join(tbind, NULL);
	if (errno)
		err(1, "pthread_join bind");


	return 0;
}
