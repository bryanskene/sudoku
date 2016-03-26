#include "sudoku.h"

/*
Gonzo-style log of my project.

3/26/16: testing 1 2 3

4/20/13: started as exercise to unrust my programming skills. I like sudoku on airplanes, but my brain continuously nags at me to write this program and "discover" the algorithms needed to solve all sudoku boards.

4/30/13: able to solve puzzles based on the simple constraints (unique occurance in col, row, box)

04/30/13: start thinking about solution for problems with no definite first move based on constraints. Found such puzzles labeled "Evil" at http://www.websudoku.com/?level=4.

5/4/13: Victory over the "Evil" puzzles! Added some brute force: walk the unset cells considering values that are not constrained (unique in box, row, col) as candidate SEED VALUES. Try a seed, then pass the board to the same algorithm to see if it solves. If not, reset the cell to zero and move to the next cell with a legit seed, try again. NOTE: I can't help think there has to be a class if even more evil start states. And also, might there be multiple legit solutions to a given starting state -- surely yes. NOTE2: project unrust is working ... starting to love the wikipedia articles on Turing machines, proofs, etc., (though I'm not able to explain it cold, I'm having fun following along).

5/7/13: First version that actually solves all puzzles entered. Only finds the first solution ... it seems obvious that any puzzle may have multiple solutions. Especially the harder ones.

1/13/15: Picked it up again, got it working! Cleaned up the printBoard() function. Able to solve everything again. Recursion is interesting. I discovered that using the minimum -r for maxDepth that nets a solution is also netting the solution a.s.a.p., in a minimum i_recurse. Q: is there a way to generate ALL the possible solutions? And analyze how the algorithm can seek them out most efficiently?

NEXT:
0. applyAdvancedConstraints();
1. Find all possible solutions and characterize them. See if there is something called an "optimal" solution.
2. Write a puzzle generator from a randomized solution, and see if I can dial in on difficulty based on characteristics.
3. Feed the puzzle generator into the puzzle solver, accumulate some stats. This might be an interesting benchmark for a computer's performance. Keep code extremely portable.
4. Port to Python (maybe look at Ruby or Lua). Curses-based UI in python.
5. Port to Golang, then maybe Java or C++. Try a different modern edirot? VDE?
6. Port to JS and run in node.js, pass board in over REST/json, results back to browser?

CLEANUP:
1. Move this log out of the .c file.
2. Consistent usage of the fpIn and fpOut. Why pass this all over the place?
3. See all XXX comments.
4. Modularize - single-page or less functions, remove repetitive nested loops if any.
5. Robust input - able to read in boards in various formats.
6. Board printed at the end to stdout is incorrect if recursion took place to solve.

*/

int     numSoln;
board_t soln[MAX_SOLNS]; // detect up to 100 soln's ... probably overkill.
board_t *pB_soln;  // first soln found; used to short circuit some guessing.

char   pathname[MAXPATHLEN+1]; // use the system-defined MACRO instead
char   fnIn[MAXPATHLEN+1];
char   fnOut[MAXPATHLEN+1];
char   fnSoln[MAXPATHLEN+1];
FILE   *fpIn;
FILE   *fpOut;
FILE   *fpSoln;
long   maxDepth;
int    maxSolns;
int    minSetVal; // cheapest answer so far.
int    maxSetVal; // most expensive answer so far.
int    debugLevel;

/* ------------------------------------------------------------------------- */
int whichBox(int x, int y)
{
        int majorRow, majorCol;
        int b = 0;

        // x, y come in as 0 .. 8

        // Each box is a 3 x 3 grid of numbers
        // So we compute box as this:
        // 0 1 2
        // 3 4 5
        // 6 7 8
        majorCol = x / 3;
        majorRow = y / 3;

        // int math should just truncate remainders
        b = 3 * majorRow + majorCol;

// printf("whichBox(x=%d, y=%d) = box %d\n", x, y, b);

        return(b);
}

/* ------------------------------------------------------------------------- */
int initBoxes(box_t *boxes)
{
    int b;
    box_t *pBox = NULL;
    char *pFN = "initBoxes";

    for(b = 0; b < MAX_DIGITS; b++) {
        pBox = &boxes[b];
        pBox->b = b;

        // set xoff
        switch(b) {
                case 0:
                case 3:
                case 6:
                        pBox->xoff = 0;
                        break;
                case 1:
                case 4:
                case 7:
                        pBox->xoff = 3;
                        break;
                case 2:
                case 5:
                case 8:
                        pBox->xoff = 6;
                        break;
                default:
                        pBox->xoff = 0;
                        return(ERROR);
        }

        // set yoff
        switch(b) {
                case 0:
                case 1:
                case 2:
                        pBox->yoff = 0;
                        break;
                case 3:
                case 4:
                case 5:
                        pBox->yoff = 3;
                        break;
                case 6:
                case 7:
                case 8:
                        pBox->yoff = 6;
                        break;
                default:
                        pBox->yoff = 0;
                        return(ERROR);
        }

        // set adjacent boxes
        switch(b) {
                case 0:
                        pBox->adjx1 = 1;
                        pBox->adjx2 = 2;
                        pBox->adjy1 = 3;
                        pBox->adjy2 = 6;
                        break;
                        
                case 1:
                        pBox->adjx1 = 0;
                        pBox->adjx2 = 2;
                        pBox->adjy1 = 4;
                        pBox->adjy2 = 7;
                        break;
                case 2:
                        pBox->adjx1 = 0;
                        pBox->adjx2 = 1;
                        pBox->adjy1 = 5;
                        pBox->adjy2 = 8;
                        break;
                case 3:
                        pBox->adjx1 = 4;
                        pBox->adjx2 = 5;
                        pBox->adjy1 = 0;
                        pBox->adjy2 = 6;
                        break;
                case 4:
                        pBox->adjx1 = 3;
                        pBox->adjx2 = 5;
                        pBox->adjy1 = 1;
                        pBox->adjy2 = 7;
                        break;
                case 5:
                        pBox->adjx1 = 3;
                        pBox->adjx2 = 5;
                        pBox->adjy1 = 1;
                        pBox->adjy2 = 7;
                        break;
                case 6:
                        pBox->adjx1 = 7;
                        pBox->adjx2 = 8;
                        pBox->adjy1 = 0;
                        pBox->adjy2 = 3;
                        break;
                case 7:
                        pBox->adjx1 = 6;
                        pBox->adjx2 = 8;
                        pBox->adjy1 = 1;
                        pBox->adjy2 = 4;
                        break;
                case 8:
                        pBox->adjx1 = 6;
                        pBox->adjx2 = 7;
                        pBox->adjy1 = 2;
                        pBox->adjy2 = 5;
                        break;
                default:
                        pBox->adjx1 = -1;
                        pBox->adjx2 = -1;
                        pBox->adjy1 = -1;
                        pBox->adjy2 = -1;
                        return(ERROR);
        }

    }
//printf("%s: box=%d (%d, %d)\n", pFN, b, *pX, *pY);
        return(SUCCESS);

}

