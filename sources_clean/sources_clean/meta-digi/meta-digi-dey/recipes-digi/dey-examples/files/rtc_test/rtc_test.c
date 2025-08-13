/*
 * rtc_test.c
 *
 * Copyright (C) 2009-2013 by Digi International Inc.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * Description: RTC test application
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <string.h>

#define	PROGRAM			"rtc_test"
#define VERSION			"3.2"

#define	DEFAULT_RTC_DEVICE_FILE	"/dev/rtc"

// Digi RTC IOCTLs custom implementation using MCA
#define RTC_IOCTL_DIGI 		0x100
#define RTC_MCA_UIE_ON 		(RTC_IOCTL_DIGI + RTC_UIE_ON)
#define RTC_MCA_UIE_OFF 	(RTC_IOCTL_DIGI + RTC_UIE_OFF)
#define RTC_MCA_PIE_ON 		(RTC_IOCTL_DIGI + RTC_PIE_ON)
#define RTC_MCA_PIE_OFF 	(RTC_IOCTL_DIGI + RTC_PIE_OFF)
#define RTC_MCA_IRQP_READ	(RTC_IOCTL_DIGI + RTC_IRQP_READ)
#define RTC_MCA_IRQP_SET 	(RTC_IOCTL_DIGI + RTC_IRQP_SET)

/* test options */
#define	RTC_TEST_RD_TIME		(1<<0)
#define	RTC_TEST_SET_TIME		(1<<1)
#define	RTC_TEST_RD_ALARM		(1<<2)
#define	RTC_TEST_SET_ALARM		(1<<3)
#define	RTC_TEST_ALARM_IRQ		(1<<4)
#define	RTC_TEST_STD_PERIODIC_IRQ	(1<<5)
#define	RTC_TEST_STD_1HZ_IRQ		(1<<6)
#define	RTC_TEST_DIGI_PERIODIC_IRQ	(1<<7)
#define	RTC_TEST_DIGI_1HZ_IRQ		(1<<8)

#define	RTC_DEFAULT_TEST_OPS	0	/* None, pass it through the command line */

#define rtc_test_usage \
	"[-abcdefhms]\n"
#define rtc_test_full_usage \
	"rtc_test [options]\n\n" \
        "Tests the real time clock driver\n" \
        "Options:\n" \
        "  -a : Read the current time\n" \
        "  -b : Set the current time\n" \
        "  -c : Read the alarm programed value\n" \
        "  -d : Set the alarm to trigger in the specified seconds\n" \
        "  -e : Test the alarm interrupt with the specified seconds\n" \
        "  -p : Test the standard periodic interrupts (uses timers, doesn't wake from low power)\n" \
        "  -u : Test the standard 1 Hz interrupt (uses RTC ALARM)\n" \
        "  -v : Test MCA periodic interrupts (uses RTC PERIODIC_IRQ)\n" \
        "  -w : Test MCA 1 Hz interrupt (uses RTC 1HZ)\n" \
        "  -f : Use specified device (default is /dev/rtc)\n" \
	"  -h : Help\n" \
	"  -m : Alarm has minutes resolution. (seconds by default)." \
	" In this mode the alarm triggers on the minutes register and the" \
	" seconds are ignored.\n" \
        "  -s : Seconds to trigger alarm (default 5)." \
	" With minutes resolution this must be a multiple of 60\n\n" \

/* Function prototypes */
static void rtc_test_banner(void);
static void exit_error(char *error_msg, int exit_val);
static void show_usage_exit(int exit_val, int full);
static int rtc_test_time_read(int fd, struct rtc_time *tm);
static int rtc_test_time_set(int fd, struct rtc_time *tm);
static int rtc_test_alarm_read(int fd);
static int rtc_test_alarm_set(int fd, struct rtc_time *tm);
static int rtc_test_alarm_irq(int fd, struct rtc_wkalrm *wkalrm);
static int rtc_test_periodic_irq(int fd, int digi);
static int rtc_test_1hz_irq(int fd, int digi);
static void rtc_test_display_test_results(unsigned int test_ops, unsigned int test_results);

static int seconds = 5;
static int minutes_res = 0;

/*
 * Function:    rtc_test_banner
 * Description: print message
 */
static void rtc_test_banner(void)
{
	fprintf(stdout, "%s %s Copyright Digi International Inc.\n\n"
		"RTC test/demo application\n\n", PROGRAM, VERSION);
}

/*
 * Function:    exit_error
 * Description: print error message and exit
 */
static void exit_error(char *error_msg, int exit_val)
{
	if (error_msg != NULL)
		fprintf(stderr, "%s", error_msg);

	exit(exit_val);
}

