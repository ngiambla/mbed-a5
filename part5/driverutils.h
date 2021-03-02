#ifndef __DRIVER_UTILS_H__
#define __DRIVER_UTILS_H__

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


// Define number of drivers
#define NUM_DRIVERS 2

// Define Buffers for the two drivers
// as well as their size.
#define SW_BUF_BYTES 6
#define KEY_BUF_BYTES 4

char SWBuffer[SW_BUF_BYTES];
char KEYBuffer[KEY_BUF_BYTES];


// Define Drivers as Integer references.
#define SW 0
#define KEY 1

// Define a DriverRef struct to simplify our
// development process.
struct DriverRef {
  char Path[15];
  int RWP;
  int FD;
};

struct DriverRef Drivers[NUM_DRIVERS] = {
    {.Path = "/dev/SW", .RWP = O_RDONLY, .FD = -1},
    {.Path = "/dev/KEY", .RWP = O_RDONLY, .FD = -1}
};

// Using a Macro to get a Driver's Open File Desc.
#define GetFD(x) (Drivers[(x)].FD)
#define IsRDONLY(x) (Drivers[(x)].RWP == O_RDONLY)


// Loop through our drivers, and close all file
// descriptors that are open.
void ReleaseDrivers() {
  int j = 0;
  for (j = 0; j < NUM_DRIVERS; ++j) {
  	if (GetFD(j) != -1)
  		close(GetFD(j));
  }
}

void ErrorHandler(char * Message) {
  ReleaseDrivers();
  printf("Error Encountered: %s\nExiting %s\n", Message, strerror(errno));
  exit(-1);
}

void OpenDrivers() {
  for (i = 0; i < NUM_DRIVERS; ++i) {
    if ((Drivers[i].FD = open(Drivers[i].Path, Drivers[i].RWP)) == -1) {
      ErrorHandler("Failed to open driver.");
    }
  }	
}



void ReadFrom(int DevId, char *Buffer, int BufSize) {
  int BytesRead = 0;
  int ReadStatus;
  while ((ReadStatus = read(GetFD(DevId), Buffer, BufSize)) != 0)
    BytesRead += ReadStatus; // read the driver until EOF

  if (ReadStatus < 0) {
    Buffer[0] = '\0';
    CleanupOnError();
  }

  Buffer[BytesRead] = '\0'; // NULL terminate

  // Recall, We've implemented the lseek function for
  // read-only drivers (i.e., SW and KEYs)
  if (IsRDONLY(DevId))
    lseek(GetFD(DevId), 0, SEEK_SET);
}


// Using strtoumax, convert a string to a uint.
// If successful, set Safe to be 1 and return the
// mapped value.
//
// Otherwise, set Safe to be 0 (indicating the conversion was
// not successful) and return 0.
uint32_t StringToUint(char *DriverMsg, uint8_t *Safe) {
  uint32_t IntegerValue = strtoumax(DriverMsg, NULL, 10);
  if (IntegerValue == UINTMAX_MAX && errno == ERANGE) {
    *Safe = 0;
    return 0;
  }
  *Safe = 1;
  return IntegerValue;
}


#endif