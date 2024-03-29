#ifndef __PLOTUTILS_H__
#define __PLOTUTILS_H__

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

// VT100 Color Codes
#define BLACK 30
#define RED 31
#define GREEN 32
#define YELLOW 33
#define BLUE 34
#define MAGENTA 35
#define CYAN 36
#define LT_GRAY 37
#define DK_GRAY 90
#define LT_RED 91
#define LT_GREEN 92
#define LT_YELLOW 93
#define LT_BLUE 94
#define LT_MAGENTA 95
#define LT_CYAN 96
#define WHITE 97

#define NUM_LETTERS 25

#define NUM_COLORS 15
const int Colors[NUM_COLORS] = {
    RED,    GREEN,    YELLOW,    BLUE,    MAGENTA,    CYAN,    LT_GRAY, DK_GRAY,
    LT_RED, LT_GREEN, LT_YELLOW, LT_BLUE, LT_MAGENTA, LT_CYAN, WHITE};

// This struct will represent a point to plot.
struct Point {
  int X;     // X Coord
  int Y;     // Y Coord
  int dX;    // Change in X direction
  int dY;    // Change in Y direction
  char Sym;  // Symbol use for ASCII disp
  int Color; // Color for ASCII disp
  struct Point *Next;
};

// Create two static global variables (i.e., storage local
// to this scope) which will represent the maximum X and Y range
// of the terminal (the terminal size)
static int XRange = 4;
static int YRange = 4;

// ColorSelector will be used to cycle through all of the colors
// as randomly generated nodes are created
static int ColorSelector = 0;
// CharacterSelector will be used to cycle through the alphabet ('A' - 'Z')
static int CharacterSelector = 0;

// AllPoints is a linked-list of ALL the points which we have created and wish to display.
struct Point *AllPoints = NULL;
// CurrentPoint points to the last node of the linked list.
struct Point *CurrentPoint = NULL;

/* BEGIN VT100 Helper Functions */

// Set Color of Text.
void SetTextColor(int Color) {
  printf("\e[%dm", Color);
  fflush(stdout);
}


// Resets terminal to initial state
void ResetTerminal() {
  printf("\ec");
  fflush(stdout);
}

// Sets the cursor at the X (i.e., col) and Y (i.e., row) of the
// terminal
void SetCursorAt(int X, int Y) {
  printf("\e[%d;%dH", Y, X);
  fflush(stdout);
}

// Provided with a coordinate (X,Y), a Color (e.g., color code for blue) 
// and Dispchar (e.g., '@'), plot it on the terminal.
void PlotChar(int X, int Y, char Color, char Dispchar) {
  printf("\e[%2dm\e[%d;%dH%c\e[0m", Color, Y, X, Dispchar);
  fflush(stdout);
}

// Clear the screen
void ClearTerminal() {
  printf("\e[2J");
  fflush(stdout);
}

// Hide the cursor
void HideCursor() {
  printf("\e[?25l");
  fflush(stdout);
}

// Show the cursor
void ShowCursor() {
  printf("\e[?25h");
  fflush(stdout);
}

// Here, we can probe the Kernel for the current terminal
// size.
// From https://man7.org/linux/man-pages/man4/tty_ioctl.4.html:
//
// ioctl_tty - ioctls for terminals and serial lines
//
// Get and set window size
//      Window sizes are kept in the kernel, but not used by the kernel
//      (except in the case of virtual consoles, where the kernel will
//      update the window size when the size of the virtual console
//      changes, for example, by loading a new font).
//
//      The following constants and structure are defined in
//      <sys/ioctl.h>.
//
//      TIOCGWINSZ     struct winsize *argp
//             Get window size.
void GetTerminalSize() {
  struct winsize w;
  ioctl(0, TIOCGWINSZ, &w);
  XRange = w.ws_col;
  YRange = w.ws_row;
}

// The terminal will be cleared, and the cursor
// will be hidden.
void InitializeTerminal() {
  ColorSelector = rand()%NUM_COLORS;
  CharacterSelector = rand()%NUM_LETTERS;
  HideCursor();
  ClearTerminal();
  GetTerminalSize();
}

/* END VT100 Helper Functions */


/* BEGIN Plot Utilities */

// Some NOTES:
// Top Left Corner is 1,1: https://en.wikipedia.org/wiki/ANSI_escape_code