/*
 * Function:    show_usage_exit
 * Description: print usage information and exit
 */
static void show_usage_exit(int exit_val, int full)
{
	if (full) {
		rtc_test_banner();
		fprintf(stdout, "%s", rtc_test_full_usage);
	} else {
		fprintf(stdout, "%s", rtc_test_usage);
	}

	exit_error(NULL, exit_val);
}

/*
 * Function:    main
 * Description: application's main function
 */
int main(int argc, char *argv[])
{
	int opt, rtc_fd, retval;
	unsigned int test_ops = RTC_DEFAULT_TEST_OPS;
	unsigned int test_results = 0;
	struct rtc_time rtc_tm;
	struct rtc_wkalrm wkalrm;
	char rtc_device_file[10] = DEFAULT_RTC_DEVICE_FILE;

	memset(&rtc_tm, 0, sizeof(struct rtc_time));
	memset(&wkalrm, 0, sizeof(struct rtc_wkalrm));

	if (argc > 1) {
		while ((opt = getopt(argc, argv, "abcdepuvwf:hms:")) > 0) {
			switch (opt) {
			case 'a':
				test_ops |= RTC_TEST_RD_TIME;
				break;
			case 'b':
				test_ops |= RTC_TEST_SET_TIME;
				break;
			case 'c':
				test_ops |= RTC_TEST_RD_ALARM;
				break;
			case 'd':
				test_ops |= RTC_TEST_SET_ALARM;
				break;
			case 'e':
				test_ops |= RTC_TEST_ALARM_IRQ;
				break;
			case 'p':
				test_ops |= RTC_TEST_STD_PERIODIC_IRQ;
				break;
			case 'u':
				test_ops |= RTC_TEST_STD_1HZ_IRQ;
				break;
			case 'v':
				test_ops |= RTC_TEST_DIGI_PERIODIC_IRQ;
				break;
			case 'w':
				test_ops |= RTC_TEST_DIGI_1HZ_IRQ;
				break;
			case 'f':
				strncpy(rtc_device_file,optarg,10);
				break;
			case 'm':
				minutes_res = 1;
				break;
			case 's':
				seconds = atoi(optarg);
				if(minutes_res && (seconds%60 != 0)){
					exit_error(
					"Seconds should be divisible by 60"
					" as you specified minutes resolution."
					"\n\n",EXIT_FAILURE);
				}
				break;
			case 'h':
			default:
				show_usage_exit((opt == 'h') ?
					 EXIT_SUCCESS : EXIT_FAILURE, 1);
			}
		}
	}

	rtc_test_banner();

	rtc_fd = open(rtc_device_file, O_RDONLY);
	if (rtc_fd < 0) {
		perror(rtc_device_file);
		exit(EXIT_FAILURE);
	}

	if (test_ops &
	    (RTC_TEST_RD_TIME | RTC_TEST_SET_ALARM | RTC_TEST_ALARM_IRQ |
		RTC_TEST_SET_TIME)) {
		retval = rtc_test_time_read(rtc_fd, &rtc_tm);
		if (retval == 1)
			test_results |= RTC_TEST_RD_TIME;
	}

	if (test_ops & RTC_TEST_SET_TIME) {
		retval = rtc_test_time_set(rtc_fd, &rtc_tm);
		if (retval == 1)
			test_results |= RTC_TEST_SET_TIME;
	}

	if (test_ops & RTC_TEST_RD_ALARM) {
		retval = rtc_test_alarm_read(rtc_fd);
		if (retval == 1)
			test_results |= RTC_TEST_RD_ALARM;
	}

	if (test_ops & (RTC_TEST_SET_ALARM | RTC_TEST_ALARM_IRQ)) {
		retval = rtc_test_alarm_set(rtc_fd, &rtc_tm);
		if (retval == 1)
			test_results |= RTC_TEST_SET_ALARM;
	}

	if (test_ops & RTC_TEST_ALARM_IRQ) {
		memcpy(&wkalrm.time, &rtc_tm, sizeof(struct rtc_time));
		wkalrm.enabled = 1;
		retval = rtc_test_alarm_irq(rtc_fd, &wkalrm);
		if (retval == 1)
			test_results |= RTC_TEST_ALARM_IRQ;
		else if (retval == 0)
			test_results &= ~RTC_TEST_ALARM_IRQ;
	}

	if (test_ops & (RTC_TEST_STD_PERIODIC_IRQ)) {
		retval = rtc_test_periodic_irq(rtc_fd, 0);
		if (retval == 1)
			test_results |= RTC_TEST_STD_PERIODIC_IRQ;
	}

	if (test_ops & (RTC_TEST_STD_1HZ_IRQ)) {
		retval = rtc_test_1hz_irq(rtc_fd, 0);
		if (retval == 1)
			test_results |= RTC_TEST_STD_1HZ_IRQ;
	}

	if (test_ops & (RTC_TEST_DIGI_PERIODIC_IRQ)) {
		retval = rtc_test_periodic_irq(rtc_fd, 1);
		if (retval == 1)
			test_results |= RTC_TEST_DIGI_PERIODIC_IRQ;
	}

	if (test_ops & (RTC_TEST_DIGI_1HZ_IRQ)) {
		retval = rtc_test_1hz_irq(rtc_fd, 1);
		if (retval == 1)
			test_results |= RTC_TEST_DIGI_1HZ_IRQ;
	}

	rtc_test_display_test_results(test_ops, test_results);

	close(rtc_fd);
	printf("\nTest finished\n");

	return 0;
}

