#include "sudoku.h"

/*
Gonzo-style log of my project.

4/20/13: started as exercise to unrust my programming skills. I like sudoku on airplanes, but my brain continuously nags at me to write this program and "discover" the algorithms needed to solve all sudoku boards.

4/30/13: able to solve puzzles based on the simple constraints (unique occurance in col, row, box)

04/30/13: start thinking about solution for problems with no definite first move based on constraints. Found such puzzles labeled "Evil" at http://www.websudoku.com/?level=4.

5/4/13: Victory over the "Evil" puzzles! Added some brute force: walk the unset cells considering values that are not constrained (unique in box, row, col) as candidate SEED VALUES. Try a seed, then pass the board to the same algorithm to see if it solves. If not, reset the cell to zero and move to the next cell with a legit seed, try again. NOTE: I can't help think there has to be a class if even more evil start states. And also, might there be multiple legit solutions to a given starting state -- surely yes. NOTE2: project unrust is working ... starting to love the wikipedia articles on Turing machines, proofs, etc., (though I'm not able to explain it cold, I'm having fun following along).

NEXT:
1. Find all possible solutions and characterize them. See if there is something called an "optimal" solution.
2. Write a puzzle generator from a randomized solution, and see if I can dial in on difficulty based on characteristics.
3. Feed the puzzle generator into the puzzle solver, accumulate some stats. This might be an interesting benchmark for a computer's performance. Keep code extremely portable.
4. Port to Python (maybe look at Ruby or Lua).
5. Port to Java or C++
6. Port to JS and run in node.js, pass board in over REST/json, results back to browser?

CLEANUP:
1. Move this log out of the .c file.
2. Consistent usage of the fpIn and fpOut. Why pass this all over the place?
3. See all XXX comments.
4. Modularize - single-page or less functions, remove repetitive nested loops if any.
5. Robust input - able to read in boards in various formats.

*/

char pathname[MAXPATHLEN+1]; // use the system-defined MACRO instead
char fnIn[MAXPATHLEN+1];
char fnOut[MAXPATHLEN+1];
FILE *fpIn;
FILE *fpOut;

/* ------------------------------------------------------------------------- */
int initBoard(board_t *pB)
{
    if(!pB) return(ERROR);

    memset(pB, 0, sizeof(board_t));

    //pB->solved = FALSE;

    return(SUCCESS);
}

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
int getBoxOrigin(int b, int *pX, int *pY)
{
    char *pFN = "getBoxOrigin";

        switch(b) {
                case 0:
                case 3:
                case 6:
                        *pX = 0;
                        break;
                case 1:
                case 4:
                case 7:
                        *pX = 3;
                        break;
                case 2:
                case 5:
                case 8:
                        *pX = 6;
                        break;
                default:
                        *pX = 0;
                        return(ERROR);
        }
        switch(b) {
                case 0:
                case 1:
                case 2:
                        *pY = 0;
                        break;
                case 3:
                case 4:
                case 5:
                        *pY = 3;
                        break;
                case 6:
                case 7:
                case 8:
                        *pY = 6;
                        break;
                default:
                        *pY = 0;
                        return(ERROR);
        }
//printf("%s: box=%d (%d, %d)\n", pFN, b, *pX, *pY);
        return(SUCCESS);

}

