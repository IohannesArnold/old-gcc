/* Support for GCC on simulated PowerPC systems targeted to embedded ELF
   systems.
   Copyright (C) 1995, 1996 Free Software Foundation, Inc.
   Contributed by Cygnus Support.

This file is part of GNU CC.

GNU CC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU CC; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include "rs6000/eabi.h"

#undef TARGET_VERSION
#define TARGET_VERSION fprintf (stderr, " (PowerPC Simulated)");

#undef CPP_PREDEFINES
#define CPP_PREDEFINES \
  "-DPPC -D__embedded__ -D__simulator__ -Asystem(embedded) -Asystem(simulator) -Acpu(powerpc) -Amachine(powerpc)"

/* Use the simulator crt0 or mvme and libgloss/newlib libraries if desired */
#undef  STARTFILE_SPEC
#define	STARTFILE_SPEC "crti.o \
%{mmvme: mvme-crt0.o%s} \
%{!mmvme: sim-crt0.o%s}"

#undef	LIB_SPEC
#define	LIB_SPEC "\
%{mmvme: -lmvme -lc -lmvme} \
%{!mmvme: -lsim -lc -lsim}"

#undef	LIBGCC_SPEC
#define	LIBGCC_SPEC "libgcc.a%s"

#undef	ENDFILE_SPEC
#define	ENDFILE_SPEC "crtn.o"
