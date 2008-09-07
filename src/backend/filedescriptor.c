/*
  filedescriptor.c  -  file handling

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

#include "lib/mutex.h"
#include "lib/safeclib.h"

#include "backend/file.h"
#include "backend/filedescriptor.h"
#include "backend/filehandle.h"
#include "backend/filehandlecache.h"

#include <sys/types.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>


NOBUG_DEFINE_FLAG_PARENT (filedescriptor, file_all);

/*
  Filedescriptor registry

  This registry stores all acquired filedescriptors for lookup, they will be freed when not referenced anymore.
 */
static PSplay registry = NULL;
static lumiera_mutex registry_mutex = {PTHREAD_MUTEX_INITIALIZER};


static int
cmp_fn (const void* keya, const void* keyb)
{
  const LumieraFiledescriptor a = (const LumieraFiledescriptor)keya;
  const LumieraFiledescriptor b = (const LumieraFiledescriptor)keyb;

  if (a->stat.st_dev < b->stat.st_dev)
    return -1;
  else if (a->stat.st_dev > b->stat.st_dev)
    return 1;

  if (a->stat.st_ino < b->stat.st_ino)
    return -1;
  else if (a->stat.st_ino > b->stat.st_ino)
    return 1;

  if ((a->flags&LUMIERA_FILE_MASK) < (b->flags&LUMIERA_FILE_MASK))
    return -1;
  else if ((a->flags&LUMIERA_FILE_MASK) > (b->flags&LUMIERA_FILE_MASK))
    return 1;

  return 0;
}


static void
delete_fn (PSplaynode node)
{
  lumiera_filedescriptor_delete ((LumieraFiledescriptor) node);
}


static const void*
key_fn (const PSplaynode node)
{
  return node;
}



void
lumiera_filedescriptor_registry_init (void)
{
  NOBUG_INIT_FLAG (filedescriptor);
  TRACE (filedescriptor);
  REQUIRE (!registry);

  registry = psplay_new (cmp_fn, key_fn, delete_fn);
  if (!registry)
    LUMIERA_DIE (NO_MEMORY);

  RESOURCE_HANDLE_INIT (registry_mutex.rh);
  RESOURCE_ANNOUNCE (filedescriptor, "mutex", "filedescriptor registry", &registry, registry_mutex.rh);
}

void
lumiera_filedescriptor_registry_destroy (void)
{
  TRACE (filedescriptor);
  REQUIRE (!psplay_nelements (registry));

  RESOURCE_FORGET (filedescriptor, registry_mutex.rh);

  if (registry)
    psplay_destroy (registry);
  registry = NULL;
}


LumieraFiledescriptor
lumiera_filedescriptor_acquire (const char* name, int flags)
{
  TRACE (filedescriptor, "%s", name);
  REQUIRE (registry, "not initialized");

  LumieraFiledescriptor dest = NULL;

  LUMIERA_MUTEX_SECTION (filedescriptor, &registry_mutex)
    {
      lumiera_filedescriptor fdesc;
      fdesc.flags = flags;

      if (stat (name, &fdesc.stat) != 0)
        {
          if (errno == ENOENT && flags&O_CREAT)
            {
              char* dir = lumiera_tmpbuf_strndup (name, PATH_MAX);
              char* slash = dir;
              while ((slash = strchr (slash+1, '/')))
                {
                  *slash = '\0';
                  INFO (filedescriptor, "try creating dir: %s", dir);
                  if (mkdir (dir, 0777) == -1 && errno != EEXIST)
                    {
                      LUMIERA_ERROR_SET (filedescriptor, ERRNO);
                      goto error;
                    }
                  *slash = '/';
                }
              int fd;
              INFO (filedescriptor, "try creating file: %s", name);
              fd = creat (name, 0777);
              if (fd == -1)
                {
                  LUMIERA_ERROR_SET (filedescriptor, ERRNO);
                  goto error;
                }
              close (fd);
              if (stat (name, &fdesc.stat) != 0)
                {
                  /* finally, no luck */
                  LUMIERA_ERROR_SET (filedescriptor, ERRNO);
                  goto error;
                }
            }
        }

      /* lookup/create descriptor */
      dest = (LumieraFiledescriptor) psplay_find (registry, &fdesc, 100);

      if (!dest)
        {
          TRACE (filedescriptor, "Descriptor not found");

          dest = lumiera_filedescriptor_new (&fdesc);
          if (!dest)
            goto error;

          psplay_insert (registry, &dest->node, 100);
        }
      else
        {
          TRACE (filedescriptor, "Descriptor already existing");
          ++dest->refcount;
        }
    error: ;
    }

  return dest;
}


void
lumiera_filedescriptor_release (LumieraFiledescriptor self)
{
  TRACE (filedescriptor, "%p", self);
  if (!--self->refcount)
    lumiera_filedescriptor_delete (self);
}


LumieraFiledescriptor
lumiera_filedescriptor_new (LumieraFiledescriptor template)
{
  LumieraFiledescriptor self = lumiera_malloc (sizeof (lumiera_filedescriptor));
  TRACE (filedescriptor, "at %p", self);

  psplaynode_init (&self->node);
  self->stat = template->stat;

  self->flags = template->flags;
  self->refcount = 1;
  self->handle = 0;

  lumiera_mutex_init (&self->lock, "filedescriptor", &NOBUG_FLAG (filedescriptor));

  return self;
}


void
lumiera_filedescriptor_delete (LumieraFiledescriptor self)
{
  TRACE (filedescriptor, "%p", self);

  LUMIERA_MUTEX_SECTION (filedescriptor, &registry_mutex)
    {
      REQUIRE (self->refcount == 0);

      psplay_remove (registry, &self->node);

      TODO ("destruct other members (WIP)");


      TODO ("release filehandle");

      lumiera_mutex_destroy (&self->lock, &NOBUG_FLAG (filedescriptor));
      lumiera_free (self);
    }
}
