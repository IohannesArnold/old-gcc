/* Definitions of target machine for GNU compiler,
   for ARM with COFF obj format.
   Copyright (C) 1995, 1996, 1997 Free Software Foundation, Inc.
   Contributed by Doug Evans (dje@cygnus.com).
   
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

#include "arm/semi.h"

/* Run-time Target Specification.  */
#undef TARGET_VERSION
#define TARGET_VERSION fputs (" (ARM/coff)", stderr)

/* ??? Maybe use --with{enable?}-fpu or some such to make hardware floating
   point the default.  NOT --nfp!  --with{enable?} is supposed to replace it
   (right?), so let's stop using it.  */
#undef TARGET_DEFAULT
#define TARGET_DEFAULT (ARM_FLAG_SOFT_FLOAT /*+ TARGET_CPU_DEFAULT*/)

/* ??? Is a big-endian default intended to be supported?  */
#if 0 /*TARGET_CPU_DEFAULT & ARM_FLAG_BIG_END*/
#define MULTILIB_DEFAULTS { "mbig-endian" }
#else
#define MULTILIB_DEFAULTS { "mlittle-endian" }
#endif

/* ??? Does arm.h really need to set this to 32?  */
#undef STRUCTURE_SIZE_BOUNDARY
#define STRUCTURE_SIZE_BOUNDARY 8

/* A C expression whose value is nonzero if IDENTIFIER with arguments ARGS
   is a valid machine specific attribute for DECL.
   The attributes in ATTRIBUTES have previously been assigned to DECL.  */
extern int arm_valid_machine_decl_attribute ();
#define VALID_MACHINE_DECL_ATTRIBUTE(DECL, ATTRIBUTES, IDENTIFIER, ARGS) \
arm_valid_machine_decl_attribute (DECL, ATTRIBUTES, IDENTIFIER, ARGS)

/* This is COFF, but prefer stabs.  */
#define SDB_DEBUGGING_INFO

#define PREFERRED_DEBUGGING_TYPE DBX_DEBUG

#include "dbxcoff.h"

#undef LOCAL_LABEL_PREFIX
#define LOCAL_LABEL_PREFIX "."

#undef USER_LABEL_PREFIX
#define USER_LABEL_PREFIX ""

/* A C statement to output assembler commands which will identify the
   object file as having been compiled with GNU CC (or another GNU
   compiler).  */
/* Define this to NULL so we don't get anything.
   We have ASM_IDENTIFY_LANGUAGE.
   Also, when using stabs, gcc2_compiled must be a stabs entry, not an
   ordinary symbol, or gdb won't see it.  The stabs entry must be
   before the N_SO in order for gdb to find it.  */
#define ASM_IDENTIFY_GCC(STREAM)

/* This outputs a lot of .req's to define alias for various registers.
   Let's try to avoid this.  */
#undef ASM_FILE_START
#define ASM_FILE_START(STREAM) \
do {								\
  extern char *version_string;					\
  fprintf (STREAM, "%s Generated by gcc %s for ARM/coff\n",	\
	   ASM_COMMENT_START, version_string);			\
} while (0)

/* A C statement to output something to the assembler file to switch to section
   NAME for object DECL which is either a FUNCTION_DECL, a VAR_DECL or
   NULL_TREE.  Some target formats do not support arbitrary sections.  Do not
   define this macro in such cases.  */
#define ASM_OUTPUT_SECTION_NAME(STREAM, DECL, NAME, RELOC) \
do {								\
  if ((DECL) && TREE_CODE (DECL) == FUNCTION_DECL)		\
    fprintf (STREAM, "\t.section %s,\"x\"\n", (NAME));		\
  else if ((DECL) && DECL_READONLY_SECTION (DECL, RELOC))	\
    fprintf (STREAM, "\t.section %s,\"\"\n", (NAME));		\
  else								\
    fprintf (STREAM, "\t.section %s,\"w\"\n", (NAME));		\
} while (0)

/* Support the ctors/dtors and other sections.  */