/* ------------------------------------------------------------------------- */
int setVal(board_t *pB, int x, int y, int val)
{
        char *strFN = "setVal";
        cell_t *pCell = NULL;

        if(!pB) return(ERROR);

        pCell = &(pB->grid[x][y]);

        // Why would we call this function if a value was already set?
        if(pCell->val != 0) {
return(ERROR);
                printf("%s: x, y = (%d, %d), val = %d", strFN, x, y, val);

                // undo constraints?
                pCell->val = val;
                pB->row[y].has[val] = FALSE;
                pB->col[y].has[val] = FALSE;
                pB->box[y].has[val] = FALSE;
                return(ERROR);
        }

        // Now set the value and also the constraints
        pCell->val = val;
        pB->row[y].has[val] = TRUE;
        pB->col[x].has[val] = TRUE;
        pB->box[whichBox(x, y)].has[val] = TRUE;

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

        pCell->val = 0;
        pB->row[y].has[val] = FALSE;
        pB->col[x].has[val] = FALSE;
        pB->box[whichBox(x, y)].has[val] = FALSE;

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
int printBoardLineH(FILE *fpOut, int length)
{
    int i;
        
    fprintf(fpOut, "       ");
    for(i = 0; i < length; i++) fprintf(fpOut, "-");
    fprintf(fpOut, "\n");

    return(SUCCESS);
}

/* ------------------------------------------------------------------------- */
int printBoard(FILE *fpOut, board_t *pB, bool_t debug, bool_t printJustSet)
{
        int i, x, y;
        cell_t *pCell = NULL;
        char majorDelim = '|';
        char minorDelim = ' ';

        for(y = 0; y < MAX_BOARD_ROWS; y++) {
                if(y % 3 == 0) {
                        // each col has pipe-space-digit-space (== 4 chars)
                        printBoardLineH(fpOut, 4 * MAX_BOARD_COLS);
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
                fprintf(fpOut, "|\n");
        }
        printBoardLineH(fpOut, 4 * MAX_BOARD_COLS);

// OPTIONAL: print "has" arrays ... debugging

    if(debug) {

        // Labels ...
        fprintf(fpOut, "        ");
        for(x = 1; x <= MAX_BOARD_COLS; x++) {
                if(x > 1) {
                        fprintf(fpOut, ", ");
                }
                fprintf(fpOut, "%d", x);
        }
        fprintf(fpOut, "\n");

        // Rows
        for(y = 0; y < MAX_BOARD_ROWS; y++) {
                fprintf(fpOut, "row %d: (", y);
                for(x = 0; x < MAX_BOARD_COLS; x++) {
                        if(x > 0) {
                                fprintf(fpOut, ", ");
                        }
                        fprintf(fpOut, "%d", pB->row[y].has[x+1]);
                }
                fprintf(fpOut, ")\n");
        }

        // XXX - HERE
        // Cols

        // Boxes
    }

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
      unless cell.justSet == sevalSeed, set cell.justSet = setvalFalse;
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
int initBoxConstraints(board_t *pB, has_t *pBox, int xoff, int yoff, int *i2, int *i3)
{
    int x, y, v;
    cell_t *pCell = NULL;

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
    // For each v not set inbox, look at ros & cols,
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
// NOTE: this will get called with many different copies of the board ...
// limit the use of globals.
//
bool_t solveBoardSimple(FILE *fpOut, board_t *pB)
{
    int b, i, j, x, y, xoff, yoff, v;
    int i1, i2, i3, i4, i5;
    has_t *pBox = NULL;
    cell_t *pCell = NULL;
    bool_t gotOne = FALSE;

    i1 = i2 = i3 = i4 = i5 = 0;

    do {
i1++;
fprintf(fpOut, "ITERATION #%d\n", i1);

        gotOne = FALSE;

        // For each box b, initialize constraints
        for(b = 0; b < MAX_DIGITS; b++) {
            pBox = &(pB->box[b]);
            getBoxOrigin(b, &xoff, &yoff);

            // PASS #1: only one possible answer in a cell ...
            initBoxConstraints(pB, pBox, xoff, yoff, &i2, &i3);

            for(x = 0; x < 3; x++) {
                for(y = 0; y < 3; y++) {
                    pCell = &(pB->grid[x + xoff][y + yoff]);

// printf("Box=%d, cell(%d, %d): numMaybe=%d, val=%d\n",
//         b, x+xoff, y+yoff, pCell->numMaybe, pCell->val);

                    if(pCell->val == 0 && pCell->numMaybe == 1) {
                        for(v = 1; v < MAX_DIGITS + 1; v++) {
                            if(pBox->has[v]) continue; // why would this ever happen?

                            if(pCell->maybe[v]) {
i4++;
                                if(pCell->val == 0) {
                                    setVal(pB, x + xoff, y + yoff, v); //pCell->val = v;
                                    pCell->justSet = setvalConstrained;
                                    pCell->numMaybe = 0;
                                    gotOne = TRUE;
                                }
                                else {
                                    printf("WTF #1 v=%d\n", v);
                                }
                            }
                        }
                    }
                }
            }

            // PASS #2: box uniqueness constraint
            //initBoxConstraints(pB, pBox, xoff, yoff, &i2, &i3);

            // for each value that has only one cell poss. in box
            for(v = 1; v < MAX_DIGITS + 1; v++) {
                cell_t *pCellFound = NULL;

                int xfound, yfound, numPoss;

                if (pBox->has[v]) continue;
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
                if(numPoss == 1) {
i5++;
                    if(pCellFound->val == 0) {
                        setVal(pB, xfound, yfound, v); // pCellFound->val = v;
                        pCellFound->justSet = setvalConstrained;
                        pCellFound->numMaybe = 0;
                        gotOne = TRUE;
                    }
                }
            }
        }

        // At the end of traversing all 9 boxes

// Print the board but <bracket> the values just set. (DEBUG)
printBoard(fpOut, pB, FALSE, TRUE);

        // now iterate until gotOne does not get set 

    } while(gotOne);

printf("solveBoardSimple: i1=%d, i2=%d, i3=%d, i4=%d, i5=%d\n", i1, i2, i3, i4, i5);

    // Any unset cells left?
    for(x = 0; x < MAX_BOARD_COLS; x++) {
        for(y = 0; y < MAX_BOARD_ROWS; y++) {
            pCell = &(pB->grid[x][y]);
            if(pCell->val == 0) {
                fprintf(fpOut, "=================\n");
                fprintf(fpOut, "solveBoardSimple: Failed to solve just on constraints\n");
                fprintf(fpOut, "=================\n");
                return(FALSE);
            }
        }
    }

    // If we get here we didn't find any cell with a val == 0, so board is solved
    fprintf(fpOut, "=================\n");
    fprintf(fpOut, "solveBoardSimple: SOLVED\n");
    fprintf(fpOut, "=================\n");
    return(TRUE);
}

/* ------------------------------------------------------------------------- */
// Begin with the board read in from file. Try to solve it the Simple way,
// without guessing. If there remain unset cells, we need to then guess.
// This should be the combinatorial algorithms part. But I'm brute forcing. :)
//
// if board STILL not solved, we need to just try putting in a value
// to see if it will result in a solved puzzle. Right?
// So let's copy the board, and guess one of the values from a cell
// where numMaybe is a minimum.
//
// Hypothesis: this could still not result in a solution, requiring a more algorithmic
// approach.
//
bool_t solveBoard(FILE *fpOut, board_t *pB_in)
{
    static int i_recurse = 0;

    int     i, box, t, x, y;
    cell_t  *pCell = NULL;
    board_t *pB_new;
    board_t boardNew;

    fprintf(fpOut, "solveBoard: i_recurse = %d\n", i_recurse);

    if(i_recurse == 0 && solveBoardSimple(fpOut, pB_in)) {
        return(TRUE);
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
                    // as long as not constrained, try t as a value
                    if(!pB_in->row[y].has[t] // XXX - should be pB_new???
                    && !pB_in->col[x].has[t]
                    && !pB_in->box[box].has[t]) { // XXX: lazy DANGEROUS deref

                        // XXX - work from a copy of board for seeding and attempting to solve, which leaves the board down a dead-end if not solved.
                        memcpy(pB_new, pB_in, sizeof(board_t));

                        setVal(pB_new, x, y, t); //pCell->val = t;
                        pCell->justSet = setvalSeed;
                        i++;
fprintf(fpOut, "solveBoard: seed attempt #%d -> (%d, %d) := %d\n", i, x, y, t);

                        if(solveBoardSimple(fpOut, pB_new)) {
                            return(TRUE);
                        }
                        else {
                            // XXX - recurse ... could be very dangerous
                            i_recurse++;
                            if(solveBoard(fpOut, pB_new)) {
                                return(TRUE);
                            }
                        }
                    }
                }

                // XXX - prob. rm these 4 lines of comments in favor or save/restore board.
                // OK, tried all the digits this cell, that did not solve it,
                // set back to 0; try another value or move on.
                // unsetVal(pB, x, y);

                // XXX - need to start doing some real combinitorial analysis of
                // this problem and come up with the simplifying process / discovery
                // of contradictions to trim the tree ...

                // if the puzzle requires guessing values for more than one cell
                // then of course this will still not yield a solution.
                // need a func() that counts # of unset cells? or keep track as we go?
            }
        }
    }

    return(FALSE);
}

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
    }

    // Try path and fnames
    if(!fpIn) {
        if(openFileTry(&fpIn, "r", pathname, fnIn)) {
            openFileTry(&fpOut, "w", pathname, fnOut);
        }
    }

    if(!fpOut) {
        if(fpIn) fclose(fpIn);
        return(ERROR);
    }
    return(SUCCESS);
}

