/*
    mpool.h - memory pool for constant sized objects

  Copyright (C)         Lumiera.org
    2009,               Christian Thaeter <ct@pipapo.org>

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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

#include "mpool.h"

/*
  Cluster and node structures are private
*/

typedef struct mpoolcluster_struct mpoolcluster;
typedef mpoolcluster* MPoolcluster;
typedef const mpoolcluster* const_MPoolcluster;

struct mpoolcluster_struct
{
  llist node;           /* all clusters */
  void* data[];         /* bitmap and elements */
};


typedef struct mpoolnode_struct mpoolnode;
typedef mpoolnode* MPoolnode;
typedef const mpoolnode* const_MPoolnode;

struct mpoolnode_struct
{
  llist node;
};


MPool
mpool_init (MPool self, size_t elem_size, unsigned elements_per_cluster, mpool_destroy_fn dtor)
{
  TRACE (mpool_dbg, "%p: elem_size %zd: elem_per_cluster %u", self, elem_size, elements_per_cluster);

  if (self)
    {
      llist_init (&self->freelist);
      llist_init (&self->clusters);
      self->elem_size = (elem_size+sizeof(void*)-1) / sizeof(void*) * sizeof(void*);    /* void* aligned */

      /* minimum size is the size of a llist node */
      if (self->elem_size < sizeof(llist))
        self->elem_size = sizeof(llist);

      self->elements_per_cluster = elements_per_cluster;

      self->elements_free = 0;
      self->destroy = dtor;
      self->locality = NULL;
    }

  return self;
}


#define MPOOL_BITMAP_SIZE(elements_per_cluster)                 \
  (((elements_per_cluster) + sizeof(uintptr_t)*CHAR_BIT - 1)    \
  / (sizeof(uintptr_t) * CHAR_BIT) * sizeof (uintptr_t))


static inline void*
cluster_element_get (MPoolcluster cluster, MPool self, unsigned n)
{
  return (void*)cluster +                                       /* start address */
    sizeof (*cluster) +                                         /* header */
    MPOOL_BITMAP_SIZE (self->elements_per_cluster) +            /* bitmap */
    self->elem_size * n;                                        /* offset*/
}


static inline bool
bitmap_bit_get_nth (MPoolcluster cluster, unsigned index)
{
  TRACE (mpool_dbg, "cluster %p: index %u", cluster, index);

  ldiv_t div = ldiv(index, sizeof(uintptr_t)*CHAR_BIT);
  uintptr_t* bitmap = (uintptr_t*)&cluster->data;

  return bitmap[div.quot] & ((uintptr_t)1<<div.rem);
}


MPool
mpool_destroy (MPool self)
{
  TRACE (mpool_dbg, "%p", self);
  if (self)
    {
      LLIST_WHILE_TAIL(&self->clusters, cluster)
        {
          if (self->destroy)
            for (unsigned i = 0; i < self->elements_per_cluster; ++i)
              {
                if (bitmap_bit_get_nth ((MPoolcluster)cluster, i))
                  {
                    void* obj = cluster_element_get ((MPoolcluster)cluster, self, i);
                    TRACE (mpool_dbg, "dtor: cluster %p: obj %p: freelist %p", cluster, obj, self->freelist);
                    self->destroy (obj);
                  }
              }

          llist_unlink_fast_ (cluster);
          TRACE (mpool_dbg, "freeing cluster %p" , cluster);
          free (cluster);
        }

      llist_init (&self->freelist);
      self->elements_free = 0;
      self->locality = NULL;
    }

  return self;
}




MPool
mpool_cluster_alloc_ (MPool self)
{
  MPoolcluster cluster = malloc (sizeof (*cluster) +                                    /* header */
                                 MPOOL_BITMAP_SIZE (self->elements_per_cluster) +       /* bitmap */
                                 self->elem_size * self->elements_per_cluster);         /* elements */

  TRACE (mpool_dbg, "%p", cluster);

  if (!cluster)
    return NULL;

  /* clear the bitmap */
  memset (&cluster->data, 0, MPOOL_BITMAP_SIZE (self->elements_per_cluster));

  /* initialize freelist */
  for (unsigned i = 0; i < self->elements_per_cluster; ++i)
    {
      MPoolnode node = cluster_element_get (cluster, self, i);
      TRACE (mpool_dbg, "node %p", node);
      llist_insert_tail (&self->freelist, llist_init (&node->node));
    }

  /* we insert the cluster at head because its likely be used next */
  llist_insert_head (&self->clusters, llist_init (&cluster->node));

  self->elements_free += self->elements_per_cluster;

  return self;
}


static int
cmp_cluster_contains_element (const_LList cluster, const_LList element, void* self)
{
  if (element < cluster)
    return -1;

  if ((void*)element >
      (void*)cluster +
      sizeof (*cluster) +                                               /* header */
      MPOOL_BITMAP_SIZE (((MPool)self)->elements_per_cluster) +         /* bitmap */
      ((MPool)self)->elem_size * ((MPool)self)->elements_per_cluster)   /* elements */
    return 1;

  return 0;
}

