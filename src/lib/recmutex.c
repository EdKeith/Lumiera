/*
  recmutex.c  -  recursive mutex

  Copyright (C)         Lumiera.org
    2008, 2009,         Christian Thaeter <ct@pipapo.org>

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

#include "lib/recmutex.h"

/**
 * @file
 * Recursive Mutexes.
 */


static pthread_once_t recmutexattr_once = PTHREAD_ONCE_INIT;
static pthread_mutexattr_t recmutexattr;

static void recmutexattr_init()
{
  pthread_mutexattr_init (&recmutexattr);
  pthread_mutexattr_settype (&recmutexattr, PTHREAD_MUTEX_RECURSIVE);
}


LumieraRecmutex
lumiera_recmutex_init (LumieraRecmutex self,
                       const char* purpose,
                       struct nobug_flag* flag,
                       const struct nobug_context ctx)
{
  if (self)
    {
      pthread_once (&recmutexattr_once, recmutexattr_init);

      NOBUG_RESOURCE_HANDLE_INIT (self->rh);
      NOBUG_RESOURCE_ANNOUNCE_RAW_CTX (flag, "recmutex", purpose, self, self->rh, ctx)
        {
          pthread_mutex_init (&self->recmutex, &recmutexattr);
        }
    }

  return self;
}


LumieraRecmutex
lumiera_recmutex_destroy (LumieraRecmutex self,
                          struct nobug_flag* flag,
                          const struct nobug_context ctx)
{
  if (self)
    {
      NOBUG_RESOURCE_FORGET_RAW_CTX (flag,  self->rh, ctx)
        {
          if (pthread_mutex_destroy (&self->recmutex))
            LUMIERA_DIE (LOCK_DESTROY);
        }
    }
  return self;
}

/*
// Local Variables:
// mode: C
// c-file-style: "gnu"
// indent-tabs-mode: nil
// End:
*/
