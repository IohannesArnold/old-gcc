/* Build expressions with type checking for C compiler.
   Copyright (C) 1987, 1988, 1989, 1992 Free Software Foundation, Inc.

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


/* This file is part of the C front end.
   It is responsible for implementing iterators,
   both their declarations and the expansion of statements using them.  */

#include "config.h"
#include <stdio.h>
#include "tree.h"
#include "c-tree.h"
#include "flags.h"

static void expand_stmt_with_iterators_1 ();
static tree collect_iterators();
static void iterator_loop_prologue ();
static void iterator_loop_epilogue ();
static void add_ixpansion ();
static void delete_ixpansion();
static int top_level_ixpansion_p ();
static void istack_sublevel_to_current ();

void
iterator_for_loop_start (idecl)
     tree idecl;
{
  iterator_loop_prologue (idecl, 0, 0);
}

void
iterator_for_loop_end (idecl)
     tree idecl;
{
  iterator_loop_epilogue (idecl, 0, 0);
}

void
iterator_for_loop_record (idecl)
     tree idecl;
{
  add_ixpansion (idecl, 0, 0, 0, 0);
}


/*
  		ITERATOR DECLS

Iterators are  implemented  as integer decls  with a special  flag set
(rms's   idea).  This  makes  eliminates  the need   for  special type
checking.  The  flag  is accesed using   the  ITERATOR_P  macro.  Each
iterator's limit is saved as a  decl with a special  name. The decl is
initialized with the limit value -- this way we  get all the necessary
semantical processing for free by calling finish  decl. We might still
eliminate  that decl  later  -- it takes up  time and  space and, more
importantly, produces strange error  messages when  something is wrong
with the initializing expresison.  */

tree
build_iterator_decl (id, limit)
    tree id, limit;
{
  tree type = integer_type_node, lim_decl;
  tree t1, t2, t3;
  tree start_node, limit_node, step_node;
  tree decl;
    
  if (limit)
    {
      limit_node = save_expr (limit);
      SAVE_EXPR_CONTEXT (limit_node) = current_function_decl;
    }
  else
    abort ();
  lim_decl = build_limit_decl (id, limit_node);
  push_obstacks_nochange ();
  decl = build_decl (VAR_DECL, id, type);
  ITERATOR_P (decl) = 1;
  ITERATOR_LIMIT (decl) = lim_decl;
  finish_decl (pushdecl (decl), 0, 0);
  return decl;
}

/*
  		ITERATOR RTL EXPANSIONS

   Expanding simple statements with iterators is  pretty straightforward:
   collect (collect_iterators) the list  of  all "free" iterators  in the
   statement and for each  of them add  a  special prologue before and an
   epilogue after the expansion for  the statement. Iterator is "free" if
   it has not been "bound" by a FOR operator. The rtx associated with the
   iterator's  decl is used as  the loop counter.  Special processing  is
   used  for "{(...)}" constructs:  each iterator expansion is registered
   (by "add_ixpansion" function)  and inner expansions are superseded  by
   outer ones. The cleanup of superseded expansions is done by  a call to
   delete_ixpansion.  */

void
iterator_expand (stmt)
    tree stmt;
{
  tree iter_list = collect_iterators (stmt, NULL_TREE);
  expand_stmt_with_iterators_1 (stmt, iter_list);
  istack_sublevel_to_current ();
}


static void 
expand_stmt_with_iterators_1 (stmt, iter_list)
     tree stmt, iter_list;
{
  if (iter_list == 0)
    expand_expr_stmt (stmt);
  else
    {
      tree current_iterator = TREE_VALUE (iter_list);
      tree iter_list_tail   = TREE_CHAIN (iter_list);
      rtx p_start, p_end, e_start, e_end;

      iterator_loop_prologue (current_iterator, &p_start, &p_end);
      expand_stmt_with_iterators_1 (stmt, iter_list_tail);
      iterator_loop_epilogue (current_iterator, &e_start, &e_end);

      /** Delete all inner expansions based on current_iterator **/
      /** before adding the outer one. **/

      delete_ixpansion (current_iterator);
      add_ixpansion (current_iterator, p_start, p_end, e_start, e_end);
    }
}


/* Return a list containing all the free (i.e. not bound by "for"
   statement or anaccumulator) iterators mentioned in EXP,
   plus those in LIST.   Duplicates are avoided.  */