static inline MPoolcluster
element_cluster_get (MPool self, void* element)
{
  return (MPoolcluster) llist_ufind (&self->clusters, (const_LList) element, cmp_cluster_contains_element, self);
}


static inline unsigned
uintptr_nearestbit (uintptr_t v, unsigned n)
{
  unsigned r = 0;
  uintptr_t mask = 1ULL<<n;

  while (1)
    {
      if (v&mask)
        {
          if (v&mask& ~(~0ULL<<n))
            return n-r;
          else
            return n+r;
        }
      if (mask == ~0ULL)
        return ~0U;
      ++r;
      mask |= ((mask<<1)|(mask>>1));
    }
}



static inline void*
alloc_near (MPoolcluster cluster, MPool self, void* locality)
{
  void* begin_of_elements =
    (void*)cluster +
    sizeof (*cluster) +                                                 /* header */
    MPOOL_BITMAP_SIZE (((MPool)self)->elements_per_cluster);            /* bitmap */

#if UINTPTR_MAX > 4294967295U   /* 64 bit */
  lldiv_t div = lldiv((locality - begin_of_elements) / self->elem_size, sizeof(uintptr_t)*CHAR_BIT);
#else                           /* 32 bit */
  ldiv_t div = ldiv((locality - begin_of_elements) / self->elem_size, sizeof(uintptr_t)*CHAR_BIT);
#endif

  uintptr_t* bitmap = (uintptr_t*)&cluster->data;
  unsigned r = ~0U;

  TRACE (mpool_dbg, "cluster %p: bitmap %p %p: elements %p: index %d", cluster, bitmap, bitmap[div.quot], begin_of_elements, div.quot);

  /* the bitmap word at locality */
  if (bitmap[div.quot] < UINTPTR_MAX)
    {
      r = uintptr_nearestbit (~bitmap[div.quot], div.rem);
    }
  /* the bitmap word before locality, this gives a slight bias towards the begin, keeping the pool compact */
  else if (div.quot && bitmap[div.quot-1] < UINTPTR_MAX)
    {
      --div.quot;
      r = uintptr_nearestbit (~bitmap[div.quot], sizeof(uintptr_t)*CHAR_BIT-1);
    }

  if (r != ~0U)
    {
      void* ret = begin_of_elements + ((uintptr_t)(div.quot*sizeof(uintptr_t)*CHAR_BIT+r)*self->elem_size);
      return ret;
    }
  return NULL;
}


static inline void
bitmap_set_element (MPoolcluster cluster, MPool self, void* element)
{
  void* begin_of_elements =
    (void*)cluster +
    sizeof (*cluster) +                                                 /* header */
    MPOOL_BITMAP_SIZE (((MPool)self)->elements_per_cluster);            /* bitmap */

#if UINTPTR_MAX > 4294967295U   /* 64 bit */
  lldiv_t div = lldiv((element - begin_of_elements) / self->elem_size, sizeof(uintptr_t)*CHAR_BIT);
#else                           /* 32 bit */
  ldiv_t div = ldiv((element - begin_of_elements) / self->elem_size, sizeof(uintptr_t)*CHAR_BIT);
#endif

  uintptr_t* bitmap = (uintptr_t*)&cluster->data;
  bitmap[div.quot] |= ((uintptr_t)1<<div.rem);

  TRACE (mpool_dbg, "set bit %d, index %d, of %p is %p", div.rem, div.quot, element, bitmap[div.quot]);
}


static inline void
bitmap_clear_element (MPoolcluster cluster, MPool self, void* element)
{
  void* begin_of_elements =
    (void*)cluster +
    sizeof (*cluster) +                                                 /* header */
    MPOOL_BITMAP_SIZE (((MPool)self)->elements_per_cluster);            /* bitmap */

#if UINTPTR_MAX > 4294967295U   /* 64 bit */
  lldiv_t div = lldiv((element - begin_of_elements) / self->elem_size, sizeof(uintptr_t)*CHAR_BIT);
#else                           /* 32 bit */
  ldiv_t div = ldiv((element - begin_of_elements) / self->elem_size, sizeof(uintptr_t)*CHAR_BIT);
#endif

  uintptr_t* bitmap = (uintptr_t*)&cluster->data;
  bitmap[div.quot] &= ~((uintptr_t)1<<div.rem);

  TRACE (mpool_dbg, "clear bit %d, index %d, of %p is %p", div.rem, div.quot, element, bitmap[div.quot]);
}