/* ------------------------------------------------------------------------- */
int initBoard(board_t *pB)
{
    if(!pB) return(ERROR);

    memset(pB, 0, sizeof(board_t));

    //pB->stats[st_solved] = FALSE;

    initBoxes(pB->box);

    return(SUCCESS);
}

/* ------------------------------------------------------------------------- */
int unsetVal(board_t *pB, int x, int y)
{
        int val;
        char *strFN = "unsetVal";
        cell_t *pCell = NULL;

        if(!pB) return(ERROR);

        pCell = &(pB->grid[x][y]);

        val = pCell->val;
        if(!val) {
            printf("%s: can't unset cell(%d, %d) already of val == 0\n", strFN, x, y);
            return(ERROR);
        }
        else {
            printf("%s: unset cell(%d, %d) val == %d\n", strFN, x, y, val);
        }

#define UNSET_HAS(foo) if(foo.has[val]) { foo.has[val] = FALSE; foo.has[0]--; }

        pCell->val = 0;

        UNSET_HAS(pB->row[y]);
        UNSET_HAS(pB->col[x]);
        UNSET_HAS(pB->box[whichBox(x, y)]);

#ifndef UNSET_HAS
        if(pB->row[y].has[val]) { pB->row[y].has[val] = FALSE; pB->row[y].has[0]--; }
        if(pB->col[x].has[val]) { pB->col[x].has[val] = FALSE; pB->col[x].has[0]--; }
        pB->box[whichBox(x, y)].has[val] = FALSE;
#endif

        return(SUCCESS);
}

/* ------------------------------------------------------------------------- */
int setVal(board_t *pB, int x, int y, int val)
{
        char *strFN = "setVal";
        cell_t *pCell = NULL;

        if(!pB) return(ERROR);
        if(!val) {
//            printf("%s: Error. x, y = (%d, %d), val = %d\n", strFN, x, y, val);
            return(ERROR);
        }

        pCell = &(pB->grid[x][y]);

        // Why would we call this function if a value was already set?
        if(pCell->val != 0) {
                printf("%s: x, y = (%d, %d), val = %d", strFN, x, y, val);
return(ERROR);

                // undo constraints?
                unsetVal(pB, x, y);
                return(ERROR);
        }

        // Now set the value and also the constraints
        pCell->val = val;

        pB->row[y].has[val] = TRUE; pB->row[y].has[0]++;
        pB->col[x].has[val] = TRUE; pB->col[x].has[0]++;
        pB->box[whichBox(x, y)].has[val] = TRUE; pB->box[whichBox(x, y)].has[0]++;

        return(SUCCESS);
}



/* ------------------------------------------------------------------------- */
int readBoard(board_t *pB)
{
    char line[MAX_LINE+1];
    int y = 0;
    int n = 0;
    int n1, n2, n3, n4, n5, n6, n7, n8, n9;

    if(!fpIn) {
            // No input file found
            printf("Ooops, can't find input file %s\n", fnIn);
            return(ERROR);
    }

    while(fgets(line, MAX_LINE, fpIn) != NULL) {
        printf("Line #%d: %s\n", y+1, line);
        n = sscanf(line, "%d %d %d %d %d %d %d %d %d",
                        &n1,
                        &n2,
                        &n3,
                        &n4,
                        &n5,
                        &n6,
                        &n7,
                        &n8,
                        &n9
                  );


        if(n < MAX_BOARD_ROWS) {
            // skip this row ... input file has blanks
            printf("--- this line has less than %d input values\n",
                            MAX_BOARD_ROWS);

        }
        else {
            setVal(pB, 0, y, n1);
            setVal(pB, 1, y, n2);
            setVal(pB, 2, y, n3);
            setVal(pB, 3, y, n4);
            setVal(pB, 4, y, n5);
            setVal(pB, 5, y, n6);
            setVal(pB, 6, y, n7);
            setVal(pB, 7, y, n8);
            setVal(pB, 8, y, n9);
            y++;
        }
    }
    fclose(fpIn);
    return(SUCCESS);
}

/* ------------------------------------------------------------------------- */
int printBoardLineH(FILE *fpOut, int length, char *strSuffix)
{
    int i;
        
    fprintf(fpOut, "       ");
    for(i = 0; i < length; i++) fprintf(fpOut, "-");

    fprintf(fpOut, "|");

    if(strSuffix) {
        fprintf(fpOut, " %s", strSuffix);
    }

    fprintf(fpOut, "\n");

    return(SUCCESS);
}