/*
 * Function:    rtc_test_display_test_results
 * Description: display test's results
 */
static void rtc_test_display_test_results(unsigned int test_ops,
		unsigned int test_results)
{
	if (test_ops) {
		printf("\nTest results:\n");
		printf("-------------------------------------------------"
			"----------\n");
	}

	if (test_ops & RTC_TEST_RD_TIME)
		printf("ioctl cmd RTC_RD_TIME:         %s\n",
		       test_results & RTC_TEST_RD_TIME ? "OK" :
				"Failure or not supported");
	if (test_ops & RTC_TEST_SET_TIME)
		printf("ioctl cmd RTC_SET_TIME:        %s\n",
		       test_results & RTC_TEST_SET_TIME ? "OK" :
				"Failure or not supported");
	if (test_ops & RTC_TEST_RD_ALARM)
		printf("ioctl cmd RTC_ALM_READ:        %s\n",
		       test_results & RTC_TEST_RD_ALARM ? "OK" :
				"Failure or not supported");
	if (test_ops & RTC_TEST_SET_ALARM)
		printf("ioctl cmd RTC_ALM_SET:         %s\n",
		       test_results & RTC_TEST_SET_ALARM ? "OK" :
				"Failure or not supported");
	if (test_ops & RTC_TEST_ALARM_IRQ)
		printf("Alarm interrupt test:          %s\n",
		       test_results & RTC_TEST_ALARM_IRQ ? "OK" :
				"Failure or not supported");
	if (test_ops & RTC_TEST_STD_PERIODIC_IRQ)
		printf("Std periodic interrupt test:   %s\n",
		       test_results & RTC_TEST_STD_PERIODIC_IRQ ? "OK" :
				"Failure or not supported");
	if (test_ops & RTC_TEST_STD_1HZ_IRQ)
		printf("Std 1 Hz interrupt test:        %s\n",
		       test_results & RTC_TEST_STD_1HZ_IRQ ? "OK" :
				"Failure or not supported");
	if (test_ops & RTC_TEST_DIGI_PERIODIC_IRQ)
		printf("Digi periodic interrupt test:  %s\n",
		       test_results & RTC_TEST_DIGI_PERIODIC_IRQ ? "OK" :
				"Failure or not supported");
	if (test_ops & RTC_TEST_DIGI_1HZ_IRQ)
		printf("Digi 1 Hz interrupt test:       %s\n",
		       test_results & RTC_TEST_DIGI_1HZ_IRQ ? "OK" :
				"Failure or not supported");
}

/*
 * Function:    rtc_test_time_read
 * Description: read time from the rtc
 */
static int rtc_test_time_read(int fd, struct rtc_time *tm)
{
	int retval;

	retval = ioctl(fd, RTC_RD_TIME, tm);
	if (retval >= 0) {
		printf("Current RTC date/time is %d-%d-%d, %02d:%02d:%02d.\n",
		       tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900,
		       tm->tm_hour, tm->tm_min, tm->tm_sec);
		return 1;
	}

	return 0;
}

/*
 * Function:    rtc_test_time_set
 * Description: set time into the rtc
 */
static int rtc_test_time_set(int fd, struct rtc_time *tm)
{
	int retval;

	tm->tm_sec += 2;
	if (tm->tm_sec >= 60) {
		tm->tm_sec %= 60;
		tm->tm_min++;
	}
	if (tm->tm_min == 60) {
		tm->tm_min = 0;
		tm->tm_hour++;
	}
	if (tm->tm_hour == 24)
		tm->tm_hour = 0;

	retval = ioctl(fd, RTC_SET_TIME, tm);
	if (retval >= 0) {
		printf("New RTC date/time is %d-%d-%d, %02d:%02d:%02d.\n",
		       tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900,
		       tm->tm_hour, tm->tm_min, tm->tm_sec);
		return 1;
	}

	return 0;
}

