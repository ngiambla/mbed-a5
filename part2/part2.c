#include <stdio.h>
#include "plotutils.h"



int main() {
	struct Point * TempPoint;

	GetTerminalSize();
	InitializeBlackTerminal();
	GenPoint(0, 0, 0, 0, RED, '$');
	GenPoint(XRange>>1, YRange>>1, 0, 0, YELLOW, '&');
	GenPoint(XRange>>1, YRange>>2, 0, 0, LT_GRAY, '&');
	GenPoint(XRange-1, YRange>>2, 0, 0, LT_BLUE, '&');
	GenPoint(XRange-1, YRange-1, 0, 0, BLUE, '@');

	TempPoint = AllPoints;

	while(TempPoint) {
		PlotLine(0, YRange-1, TempPoint->X, TempPoint->Y, TempPoint->Color);
		PlotPoint(TempPoint);
		TempPoint = TempPoint->Next;
	}

	getchar();
	ResetTerminal();
	fflush(stdout);
	DeletePoints();
	return 0;
}