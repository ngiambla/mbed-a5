#include <signal.h>
#include <stdio.h>
#include <time.h>
#include "plotutils.h"


volatile sig_atomic_t Running = 1;

void IntHandler(int inter) {
  Running = 0;
}


void HandleTerminalResize() {
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);

    if(XRange != w.ws_col || YRange != w.ws_row) {
    	InitializeBlackTerminal();  
    }

    XRange = w.ws_col;
    YRange = w.ws_row;
}

int main() {

	int CurrentY = 0;
	int Inc = 1;
	int i = 0;

	signal(SIGINT, IntHandler);
	signal(SIGWINCH, HandleTerminalResize);

	InitializeBlackTerminal();	
	GetTerminalSize();

	while(Running) {
		PlotLine(0, CurrentY, XRange, CurrentY, Colors[i%NUM_COLORS]);
		nanosleep((const struct timespec[]){{0, 100000000L}}, NULL);
		PlotLine(0, CurrentY, XRange, CurrentY, BLACK);
		if(Inc)
			CurrentY += 1;
		else
			CurrentY -= 1;

		if(CurrentY >= YRange) Inc = 0;
		if(CurrentY <= 1) Inc = 1;
		i++;
	}

	// Reset's the terminal
	ResetTerminal();
	fflush(stdout);
	return 0;
}