/*
  references.h  -  strong and weak references

  Copyright (C)         CinelerraCV
    2007,               Christian Thaeter <ct@pipapo.org>

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

#ifndef CINELERRA_REFERENCES_H
#define CINELERRA_REFERENCES_H

#include <nobug.h>

typedef struct cinelerra_reference_struct cinelerra_reference;
typedef cinelerra_reference* CinelerraReference;

#include "lib/error.h"
#include "lib/mutex.h"

/**
 * @file Strong and Weak references
 * Strong references keep some object alive while they existing
 * Weak references become invalidated when the referenced object gets destroyed
 *
 * Thread safety: references protect their internal operations with a mutex (optimization using futex may come in future)
 * while operations on the reference itself (initialization, destruction, strengthen, weaken) and on the referenced object (get)
 * should be protected from elsewhere.
 */

/**
 *
 */

struct cinelerra_reftarget_struct
{
  void* object;
  void (*dtor)(void*);
  unsigned strong_cnt;          /*when strong becomes 0 obj is destroyed, if weak is 0 destroy target too*/
  unsigned weak_cnt;            /*when weak becomes 0 and !obj  and lock strong is 0, destroy target */
  cinelerra_mutex lock;
};
typedef struct cinelerra_reftarget_struct cinelerra_reftarget;
typedef cinelerra_reftarget* CinelerraReftarget;


/**
 *
 */
struct cinelerra_reference_struct
{
  void* object;    /*set for strong, NULL for weak*/
  CinelerraReftarget target;
};

#define cinelerra_reference \
cinelerra_reference NOBUG_CLEANUP(cinelerra_reference_ensuredestroyed)

/* helper function for nobug */
static inline void
cinelerra_reference_ensuredestroyed (CinelerraReference self)
{
  ENSURE (!self->target, "forgot to destroy reference");
}


/**
 * Construct an initial strong reference from an object.
 * For every object which should be managed with references you need to construct an initial strong reference
 * which then will be used to initialize all further references.
 * @param self pointer to the reference to be initialized
 * @param obj pointer to the object being referenced
 * @param dtor destructor function which will be called on obj when the last strong reference gets deleted
 * @return self as given
 */
CinelerraReference
cinelerra_reference_strong_init_once (CinelerraReference self, void* obj, void (*dtor)(void*))
{
  CinelerraReftarget target = malloc (sizeof(cinelerra_reftarget));
  if (!target) CINELERRA_DIE;

  target->object = obj;
  target->dtor = dtor;
  target->strong_cnt = 1;
  target->weak_cnt = 0;
  cinelerra_mutex_init (&target->lock);

  self->object = obj;
  self->target = target;
  return self;
}

/**
 * Destroy a reference.
 * All references need to be destroyed when not used any more.
 * When the last strong reference gets destroyed, the object's destructor is called.
 * Remaining weak references stay invalidated then until they get destroyed too.
 * @param self reference to be destroyed
 * @return self as given
 * destroying a reference is not thread safe as far as 2 threads try to concurrently destroy it!
 */
CinelerraReference
cinelerra_reference_destroy (CinelerraReference self)
{
  CinelerraReftarget target = self->target;

  /* defensive, lets detect errors if anything still tries to use this reference */
  self->target = NULL;

  cinelerra_mutexacquirer lock;
  cinelerra_mutexacquirer_init_mutex (&lock, &target->lock, CINELERRA_LOCKED);

  if (self->object)
    {
      /* strong reference */
      if (!--target->strong_cnt)
        {
          /* was last strong reference */
          if (target->dtor)
            target->dtor (target->object);
          target->object = NULL;
          if (!target->weak_cnt)
            {
              /* no weak refs either, destroy it */
              cinelerra_mutexacquirer_unlock (&lock);
              cinelerra_mutex_destroy (&target->lock);
              free (&target);
              return self;
            }
        }
    }
  else
    {
      /* weak reference */
      if (!--target->weak_cnt && !target->strong_cnt)
        {
          /* was last weak reference, and no strong refs left */
          cinelerra_mutexacquirer_unlock (&lock);
          cinelerra_mutex_destroy (&target->lock);
          free (&target);
          return self;
        }
    }
  cinelerra_mutexacquirer_unlock (&lock);
  return self;
}

/**
 * Get object from a strong reference.
 * @return pointer to object, NULL if applied to a weak reference
 */
static inline void*
cinelerra_reference_get (CinelerraReference self)
{
  ENSURE (self->target, "illegal reference (not initialized or already destroyed?)");
  return self->object;
}

/**
 * Copy construct a reference as strong reference
 * @param source reference to copy
 * @return self as strong reference (always for strong references) or NULL if source is an invalidated weak reference,
 * in the later case the reference is constructed as weak reference barely to allow it be destroyed
 */
static inline CinelerraReference
cinelerra_reference_strong_init (CinelerraReference self, CinelerraReference source)
{
  cinelerra_mutexacquirer lock;
  cinelerra_mutexacquirer_init_mutex (&lock, &source->target->lock, CINELERRA_LOCKED);

  self->object = source->target->object;
  self->target = source->target;

  if (self->object)
    {
      ++self->target->strong_cnt;
    }
  else
    {
      ++self->target->weak_cnt;
      self = NULL;
    }
  cinelerra_mutexacquirer_unlock (&lock);
  return self;
}

/**
 * Copy construct a reference as weak reference
 * @param source reference to copy
 * @return self (always for strong references) or NULL if self is an invalidated weak reference
 */
static inline CinelerraReference
cinelerra_reference_weak_init (CinelerraReference self, CinelerraReference source)
{
  cinelerra_mutexacquirer lock;
  cinelerra_mutexacquirer_init_mutex (&lock, &source->target->lock, CINELERRA_LOCKED);

  self->object = NULL;
  self->target = source->target;

  ++self->target->weak_cnt;
  if (!self->target->object)
    /* already invalidated */
    self = NULL;

  cinelerra_mutexacquirer_unlock (&lock);
  return self;
}

/**
 * turn a (strong) reference into a weak reference
 * Weaken a reference may remove its last strong reference and thus destroy the object
 * do nothing if the referene is already weak
 * @return self or NULL if the final strong reference got removed,
 */
static inline CinelerraReference
cinelerra_reference_weaken (restrict CinelerraReference self)
{
  /* is this a strong reference? */
  if (self->object)
    {
      cinelerra_mutexacquirer lock;
      cinelerra_mutexacquirer_init_mutex (&lock, &self->target->lock, CINELERRA_LOCKED);

      self->object = NULL;
      ++self->target->weak_cnt;
      if (!--self->target->strong_cnt)
        {
          if (self->target->dtor)
            self->target->dtor (self->target->object);
          self->target->object = NULL;
          self = NULL;
        }
      cinelerra_mutexacquirer_unlock (&lock);
    }
  return self;
}


/**
 * turn a (weak) reference into a strong reference
 * only references of object which are not already destroyed can be strengthened
 * @return self when successful, NULL when the object was already destroyed, 'self' stays a dead weak reference in that case
 */
static inline CinelerraReference
cinelerra_reference_strengthen (CinelerraReference self)
{
  /* is this a weak reference? */
  if (!self->object)
    {
      cinelerra_mutexacquirer lock;
      cinelerra_mutexacquirer_init_mutex (&lock, &self->target->lock, CINELERRA_LOCKED);

      if (self->target->object)
        {
          self->object = self->target->object;
          --self->target->weak_cnt;
          ++self->target->strong_cnt;
        }
      else
        self = NULL;
      cinelerra_mutexacquirer_unlock (&lock);
    }
  return self;
}


#endif