static tree
collect_iterators (exp, list)
     tree exp, list;
{
  if (exp == 0) return list;

  switch (TREE_CODE (exp))
    {
    case VAR_DECL:
      if (! ITERATOR_P (exp) || ITERATOR_BOUND_P (exp))
	return list;
      if (value_member (exp, list))
	return list;
      return tree_cons (NULL_TREE, exp, list);

    case TREE_LIST:
      {
	tree tail;
	for (tail = exp; tail; tail = TREE_CHAIN (tail))
	  list = collect_iterators (TREE_VALUE (tail), list);
	return list;
      }

      /* we do not automatically iterate blocks -- one must */
      /* use the FOR construct to do that */

    case BLOCK:
      return list;

    default:
      switch (TREE_CODE_CLASS (code))
	{
	case '1':
	case '2':
	case '<':
	case 'e':
	case 'r':
	  {
	    int num_args = tree_code_length[code];
	    int i;
	    the_list = (tree) 0;
	    for (i = 0; i < num_args; i++)
	      list = collect_iterators (TREE_OPERAND (exp, i), list);
	    return list;
	  }
	}
    }
}

/* Emit rtl for the start of a loop for iterator IDECL.

   If necessary, create loop counter rtx and store it as DECL_RTL of IDECL.

   The prologue normally starts and ends with notes, which are returned
   by this function in *START_NOTE and *END_NODE.
   If START_NOTE and END_NODE are 0, we don't make those notes.  */

static void
iterator_loop_prologue (idecl, start_note, end_note)
     tree idecl;
     rtx *start_note, *end_note;
{
  /* Force the save_expr in DECL_INITIAL to be calculated
     if it hasn't been calculated yet.  */
  expand_expr (DECL_INITIAL (idecl), 0, VOIDmode, 0);

  if (DECL_RTL (idecl) == 0)
    expand_decl (idecl);

  if (start_note)
    *start_note = emit_note (0, NOTE_INSN_DELETED);
  /* Initialize counter.  */
  expand_expr (build_modify_expr (idecl, NOP_EXPR, integer_zero_node),
	       0, VOIDmode, 0);

  expand_start_loop_continue_elsewhere (1);

  ITERATOR_BOUND_P (idecl) = 1;

  if (end_note)
    *end_note = emit_note (0, NOTE_INSN_DELETED);
}

/* Similar to the previous function, but for the end of the loop.

   DECL_RTL is zeroed unless we are inside "({...})". The reason for that is
   described below.

   When we create two (or more)  loops based on the  same IDECL, and both
   inside the same "({...})"  construct, we  must be prepared  to  delete
   both of the loops  and create a single one  on the  level  above, i.e.
   enclosing the "({...})". The new loop has to use  the same counter rtl
   because the references to the iterator decl  (IDECL) have already been
   expanded as references to the counter rtl.

   It is incorrect to use the same counter reg in different functions,
   and it is desirable to use different counters in disjoint loops
   when we know there's no need to combine them
   (because then they can get allocated separately).  */

static void
iterator_loop_epilogue (idecl, start_note, end_note)
     tree idecl;
     rtx *start_note, *end_note;
{
  tree test, incr;

  if (start_note)
    *start_note = emit_note (0, NOTE_INSN_DELETED);
  expand_loop_continue_here ();
  incr = build_binary_op (PLUS_EXPR, idecl, integer_one_node, 0);
  expand_expr (build_modify_expr (idecl, NOP_EXPR, incr));
  test = build_binary_op (LT_EXPR, idecl, DECL_INITIAL (idecl), 0);
  expand_exit_loop_if_false (0, test);
  expand_end_loop ();

  ITERATOR_BOUND_P (idecl) = 0;
  /* we can reset rtl since there is not chance that this expansion */
  /* would be superceded by a higher level one */
  if (top_level_ixpansion_p ())
    DECL_RTL (idecl) = 0;
  if (end_note)
    *end_note = emit_note (0, NOTE_INSN_DELETED);
}