/* ------------------------------------------------------------------------- */
int printStats(FILE *fpOut, board_t *pB)
{
    int i;
    int *stats;

    static const char statsLabel[st_numItems][30] = {
        "solved",
        "nSetVal",
        "cellInits",
        "nMaybes",
        "P1P2_i",
        "P1_setval",
        "P2_setval",
        "totGuess",
        "nRecurse",
        "recDepth",
        "sumRecurse",
        "bailBox8",
        "bailTooDeep",
        "XXX",
    };

    if(debugLevel && pB->stats[st_nSetVal] > 0) {

        // header row in one line
        if(debugLevel > 1) {
            fprintf(fpOut, "DBG:\tBoard");
            for(i = 0; i < st_numItems; i++) {
                fprintf(fpOut, "\t%s", statsLabel[i]);
            }
            fprintf(fpOut, "\n");
        }

        // stats, tab-delimited, to be grep'd out for analysis.
        fprintf(fpOut, "DBG:\t%s", fnOut);
        for(i = 0; i < st_numItems; i++) {
            fprintf(fpOut, "\t%d", pB->stats[i]);
        }
        fprintf(fpOut, "\n");
    }
    else {

        printBoardLineH(fpOut, 4 * MAX_BOARD_COLS, "Stats ...");

        fprintf(fpOut, "nSetVal: (min, this, max) = (%d, %d, %d)\n",
                minSetVal, pB->stats[st_nSetVal], maxSetVal);

        for(i = 0; i < st_numItems; i++) {
            fprintf(fpOut, "%8s ", statsLabel[i]);
        }
        fprintf(fpOut, "\n");
        for(i = 0; i < st_numItems; i++) {
            fprintf(fpOut, "-------- ");
        }
        fprintf(fpOut, "\n");
        for(i = 0; i < st_numItems; i++) {
            fprintf(fpOut, "%8d ", pB->stats[i]);
        }
        fprintf(fpOut, "\n");
    }

    return(SUCCESS);
}

/* ------------------------------------------------------------------------- */
int printBoard(FILE *fpOut, board_t *pB, bool_t debug, bool_t printJustSet)
{
        int x, y, i, d, b;
        box_t *pBox = NULL;
        cell_t *pCell = NULL;
        char majorDelim = '|';
        char minorDelim = ' ';

        printStats(fpOut, pB);

        for(y = 0; y < MAX_BOARD_ROWS; y++) {
            if(y % 3 == 0) {
                // each col has pipe-space-digit-space (== 4 chars)
                printBoardLineH(fpOut, 4 * MAX_BOARD_COLS,
                        (debug && y == 0) ? "The Row Has ..." : "");
            }

            fprintf(fpOut, "row: %d ", y);

            for(x = 0; x < MAX_BOARD_COLS; x++) {
                    char delimiter;
                    char delim1;
                    char delim2;

                    pCell = &(pB->grid[x][y]);

                    delimiter = (x % 3) ? minorDelim : majorDelim;
                    delim1 = ' '; delim2 = ' ';

                    if(pCell->val) {

                        if(printJustSet) {
                            switch(pCell->justSet) {
                                case setvalConstrained:
                                    delim1 = '<'; delim2 = '>'; break;

                                case setvalSeed:
                                    delim1 = '{'; delim2 = '}'; break;

                                case setvalFalse:
                                default:
                                    delim1 = ' '; delim2 = ' '; break;
                            }
                        }

                        fprintf(fpOut, "%c%c%d%c", delimiter, delim1, pCell->val, delim2);
                    }
                    else {
                            fprintf(fpOut, "%c . ", delimiter);
                    }
            }

            // Print out the HAS for the ROW
            if(debug) {
                fprintf(fpOut, "|");
                // we stuff the count of how many digits in has[0]
                fprintf(fpOut, " (%d)  ", pB->row[y].has[0]);

                // For 8, Has[8], not Has[8-1]. Start at 1 ...
                for(i = 1; i < MAX_DIGITS + 1; i++) {
                    if(pB->row[y].has[i]) {
                        fprintf(fpOut, "%d  ", i);
                    }
                    else {
                        //fprintf(fpOut, " .");
                        fprintf(fpOut, "   ");
                    }
                }
                fprintf(fpOut, " ");
            }
            fprintf(fpOut, "|\n");
        }
        printBoardLineH(fpOut, 4 * MAX_BOARD_COLS, (debug) ? "The BOARD Has ..." : "");

    // Print out the HAS for the COLumns
    if(debug) {
        for(d = 0, b = 0; d < MAX_DIGITS+1; d++, b++) {

            // Columns
            fprintf(fpOut, " ");
            // we stuff the count of how many digits in has[0]
            fprintf(fpOut, " n=%d:  ", d);
            for(x = 0; x < MAX_BOARD_COLS; x++) {
                if(d == 0) {
                    fprintf(fpOut, "(%d) ", pB->col[x].has[d]);
                }
                else if(pB->col[x].has[d] != 0) {
                    fprintf(fpOut, " %d  ", d);
                }
                else {
                    fprintf(fpOut, " .  ");
                }
            }

            // Boxes
            pBox = &(pB->box[b]);
            fprintf(fpOut, "B%d (%d): ", pBox->b, pBox->has[0]);

            for(i = 1; i < MAX_DIGITS+1; i++) {
                if(pBox->has[i]) {
                    fprintf(fpOut, "%d  ", i);
                }
                else {
                    fprintf(fpOut, ".  ");
                }
            
            }
            fprintf(fpOut, "\n");
        }
    }

    return(SUCCESS);
}

/* ------------------------------------------------------------------------- */
int addSoln(board_t *pB)
{
    int i, x, y;
    int nSetVal; // just a temp
    bool_t different = FALSE;

    // ensure this is a unique soln
    for(i = 0; i < numSoln; i++) {

//        if(memcmp(&(soln[i].grid), pB->grid, sizeof(pB->grid)) == 0) {

        for(x = 0; x < MAX_BOARD_COLS && !different; x++) {
            for(y = 0; y < MAX_BOARD_ROWS && !different; y++) {
                if(pB->grid[x][y].val != soln[i].grid[x][y].val) {
                    different = TRUE;
                    break;
                }
            }
        }

        if(!different) {
            fprintf(fpOut, "addSoln: we've seen this solution before!\n");
            printBoard(fpOut, pB, TRUE, TRUE);
            return(ERROR);
        }
    }

    // OK, append to the list
    memcpy(&(soln[numSoln]), pB, sizeof(board_t));
    numSoln++;

    nSetVal = pB->stats[st_nSetVal];

    if(!pB_soln) {
        pB_soln = &(soln[0]);

        minSetVal = maxSetVal = nSetVal;
    }
    else {
        if(minSetVal > nSetVal) minSetVal = nSetVal;
        if(maxSetVal < nSetVal) maxSetVal = nSetVal;
    }

    printf("addSoln: got soln #%d\n", numSoln);
    printBoard(stdout, pB, TRUE, TRUE);

    return(SUCCESS);
}

