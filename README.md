# Assignment 5

There are 5 parts to this assignments, each implemented in a unique directoy.

`part1/`: 
`part2/`:
`part3/`:
`part4/`:
`part5/`:

Additionally, most of the logic was implemented in `plotutils.h`.
Since there was quite a bit of shared logic between the parts of the assignment, I opted to use a header file. 

# Part 1

This part of the lab was provided as an example. I implemented the example using a more user-friendly API (handled in `plotutils.h`).

To Build: `cd part1; make clean; make;`
To Use: `./part1.exe`
To Exit: press `[return]`

# Part 2

Here, we explore the use of Bresenham's Line Algorithm. We draw several lines on the terminal. 

To Build: `cd part2; make clean; make;`
To Use: `./part2.exe`
To Exit: press `[return]`


# Part 3
A horizontal line is drawn and bounced between the bottom and top of the terminal screen.

To Build: `cd part3; make clean; make;`
To Use: `./part3.exe`
To Exit: press `[ctrl]+c`


# Part 4

Three points are randomly generated, and lines will be drawn between points following this rule:

`P(i) will connect to P(i-1) for all i in Size(P). The last point P(term) will connect to P(0)`

To Build: `cd part4; make clean; make;`
To Use: `./part4.exe`
To Exit: press `[ctrl]+c`

# Part 5

Initially, three points are randomly generated, and lines will be drawn between points following this rule:

`P(i) will connect to P(i-1) for all i in Size(P). The last point P(term) will connect to P(0)`

The user can interact with the `KEY`s and `SW`s to change the behaviour of this program.

Specifically:

`KEY0`: Decrease the sleep interval by 0.02 Seconds (until we cap at 0.03 Seconds)
`KEY1`: Increase the sleep interval by 0.02 Seconds (until we cap at 0.3 Seconds)
`KEY2`: Introduce a new point to the terminal (limited by malloc...)
`KEY3`: Remove the most recently created point (all points can be removed...)

and

`SW > 0`: Do NOT draw lines.
`SW == 0`: Draw lines.

To Build: `cd part5; make clean; make;`
To Use: `./part5.exe`
To Exit: press `[ctrl]+c`


# NOTES

1. For Part{2,3,4,5}, we fetch the terminal window by querying the kernel.

2. For Part{3,4,5}, we handle terminal window resizing by attaching a signal handler to `SIGWINCH`. This signal indicates that the terminal window has been changed.

3. The function `ResetTerminal` issues a special VT100 command, which resets the state of the terminal to it's default settings.

```c

// Resets terminal to initial state
void ResetTerminal() { printf("\ec\n"); }
```

4. We draw over animations with black (but have the ability to also clear the screen).