/*
		KEEPING TRACK OF EXPANSIONS

   In order to clean out expansions corresponding to statements inside
   "{(...)}" constructs we have to keep track of all expansions.  The
   cleanup is needed when an automatic, or implicit, expansion on
   iterator, say X, happens to a statement which contains a {(...)}
   form with a statement already expanded on X.  In this case we have
   to go back and cleanup the inner expansion.  This can be further
   complicated by the fact that {(...)} can be nested.

   To make this cleanup possible, we keep lists of all expansions, and
   to make it work for nested constructs, we keep a stack.  The list at
   the top of the stack (ITER_STACK.CURRENT_LEVEL) corresponds to the
   currently parsed level.  All expansions of the levels below the
   current one are kept in one list whose head is pointed to by
   ITER_STACK.SUBLEVEL_FIRST (SUBLEVEL_LAST is there for making merges
   easy).  The process works as follows:

   -- On "({"  a new node is added to the stack by PUSH_ITERATOR_STACK.
	       The sublevel list is not changed at this point.

   -- On "})" the list for the current level is appended to the sublevel
	      list. 

   -- On ";"  sublevel lists are appended to the current level lists.
	      The reason is this: if they have not been superseded by the
	      expansion at the current level, they still might be
	      superseded later by the expansion on the higher level.
	      The levels do not have to distinguish levels below, so we
	      can merge the lists together.  */

struct  ixpansion
{
  tree ixdecl;			/* Iterator decl */
  rtx  ixprologue_start;	/* First insn of epilogue. NULL means */
  /* explicit (FOR) expansion*/
  rtx  ixprologue_end;
  rtx  ixepilogue_start;
  rtx  ixepilogue_end;
  struct ixpansion *next;	/* Next in the list */
};

static struct obstack ixp_obstack;

static char *ixp_firstobj;

struct iter_stack_node
{
  struct ixpansion *first;	/* Head of list of ixpansions */
  struct ixpansion *last;	/* Last node in list  of ixpansions */
  struct iter_stack_node *next; /* Next level iterator stack node  */
};

struct iter_stack_node *iter_stack;

struct iter_stack_node sublevel_ixpansions;

/** Return true if we are not currently inside a "({...})" construct */

static int
top_level_ixpansion_p ()
{
  return iter_stack == 0;
}

/* Given two chains of iter_stack_nodes,
   append the nodes in X into Y.  */

static void
isn_append (x, y)
     struct iter_stack_node *x, *y;
{
  if (x->first == 0) 
    return;

  if (y->first == 0)
    {
      y->first = x->first;
      y->last  = x->last;
    }
  else
    {
      y->last->next = x->first;
      y->last = x->last;
    }
}

/** Make X empty **/

#define ISN_ZERO(X) (X).first=(X).last=0

/* Move the ixpansions in sublevel_ixpansions into the current
   node on the iter_stack, or discard them if the iter_stack is empty.
   We do this at the end of a statement.  */

static void
istack_sublevel_to_current ()
{
  /* At the top level we can throw away sublevel's expansions  **/
  /* because there is nobody above us to ask for a cleanup **/
  if (iter_stack != 0)
    /** Merging with empty sublevel list is a no-op **/
    if (sublevel_ixpansions.last)
      isn_append (&sublevel_ixpansions, iter_stack);

  if (iter_stack == 0)
    obstack_free (&ixp_obstack, ixp_firstobj);

  ISN_ZERO (sublevel_ixpansions);
}

/* Push a new node on the iter_stack, when we enter a ({...}).  */

void
push_iterator_stack ()
{
  struct iter_stack_node *new_top
    = (struct iter_stack_node*) 
      obstack_alloc (&ixp_obstack, sizeof (struct iter_stack_node));

  new_top->first = 0;
  new_top->last = 0;
  new_top->next = iter_stack;
  iter_stack = new_top;
}

/* Pop iter_stack, moving the ixpansions in the node being popped
   into sublevel_ixpansions.  */

void
pop_iterator_stack ()
{
  if (iter_stack == 0)
    abort ();

  isn_append (iter_stack, &sublevel_ixpansions);
  /** Pop current level node: */
  iter_stack = iter_stack->next;
}


/* Record an iterator expansion ("ixpansion") for IDECL.
   The remaining paramters are the notes in the loop entry
   and exit rtl.  */

