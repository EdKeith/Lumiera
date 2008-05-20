/*
  test-locking.c  -  test locking functions

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

#include <stdio.h>
#include <string.h>
#include "tests/test.h"

int conditionforgotunlock ();
int mutexforgotunlock ();

TESTS_BEGIN

TEST ("conditionforgotunlock")
{
  return conditionforgotunlock ();
}

TEST ("mutexforgotunlock")
{
  return mutexforgotunlock ();
}

TESTS_END
