/*	$OpenBSD: pthread_cond_timedwait.c,v 1.1.1.1 2001/08/15 14:37:12 fgsch Exp $	*/
/*
 * Copyright (c) 1993, 1994, 1995, 1996 by Chris Provenzano and contributors, 
 * proven@mit.edu All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Chris Provenzano,
 *	the University of California, Berkeley, and contributors.
 * 4. Neither the name of Chris Provenzano, the University, nor the names of
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY CHRIS PROVENZANO AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL CHRIS PROVENZANO, THE REGENTS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */ 

/* ==== test_pthread_cond.c =========================================
 * Copyright (c) 1993 by Chris Provenzano, proven@athena.mit.edu
 *
 * Description : Test pthread_cond(). Run this after test_create()
 *
 *  1.23 94/05/04 proven
 *      -Started coding this file.
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "test.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void* thread_1(void * new_buf)
{
	CHECKr(pthread_mutex_lock(&mutex));
	CHECKr(pthread_cond_signal(&cond));
	CHECKr(pthread_mutex_unlock(&mutex));
	pthread_exit(NULL);
}

void* thread_2(void * new_buf)
{
	sleep(1);
	CHECKr(pthread_mutex_lock(&mutex));
	CHECKr(pthread_cond_signal(&cond));
	CHECKr(pthread_mutex_unlock(&mutex));
	pthread_exit(NULL);
}

int
main()
{
	struct timespec abstime = { 0, 0 };
	struct timeval curtime;
	pthread_t thread;
	int ret;

	printf("pthread_cond_timedwait START\n");

	CHECKr(pthread_mutex_lock(&mutex));
	CHECKe(gettimeofday(&curtime, NULL));
	abstime.tv_sec = curtime.tv_sec + 5; 

	/* Test a condition timeout */
	switch((ret = pthread_cond_timedwait(&cond, &mutex, &abstime))) {
	case 0:
		PANIC("pthread_cond_timedwait #0 failed to timeout");
		/* NOTREACHED */
	case ETIMEDOUT:
		/* expected behaviour */
		printf("Got first timeout ok\n");	/* Added by monty */
		break;
	default:
		DIE(ret, "pthread_cond_timedwait");
		/* NOTREACHED */
	}

	/* Test a normal condition signal */
	CHECKr(pthread_create(&thread, NULL, thread_1, NULL));

	abstime.tv_sec = curtime.tv_sec + 10; 
	CHECKr(pthread_cond_timedwait(&cond, &mutex, &abstime));

	/* Test a normal condition signal after a sleep */
	CHECKr(pthread_create(&thread, NULL, thread_2, NULL));

	pthread_yield();

	abstime.tv_sec = curtime.tv_sec + 10; 
	CHECKr(pthread_cond_timedwait(&cond, &mutex, &abstime));

	SUCCEED;
}