static void
add_ixpansion (idecl, pro_start, pro_end, epi_start, epi_end)
     tree idecl;
     rtx pro_start, pro_end, epi_start, epi_end;
{
  struct ixpansion* newix;
    
  /* Do nothing if we are not inside "({...})",
     as in that case this expansion can't need subsequent RTL modification.  */
  if (iter_stack == 0)
    return;

  newix = (struct ixpansion*) obstack_alloc (&ixp_obstack,
					     sizeof (struct ixpansion));
  newix->ixdecl = idecl;
  newix->ixprologue_start = pro_start;
  newix->ixprologue_end   = pro_end;
  newix->ixepilogue_start = epi_start;
  newix->ixepilogue_end   = epi_end;

  newix->next = iter_stack->first;
  iter_stack->first = newix;
  if (iter_stack->last == 0)
    iter_stack->last = newix;
}

/* Delete the RTL for all ixpansions for iterator IDECL
   in our sublevels.  We do this when we make a larger
   containing expansion for IDECL.  */

static void
delete_ixpansion (idecl)
     tree idecl;
{
  struct ixpansion* previx = 0, *ix;

  for (ix = sublevel_ixpansions.first; ix; ix = ix->next)
    if (ix->ixdecl == idecl)
      {
	/** zero means that this is a mark for FOR -- **/
	/** we do not delete anything, just issue an error. **/

	if (ix->ixprologue_start == 0)
	  error_with_decl (idecl,
			   "`for (%s)' appears within implicit iteration")
	else
	  {
	    rtx insn;
	    /* We delete all insns, including notes because leaving loop */
	    /* notes and barriers produced by iterator expansion would */
	    /* be misleading to other phases */

	    for (insn = NEXT_INSN (ix->ixprologue_start);
		 insn != ix->ixprologue_end;
		 insn = NEXT_INSN (insn)) 
	      delete_insn (insn);
	    for (insn = NEXT_INSN (ix->ixepilogue_start);
		 insn != ix->ixepilogue_end;
		 insn = NEXT_INSN (insn)) 
	      delete_insn (insn);
	  }

	/* Delete this ixpansion from sublevel_ixpansions.  */
	if (previx)
	  previx->next = ix->next;
	else 
	  sublevel_ixpansions.first = ix->next;
	if (sublevel_ixpansions.last == ix)
	  sublevel_ixpansions.last = previx;
      }
    else
      previx = ix;
}

/*
We initialize iterators obstack once per file
*/

init_iterators ()
{
  gcc_obstack_init (&ixp_obstack);
  ixp_firstobj = (char *) obstack_alloc (&ixp_obstack, 0);
}

#ifdef DEBUG_ITERATORS

/*
The functions below are for use from source level debugger.
They print short forms of iterator lists and the iterator stack.
*/

/* Print the name of the iterator D */
void
PRDECL (D)
     tree D;
{
  if (D)
    {
      if (TREE_CODE (D) == VAR_DECL)
	{
	  tree tname = DECL_NAME (D);
	  char *dname = IDENTIFIER_POINTER (tname);
	  fprintf (stderr, dname);
	}
      else
	fprintf (stderr, "<<Not a Decl!!!>>");
    }
  else
    fprintf (stderr, "<<NULL!!>>");
}

/* Print Iterator List -- names only */

tree
pil (head)
     tree head;
{
  tree current, next;
  for (current=head; current; current = next)
    {
      tree node = TREE_VALUE (current);
      PRDECL (node);
      next = TREE_CHAIN (current);
      if (next) fprintf (stderr, ",");
    }
  fprintf (stderr, "\n");
}

/* Print IXpansion List */

struct ixpansion *
pixl (head)
     struct ixpansion *head;
{
  struct ixpansion *current, *next;
  fprintf (stderr, "> ");
  if (head == 0)
    fprintf (stderr, "(empty)");
	
  for (current=head; current; current = next)
    {
      tree node = current->ixdecl;
      PRDECL (node);
      next = current->next;
      if (next)
	fprintf (stderr, ",");
    }
  fprintf (stderr, "\n");
  return head;
}

/* Print Iterator Stack*/

void
pis ()
{
  struct iter_stack_node *stack_node;

  fprintf (stderr, "--SubLevel: ");
  pixl (sublevel_ixpansions.first);
  fprintf (stderr, "--Stack:--\n");
  for (stack_node = iter_stack;
       stack_node;
       stack_node = stack_node->next)
    pixl (stack_node->first);
}
#endif