// GenRandPoint will generate a random point within the terminal window.
void GenRandPoint() {
  
  // First, Ask Malloc to create a new node.
  struct Point *NewPt = (struct Point *)malloc(sizeof(struct Point));
  // Return if we cannot create a new node.
  if (!NewPt)
    return;
  // Now, fill in the fields of the newly created struct.
  // Here, we randomly select between (1 .. XRange) and (1 .. YRange)
  // for the coordinates.
  NewPt->X = rand() % (XRange)+1;
  NewPt->Y = rand() % (YRange)+1;
  // We also randomly choose to move each point in 
  // one of the four diagonal directions.
  if(NewPt->X == 1)
    NewPt->dX = 1;
  else if(NewPt->X == XRange)
    NewPt->dX = -1;
  else
    NewPt->dX = ((rand() % 100) > 50) ? 1 : -1;

  if(NewPt->Y == 1)
    NewPt->dY = 1;
  else if(NewPt->Y == YRange)
    NewPt->dY = -1;
  else
    NewPt->dY = ((rand() % 100) > 50) ? 1 : -1;


  // Select a color.
  NewPt->Color = Colors[ColorSelector%NUM_COLORS];
  ColorSelector++;
  // Generate a symbol between 'A' and 'Z'
  NewPt->Sym = 'A' + CharacterSelector%NUM_LETTERS;
  CharacterSelector++;
  // Set the NextPoint to NULL
  NewPt->Next = NULL;

  // If this is the first time we've entered this function,
  // AllPoints would have been NULL.
  if (!AllPoints) {
    AllPoints = NewPt;
    CurrentPoint = AllPoints;
    return;
  }
  // Otherwise, CurrentPoint will be updated by linking in the new node.
  CurrentPoint->Next = NewPt;
  CurrentPoint = NewPt;
}


// GenPoint allows the user to manually specify all of the fields
// of a newly created point (which is then linked into the global
// pointers).
// The user is responsible for geneting valid coordinates...
void GenPoint(int X, int Y, int dX, int dY, int Color, int Sym) {
  struct Point *NewPt = (struct Point *)malloc(sizeof(struct Point));
  if (!NewPt)
    return;

  NewPt->X = X;
  NewPt->Y = Y;
  NewPt->dX = dX;
  NewPt->dY = dY;
  NewPt->Color = Color;
  NewPt->Sym = Sym;
  NewPt->Next = NULL;

  if (!AllPoints) {
    AllPoints = NewPt;
    CurrentPoint = AllPoints;
    return;
  }

  CurrentPoint->Next = NewPt;
  CurrentPoint = NewPt;
}

// UpdatePoints will update each point in the Point Linked List
// (i.e., AllPoints) by adding the change in X (dX) and Y (dX) to the 
// the X and Y coordinates.
// We check for boundary conditions here: if we are 1 away from the XRange/YRange
// or 0/0, then we need to flip the sign on dX and dY (move away from the boundaries)
void UpdatePoints() {
  struct Point *Tmp = AllPoints;
  while (Tmp) {
    // Update the points.
    Tmp->X += Tmp->dX;
    Tmp->Y += Tmp->dY;

    // Check if we need to flip our directions.
    if (Tmp->X <= 1) {
      Tmp->dX = 1;
    }

    if (Tmp->X >= XRange) {
      Tmp->dX = -1;
    }

    if (Tmp->Y <= 1) {
      Tmp->dY = 1;
    }

    if (Tmp->Y >= YRange) {
      Tmp->dY = -1;
    }

    Tmp = Tmp->Next;
  }
}

// As the title suggests, this function
// will remove the last point of AllPoints.
void DeleteLastPoint() {
  struct Point *Tmp1 = AllPoints;
  struct Point *Tmp2;

  // Nothing to free.
  if (!AllPoints)
    return;

  // Only the head of the list remains
  if (!AllPoints->Next) {
    free(AllPoints);
    AllPoints = NULL;
    CurrentPoint = NULL;
    return;
  }

  // Travel through the list.
  while (Tmp1->Next) {
    Tmp2 = Tmp1;
    Tmp1 = Tmp1->Next;
  }
  free(Tmp2->Next);
  Tmp2->Next = NULL;
  CurrentPoint = Tmp2;
}

void DeletePoints() {
  struct Point *Tmp;
  while (AllPoints) {
    Tmp = AllPoints;
    AllPoints = AllPoints->Next;
    free(Tmp);
  }
  AllPoints = NULL;
  CurrentPoint = NULL;
}

void PlotPoint(struct Point *Pt) { PlotChar(Pt->X, Pt->Y, Pt->Color, Pt->Sym); }

// Follows Bresenham's Algorithm (this ver. is valid for ALL quadrants.)
void GeneralizedPlotLine(int X0, int Y0, int X1, int Y1, int Color, char Sym) {
  // Absolute change in X
  int dX = abs(X1 - X0);
  // Change in slope for X
  int sX = X0 < X1 ? 1 : -1;

  // Absolute change in Y
  int dY = -abs(Y1 - Y0);
  // Change in slop for Y
  int sY = Y0 < Y1 ? 1 : -1;
  // Error
  int E = dX + dY;
  int DoubleE;

  for (;;) {
    PlotChar(X0, Y0, Color, Sym);
    DoubleE = E << 1;
    /* Check if the Error between X and Y is > dX */
    if (DoubleE >= dY) {
      if (X0 == X1)
        break;
      E += dY;
      X0 += sX;
    }
    /* Check if the Error between X and Y is > dY */
    if (DoubleE <= dX) {
      if (Y0 == Y1)
        break;
      E += dX;
      Y0 += sY;
    }
  }
}


void PlotLine(int X0, int Y0, int X1, int Y1, int Color) {
  GeneralizedPlotLine(X0, Y0, X1, Y1, Color, '*');
}


void ClearLine(int X0, int Y0, int X1, int Y1) {
  GeneralizedPlotLine(X0, Y0, X1, Y1, BLACK, ' ');
}
/* END PLOT UTILITIES */

#endif
