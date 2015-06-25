/* @(#)root/clib:$Name:  $:$Id: demangle.h,v 1.1 2001/04/26 03:25:11 maelstorm Exp $ */
/* Author: */
/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/


/* Defs for interface to demanglers.
   Copyright 1992 Free Software Foundation, Inc.
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#if !defined (DEMANGLE_H)
#define DEMANGLE_H

#define DMGL_NO_OPTS 0               /* For readability... */
#define DMGL_PARAMS (1 << 0)        /* Include function args */
#define DMGL_ANSI (1 << 1)        /* Include const, volatile, etc */
#define DMGL_AUTO (1 << 8)
#define DMGL_GNU (1 << 9)
#define DMGL_LUCID (1 << 10)
#define DMGL_ARM (1 << 11)

#ifdef __cplusplus
extern "C" {
#endif

extern enum demangling_styles
{
  unknown_demangling = 0,
  auto_demangling = DMGL_AUTO,
  gnu_demangling = DMGL_GNU,
  lucid_demangling = DMGL_LUCID,
  arm_demangling = DMGL_ARM
} current_demangling_style;


extern char *
cplus_demangle(const char *mangled, int options);
extern int
cplus_demangle_opname (const char *opname, char *result, int options);

extern const char*
cplus_mangle_opname (const char *opname, int options); 

#ifdef __cplusplus
}
#endif

#endif /* DEMANGLE_H */
