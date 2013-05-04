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

Visit blank cells in B1. If a number is not represented, then determine which rows and cols
are not covered by adjacent ones. For example, look at these criteria:
1. Suppose B1 contains a 1, 2, but not a 3.
2. Cols 0, 1 are covered by 3's in B4 and B7
3. Rows 0, 2 are covered by 3's in B2 + B3
4. d + e are not blank

... then f = 3.

Pick off all the easy ones first. There will come a point where there are no
definitive answers. We will need to use the stack (recursive calls). Key is to
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

#define MAX_DIGITS     9
#define MAX_BOARD_ROWS 9
#define MAX_BOARD_COLS 9

#define MAXPATHLEN     1024
#define DEFAULT_PATH   "boards"
#define DEFAULT_FILE   "b1"
#define MAX_LINE       200

#define xyz 54

#define ERROR   -1
#define SUCCESS 1
#define FALSE   0
#define TRUE    1

typedef unsigned char bool_t;

typedef enum setval_en {
    setvalFalse = 0,
    setvalConstrained,
    setvalSeed,
    setvalNumItems
} setval_e;

typedef struct _cell_t {
	int val;     // 0=blank, 1-9=answer, 10=blank, 11-19=attempt.
    setval_e justSet;
    int numMaybe;
    int maybe[MAX_DIGITS + 1]; // determine as we go. Then consult this .. no linear searches.
} cell_t;

typedef struct _has_t {
        bool_t has[MAX_DIGITS + 1];
} has_t;

typedef struct _board_t {
		bool_t solved;
		has_t row[MAX_BOARD_ROWS]; // tracks numbers in rows
		has_t col[MAX_BOARD_COLS]; // tracks numbers in cols
		has_t box[MAX_DIGITS + 1]; // tracks numbers in boxes
		cell_t grid[MAX_BOARD_ROWS][MAX_BOARD_COLS];    // the actual board
} board_t;

#endif
