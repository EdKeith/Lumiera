/*
  test-threads.c  -  test thread management

  Copyright (C)         Lumiera.org
    2008,               Christian Thaeter <ct@pipapo.org>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

//#include <stdio.h>
//#include <string.h>

#include "common/config.h"

#include "include/logging.h"
#include "lib/mutex.h"
#include "backend/threads.h"
#include "tests/test.h"

#include <unistd.h>
#include <errno.h>
#include <time.h>

void threadfn(void* blah)
{
  (void) blah;
  struct timespec wait = {0,300000000};

  fprintf (stderr, "thread running %s\n", NOBUG_THREAD_ID_GET);
  nanosleep (&wait, NULL);
  fprintf (stderr, "thread done %s\n", NOBUG_THREAD_ID_GET);
}


void threadsyncfn(void* blah)
{
  struct timespec wait = {0,200000000};
  LumieraReccondition sync = (LumieraReccondition) blah;

  ECHO ("thread starting up %s", NOBUG_THREAD_ID_GET);
  LUMIERA_RECCONDITION_SECTION(cond_sync, sync)
    {
      ECHO ("send startup signal %s", NOBUG_THREAD_ID_GET);
      LUMIERA_RECCONDITION_SIGNAL;
      ECHO ("wait for trigger %s", NOBUG_THREAD_ID_GET);
      LUMIERA_RECCONDITION_WAIT(1);
    }

  ECHO ("thread running %s", NOBUG_THREAD_ID_GET);
  nanosleep (&wait, NULL);
  ECHO ("thread done %s", NOBUG_THREAD_ID_GET);
}



lumiera_mutex testmutex;

void mutexfn(void* blah)
{
  (void) blah;
  struct timespec wait = {0,300000000};

  LUMIERA_MUTEX_SECTION (NOBUG_ON, &testmutex)
    {
      fprintf (stderr, "mutex thread running %s\n", NOBUG_THREAD_ID_GET);
      nanosleep (&wait, NULL);
      fprintf (stderr, "thread done %s\n", NOBUG_THREAD_ID_GET);
    }
}



TESTS_BEGIN

TEST ("simple_thread")
{
  fprintf (stderr, "main before thread %s\n", NOBUG_THREAD_ID_GET);

  lumiera_thread_run (LUMIERA_THREAD_WORKER,
                      threadfn,
                      NULL,
                      NULL,
                      argv[1],
                      NULL);

  struct timespec wait = {0,600000000};
  nanosleep (&wait, NULL);
  fprintf (stderr, "main after thread %s\n", NOBUG_THREAD_ID_GET);
}

TEST ("thread_synced")
{
  lumiera_reccondition cnd;
  lumiera_reccondition_init (&cnd, "threadsync", &NOBUG_FLAG(NOBUG_ON));

  LUMIERA_RECCONDITION_SECTION(cond_sync, &cnd)
    {
      ECHO ("main before thread %s", NOBUG_THREAD_ID_GET);

      lumiera_thread_run (LUMIERA_THREAD_WORKER,
                          threadsyncfn,
                          &cnd,
                          &cnd,
                          argv[1],
                          NULL);

      ECHO ("main wait for thread being ready %s", NOBUG_THREAD_ID_GET);
      LUMIERA_RECCONDITION_WAIT(1);

      ECHO ("main trigger thread %s", NOBUG_THREAD_ID_GET);
      LUMIERA_RECCONDITION_SIGNAL;

      ECHO ("wait for thread end %s", NOBUG_THREAD_ID_GET);
      LUMIERA_RECCONDITION_WAIT(1);
      ECHO ("thread ended %s", NOBUG_THREAD_ID_GET);
    }

  lumiera_reccondition_destroy (&cnd, &NOBUG_FLAG(NOBUG_ON));
}



TEST ("mutex_thread")
{
  lumiera_mutex_init (&testmutex, "test", &NOBUG_FLAG(NOBUG_ON));

  LUMIERA_MUTEX_SECTION (NOBUG_ON, &testmutex)
    {
      fprintf (stderr, "main before thread %s\n", NOBUG_THREAD_ID_GET);

      lumiera_thread_run (LUMIERA_THREAD_WORKER,
                          mutexfn,
                          NULL,
                          NULL,
                          argv[1],
                          NULL);

      struct timespec wait = {0,600000000};
      nanosleep (&wait, NULL);
      fprintf (stderr, "main after thread %s\n", NOBUG_THREAD_ID_GET);
    }

  lumiera_mutex_destroy (&testmutex, &NOBUG_FLAG(NOBUG_ON));
}


TESTS_END