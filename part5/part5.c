#include <signal.h>
#include <stdio.h>
#include <time.h>

#include "driverutils.h"
#include "plotutils.h"

// 0.02 Second [Dec/Inc]rements
#define ANIMETIME 20000000

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
    if (Tmp->X > XRange)
      Tmp->X = XRange;
    if (Tmp->Y > YRange)
      Tmp->Y = YRange;
    Tmp = Tmp->Next;
  }
}

int main() {

  int i = 0;
  int SWValue;
  int KEYValue;
  uint8_t SafelyRead;

  int ShowLines = 1;

  struct Point *T1 = NULL;
  struct Point *T2 = NULL;

  signal(SIGINT, IntHandler);
  signal(SIGWINCH, HandleTerminalResize);

  // First, we open all drivers.
  OpenDrivers();


  srand(time(NULL));
  InitializeTerminal();

  for (i = 0; i < 3; ++i) {
    GenRandPoint();
  }

  // Pause the animation every 0.2 Seconds.
  AnimationTime.tv_sec = 0;
  AnimationTime.tv_nsec = 200000000;

  while (Running) {

  	// Read from the SWs.
    ReadFrom(SW, SWBuffer, SW_BUF_BYTES);
    // Attempt to convert the contents of what we read from /dev/SW
    //    into an integer.
    SWValue = StringToUint(SWBuffer, &SafelyRead);
    if (!SafelyRead)
      goto READ_KEYS;

    if (SWValue > 0)
      ShowLines = 0;
    else
      ShowLines = 1;

  READ_KEYS:
    // Read from the Keys
    ReadFrom(KEY, KEYBuffer, KEY_BUF_BYTES);
    // Attempt to convert the contents of what we read from /dev/KEY
    //    into an integer.
    KEYValue = StringToUint(KEYBuffer, &SafelyRead);
    // If there has been no activity with the KEYs, go back to the
    // beginning of the loop.
    if (!(SafelyRead && KEYValue))
      goto DRAW;

    // Increase Animation Speed
    if (KEYValue & 0x1) {
      // Cap at 0.03 Seconds.
      if (AnimationTime.tv_nsec > 30000000)
        AnimationTime.tv_nsec -= ANIMETIME;
    }

    // Decrease Animation Speed
    if ((KEYValue >> 1) & 0x1) {
      // Cap at 0.3 Seconds.
      if (AnimationTime.tv_nsec < 300000000)
        AnimationTime.tv_nsec += ANIMETIME;
    }

    if ((KEYValue >> 2) & 0x1) {
      GenRandPoint();
    }

    if ((KEYValue >> 3) & 0x1) {
      DeleteLastPoint();
    }

  DRAW:
    // First, Clear the terminal:
    ClearTerminal();
    // Then, Draw the Points and Lines.
    T1 = AllPoints;
    while (T1) {
      if (ShowLines) {
        T2 = T1->Next;
        // Get ith and (i+1)th Nodes.
        // Draw a line between them, and use the ith color.
        if (T1 && T2) {
          PlotLine(T1->X, T1->Y, T2->X, T2->Y, T1->Color);
        }
        // If we are the end of all the points, wrap around (as long as
        // AllPoints is valid.) Draw the line.
        if (T1 && !T2 && AllPoints && AllPoints->Next != T1) {
          PlotLine(T1->X, T1->Y, AllPoints->X, AllPoints->Y, T1->Color);
          PlotPoint(AllPoints);
        }
      }
      PlotPoint(T1);
      T1 = T1->Next;
    }
    // Update the points based on their dX and dY
    UpdatePoints();
    // Show the animation for a while.
    nanosleep(&AnimationTime, NULL);
  }

  ReleaseDrivers();
  // Reset's the terminal
  ResetTerminal();
  fflush(stdout);
  DeletePoints();
  return 0;
}