/*
 * Function:    rtc_test_alarm_read
 * Description: read the programmed time into the rtc alarm
 */
static int rtc_test_alarm_read(int fd)
{
	int retval;
	struct rtc_time alarm_tm;

	retval = ioctl(fd, RTC_ALM_READ, &alarm_tm);
	if (retval >= 0) {
		printf("Alarm was programmed to %02d:%02d:%02d.\n",
				alarm_tm.tm_hour, alarm_tm.tm_min,
					alarm_tm.tm_sec);
		return 1;
	}

	return 0;
}

/*
 * Function:    rtc_test_alarm_set
 * Description: set time into rtc's alarm
 */
static int rtc_test_alarm_set(int fd, struct rtc_time *tm)
{
	int retval;
	struct rtc_time alarm_tm;
	struct rtc_time tm_now;

	if(minutes_res){
		tm->tm_sec = 0;
		tm->tm_min += seconds/60;
	}
	else
		tm->tm_sec += seconds;
	if (tm->tm_sec >= 60) {
		tm->tm_sec %= 60;
		tm->tm_min++;
	}
	if (tm->tm_min == 60) {
		tm->tm_min = 0;
		tm->tm_hour++;
	}
	if (tm->tm_hour == 24)
		tm->tm_hour = 0;

	retval = ioctl(fd, RTC_ALM_SET, tm);
	if (retval >= 0) {
		/* Enable alarm interrupts */
		retval = ioctl(fd, RTC_AIE_ON, tm);
		if (retval >= 0 ) {
			retval = ioctl(fd, RTC_ALM_READ, &alarm_tm);
			if (retval >= 0) {
				printf("Alarm re-programmed to "
					"%02d:%02d:%02d.\n",
					alarm_tm.tm_hour,
					alarm_tm.tm_min,
					alarm_tm.tm_sec);
			}
			return 1;
		}
	}

	return 0;
}

/*
 * Function:    rtc_test_alarm_irq
 * Description: check the irq alarm
 */
static int rtc_test_alarm_irq(int fd, struct rtc_wkalrm *wkalrm)
{
	int retval, result = 0;
	unsigned long data;
	struct timeval tv = {0, 0};
	fd_set readfds;
	struct rtc_time tm_now;

	if (!minutes_res){
		/* seconds+2 second timeout on select  */
		tv.tv_sec += seconds +2;
	}
	else {
		retval = ioctl(fd, RTC_RD_TIME, &tm_now);
		/* Round up to the next minute and add 2 seconds
			to the timeout on select  */
		tv.tv_sec += seconds - tm_now.tm_sec + 2;
	}

	/* Enable the alarm irq */
	retval = ioctl(fd, RTC_WKALM_SET, wkalrm);
	if (retval >= 0) {
		printf("Waiting %d seconds for alarm... ",tv.tv_sec);
		fflush(stdout);

		FD_ZERO(&readfds);
		FD_SET(fd, &readfds);

		/* The select will wait until an RTC interrupt happens. */
		retval = select(fd + 1, &readfds, NULL, NULL, &tv);
		if (retval < 0) {
			perror("select");
			exit(EXIT_FAILURE);
		} else if (retval == 0) {
			/* Timeout */
			printf(" Timeout!\n");
		} else {
			retval = read(fd, &data, sizeof(unsigned long));
			if (retval < 0) {
				perror("read");
				exit(EXIT_FAILURE);
			}
			printf(" RING, RING, RING\n");
			result = 1;
		}
		/* Disable the alarm */
		wkalrm->enabled = 0;
		retval = ioctl(fd, RTC_WKALM_SET, wkalrm);
		if (retval < 0)
			result = 0;
	}

	return result;
}

/*
 * Function:    rtc_test_periodic_irq
 * Description: check periodic irq
 */