/* ------------------------------------------------------------------------- */
/*

   This feels kinda lame, traversing all cells (1 + 9 + 1).
   times in each box. Must be better ways!

do {
  gotOne = FALSE;
  Foreach box b
    Traverse the 9 cells of the box (for x, y)
      set cell.maybe[0 .. 9] = FALSE; cell.numMaybe = 0;
      unless cell.justSet == setvalSeed, set cell.justSet = setvalFalse;
    Foreach value v where !b.has[v]
      Traverse the 9 cells of the box (for x, y)
        If value not set in row[y] and col[x]
          Then v is a possibility for the cell x, y; cell.maybe[v] = TRUE;
    Traverse the 9 cells of the box (for x, y)
      if cell.val == 0 && numMaybe == 1
        v = non-zero item in cell.maybe
        set cell.val = v;
        set justSet = setvalConstrained;  // highlight what was just found in each iteration
        set gotOne = TRUE;

  // At the end of traversing all 9 boxes
  Print the board but <bracket> the values just set. (DEBUG)

  // now iterate until gotOne does not get set 
} while(gotOne);

*/

/* ------------------------------------------------------------------------- */
int initBoxConstraints(board_t *pB, box_t *pBox, int *i2, int *i3)
{
    int x, y, v;
    int xoff, yoff;
    cell_t *pCell = NULL;

    xoff = pBox->xoff;
    yoff = pBox->yoff;

    // Initialization of box for an iteration
    // traverse the cells of the box
    for(x = 0; x < 3; x++) {
        for(y = 0; y < 3; y++) {
(*i2)++;
            pCell = &(pB->grid[x + xoff][y + yoff]);
            if(pCell->justSet != setvalSeed) {
                pCell->justSet = setvalFalse;
            }
            pCell->numMaybe = 0;
            memset(pCell->maybe, 0, (MAX_DIGITS+1)*sizeof(int));
        }
    }
    // Find some possibilities ...
    // For each v not set in box, look at rows & cols,
    // and if not set it is a possibility.
    for(v = 1; v < MAX_DIGITS + 1; v++) {
        if(!pBox->has[v]) {
            for(x = 0; x < 3; x++) {
                for(y = 0; y < 3; y++) {
                    pCell = &(pB->grid[x + xoff][y + yoff]);
                    if(!pCell->val
                    && !(pB->row[y + yoff].has[v])
                    && !(pB->col[x + xoff].has[v])) {
(*i3)++;
                        pCell->numMaybe++;
                        pCell->maybe[v] = TRUE;
                    }
                }
            }
        }
    }

    return(SUCCESS);
}

/* ------------------------------------------------------------------------- */
int disallowValInRow(board_t *pB, box_t *pBox, int row, int v)
{
    int x, y, xoff, yoff;
    cell_t *pCell = NULL;

    xoff = pBox->xoff;
    yoff = pBox->yoff;

    y = row;
    for(x = 0; x < 3; x++) {
        pCell = &(pB->grid[x + xoff][y + yoff]);
        if(pCell->maybe[v]) {
            pCell->numMaybe--;
            pCell->maybe[v] = FALSE;
        }
    }

    return(SUCCESS);
}

/* ------------------------------------------------------------------------- */
int disallowValInCol(board_t *pB, box_t *pBox, int col, int v)
{
    int x, y, xoff, yoff;
    cell_t *pCell = NULL;

    xoff = pBox->xoff;
    yoff = pBox->yoff;

    x = col;
    for(y = 0; y < 3; y++) {
        pCell = &(pB->grid[x + xoff][y + yoff]);
        if(pCell->maybe[v]) {
            pCell->numMaybe--;
            pCell->maybe[v] = FALSE;
        }
    }

    return(SUCCESS);
}

/* ------------------------------------------------------------------------- */
// There are secondary constraints we can apply.
//
// XXX - THIS MAY BE COMPLETELY UNECESSARY, and it is not proven to me whether
// it actually optimizes the work required to get a solution. Once multiple sol'ns
// can be found and compared, add this back to see the affect.
// Till then DO NOT CALL THIS.
//
int applyAdvancedConstraints(board_t *pB, box_t *pBox, int *i6, int *i7)
{
    int x, y, v;
    int xoff, yoff;
    int lastRow = 0;
    int lastCol = 0;
    int numRows = 0;
    int numCols = 0;
    cell_t *pCell = NULL;

    // 1. If the only possibilities for a val in box are in one row or col, then 
    // unset maybe[val] in adjacent boxes along the row or col.
    // In other words, if we can limit a val to one row or col in a box, then we
    // can rule that val out for the rest of the row or col in the adjacent boxes.
    //
    // EX: if B1 has no 5, and only R1 has room in B1 for a 5,
    // then disallow a 5 in all the cells of B2 and B3 in R1. B1 R1 will have a 5.

    // XXX - note that this could be done in the pass above, in basic constraints
    //       but I'm isolating it here for readability.
    //
    // XXX - not implementing the below yet ...
    // 2. OK, last crazy idea = use some sort of a mask and apply a matrix to the box.
    // wouldn't it be faster to mask off (or surgically factor in) just the cells that
    // form a certain pattern? We do have random access memory here folks. Oh, don't
    // forget about folding bits over .. a UINT is 32 bits dude. Binary coded decimal?

    xoff = pBox->xoff;
    yoff = pBox->yoff;

    for(v = 1; v < MAX_DIGITS + 1; v++) {
        if(!pBox->has[v]) {
            // detect if #rows or #cols where this val could occur is just one for this box.
            lastRow = 0;
            lastCol = 0;
            numRows = 0;
            numCols = 0;

            // ROWS
            // count possible rows of board for this v
            for(y = 0; y < 3; y++) {
                if(!(pB->row[y + yoff].has[v])) { // i.e. row doesn't have v yet
                    lastRow = y;
                    numRows++;
                }
            }
            if(numRows == 1) {
                // unset maybe for all cells in x-adjacent boxes for row`
                disallowValInRow(pB, &(pB->box[pBox->adjx1]), lastRow, v);
                disallowValInRow(pB, &(pB->box[pBox->adjx2]), lastRow, v);
(*i6)++;
            }

            // COLUMNS
            // count possible columns of board for this v
            for(x = 0; x < 3; x++) {
                if(!(pB->col[x + xoff].has[v])) { // i.e. col doesn't have v yet
                    lastCol = x;
                    numCols++;
                }
            }
            if(numCols == 1) {
                // unset maybe for all cells in y-adjacent boxes for column
                disallowValInCol(pB, &(pB->box[pBox->adjy1]), lastCol, v);
                disallowValInCol(pB, &(pB->box[pBox->adjy2]), lastCol, v);
(*i7)++;
            }
        }
    }

    return(SUCCESS);
}

