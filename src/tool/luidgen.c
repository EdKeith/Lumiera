/*
  luidgen.c  - generate a lumiera uuid

  Copyright (C)         Lumiera.org
    2008                Christian Thaeter <ct@pipapo.org>

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

#include "lib/luid.h"

#include <stdio.h>

/**
 * @file
 * Generate amd print a Lumiera uid as octal escaped string
 */

int
main (int argc, char** argv)
{
  lumiera_uid luid;
  lumiera_uid_gen (&luid);

  printf ("\"");
  for (int i = 0; i < 16; ++i)
    printf ("\\%.3hho", *(((char*)&luid)+i));
  printf ("\"\n");

  return 0;
}

/*
// Local Variables:
// mode: C
// c-file-style: "gnu"
// indent-tabs-mode: nil
// End:
*/