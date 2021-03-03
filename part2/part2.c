#include <stdio.h>
#include <string.h>

#include "plotutils.h"


char HelpMessage[] = "press [return] to exit!";

int main() {
  struct Point *TempPoint;
  int i;

  GetTerminalSize();
  InitializeBlackTerminal();
  // As a reminder for the API call...
  // void GenPoint(int X, int Y, int dX, int dY, int Color, int Sym)
  GenPoint(1, 1, 0, 0, RED, '1');
  GenPoint(XRange >> 1, YRange >> 1, 0, 0, YELLOW, '2');
  GenPoint(XRange >> 1, YRange >> 2, 0, 0, LT_GRAY, '3');
  GenPoint(XRange - 1, YRange >> 2, 0, 0, LT_BLUE, '4');
  GenPoint(XRange, YRange-2, 0, 0, BLUE, '5');

  TempPoint = AllPoints;
  while (TempPoint) {
    PlotLine(1, YRange, TempPoint->X, TempPoint->Y, TempPoint->Color);
    PlotPoint(TempPoint);
    TempPoint = TempPoint->Next;
  }

  // Write Help Message
  for (i = 0; i < strlen(HelpMessage); i+=1) {
    PlotChar(i+1, YRange, RED, HelpMessage[i]);
  }


  getchar();
  ResetTerminal();
  fflush(stdout);
  DeletePoints();
  return 0;
}