/* ------------------------------------------------------------------------- */
// Take a seeded board, play out all constraints till no next move w/o guessing.
//
// NOTE: this will get called with many different copies of the board ...
//
// If soln found, archive it.
//
bool_t solveByConstraints(board_t *pB_in, bool_t copyAnswer)
{
    int     b, i, j, x, y, xoff, yoff, v;
    board_t board_temp;
    board_t *pB = NULL;
    box_t   *pBox = NULL;
    cell_t  *pCell = NULL;
    bool_t  gotOne = FALSE;
    bool_t  setOrDieFuse = FALSE;
    char    *strFN = "solveByConstraints";

    //memset(pB->stats, 0, sizeof(pB->stats));
    pB = &board_temp; // work with temp board
    memcpy(pB, pB_in, sizeof(board_t));

    pB->toast = FALSE;

    do {
pB->stats[st_P1P2_i]++;
fprintf(fpOut, "ITERATION #%d\n", pB->stats[st_P1P2_i]);

        gotOne = FALSE;

        // For each box b, initialize constraints
        for(b = 0; b < MAX_DIGITS; b++) {
            pBox = &(pB->box[b]);
            xoff = pBox->xoff;
            yoff = pBox->yoff;

            // PASS #1: if only one possible answer in a cell ...
            
            // Set up the has[] and the possibilities (maybe) per cell.
            initBoxConstraints(pB, pBox, &pB->stats[st_cellInits], &pB->stats[st_nMaybes]);

            // (terminal: if a box has one spot left and we can't set it, board is toast)
            setOrDieFuse = (pBox->has[0] == 8) ? TRUE : FALSE;

            // Then look for any cell that has only one maybe. Easy case.
            for(x = 0; x < 3; x++) {
                for(y = 0; y < 3; y++) {
                    pCell = &(pB->grid[x + xoff][y + yoff]);

// fprintf(fpOut, "Box=%d, cell(%d, %d): numMaybe=%d, val=%d\n",
//          b, x+xoff, y+yoff, pCell->numMaybe, pCell->val);

                    if(pCell->val == 0 && pCell->numMaybe == 1) {
                        for(v = 1; v < MAX_DIGITS + 1; v++) {
                            if(pCell->maybe[v]
                            && !pBox->has[v]) { // must check; may have set peer cell

pB->stats[st_P1_setval]++; // PASS 1 setVal calls.

                                setVal(pB, x + xoff, y + yoff, v); //pCell->val = v;
                                pCell->justSet = setvalConstrained;
                                pCell->numMaybe = 0;
                                setOrDieFuse = FALSE;
                                gotOne = TRUE;
                            }
                        }
                    }
                }
            }

#define PASS2
#ifdef PASS2
            // XXX HERE applyAdvancedConstraints(pB, pBox, &pB->stats[X], &pB->stats[Y]);

            // PASS #2: box uniqueness constraint

            // for each value that has only one cell poss. in box, set it.
            for(v = 1; v < MAX_DIGITS + 1; v++) {
                cell_t *pCellFound = NULL;

                int xfound, yfound, numPoss;

                if (pBox->has[v]) continue;

                // traverse the cells of box, finding (and counting) candidate cells.
                numPoss = 0;
                for(x = 0; x < 3; x++) {
                    for(y = 0; y < 3; y++) {
                        pCell = &(pB->grid[x + xoff][y + yoff]);
                        if(pCell->maybe[v]) {
                            if(numPoss == 0) {
                                pCellFound = pCell;
                                // save these so we can call setVall if only one cell poss.
                                xfound = x + xoff;
                                yfound = y + yoff;
                            }
                            numPoss++;
                        }
                    }
                }

                // if there's only one cell then bingo.
                if(numPoss == 1) {

pB->stats[st_P2_setval]++; // PASS 2 setVal calls.

                    if(pCellFound->val == 0) {
                        setVal(pB, xfound, yfound, v); // pCellFound->val = v;
                        pCellFound->justSet = setvalConstrained;
                        pCellFound->numMaybe = 0;
                        setOrDieFuse = FALSE;
                        gotOne = TRUE;
                    }
                }
            }
#endif

            // If last cell in box can't be set, BAIL on this solution.
            if(setOrDieFuse) {

pB_in->stats[st_bailBox8]++; // counter of times last cell in box unsettable

                pB->toast = TRUE;
                fprintf(fpOut, "Last cell in box %d UNSETTABLE, bail out\n", b);
                printBoard(fpOut, pB, TRUE, TRUE);
                if(copyAnswer) memcpy(pB_in, pB, sizeof(board_t));
                return(FALSE);
            }
        }

        // At the end of traversing all 9 boxes

// Print the board but <bracket> the values just set. (DEBUG)
printBoard(fpOut, pB, FALSE, TRUE);
//printBoard(fpOut, pB, TRUE, TRUE);

        // now iterate until gotOne does not get set 

pB->stats[st_nSetVal] += pB->stats[st_P1_setval] + pB->stats[st_P2_setval];

        if(minSetVal && pB->stats[st_nSetVal] > 100*minSetVal) {

pB_in->stats[st_bailTooDeep]++; // num times bailed out of a solution for being too expensive.

            // We are bailing out early; this search for a solution is 100x more
            // expensive than our best solution so far for this board.
            fprintf(fpOut, "=================\n");
            fprintf(fpOut, "%s: Failed ... search too deep, minSetVal=%d\n", strFN, minSetVal);
            fprintf(fpOut, "=================\n");
            pB->toast = TRUE;
            if(copyAnswer) memcpy(pB_in, pB, sizeof(board_t));
            return(FALSE);
        }

    } while(gotOne);

printStats(fpOut, pB);

    // Any unset cells left?
    for(x = 0; x < MAX_BOARD_COLS; x++) {
        for(y = 0; y < MAX_BOARD_ROWS; y++) {
            pCell = &(pB->grid[x][y]);
            if(pCell->val == 0) {
                fprintf(fpOut, "=================\n");
                fprintf(fpOut, "%s: Failed to solve just on constraints\n", strFN);
                fprintf(fpOut, "=================\n");
                if(copyAnswer) memcpy(pB_in, pB, sizeof(board_t));
                return(FALSE);
            }
        }
    }

    // If we get here we didn't find any cell with a val == 0, so board is solved

pB->stats[st_solved] = 1;

    fprintf(fpOut, "=================\n");
    fprintf(fpOut, "%s: SOLVED\n", strFN);
    fprintf(fpOut, "=================\n");
    addSoln(pB);
    if(copyAnswer) memcpy(pB_in, pB, sizeof(board_t));
    return(TRUE);
}

