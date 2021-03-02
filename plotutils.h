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


#define NUM_COLORS 15
const int Colors[NUM_COLORS] = { RED, GREEN, YELLOW, BLUE, 
								 MAGENTA, CYAN, LT_GRAY, DK_GRAY,
								 LT_RED, LT_GREEN, LT_YELLOW, LT_BLUE,
								 LT_MAGENTA, LT_CYAN, WHITE
								};



struct Point {
	int X;  // X Coord
	int Y;  // Y Coord
	int dX; // Change in X direction
	int dY; // Change in Y direction
	char Sym; // Symbol use for ASCII disp
	int Color; // Color for ASCII disp 
	struct Point * Next;
};

static int XRange = 1;
static int YRange = 1;

struct Point * AllPoints = NULL;
struct Point * CurrentPoint = NULL;


/* BEGIN VT100 Helper Functions */
void SetTextColor(int Color) {
	printf("\e[%2dm", Color);
}

void SetBackgroundBlack() {
	printf("\e[40m\n");
}

void ResetTerminal() {
	printf("\ec\n");
}

void SetCursorAt(int X, int Y) {
	printf("\e[%d;%dH", Y, X);
}

void PlotChar(int X, int Y, int Color, int Dispchar) {
	SetTextColor(Color);
	SetCursorAt(X, Y);
	printf("%c", Dispchar);
	fflush(stdout);
}

void ClearScreen() {
	printf("\e[2J");
}

void HideCursor() {
	printf("\e[?25l");
}

void ShowCursor() {
	printf("\e[?25h");
}


void InitializeBlackTerminal() {
	SetBackgroundBlack();
	ClearScreen();
	HideCursor();
}

void GetTerminalSize() {
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    XRange = w.ws_col;
    YRange = w.ws_row;	
}



/* END VT100 Helper Functions */


void GenRandPoint() {
	// Unitialized is okay for now.
	int TmpColor;

	struct Point * NewPt = (struct Point *)malloc(sizeof(struct Point));
	if(!NewPt) return;

	NewPt->X = rand()%(XRange-2)+1;
	NewPt->Y = rand()%(YRange-2)+1;
	NewPt->dX = ((rand()%100) > 50) ? 1 : -1;
	NewPt->dY = ((rand()%100) > 50) ? 1 : -1;
	if(CurrentPoint) {
		TmpColor = Colors[rand()%NUM_COLORS]; 
		while(TmpColor == CurrentPoint->Color) {
			TmpColor = Colors[rand()%NUM_COLORS];
		}
		NewPt->Color = TmpColor;
	} else { 
		NewPt->Color = Colors[rand()%NUM_COLORS];
	}
	NewPt->Sym = (rand()%93)+34;
	NewPt->Next = NULL;


	if (!AllPoints) {
		AllPoints = NewPt;
		CurrentPoint = AllPoints;
		return;
	}

	CurrentPoint->Next = NewPt;
	CurrentPoint = NewPt;
}

void GenPoint(int X, int Y, int dX, int dY, int Color, int Sym) {
	struct Point * NewPt = (struct Point *)malloc(sizeof(struct Point));
	if(!NewPt) return;

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


void UpdatePoints() {
	struct Point * Tmp = AllPoints;
	while(Tmp) {
		Tmp->X += Tmp->dX;
		Tmp->Y += Tmp->dY;
		if(Tmp->X <= 1 || Tmp->X >= XRange-1) {
			Tmp->dX *= -1;
		}

		if(Tmp->Y <= 1 || Tmp->Y >= YRange-1) {
			Tmp->dY *= -1;
		}
		Tmp = Tmp->Next;
	}
}

void DeleteLastPoint () {
	struct Point * Tmp1 = AllPoints;
	struct Point * Tmp2;

	// Nothing to free.
	if(!AllPoints)
		return;

	// This is only the head of the list
	if(!AllPoints->Next) {
		free(AllPoints);
		AllPoints=NULL;
		return;
	}

	while(Tmp1->Next) {
		Tmp2 = Tmp1;
		Tmp1 = Tmp1->Next;
	}
	free(Tmp2->Next);
	Tmp2->Next=NULL;
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


void PlotPoint(struct Point * Pt) {
	PlotChar(Pt->X, Pt->Y, Pt->Color, Pt->Sym);
}


void PlotLine(int X0, int Y0, int X1, int Y1, int Color) {
	// Absolute change in X
    int dX =  abs(X1-X0);
    // Change in slope for X
    int sX = X0<X1 ? 1 : -1;

    // Absolute change in Y
    int dY = -abs(Y1-Y0);
    // Change in slop for Y
    int sY = Y0<Y1 ? 1 : -1;
    // Error
    int E = dX+dY;
    int DoubleE;

    for (;;) {                       
        PlotChar(X0, Y0, Color, '*');
        DoubleE = E<<1;
        /* Check if the Error between X and Y is > dX */
        if (DoubleE >= dY) {
            if (X0 == X1) break;
            E += dY; 
            X0 += sX;
        }
        /* Check if the Error between X and Y is > dY */
        if (DoubleE <= dX) {
            if (Y0 == Y1) break;
            E += dX; 
            Y0 += sY;
        }
    }
}



#endif
