/* Definitions of target machine for GNU compiler.  Iris version.
   Copyright (C) 1991 Free Software Foundation, Inc.

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
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#define SGI_TARGET 1		/* inform other mips files this is SGI */

/* Names to predefine in the preprocessor for this target machine.  */

#define CPP_PREDEFINES	"-Dunix -Dmips -Dsgi -DSVR3 -Dhost_mips -DMIPSEB -DSYSTYPE_SYSV"

#define STARTFILE_SPEC	"%{pg:gcrt1.o%s}%{!pg:%{p:mcrt1.o%s}%{!p:crt1.o%s}}"

#define CPP_SPEC "\
%{!ansi:-D__EXTENSIONS__} -D_MIPSEB -D_SYSTYPE_SYSV \
%{.S:	-D_LANGUAGE_ASSEMBLY %{!ansi:-DLANGUAGE_ASSEMBLY}} \
%{.s:	-D_LANGUAGE_ASSEMBLY %{!ansi:-DLANGUAGE_ASSEMBLY}} \
%{.cc:	-D_LANGUAGE_C_PLUS_PLUS} \
%{.cxx:	-D_LANGUAGE_C_PLUS_PLUS} \
%{.C:	-D_LANGUAGE_C_PLUS_PLUS} \
%{.m:	-D_LANGUAGE_OBJECTIVE_C} \
%{!.S: %{!.s: %{!.cc: %{!.cxx: %{!.C: %{!.m: -D_LANGUAGE_C %{!ansi:-DLANGUAGE_C}}}}}}}"

#define LIB_SPEC	"%{!p:%{!pg:-lc}}%{p:-lc_p}%{pg:-lc_p} crtn.o%s"

#define MACHINE_TYPE	"Silicon Graphics Mips"

/* SGI Iris doesn't support -EB/-EL like other MIPS processors.  */

#define ASM_SPEC "\
%{!mgas: \
	%{!mrnames: %{!.s:-nocpp} %{.s: %{cpp} %{nocpp}}} \
	%{pipe: %e-pipe is not supported.} \
	%{mips1} %{mips2} %{mips3} \
	%{noasmopt:-O0} \
	%{!noasmopt:%{O:-O2} %{O1:-O2} %{O2:-O2} %{O3:-O3}} \
	%{g} %{g0} %{g1} %{g2} %{g3} %{v} %{K} \
	%{ggdb:-g} %{ggdb0:-g0} %{ggdb1:-g1} %{ggdb2:-g2} %{ggdb3:-g3} \
	%{gstabs:-g} %{gstabs0:-g0} %{gstabs1:-g1} %{gstabs2:-g2} %{gstabs3:-g3} \
	%{gstabs+:-g} %{gstabs+0:-g0} %{gstabs+1:-g1} %{gstabs+2:-g2} %{gstabs+3:-g3} \
	%{gcoff:-g} %{gstabs0:-g0} %{gcoff1:-g1} %{gcoff2:-g2} %{gcoff3:-g3}} \
%{G*}"

#define LINK_SPEC "\
%{G*} \
%{!mgas: %{mips1} %{mips2} %{mips3} \
	 %{bestGnum} %{shared} %{non_shared}}"

/* Always use 1 for .file number.  I [meissner@osf.org] wonder why
   IRIS needs this.  */

#define SET_FILE_NUMBER() num_source_filenames = 1

/* Put out a label after a .loc.  I [meissner@osf.org] wonder why
   IRIS needs this.  */

#define LABEL_AFTER_LOC(STREAM) fprintf (STREAM, "LM%d:\n", ++sym_lineno)

#define STACK_ARGS_ADJUST(SIZE)                                         \
{                                                                       \
  SIZE.constant += 4;                                                   \
  if (SIZE.constant < 32)						\
    SIZE.constant = 32;                                                 \
}

/* Define this macro to control use of the character `$' in
   identifier names.  The value should be 0, 1, or 2.  0 means `$'
   is not allowed by default; 1 means it is allowed by default if
   `-traditional' is used; 2 means it is allowed by default provided
   `-ansi' is not used.  1 is the default; there is no need to
   define this macro in that case. */

#define DOLLARS_IN_IDENTIFIERS 0

/* Tell G++ not to create constructors or destructors with $'s in them.  */

#define NO_DOLLAR_IN_LABEL 1

/* Specify size_t, ptrdiff_t, and wchar_t types.  */
#define SIZE_TYPE	"unsigned int"
#define PTRDIFF_TYPE	"int"
#define WCHAR_TYPE	"unsigned char"
#define WCHAR_TYPE_SIZE BITS_PER_UNIT

/* Generate calls to memcpy, etc., not bcopy, etc.  */
#define TARGET_MEM_FUNCTIONS

/* Plain char is unsigned in the SGI compiler.  */
#define DEFAULT_SIGNED_CHAR 0


/* A C statement to initialize the variable parts of a trampoline. 
   ADDR is an RTX for the address of the trampoline; FNADDR is an
   RTX for the address of the nested function; STATIC_CHAIN is an
   RTX for the static chain value that should be passed to the
   function when it is called.

   Silicon Graphics machines are supposed to not have a mprotect
   function to enable execute protection, but the stack already
   has execute protection turned on.  Because the MIPS chips have
   no method to flush the icache without a system call, this can lose
   if the same address is used for multiple trampolines.  */

#define INITIALIZE_TRAMPOLINE(ADDR, FUNC, CHAIN)			    \
{									    \
  rtx addr = ADDR;							    \
  emit_move_insn (gen_rtx (MEM, SImode, plus_constant (addr, 28)), FUNC);   \
  emit_move_insn (gen_rtx (MEM, SImode, plus_constant (addr, 32)), CHAIN);  \
}


/* Attempt to turn on access permissions for the stack.  */

#define TRANSFER_FROM_TRAMPOLINE					\
									\
void									\
__enable_execute_stack (addr)						\
     char *addr;							\
{									\
}

#include "mips/mips.h"