/* ------------------------------------------------------------------------- */
// Given the last move attempted (i1), find the next one and store in (i2)
// If applyMove, then also store in (i1) and set the value in the grid. (i.e. iterate)
/*
   THE HEURISTIC to trim our tree:
   The guess function needs to search all known solutions to not repeat the value.
   This ensures we are always trying a different solution.
   Makes the guess finding much more complex, but allows for it to short cicuit an
   exhastive walk which would take 13B years to get .1% complete.
   If it didn't, it would brute-force search for a repeat solution.

   XXX - will this work after I've stored 2+ soln's? guesses can't be ruled out across
   multiple soln's, just one. So if I just use the first sol'n to rule out guesses, we can
   find all slight variances on the first sol'n. This is what we will start with. May
   not trim tree aggressively enough to reduce from NP-complete to NP-hard?
*/
cell_t *nextSeed(board_t *pB)
{
    int x, y, t;
    int x0, y0, t0;

    move_t *pMV;

    pMV = &(pB->lastSeed);

    // start with very next possible move.
    x0 = pMV->x;
    y0 = pMV->y;
    t0 = pMV->t + 1;

    // if needed, wrap test value, and inc x.
    if(t0 >= MAX_DIGITS) {
        t0 = 1;
        x0++;
    }

    // if needed, wrap x, and inc y.
    if(x0 >= MAX_BOARD_COLS) {
        x0 = 0;
        y0++;
        t0 = 1; // and this should be totally unecessary.
    }

    // if y goes off bottom, DONE.
    if(y0 >= MAX_BOARD_ROWS) {
        return(FALSE);
    }

    // search for the next legal move.
    for (y = y0; y < MAX_BOARD_ROWS; y++) {
        for (x = x0; x < MAX_BOARD_COLS; x++) {
            int box;
            cell_t *pCell;

            box = whichBox(x, y);
// HERE - try keeping guesses in outter corners of boxes
// if(!(box == 0 || box == 2 || box == 4 || box == 6 || box == 8)) continue;
//if(!(box == 0)) continue;

            pCell = &(pB->grid[x][y]);

            if(pCell->val == 0) {

                // start with next value to test ...
                for(t = t0; t < MAX_DIGITS + 1; t++) {
                    // as long as not constrained, try t as a value
                    if(!pB->row[y].has[t]
                    && !pB->col[x].has[t]
                    && !pB->box[box].has[t]
                    ) {
                        if(pB_soln && pB_soln->grid[x][y].val == t) {
                            // Bail, see HEURISTIC
                            continue;
                        }

                        // i2 becomes the next move to make
                        pMV->x = x;
                        pMV->y = y;
                        pMV->t = t;

                        setVal(pB, x, y, t); //pCell->val = t;
                        pCell->justSet = setvalSeed;

//pB->stats[15]++;
//fprintf(fpOut, "nextSeed: seed attempt #%d -> (%d, %d) := %d\n", pB->stats[15], x, y, t);

                        return(pCell);
                    }
                }
                // after one iteration, start at beginning
                t0 = 1;
            }
        }
        // hit end of row, start at beginning
        x0 = 0;
    }

    // couldn't find a next guess, DONE.
    return(NULL);
}

