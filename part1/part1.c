#include <stdio.h>

#include "plotutils.h"


int main(void) {
	int i;

	InitializeBlackTerminal();
	
	PlotChar(1, 1, CYAN, 'X');
	PlotChar(80, 24, CYAN, 'X');
	for(i = 8; i < 18; ++i)
		PlotChar(40, i+12, YELLOW, '*');
	
	getchar();
	ResetTerminal();
	fflush(stdout);
	return 0;
}