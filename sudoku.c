#include "sudoku.h"

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
int printBoard(FILE *fpOut, board_t *pB, bool_t debug, bool_t justSet)
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

                        pCell = &(pB->grid[x][y]);

                        delimiter = (x % 3) ? minorDelim : majorDelim;
                        if(pCell->val) {
                            if(justSet && pCell->justSet == TRUE) {
                                fprintf(fpOut, "%c<%d>", delimiter, pCell->val);
                            }
                            else {
                                fprintf(fpOut, "%c %d ", delimiter, pCell->val);
                            }
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
      set cell.maybe[0 .. 9] = FALSE; cell.numMaybe = 0; cell.justSet = FALSE;
    Foreach value v where !b.has[v]
      Traverse the 9 cells of the box (for x, y)
        If value not set in row[y] and col[x]
          Then v is a possibility for the cell x, y; cell.maybe[v] = TRUE;
    Traverse the 9 cells of the box (for x, y)
      if cell.val == 0 && numMaybe == 1
        v = non-zero item in cell.maybe
        set cell.val = v;
        set justSet = TRUE;  // used to highlight what was just found in each iteration
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
            pCell->justSet = FALSE;
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
                                    pCell->justSet = TRUE;
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
                        pCellFound->justSet = TRUE;
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
                return(FALSE);
            }
        }
    }

    // If we get here we didn't find any cell with a val == 0, so board is solved
    return(TRUE);
}

/* ------------------------------------------------------------------------- */
// Begin with the boar read in from file. Try to solve it the Simple way,
// without guessing. If there remain unset cells, we need to then guess.
// This is the combinatorial algorithms part :)
//
// if board STILL not solved, we need to just try putting in a value
// to see if it will result in a solved puzzle. Right?
// So let's copy the board, and guess one of the values from a cell
// where numMaybe is a minimum.
//
bool_t solveBoard(FILE *fpOut, board_t *pBoardIn)
{
    int x, y;
    board_t *pBoardNew = NULL;
    board_t boardNew;

    if(solveBoardSimple(fpOut, pBoardIn)) {
        // this may just do it for simple puzzles
        return(TRUE);
    }

    // XXX - maybe make a copy of the original board after trying to solve once.
    //pB = pBoardIn;

    do {
        solveBoardSimple(fpOut, pBoardIn);
        for (x = 0; x < MAX_BOARD_COLS; x++) {
            for (y = 0; x < MAX_BOARD_ROWS; y++) {

                // Find first unset cell in the grid ...
                pCell = &(pBoardIn->grid[x][y]);

                if(pCell->val == 0) {

                    for(t = 1; t < MAX_DIGITS + 1; t++) {
                        pCell->val = t;
                        if(solveBoardSimple(fpOut, pBoardIn)) {
                            return(TRUE);
                        }
                    }
                    pCell->val = 0; // that didn't work! Better luck next cell.

                    // And try guessing different values to see if solving will complete!
                    // XXX - save off the cell coord's and value guessed
                    // then try to solve simply. If that works great.
                    // if it doesn't, iterate through values to guess.
                    // if that still doesn't work, restore this x,y to 0 and try
                    // guessing values for another unset cell in grid. Etc.
                    // if the puzzle requires guessing values for more than one cell
                    // then of course this will still not yield a solution.
                    // need a func() that counts # of unset cells? or keep track as we go?
                }
            }
        }
    } while i < numCells;

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

    return(*fp);
}

/* ------------------------------------------------------------------------- */
int openFiles()
{
    if(openFileTry(&fpIn, "r", NULL, fnIn)) {
        openFileTry(&fpOut, "w", NULL, fnOut);
    }
    else if(openFileTry(&fpIn, "r", "boards", fnIn)) {
        openFileTry(&fpOut, "w", "boards", fnOut);
    }
    else if(openFileTry(&fpIn, "r", "/Users/bryan/sudoku/boards", fnIn)) {
        openFileTry(&fpOut, "w", "/Users/bryan/sudoku/boards", fnOut);
    }

    if(!fpOut) {
        if(fpIn) fclose(fpIn);
        return(ERROR);
    }
    return(SUCCESS);
}

/* ------------------------------------------------------------------------- */
int main(int argc, char *argv[])
{
    // XXX - someday download and use argtable project

    board_t board;

    int i;

    strcpy(fnIn, INPUT_FILE);
    strcpy(fnOut, OUTPUT_FILE);

    for (i = 1; i < argc; i++) {  /* Skip argv[0] (program name). */

        if(strncmp(argv[i], "-i", 2) == 0) {  /* Process optional arguments. */

            // The last argument is argv[argc-1].  Make sure there are enough arguments.
            if (i + 1 <= argc - 1) {
                i++;
                strncpy(fnIn, argv[i], MAXPATHLEN);
            }
            else {
                // Print usage & exit
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
            }
        }
        // -board <root>
        else if(strncmp(argv[i], "-b", 2) == 0) {  /* Process optional arguments. */
            if (i + 1 <= argc - 1) {
                i++;
                snprintf(fnIn , MAXPATHLEN, "%s.in" , argv[i]);
                snprintf(fnOut, MAXPATHLEN, "%s.out", argv[i]);
            }
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
        printf("CONGRATULATIONS!!! Board is solved.\n);

	// print solution to screen
	printBoard(fpOut, &board, TRUE, FALSE);

    fclose(fpOut);
	
	return (0);
}