/* ------------------------------------------------------------------------- */
/*
   Basic strategy: use constraints to get stuck, use a guess to unstick it, repeat.
   This will find 1 soln. To find more, you need to start with a guess first (that is
   not a repeat of a value of soln #1). This would get you a 2nd solution, if it exists.

   There are 3 cases:
   1. pB_in solves on just constraints w/o getting stuck; no recursion needed.
   2. PB_soln is NULL, and pB_in requires recursion to get the first soln.
   3. PB_soln holds 1st soln, so now we just want to walk the board to seed a starting
      point. In seeding, exclude any value that is part of the first soln.
      Beyond the top-level seed, proceed just as in case 2.

    1st time through, try constraints.
        Solved? This board has only one soln, return.
        Stuck?  This becomes starting point.
    Then guess seed.
    Try constraints again.
        If solved, archive it, return.
        If !solved: recurse to take an additive guess / soln attempt.
*/
bool_t huntForSolutions(board_t *pB_in)
{
    static int i_recurse = 0;
    static int recursionDepth = 0;

    int     i, box, t, x, y;
    cell_t  *pCell = NULL;
    board_t *pB_new;
    board_t boardNew;
    char *strFN = "huntForSolutions";

    fprintf(fpOut, "%s: i_recurse = %d, recursionDepth = %d\n",
            strFN, i_recurse, recursionDepth);

    if(recursionDepth >= maxDepth) {
        return(FALSE);
    }
    if(maxSolns && numSoln >= maxSolns) {
        return(FALSE);
    }

    if(!pB_soln
    && i_recurse == 0
    && solveByConstraints(pB_in, FALSE)) {
        // 1st time through; look for 1st soln. Case 1 or 2 hits this.
        // NOTE: this would also set pB_soln.
        return(TRUE);
    }

    if(pB_soln && recursionDepth == 0) {
        // 2nd time called (bec/ the first hunt was successful).
        // (I think we can drop into the stuff below w/o doing anything ...)
        //
        // if we got the first soln, then we need to start with a guess that is NOT
        // in the soln*. Then solveByContraints(), then get stuck, guess, the usual.
        // This will find all soln beyond the first one.
        //
        // * once there are 2 solns, may need to start over and exclude any guesses
        //   that match any existing discovered soln? How to prevent re-discovering
        //   the same one? Can't exclude value in one cell based on soln[0], and
        //   exclude a val in a different cell based on soln[1]. Think about this
        //   before attempting to find anything beyond the second soln.
    }

    pB_new = &boardNew;
    memcpy(pB_new, pB_in, sizeof(board_t));

    // not solved so look for first unset cell and try all the values
    i = 0;
    for (y = 0; y < MAX_BOARD_ROWS; y++) {
        for (x = 0; x < MAX_BOARD_COLS; x++) {

            // Find first unset cell in the grid ...
            box = whichBox(x, y);

            pCell = &(pB_new->grid[x][y]);

            if(pCell->val == 0) {

                for(t = 1; t < MAX_DIGITS + 1; t++) {

                    if(recursionDepth == 0
                    && pB_soln && pB_soln->grid[x][y].val == t) {
                        // in Case 3, hunting for subsequent soln, don't allow
                        // a top-level seed with anything from the 1st soln.
                        continue;
                    }

                    // as long as not constrained, try t as a value
                    if(!pB_in->row[y].has[t] // XXX - pB_new works, but slower (?)
                    && !pB_in->col[x].has[t]
                    && !pB_in->box[box].has[t]) {

pB_in->stats[st_totGuess]++; // counter of guesses made.

                        // start with input board
                        memcpy(pB_new, pB_in, sizeof(board_t));

                        // lay down a guess
                        setVal(pB_new, x, y, t); //pCell->val = t;
                        pCell->justSet = setvalSeed;
                        i++;
fprintf(fpOut, "%s: seed attempt #%d -> (%d, %d) := %d\n", strFN, i, x, y, t);

                        // try to solve with constraints
                        if(solveByConstraints(pB_new, TRUE)) {
                            return(TRUE);
                        }
                        else {
                            recursionDepth++;
                            if(recursionDepth < maxDepth) {
                                i_recurse++;

pB_new->stats[st_nRecurse]++; // counter of recursive calls.
pB_new->stats[st_recDepth] = recursionDepth; // saved for debugging.
pB_new->stats[st_sumRecurse] += recursionDepth; // measure of how deep the searching was.

                                if(huntForSolutions(pB_new)) {
                                    return(TRUE);
                                }
                                else {
                                    // save the stats we worked for ...
                                    memcpy(&(pB_in->stats),
                                            &(pB_new->stats), sizeof(pB_in->stats));
                                }
                            }
                            recursionDepth--;
                        }
                    }
                }
            }
        }
    }

    if(recursionDepth == 0) {
        // BUMMER ... so let's print out some stats on the way up.
        printStats(fpOut, pB_new);
        printStats(fpSoln, pB_new);
        printStats(stdout, pB_new);
    }

    return(FALSE);
}

/* ------------------------------------------------------------------------- */
    // XXX - put a while() { ... } around solveBoard(). Find all the possible solutions.
    // Maybe multiple solutions per recursion depth. Maybe also print file of just solutions
    // and key inputs / outputs per solution, like recursion depth, i_recurse needed,
    // nSetVal, etc. Just keep going per recursion depth till board is not solved.
    // Keep track of best nSetVal, and when a solution attempts to go above 100X
    // (or some cmdline arg) that value abandon it. <-- short circuit the tree!
    //
    // 1. Find all solutions (except ridiculous inefficient tree searches)
    // 2. Be able to compare them. Needs good metrics. nSetVal is a good start.
    // 3. Discover properties of the solutions, how to get them. Maybe manually?
    // 4. Discover optimal /success. Compare the solutions.

