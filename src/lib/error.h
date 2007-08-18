/*
  error.h  -  Cinelerra Error handling

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
#ifndef CINELERRA_ERROR_H
#define CINELERRA_ERROR_H

#include <nobug.h>
#include <stdlib.h>

#define CINELERRA_DIE do { NOBUG_ERROR(NOBUG_ON, "Fatal Error!"); abort(); } while(0)

#define CINELERRA_ERROR_DECLARE(err) \
extern const char* CINELERRA_ERROR_##err

#define CINELERRA_ERROR_DEFINE(err, msg) \
const char* CINELERRA_ERROR_##err = "CINELERRA_ERROR_" #err ":" msg

#define CINELERRA_ERROR_SET(flag, err) \
ERROR (flag, "%s", strchr(CINELERRA_ERROR_##err, ':')+1); \
cinelerra_error_set(CINELERRA_ERROR_##err)

const char*
cinelerra_error_set (const char * err);

const char*
cinelerra_error ();


#endif /* CINELERRA_ERROR_H */