static int rtc_test_periodic_irq(int fd, int digi)
{
	int retval, i, irqcount = 0;
	unsigned long tmp, data;
	struct timeval start, end, diff;
	int result = 1;

	/* Read periodic IRQ rate */
	retval = ioctl(fd, digi ? RTC_MCA_IRQP_READ : RTC_IRQP_READ, &tmp);
	if (retval == -1) {
		/* not all RTCs support periodic IRQs */
		if (errno == EINVAL) {
			fprintf(stderr, "\nNo periodic IRQ support\n");
		}
		perror("IRQP_READ ioctl");
		return 0;
	}
	fprintf(stderr, "\nPeriodic IRQ rate is %ld Hz.\n", tmp);

	fprintf(stderr, "Counting 20 interrupts at:");
	fflush(stderr);

	/* The frequencies 128 Hz, 256 Hz, ... 8192Hz are only allowed for root. */
	for (tmp = 2; tmp <= 64; tmp *= 2) {
		retval = ioctl(fd, digi ? RTC_MCA_IRQP_SET : RTC_IRQP_SET, tmp);
		if (retval == -1) {
			/* not all RTCs can change their periodic IRQ rate */
			if (errno == EINVAL) {
				fprintf(stderr,
					"\n...Periodic IRQ rate is fixed\n");
			}
			perror("IRQP_SET ioctl");
			return 0;
		}

		fprintf(stderr, "\n%ld Hz:\t", tmp);
		fflush(stderr);

		/* Enable periodic interrupts */
		retval = ioctl(fd, digi ? RTC_MCA_PIE_ON : RTC_PIE_ON, 0);
		if (retval == -1) {
			perror("PIE_ON ioctl");
			return 0;
		}

		for (i = 1; i < 21; i++) {
			gettimeofday(&start, NULL);
			/* This blocks */
			retval = read(fd, &data, sizeof(unsigned long));
			if (retval == -1) {
				perror("read");
				exit(errno);
			}
			gettimeofday(&end, NULL);
			timersub(&end, &start, &diff);
			if (diff.tv_sec > 0 ||
			    diff.tv_usec > ((1000000L / tmp) * 1.10)) {
				fprintf(stderr, "\nPIE delta error: %ld.%06ld should be close to 0.%06ld\n",
				       diff.tv_sec, diff.tv_usec,
				       (1000000L / tmp));
				fflush(stdout);
				result = 0;
			}

			fprintf(stderr, " %d",i);
			fflush(stderr);
			irqcount++;
		}

		/* Disable periodic interrupts */
		retval = ioctl(fd, digi ? RTC_MCA_PIE_OFF : RTC_PIE_OFF, 0);
		if (retval == -1) {
			perror("PIE_OFF ioctl");
			return 0;
		}
	}
	fprintf(stderr, "\n\n");
	fflush(stderr);

	return result;
}

/*
 * Function:    rtc_test_1hz_irq
 * Description: check 1 Hz irq
 */
static int rtc_test_1hz_irq(int fd, int digi)
{
	int retval, i, irqcount = 0;
	unsigned long tmp, data;
	struct timeval start, end, diff;

	/* Turn on update interrupts (one per second) */
	retval = ioctl(fd, digi ? RTC_MCA_UIE_ON : RTC_UIE_ON, 0);
	if (retval == -1) {
		if (errno == EINVAL) {
			fprintf(stderr,
				"\n...Update IRQs not supported.\n");
		}
		perror("UIE_ON ioctl");
		return 0;
	}

	fprintf(stderr, "Counting 5 update (1/sec) interrupts from reading rtc");
	fflush(stderr);
	for (i = 1; i < 6; i++) {
		/* This read will block */
		retval = read(fd, &data, sizeof(unsigned long));
		if (retval == -1) {
			perror("read");
			exit(errno);
		}
		fprintf(stderr, " %d",i);
		fflush(stderr);
		irqcount++;
	}

	fprintf(stderr, "\nCounting 5 update (1/sec) interrupts from using select(2) on /dev/rtc:");
	fflush(stderr);
	for (i = 1; i < 6; i++) {
		struct timeval tv = {5, 0};     /* 5 second timeout on select */
		fd_set readfds;

		FD_ZERO(&readfds);
		FD_SET(fd, &readfds);
		/* The select will wait until an RTC interrupt happens. */
		retval = select(fd + 1, &readfds, NULL, NULL, &tv);
		if (retval == -1) {
		        perror("select");
		        exit(errno);
		}
		/* This read won't block unlike the select-less case above. */
		retval = read(fd, &data, sizeof(unsigned long));
		if (retval == -1) {
		        perror("read");
		        exit(errno);
		}
		fprintf(stderr, " %d",i);
		fflush(stderr);
		irqcount++;
	}

	/* Turn off update interrupts */
	retval = ioctl(fd, digi ? RTC_MCA_UIE_OFF : RTC_UIE_OFF, 0);
	if (retval == -1) {
		perror("UIE_OFF ioctl");
		return 0;
	}
	fprintf(stderr, "\n");
	fflush(stderr);

	return 1;
}