int findAllSolutions(board_t *pB_orig)
{
    board_t board_i;
    board_t *pB_i;
    int i = 0;

    // INIT moves
    memset(&(pB_orig->lastSeed), 0, sizeof(move_t));

    // INIT global stats
    pB_soln   = NULL;
    maxSolns  = MAX_SOLNS; // XXX - allow this to be overridden on cmd line (<100)
    minSetVal = 0;
    maxSetVal = 0;

    pB_i = &board_i;
    memcpy(pB_i, pB_orig, sizeof(board_t));

    // INIT: find a sol'n w/o guessing
    // (useful to short circuit the guesses for subsequent solns)
    if(solveByConstraints(pB_i, FALSE) == TRUE) {
        printf("---> able to solve without iterating\n");
    }
    else {
        // got the board to a "stuck" point .. which is a great place to start recursing.

        // BEGIN the hunt for all solutions. Recursive depth-first search.
        // Archives soln's found.

        printf("---> UNABLE to solve without iterating / guessing\n");

        memcpy(pB_i, pB_orig, sizeof(board_t)); // start clean, and avoid the 1st soln
        if(huntForSolutions(pB_i)) {

            printf("---> Found a first solution!\n");

            printf("---> Hunting for one more ...\n");

// XXX should we put a while() around the hunt to get 3+ soln? Think ...

            // calling 2nd time if pB_soln found may yield another one.
            // See case 3 ... logic in there to avoid repeat.
            memcpy(pB_i, pB_orig, sizeof(board_t));
            if(huntForSolutions(pB_i)) {
                printf("Found a second (or duplicate) solution!\n");
            }
            else {
                printf("---> NO second soln.\n");
            }
        }
        printf("huntForSolutions completed\n");
    }

    // Find "best" solutions per the stats. Search soln[] ...
    // HERE - need to write this code.

    // DUMP solutions.
    fprintf(fpSoln, "ORIGINAL board:\n");
    printBoard(fpSoln, pB_orig, FALSE, FALSE);
    fprintf(fpSoln, "\n");

    if(numSoln) {
        int j;
        board_t *pB_j;

        printf("CONGRATULATIONS!!! Found %d solutions:\n", numSoln);

        fprintf(fpSoln, "FOUND %d solutions:\n", numSoln);
        fprintf(stdout, "FOUND %d solutions:\n", numSoln);

        for(j = 0; j < numSoln; j++) {
            pB_j = &soln[j];

            fprintf(fpSoln, "soln[%d]: nSetVal=%d\n", j, pB_j->stats[st_nSetVal]);
            fprintf(stdout, "soln[%d]: nSetVal=%d\n", j, pB_j->stats[st_nSetVal]);

            // print solution to screen
            printBoard(fpSoln, pB_j, FALSE, FALSE);
            printBoard(stdout, pB_j, FALSE, FALSE);

            fprintf(fpSoln, "\n");
            fprintf(stdout, "\n");
        }
    }
    else {
        printf("BUMMER!!! Board was NOT solved.\n");
    }

    return(SUCCESS);
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */


/* ------------------------------------------------------------------------- */
FILE *openFileTry(FILE **fp, char *fargs, char *folder, char *leaf)
{
    char fn[MAXPATHLEN + 1];

    if(!fargs || !leaf) {
        printf("openFileTry: bad input!\n");
        *fp = NULL;
        return(NULL);
    }

    if(folder) {
        sprintf(fn, "%s/%s", folder, leaf);
    }
    else strcpy(fn, leaf);

    *fp = fopen(fn, fargs);

    if(fp) {
        printf("openFileTry: opened (%s) %s\n", fargs, fn);
    }
    else {
        printf("openFileTry: open FAILED (%s) %s\n", fargs, fn);
    }

    return(*fp);
}

/* ------------------------------------------------------------------------- */
int openFiles()
{
    // Try CWD and just the fnames
    if(openFileTry(&fpIn, "r", NULL, fnIn)) {
        openFileTry(&fpOut, "w", NULL, fnOut);
        openFileTry(&fpSoln, "w", NULL, fnSoln);
    }

    // Try path and fnames
    if(!fpIn) {
        if(openFileTry(&fpIn, "r", pathname, fnIn)) {
            openFileTry(&fpOut, "w", pathname, fnOut);
            openFileTry(&fpSoln, "w", pathname, fnSoln);
        }
    }

    if(!fpOut) {
        if(fpIn) fclose(fpIn);
        return(ERROR);
    }
    if(!fpSoln) {
        if(fpIn) fclose(fpIn);
        if(fpOut) fclose(fpOut);
        return(ERROR);
    }
    return(SUCCESS);
}

/* ------------------------------------------------------------------------- */
void printUsage()
{
    printf("SUMMARY: sudoku -d <pathname for boards> -if <input file> -of <output file> -b <board root fname>");
    printf("\n");
    printf("EXAMPLE: ./sudoku -if xxx.in -of xxx.out -sf xxx.soln\n");
    printf("EXAMPLE: ./sudoku -path boards -b xxx\n");
}

/* ------------------------------------------------------------------------- */
int procCmdLine(int argc, char *argv[])
{
    int i;

    strcpy(pathname, DEFAULT_PATH);
    sprintf(fnIn, "%s", DEFAULT_FILE);
    sprintf(fnOut, "%s.out", DEFAULT_FILE);
    sprintf(fnSoln, "%s.soln", DEFAULT_FILE);
    maxDepth = 0;
    debugLevel = 0;

    for (i = 1; i < argc; i++) {  /* Skip argv[0] (program name). */

        if(strncmp(argv[i], "-i", 2) == 0) {  /* Process optional arguments. */

            // The last argument is argv[argc-1].  Make sure there are enough arguments.
            if (i + 1 <= argc - 1) {
                i++;
                strncpy(fnIn, argv[i], MAXPATHLEN);
            }
            else {
                // Print usage & exit
                printUsage(); return(0);
            }
        }
        else if(strncmp(argv[i], "-o", 2) == 0) {  /* Process optional arguments. */

            // The last argument is argv[argc-1].  Make sure there are enough arguments.
            if (i + 1 <= argc - 1) {
                i++;
                strncpy(fnOut, argv[i], MAXPATHLEN);
            }
            else {
                // Print usage & exit
                printUsage(); return(0);
            }
        }
        else if(strncmp(argv[i], "-s", 2) == 0) {  /* Process optional arguments. */

            // The last argument is argv[argc-1].  Make sure there are enough arguments.
            if (i + 1 <= argc - 1) {
                i++;
                strncpy(fnSoln, argv[i], MAXPATHLEN);
            }
            else {
                // Print usage & exit
                printUsage(); return(0);
            }
        }
        else if(strncmp(argv[i], "-d", 2) == 0) {  /* Process optional arguments. */
            debugLevel++;
        }
        // -board <root board fname>
        else if(strncmp(argv[i], "-b", 2) == 0) {  /* Process optional arguments. */
            if (i + 1 <= argc - 1) {
                i++;
                snprintf(fnIn  , MAXPATHLEN, "%s" , argv[i]);
                snprintf(fnOut , MAXPATHLEN, "%s.out", argv[i]);
                snprintf(fnSoln, MAXPATHLEN, "%s.soln", argv[i]);
            }
        }
        // -path <root path>
        else if(strncmp(argv[i], "-p", 2) == 0) {  /* Process optional arguments. */
            if (i + 1 <= argc - 1) {
                i++;
                snprintf(pathname , MAXPATHLEN, "%s" , argv[i]);
            }
        }
        // -recursion_depth
        else if(strncmp(argv[i], "-r", 2) == 0) {
            if (i + 1 <= argc - 1) {
                i++;
                maxDepth = strtol((char *) argv[i], (char **) NULL, 10);
            }
        }
        else if(strncmp(argv[i], "-h", 2) == 0
             || strncmp(argv[i], "-u", 2) == 0) {  /* Process optional arguments. */
            printUsage(); exit(0);
        }
        else {
            // Process non-optional arguments here.
        }
    }

    return(SUCCESS);
}

/* ------------------------------------------------------------------------- */
int main(int argc, char *argv[])
{
    board_t board;
    board_t board_in;
    board_t board_solved;

    // XXX - someday download and use argtable project
    // XXX - no, actually use getopt
    procCmdLine(argc, argv);

    // Open the output file
    openFiles();

    // OK LET'S GO ... 
    initBoard(&board);

	// read in the starting board to be solved
	readBoard(&board);

	// print board as read in from file
	printBoard(fpOut, &board, TRUE, FALSE);

    findAllSolutions(&board);
	
    fclose(fpOut);
    fclose(fpSoln);
	
	return (0);
}
