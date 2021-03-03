#include <signal.h>
#include <stdio.h>
#include <time.h>

#include "plotutils.h"


volatile sig_atomic_t Running = 1;

void IntHandler(int inter) { Running = 0; }

void HandleTerminalResize() {
  struct winsize w;
  ioctl(0, TIOCGWINSZ, &w);

  // Clear the screen if the window was resized.
  // Technically, SIGWINCH may be issued by a non
  // resize event, therefore this prevents us
  // from resizing too frequently.
  if (XRange != w.ws_col || YRange != w.ws_row) {
    InitializeBlackTerminal();
  }

  // Now, update the XRange and YRange (the limits of the term.)
  XRange = w.ws_col;
  YRange = w.ws_row;
}

int main() {

  int CurrentY = 0;
  int Inc = 1;
  int i = 0;

  // Register the signal handlers.
  signal(SIGINT, IntHandler);
  signal(SIGWINCH, HandleTerminalResize);

  // Get the terminal ready for animations.
  InitializeBlackTerminal();
  // Fetch the current terminal size.
  GetTerminalSize();

  while (Running) {
    // Here, we draw the line,
    // pause, and then draw black over the line.
    // We could have also just cleared the screen.
    PlotLine(0, CurrentY, XRange, CurrentY, Colors[i % NUM_COLORS]);
    nanosleep((const struct timespec[]){{0, 100000000L}}, NULL);
    PlotLine(0, CurrentY, XRange, CurrentY, BLACK);
    // Inc will indicate if we are moving up or down
    // in the animation.
    if (Inc)
      CurrentY += 1;
    else
      CurrentY -= 1;

    // When the line hits the terminal boundaries, flip the direction
    // of travel.
    if (CurrentY >= YRange)
      Inc = 0;
    if (CurrentY <= 1)
      Inc = 1;
    i++;
  }

  // Reset's the terminal
  ResetTerminal();
  fflush(stdout);
  return 0;
}
