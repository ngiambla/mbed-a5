#include <stdio.h>
#include <string.h>

#include "plotutils.h"

char HelpMessage[] = "Press [return] to exit!";

int main(void) {
  int i;

  InitializeTerminal();

  // Plot the Character 'X' at 1,1 with CYAN color.
  PlotChar(1, 1, CYAN, 'X');
  // Plot the Character 'X' at 80,24 with CYAN color.
  PlotChar(80, 24, CYAN, 'X');

  // Create a vertical yellow line consisting of '*'
  for (i = 8; i < 18; ++i)
    PlotChar(40, i + 12, YELLOW, '*');

  // Write Help Message
  for (i = 0; i < strlen(HelpMessage); ++i)
    PlotChar(i+1, 3, RED, HelpMessage[i]);

  // Block until the user hits enter.
  getchar();
  // Reset the terminal
  ResetTerminal();
  // Flush all in buffer to stdout.
  fflush(stdout);
  return 0;
}