void*
mpool_alloc (MPool self)
{
  if (!self->elements_free)
    {
      if (mpool_cluster_alloc_ (self))
        {
          self->locality = NULL; /* supress alloc_near() */
        }
      else
        {
          ERROR (mpool_dbg, "allocation failure");
          return NULL;
        }
    }

  void* ret = NULL;

  if (self->locality)
    {
      ret = alloc_near (element_cluster_get (self, self->locality), self, self->locality);
      TRACE_IF (ret, mpool_dbg, "near allocation %p", ret);
    }

  if (!ret)
    {
      ret = llist_head (&self->freelist);
      TRACE_IF (ret, mpool_dbg, "far allocation %p", ret);
    }

  if (ret)
    {
      llist_unlink_fast_ ((LList)ret);
      bitmap_set_element (element_cluster_get (self, ret), self, ret);
    }

  TRACE (mpool_dbg, "%p", ret);
  self->locality = ret;
  --self->elements_free;

  return ret;
}




/*
  put a element back on the pool
*/
static inline MPoolnode
find_near (MPoolcluster cluster, MPool self, void* element)
{
  void* begin_of_elements =
    (void*)cluster +
    sizeof (*cluster) +                                                 /* header */
    MPOOL_BITMAP_SIZE (((MPool)self)->elements_per_cluster);            /* bitmap */

  uintptr_t index = (element - begin_of_elements) / self->elem_size;

#if UINTPTR_MAX > 4294967295U   /* 64 bit */
  lldiv_t div = lldiv (index, sizeof(uintptr_t)*CHAR_BIT);
#else                           /* 32 bit */
  ldiv_t div = ldiv (index, sizeof(uintptr_t)*CHAR_BIT);
#endif

  uintptr_t* bitmap = (uintptr_t*)&cluster->data;
  unsigned r = ~0U;

  TRACE (mpool_dbg, "cluster %p: bitmap %p %p: elements %p: index %d", cluster, bitmap, bitmap[div.quot], begin_of_elements, div.quot);

  /* the bitmap word at locality */
  if (bitmap[div.quot] < UINTPTR_MAX)
    {
      r = uintptr_nearestbit (~bitmap[div.quot], div.rem);
    }
  /* the bitmap word after element, we assume that elements after the searched element are more likely be free */
  else if (index < self->elements_per_cluster && bitmap[div.quot+1] < UINTPTR_MAX)
    {
      ++div.quot;
      r = uintptr_nearestbit (~bitmap[div.quot], 0);
    }
  /* finally the bitmap word before element */
  else if (index > 0 && bitmap[div.quot-1] < UINTPTR_MAX)
    {
      --div.quot;
      r = uintptr_nearestbit (~bitmap[div.quot], sizeof(uintptr_t)*CHAR_BIT-1);
    }

  if (r != ~0U)
    return begin_of_elements + ((uintptr_t)(div.quot*sizeof(uintptr_t)*CHAR_BIT+r)*self->elem_size);

  return NULL;
}

void
mpool_free (MPool self, void* element)
{
  if (self && element)
    {
      TRACE (mpool_dbg, "mpool %p: element %p", self, element);

      MPoolcluster cluster = element_cluster_get (self,element);

      bitmap_clear_element (cluster, self, element);
      llist_init (&((MPoolnode)element)->node);

      MPoolnode near = find_near (cluster, self, element);
      TRACE (mpool_dbg, "near %p", near);

      if (near)
        {
          if (near < (MPoolnode)element)
            llist_insert_next (&near->node, &((MPoolnode)element)->node);
          else
            llist_insert_prev (&near->node, &((MPoolnode)element)->node);
        }
      else
        llist_insert_tail (&self->freelist, &((MPoolnode)element)->node);

      bitmap_clear_element (cluster, self, element);

      ++self->elements_free;
    }
}


unsigned
mpool_available (MPool self)
{
  return self->elements_free;
}


MPool
mpool_reserve (MPool self, unsigned nelements)
{
  if (self)
    while (self->elements_free < nelements)
      if (!mpool_cluster_alloc_ (self))
        return NULL;

  return self;
}



void
nobug_mpool_dump (const_MPool self,
                  const int depth,
                  const char* file,
                  const int line,
                  const char* func)
{
  if (self && depth)
    {
      DUMP_LOG ("mpool %p: ", self);

      if (depth > 1)
        {
          DUMP_LOG ("  elements_per_cluster %u: ", self->elements_per_cluster);
          DUMP_LOG ("  elements_free %u: ", self->elements_free);
        }

      if (depth > 2)
        {
          DUMP_LOG ("  clusters %p: ", self->clusters);
          int i = 0;
          LLIST_FOREACH (&self->clusters, cluster)
            DUMP_LOG ("    %p: %u", cluster, ++i);
        }

      if (depth > 3)
        {
          DUMP_LOG ("  freelist %p: ", self->freelist);
          int i = 0;
          LLIST_FOREACH (&self->freelist, node)
            DUMP_LOG ("    %p: %u", node, ++i);
        }
    }
}


