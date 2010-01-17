/*
  test-threadpool.c  -  test thread pool creation and usage

  Copyright (C)         Lumiera.org
    2009,               Michael Ploujnikov <ploujj@gmail.com>

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

#include "tests/test.h"

#include "backend/threadpool.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

void is_prime(void * arg)
{
  int number = *(int *)arg;
  int prime = 1;

  for (int x = number; x >= sqrt(number); --x)
    {
      if (number % x == 0)
	{
	  prime = 0;
	  break;
	}
    }
  *(int *)arg = prime;
}

TESTS_BEGIN

TEST ("threadpool-basic")
{
  lumiera_threadpool_init();
  lumiera_threadpool_destroy();
}

TEST ("threadpool1")
{
  ECHO("start by initializing the threadpool");
  lumiera_threadpool_init();
  LumieraThread t1 =
    lumiera_threadpool_acquire_thread(LUMIERA_THREADCLASS_INTERACTIVE,
				      "test purpose",
				      &NOBUG_FLAG(NOBUG_ON));
  //  lumiera_threadpool_release_thread(t1);
  ECHO("acquired thread 1 %p",t1);
  lumiera_threadpool_destroy();
}


TEST ("basic-acquire-release")
{
  ECHO("start by initializing the threadpool");
  lumiera_threadpool_init();
  ECHO("acquiring thread 1");
  LumieraThread t1 =
    lumiera_threadpool_acquire_thread(LUMIERA_THREADCLASS_INTERACTIVE,
				      "test purpose",
				      &NOBUG_FLAG(NOBUG_ON));
  ECHO("acquiring thread 2");
  LumieraThread t2 =
    lumiera_threadpool_acquire_thread(LUMIERA_THREADCLASS_IDLE,
				      "test purpose",
				      &NOBUG_FLAG(NOBUG_ON));

  ECHO("thread 1 kind=%s", lumiera_threadclass_names[t1->kind]);
  CHECK(LUMIERA_THREADCLASS_INTERACTIVE == t1->kind);
  ECHO("thread 1 state=%s", lumiera_threadstate_names[t1->state]);
  CHECK(LUMIERA_THREADSTATE_IDLE == t1->state);
  ECHO("thread 2 kind=%s", lumiera_threadclass_names[t2->kind]);
  CHECK(LUMIERA_THREADCLASS_IDLE == t2->kind);
  ECHO("thread 2 state=%s", lumiera_threadstate_names[t2->state]);
  CHECK(LUMIERA_THREADSTATE_IDLE == t2->state);

  ECHO("releasing thread 1");
  //lumiera_threadpool_release_thread(t1);
  ECHO("thread 1 has been released");

  ECHO("releasing thread 2");
  //lumiera_threadpool_release_thread(t2);
  ECHO("thread 2 has been released");

  lumiera_threadpool_destroy();
}

#if 0
TEST ("many-acquire-release")
{

  const int threads_per_pool_count = 10;

  lumiera_threadpool_init(10);
  LumieraThread threads[threads_per_pool_count*LUMIERA_THREADCLASS_COUNT];
  
  for (int kind = 0; kind < LUMIERA_THREADCLASS_COUNT; ++kind)
    {
      for (int i = 0; i < threads_per_pool_count; ++i)
	{
	  threads[i+kind*threads_per_pool_count] =
	    lumiera_threadpool_acquire_thread(kind,
					      "test purpose",
					      &NOBUG_FLAG(NOBUG_ON));
	}
    }

  for (int i = 0; i < threads_per_pool_count*LUMIERA_THREADCLASS_COUNT; ++i)
    {
      lumiera_threadpool_release_thread(threads[i]);
    }

  lumiera_threadpool_destroy();

}

TEST ("toomany-acquire-release")
{

  const int threads_per_pool_count = 11;

  lumiera_threadpool_init(10);
  LumieraThread threads[threads_per_pool_count*LUMIERA_THREADCLASS_COUNT];
  
  for (int kind = 0; kind < LUMIERA_THREADCLASS_COUNT; ++kind)
    {
      for (int i = 0; i < threads_per_pool_count; ++i)
	{
	  threads[i+kind*threads_per_pool_count] =
	    lumiera_threadpool_acquire_thread(kind,
					      "test purpose",
					      &NOBUG_FLAG(NOBUG_ON));
	}
    }

  for (int i = 0; i < threads_per_pool_count*LUMIERA_THREADCLASS_COUNT; ++i)
    {
      lumiera_threadpool_release_thread(threads[i]);
    }

  lumiera_threadpool_destroy();

}
#endif

TEST ("no-function")
{
  LumieraThread t;

  lumiera_threadpool_init();

  t = lumiera_thread_run (LUMIERA_THREADCLASS_INTERACTIVE,
			  NULL,
			  NULL,
			  "process my test function",
			  &NOBUG_FLAG(NOBUG_ON));

  // cleanup
  ECHO("wait 1 sec");
  usleep(1000000);
  ECHO("finished waiting");
  lumiera_threadpool_destroy();
}

TEST ("process-function")
{
  // this is what the scheduler would do once it figures out what function a job needs to run
  LumieraThread t;
  int number = 440616;

  lumiera_threadpool_init();

  ECHO ("the input to the function is %d", number);

  t = lumiera_thread_run (LUMIERA_THREADCLASS_INTERACTIVE,
			  &is_prime,
			  (void *)&number, //void * arg,
			  "process my test function",
			  &NOBUG_FLAG(NOBUG_ON)); // struct nobug_flag* flag)

  // cleanup
  ECHO("wait 1 sec");
  usleep(1000000);
  ECHO("finished waiting");
  lumiera_threadpool_destroy();
}

TESTS_END
