#ifndef SUDOKU_H
#define SUDOKU_H

/*

High level strategy:
--------------------
Read in a board from a file
Look for constraints starting upper left to lower right
Traverse from upper left to lower right:
  For each cell, look for any answers (*)
  If answer found, update the constraints for the box, row, and col
  iterate

Questions:
----------
  I wonder what we get after just one pass?
  How many passes this would take to complete?
  Does this even complete for all solvable puzzles?
    (it might require a deeper, more sophisticated set of constraints)
    This could be a combinatorial problem.
  After solving for 9 x 9, it might be good to take 16 x 16 with the hex digits.


Data Structures:
----------------

We input a 9x9 grid of numbers

Add a guess based on direct constraints:
1. Row contains all 9 numbers
2. Col contains all 9 numbers
3. Box contains all 9 numbers

There are 9 boxes of 3x3 in the grid.

Here is a box:
   0 1 2
 -------
0| a b c
1| d e f
2| g h i

And here is a grid of 9 boxes:

B1 B2 B3
B4 B5 B6
B7 B8 B9

Strategy:

Visit blank cells in B1. If a number is not represented, then determine which rows
and cols are not covered by adjacent ones. For example, look at these criteria:
1. Suppose B1 contains a 1, 2, but not a 3.
2. Cols 0, 1 are covered by 3's in B4 and B7
3. Rows 0, 2 are covered by 3's in B2 + B3
4. d + e are not blank

... then f = 3.

Pick off all the easy ones first. There will come a point where there are no
definitive answers. The next phase is to walk through the boxes looking for unset cells
and guess the values not yet set in the box. We make a copy of the grid, add the guess,
and then call the brute force solve routine described above. By having one item
guessed, we usually will solve most puzzles. There are a few that require 2 levels
of guessing but that is not yet implemented. Other ideas:

We will need to use the stack (recursive calls). Key is to
pass a very small amount of context state to the next layer of the permutations.
Maybe instead of a multidimensional data struct we pass in a simple array? this
may make it reasonable to try a brute force.

Treat like a maze? Have a definitive, methodical flood fill algorithm and just
attempt to fill in stuff in order. Maybe there are multiple answers to a given
puzzle. It would be interesting to solve it and then determine the moves in
order that gets you there. Give out hints.

 */

#include <stdio.h>
#include <strings.h>
#include <stdlib.h>

#define MAX_DIGITS     9
#define MAX_BOARD_ROWS 9
#define MAX_BOARD_COLS 9

#define MAX_SOLNS      100

#define MAXPATHLEN     1024     // this must be defined in a hdr file.
#define DEFAULT_PATH   "boards" // relative to CWD where launched.
#define DEFAULT_FILE   "b1"
#define MAX_LINE       200

#define xyz 54

#define ERROR   -1
#define SUCCESS 1
#define FALSE   0
#define TRUE    1

// STATS
/*
0: nSetVal - # times setval() called trying to solve board. s/b [4] + [5].
1: initBoxConstraints - each time a cell is initialized. Should be mult. of 81.
2: initBoxConstraints - # maybes found.
3: solveByConstraints - # PASS 1 + PASS 2 iterations in the do { .. } loop.
4: solveByConstraints - # PASS 1 setVal calls due to only one poss. val for a cell.
5: solveByConstraints - # PASS 2 setVal calls due to box uniqueness.
6: huntForSolutions - # guesses made total
7: huntForSolutions - # recursive calls (to self)
8: huntForSolutions - current recursion depth (will tell us depth when soln found).
9: huntForSolutions - sum(recursionDepth) for each guess. <-- what lvl of recursion work was performed.
10: times we short-circuited because box's last cell unsettable.
11: times we short-circuited a search for being too deep.
12: bool_t: board solved?
*/
#define NUM_STATS 13

typedef enum _statType_e {
    st_solved = 0,
    st_nSetVal,
    st_cellInits,
    st_nMaybes,
    st_P1P2_i,
    st_P1_setval,
    st_P2_setval,
    st_totGuess,
    st_nRecurse,
    st_recDepth,
    st_sumRecurse,
    st_bailBox8,
    st_bailTooDeep,
    st_XXX,
    st_numItems,
} statType_e;

typedef unsigned char bool_t;

typedef enum setval_en {
    setvalFalse = 0,
    setvalConstrained,
    setvalSeed,
    setvalNumItems
} setval_e;

typedef struct _cell_t {
	int      val;     // 0=blank, 1-9=answer
    setval_e justSet;
    int      numMaybe;
    int      maybe[MAX_DIGITS + 1]; // determine as we go. Then consult this .. no linear searches.
} cell_t;

// XXX - not used?
typedef struct _has_t {
    bool_t has[MAX_DIGITS + 1];
} has_t;

typedef struct _box_t { // we pass this around
    int    b; // which box is this?
    int    xoff;
    int    yoff;
    int    adjx1;
    int    adjx2;
    int    adjy1;
    int    adjy2;
    bool_t has[MAX_DIGITS + 1];
    int    multiCandidatesRow;
    int    multiCandidatesCol;
} box_t;

typedef struct _move_t {
    int x;
    int y;
    int t;
} move_t;

typedef struct _board_t {
    bool_t toast;
    int    nSetVal;
    int    stats[st_numItems];
    move_t lastSeed;
    has_t  row[MAX_BOARD_ROWS]; // tracks numbers in rows
    has_t  col[MAX_BOARD_COLS]; // tracks numbers in cols
    box_t  box[MAX_DIGITS + 1]; // dim should be sqrt(NUM_BOARD_ROWS * NUM_BOARD_COLS)
    cell_t grid[MAX_BOARD_ROWS][MAX_BOARD_COLS];    // the actual board
} board_t;

#endif
