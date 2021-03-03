#include <signal.h>
#include <stdio.h>
#include <time.h>

#include "plotutils.h"

volatile sig_atomic_t Running = 1;
struct timespec AnimationTime;

void IntHandler(int inter) { Running = 0; }

void HandleTerminalResize() {
  struct winsize w;
  struct Point *Tmp = AllPoints;

  ioctl(0, TIOCGWINSZ, &w);

  if (XRange != w.ws_col || YRange != w.ws_row) {
    ClearTerminal();
  }

  XRange = w.ws_col;
  YRange = w.ws_row;

  // We loop through all of our points
  // and check if any of the points were outside
  // the terminal window.
  //
  // If so, we set the new X,Y coordinate to be within the window
  // within a 4-char space away from the edge.
  while (Tmp) {
    if (Tmp->X > XRange) {
      Tmp->X = XRange;
      Tmp->dX = -1;
    }
    if (Tmp->Y > YRange) {
      Tmp->Y = YRange;
      Tmp->dY = -1;
    }
    Tmp = Tmp->Next;
  }
}

int main() {

  int i = 0;
  struct Point *T1 = NULL;
  struct Point *T2 = NULL;

  signal(SIGINT, IntHandler);
  signal(SIGWINCH, HandleTerminalResize);

  srand(time(NULL));
  InitializeTerminal();

  for (i = 0; i < 3; ++i) {
    GenRandPoint();
  }

  // Pause the animation every 0.05 Seconds.
  AnimationTime.tv_sec = 0;
  AnimationTime.tv_nsec = 50000000;

  while (Running) {

    // First, Clear the terminal:
    // ClearTerminal();

    // Then, Draw the Points and Lines.
    T1 = AllPoints;
    while (T1) {
      T2 = T1->Next;
      // Get ith and (i+1)th Nodes.
      // Draw a line between them, and use the ith color.
      if (T1 && T2) {
        PlotLine(T1->X, T1->Y, T2->X, T2->Y, T1->Color);
      }
      // If we are the end of all the points, wrap around (as long as AllPoints
      // is valid.) Draw the line.
      if (T1 && !T2 && AllPoints && AllPoints->Next != T1) {
        PlotLine(T1->X, T1->Y, AllPoints->X, AllPoints->Y, T1->Color);
        PlotPoint(AllPoints);
      }
      PlotPoint(T1);
      T1 = T1->Next;
    }

    nanosleep(&AnimationTime, NULL);

    T1 = AllPoints;
    while (T1) {
      T2 = T1->Next;
      // Get ith and (i+1)th Nodes.
      // Draw a line between them, and use the ith color.
      if (T1 && T2) {
        ClearLine(T1->X, T1->Y, T2->X, T2->Y);
      }
      // If we are the end of all the points, wrap around (as long as AllPoints
      // is valid.) Draw the line.
      if (T1 && !T2 && AllPoints && AllPoints->Next != T1) {
        ClearLine(T1->X, T1->Y, AllPoints->X, AllPoints->Y);
      }
      T1 = T1->Next;
    }

    // Update the points based on their dX and dY
    UpdatePoints();

    // Show the animation for a while.
    // nanosleep(&AnimationTime, NULL);
  }

  // Reset's the terminal
  ResetTerminal();
  fflush(stdout);
  DeletePoints();
  return 0;
}