#undef INIT_SECTION_ASM_OP

/* Define this macro if jump tables (for `tablejump' insns) should be
   output in the text section, along with the assembler instructions.
   Otherwise, the readonly data section is used.  */
#define JUMP_TABLES_IN_TEXT_SECTION

#undef READONLY_DATA_SECTION
#define READONLY_DATA_SECTION	rdata_section
#undef RDATA_SECTION_ASM_OP
#define RDATA_SECTION_ASM_OP	"\t.section .rdata"

#undef CTORS_SECTION_ASM_OP
#define CTORS_SECTION_ASM_OP	"\t.section .ctors,\"x\""
#undef DTORS_SECTION_ASM_OP
#define DTORS_SECTION_ASM_OP	"\t.section .dtors,\"x\""

/* A list of other sections which the compiler might be "in" at any
   given time.  */

#undef EXTRA_SECTIONS
#define EXTRA_SECTIONS SUBTARGET_EXTRA_SECTIONS in_rdata, in_ctors, in_dtors

#define SUBTARGET_EXTRA_SECTIONS

/* A list of extra section function definitions.  */

#undef EXTRA_SECTION_FUNCTIONS
#define EXTRA_SECTION_FUNCTIONS \
  RDATA_SECTION_FUNCTION	\
  CTORS_SECTION_FUNCTION	\
  DTORS_SECTION_FUNCTION	\
  SUBTARGET_EXTRA_SECTION_FUNCTIONS

#define SUBTARGET_EXTRA_SECTION_FUNCTIONS

#define RDATA_SECTION_FUNCTION \
void									\
rdata_section ()							\
{									\
  if (in_section != in_rdata)						\
    {									\
      fprintf (asm_out_file, "%s\n", RDATA_SECTION_ASM_OP);		\
      in_section = in_rdata;						\
    }									\
}

#define CTORS_SECTION_FUNCTION \
void									\
ctors_section ()							\
{									\
  if (in_section != in_ctors)						\
    {									\
      fprintf (asm_out_file, "%s\n", CTORS_SECTION_ASM_OP);		\
      in_section = in_ctors;						\
    }									\
}

#define DTORS_SECTION_FUNCTION \
void									\
dtors_section ()							\
{									\
  if (in_section != in_dtors)						\
    {									\
      fprintf (asm_out_file, "%s\n", DTORS_SECTION_ASM_OP);		\
      in_section = in_dtors;						\
    }									\
}

/* Support the ctors/dtors sections for g++.  */

#define INT_ASM_OP ".word"

/* A C statement (sans semicolon) to output an element in the table of
   global constructors.  */
#undef ASM_OUTPUT_CONSTRUCTOR
#define ASM_OUTPUT_CONSTRUCTOR(STREAM,NAME) \
do {						\
  ctors_section ();				\
  fprintf (STREAM, "\t%s\t ", INT_ASM_OP);	\
  assemble_name (STREAM, NAME);			\
  fprintf (STREAM, "\n");			\
} while (0)

/* A C statement (sans semicolon) to output an element in the table of
   global destructors.  */
#undef ASM_OUTPUT_DESTRUCTOR
#define ASM_OUTPUT_DESTRUCTOR(STREAM,NAME) \
do {						\
  dtors_section ();                   		\
  fprintf (STREAM, "\t%s\t ", INT_ASM_OP);	\
  assemble_name (STREAM, NAME);              	\
  fprintf (STREAM, "\n");			\
} while (0)

/* __CTOR_LIST__ and __DTOR_LIST__ must be defined by the linker script.  */
#define CTOR_LISTS_DEFINED_EXTERNALLY

#undef DO_GLOBAL_CTORS_BODY
#undef DO_GLOBAL_DTORS_BODY

/* The ARM development system has atexit and doesn't have _exit,
   so define this for now.  */
#define HAVE_ATEXIT

/* The ARM development system defines __main.  */
#define NAME__MAIN "__gccmain"
#define SYMBOL__MAIN __gccmain
