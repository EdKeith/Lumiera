/*
  error.c  -  Lumiera Error handling

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

#include <pthread.h>

#include "lib/error.h"

/**
 * @file
 * C Error handling in Lumiera.
 */


/*
  predefined errors
*/
LUMIERA_ERROR_DEFINE (ERRNO, "errno");
LUMIERA_ERROR_DEFINE (EERROR, "could not initialize error system");
LUMIERA_ERROR_DEFINE (UNKNOWN, "unknown error");


/* Thread local storage */
static pthread_key_t lumiera_error_tls;
static pthread_once_t lumiera_error_initialized = PTHREAD_ONCE_INIT;

/**
 * Holding error and some context data.
 */
struct lumiera_errorcontext_struct
{
  lumiera_err err;
  char* extra;
};

typedef struct lumiera_errorcontext_struct lumiera_errorcontext;
typedef lumiera_errorcontext* LumieraErrorcontext;


static void
lumiera_error_tls_delete (void* err)
{
  if (err)
    free (((LumieraErrorcontext)err)->extra);
}

static void
lumiera_error_tls_init (void)
{
  if (!!pthread_key_create (&lumiera_error_tls, lumiera_error_tls_delete))
    LUMIERA_DIE (EERROR);
}


LumieraErrorcontext
lumiera_error_get (void)
{
  if (lumiera_error_initialized == PTHREAD_ONCE_INIT)
    pthread_once (&lumiera_error_initialized, lumiera_error_tls_init);

  LumieraErrorcontext self = pthread_getspecific (lumiera_error_tls);
  if (!self)
    {
      /* malloc() and not lumiera_malloc() here because nothing else might be initialized when calling this */
      self = malloc (sizeof *self);
      if (!self)
        LUMIERA_DIE (EERROR);

      self->err = NULL;
      self->extra = NULL;
      pthread_setspecific (lumiera_error_tls, self);
    }

  return self;
}


lumiera_err
lumiera_error_set (lumiera_err nerr, const char* extra)
{
  LumieraErrorcontext self = lumiera_error_get ();

  if (!self->err)
    {
      self->err = nerr;
      free (self->extra);
      if (extra)
        self->extra = strdup (extra);
      else
        self->extra = NULL;
    }

  return self->err;
}


lumiera_err
lumiera_error (void)
{
  LumieraErrorcontext self = lumiera_error_get ();
  lumiera_err err = self->err;

  if (err)
    self->err = NULL;
  return err;
}

const char*
lumiera_error_extra (void)
{
  return lumiera_error_get ()->extra;
}


lumiera_err
lumiera_error_peek (void)
{
  return lumiera_error_get ()->err;
}
