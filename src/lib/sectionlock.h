/*
  sectionlock.h  -  mutex state handle

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

#ifndef LUMIERA_SECTIONLOCK_H
#define LUMIERA_SECTIONLOCK_H

#include "lib/error.h"

#include <pthread.h>
#include <nobug.h>

LUMIERA_ERROR_DECLARE (LOCK_ACQUIRE);
LUMIERA_ERROR_DECLARE (LOCK_RELEASE);
LUMIERA_ERROR_DECLARE (LOCK_DESTROY);


typedef int (*lumiera_sectionlock_unlock_fn)(void*);

/**
 * sectionlock used to manage the state of mutexes.
 */
struct lumiera_sectionlock_struct
{
  void* lock;
  lumiera_sectionlock_unlock_fn unlock;
  NOBUG_IF_ALPHA(struct nobug_flag* flag);
  RESOURCE_HANDLE (rh);
};
typedef struct lumiera_sectionlock_struct lumiera_sectionlock;
typedef struct lumiera_sectionlock_struct* LumieraSectionlock;

/* helper function for nobug */
static inline void
lumiera_sectionlock_ensureunlocked (LumieraSectionlock self)
{
  ENSURE (!self->lock, "forgot to unlock");
}



/**
 * Unlock the lock hold in a SECTION
 * @internal
 * @param sectionname name used for the sectionlock instance
 * @param ... some extra code to execute
 */
#define LUMIERA_SECTION_UNLOCK_(section)                        \
  do if ((section)->lock)                                       \
    {                                                           \
      NOBUG_RESOURCE_LEAVE_RAW((section)->flag, (section)->rh)  \
        {                                                       \
          if ((section)->unlock((section)->lock))               \
            LUMIERA_DIE (LOCK_RELEASE);                         \
          (section)->lock = NULL;                               \
        }                                                       \
    } while (0)


#endif
/*
// Local Variables:
// mode: C
// c-file-style: "gnu"
// indent-tabs-mode: nil
// End:
*/
