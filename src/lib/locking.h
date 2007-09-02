/*
  locking.h  -  shared declarations for all locking primitives

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

#ifndef CINELERRA_LOCKING_H
#define CINELERRA_LOCKING_H

#include <pthread.h>
#include <nobug.h>

#include "lib/error.h"

/**
 * used to store the current lock state.
 *
 *
 */
enum cinelerra_lockstate
  {
    CINELERRA_UNLOCKED,
    CINELERRA_LOCKED,
    CINELERRA_RLOCKED,
    CINELERRA_WLOCKED
  };

#endif