/* ------------------------------------------------------------------------- */
void printUsage()
{
    printf("SUMMARY: sudoku -d <pathname for boards> -if <input file> -of <output file> -b <board root fname>");
    printf("\n");
    printf("EXAMPLE: ./sudoku -if xxx.in -of xxx.out\n");
    printf("EXAMPLE: ./sudoku -path boards -b xxx\n");
}

/* ------------------------------------------------------------------------- */
int main(int argc, char *argv[])
{
    // XXX - someday download and use argtable project
    // XXX - no, actually use getopt

    board_t board;

    int i;

    strcpy(pathname, DEFAULT_PATH);
    sprintf(fnIn, "%s", DEFAULT_FILE);
    sprintf(fnOut, "%s.out", DEFAULT_FILE);

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
        // -board <root board fname>
        else if(strncmp(argv[i], "-b", 2) == 0) {  /* Process optional arguments. */
            if (i + 1 <= argc - 1) {
                i++;
                snprintf(fnIn , MAXPATHLEN, "%s" , argv[i]);
                snprintf(fnOut, MAXPATHLEN, "%s.out", argv[i]);
            }
        }
        // -path <root path>
        else if(strncmp(argv[i], "-p", 2) == 0) {  /* Process optional arguments. */
            if (i + 1 <= argc - 1) {
                i++;
                snprintf(pathname , MAXPATHLEN, "%s" , argv[i]);
            }
        }
        else if(strncmp(argv[i], "-h", 2) == 0
             || strncmp(argv[i], "-u", 2) == 0) {  /* Process optional arguments. */
            printUsage(); return(0);
        }
        else {
            // Process non-optional arguments here.
        }
    }

    // Open the output file
    openFiles();

    // OK LET'S GO ... 
    initBoard(&board);

	// read in the starting board to be solved
	readBoard(&board);

	// print board as read in from file
	printBoard(fpOut, &board, TRUE, FALSE);
	
	// solve it!
	if(solveBoard(fpOut, &board)) {
        printf("CONGRATULATIONS!!! Board is solved.\n");
    }
    else {
        printf("BUMMER!!! Board was NOT solved.\n");
    }

    // XXX - unecessary since the solution trace does this (aplenty)
	// print solution to screen
	// printBoard(fpOut, &board, TRUE, FALSE);

    fclose(fpOut);
	
	return (0);
}
