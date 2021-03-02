#include <asm/io.h>           // for mmap
#include <linux/fs.h>         // struct file, struct file_operations
#include <linux/init.h>       // for __init, see code
#include <linux/miscdevice.h> // for misc_device_register and struct miscdev
#include <linux/module.h>     // for module init and exit macros
#include <linux/uaccess.h>    // for copy_to_user, see code

#include "../address_map_arm.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nicholas Giamblanco");
MODULE_DESCRIPTION("KEY and SW Device Drivers");

#define SUCCESS 0

// Defines for Registered State.
#define NOT_REGISTERED 0
#define REGISTERED 1

// Constant Strings for KEY and SW.
#define KEY_DEV_NAME "KEY"
#define SW_DEV_NAME "SW"

// Define the PTRs to LW-Bridge, KEY and SW.
static void *LWVirtual;
static volatile int *KEYPtr;
static volatile int *SWPtr;

// Declare the methods that both KEY and SW device drivers will
// require.
// NOTES:
// 1. There is NO write function declared (cannot write to
//    these drivers).
// 2. Three functions can be shared (to reduce code clutter).
//    which have the prefix KEYSW
static int KEYSW_device_open(struct inode *, struct file *);
static int KEYSW_device_release(struct inode *, struct file *);
static loff_t KEYSW_device_seek(struct file *, loff_t, int);

static ssize_t KEY_device_read(struct file *, char *, size_t, loff_t *);
static ssize_t SW_device_read(struct file *, char *, size_t, loff_t *);

// Define the File Operations for both /dev/KEY and /dev/SW
//
// NOTES:
// 1. Since we can only read from KEYs and SW,
//    there is no need to include a file-write operation.
// 2. I've opted to use llseek to reset the position of the
//    the file offset: an alternative is to check if 0-bytes
//    were sent, and then correct the offset then.
//    However, by having the user reset the file pointer's offset
//    the user can now control when to read in NEW data from the devices.
static struct file_operations KEYDevFops = {.owner = THIS_MODULE,
                                            .read = KEY_device_read,
                                            .write = NULL,
                                            .open = KEYSW_device_open,
                                            .release = KEYSW_device_release,
                                            .llseek = KEYSW_device_seek};

static struct file_operations SWDevFops = {.owner = THIS_MODULE,
                                           .read = SW_device_read,
                                           .write = NULL,
                                           .open = KEYSW_device_open,
                                           .release = KEYSW_device_release,
                                           .llseek = KEYSW_device_seek};

// Setup Miscellaneous Dev Struct
// We need to set the permissions
// for the driver file:
//
//  User | Group | Other
// ------+-------+------
// R W X | R W X | R W X
// ------+-------+------
// 1 1 0 | 1 1 0 | 1 1 0
//
// Therefore .mode is 0666
static struct miscdevice KEYDev = {.minor = MISC_DYNAMIC_MINOR,
                                   .name = KEY_DEV_NAME,
                                   .fops = &KEYDevFops,
                                   .mode = 0666};

static struct miscdevice SWDev = {.minor = MISC_DYNAMIC_MINOR,
                                  .name = SW_DEV_NAME,
                                  .fops = &SWDevFops,
                                  .mode = 0666};

static int KEYDevRegistered = NOT_REGISTERED;
static int SWDevRegistered = NOT_REGISTERED;

// 4-Keys can provide up to 2 chars (in base 10).
// i.e., 2^4-1 = 15
// Therefore 2 chars for representation +
// 1 Newline character +
// 1 Terminating character
// = 4 total characters for the buffer.
#define KEYBUF_MAX_SIZE 4
static char KEYDevMsg[KEYBUF_MAX_SIZE];

// 10-Switches can provude up to 4 chars (in base 10).
// i.e., 2^10-1 = 1023
// Therefore 4 chars for representation +
// 1 Newline character +
// 1 Terminating character
// = 6 total chars for the buffer.
#define SWBUF_MAX_SIZE 6
static char SWDevMsg[SWBUF_MAX_SIZE];

static int __init init_drivers(void) {
  // This is an "all-or-nothing" approach, such that we will only
  // complete the initialization of both KEYs and SWs if both are registered.
  // 1. Register the KEY Device Driver.
  int KEYRegisterStatus;
  int SWRegisterStatus;
  // 1. Register the KEY Device Driver.
  KEYRegisterStatus = misc_register(&KEYDev);
  if (KEYRegisterStatus < 0) {
    // If the status returned by misc_register is less than 0,
    // early exist the init... (something has gone wrong).
    printk(KERN_ERR "/dev/%s: misc_register() failed\n", KEY_DEV_NAME);
    return KEYRegisterStatus;
  }
  // Log that we've registered the KEY Device driver
  printk(KERN_INFO "/dev/%s driver registered\n", KEY_DEV_NAME);
  KEYDevRegistered = REGISTERED;

  // 2. Register the SW Device Driver.
  SWRegisterStatus = misc_register(&SWDev);
  if (SWRegisterStatus < 0) {
    // Again, if misc_register returns a status of less than 0,
    // something is awry, and we early exit.
    // Also, we will de-register the KEYdev
    misc_deregister(&KEYDev);
    KEYDevRegistered = NOT_REGISTERED;
    printk(KERN_ERR "/dev/%s: misc_register() failed\n", SW_DEV_NAME);
    return SWRegisterStatus;
  }

  printk(KERN_INFO "/dev/%s driver registered\n", SW_DEV_NAME);
  SWDevRegistered = REGISTERED;

  // 3. Complete Initialization of both KEYs and SWs by setting
  //    their PTRs.
  LWVirtual = ioremap_nocache(LW_BRIDGE_BASE, LW_BRIDGE_SPAN);
  SWPtr = LWVirtual + SW_BASE;
  KEYPtr = LWVirtual + KEY_BASE;

  // Clear the PIO edgecapture register (clear any pending interrupt)
  *(KEYPtr + 3) = 0xF;

  return KEYRegisterStatus | SWRegisterStatus;
}

static void __exit stop_drivers(void) {
  if (KEYDevRegistered && SWDevRegistered) {
    // First, unmap the address-space.
    // NOTE: the address space is ONLY mapped
    //       if both drivers are successfully registered.
    //       so it's SAFE to only unmap in this if block.
    iounmap(LWVirtual);
    // Proceed with de-registering these character drivers.
    misc_deregister(&KEYDev);
    printk(KERN_INFO "/dev/%s driver de-registered\n", KEY_DEV_NAME);
    misc_deregister(&SWDev);
    printk(KERN_INFO "/dev/%s driver de-registered\n", SW_DEV_NAME);
  }
}

/* Called when a process opens /dev/KEY or /dev/SW */
static int KEYSW_device_open(struct inode *inode, struct file *file) {
  return SUCCESS;
}

/* Called when a process closes /dev/KEY ot /dev/SW */
static int KEYSW_device_release(struct inode *inode, struct file *file) {
  return 0;
}

loff_t KEYSW_device_seek(struct file *FilP, loff_t Off, int Whence) {
  // Check if the user has requested for SEEK_SET
  // If not, return invalid.
  if (Whence != 0)
    return -EINVAL;

  // Now check that the offset is 0 (go back to beginning of file)
  // If it's not return invalid.
  if (Off != 0)
    return -EINVAL;

  // Set the file position to be 0
  FilP->f_pos = Off;
  // Return the user-supplied offset.
  return Off;
}

void pretty_print(int Value, char *OutputString, int OutSize) {
  if (snprintf(OutputString, OutSize, "%d\n", Value) < 0) {
    printk(KERN_ERR "Error: snprintf was unsuccessful");
    // Terminate the string at pos 0.
    OutputString[0] = '\0';
  }
}

static ssize_t KEY_device_read(struct file *FilP, char *Buffer, size_t Length,
                               loff_t *Offset) {
  size_t BytesToSend;
  // Grab the KEY_value
  // If Offset is 0, we are at the beginning of the file.
  if (!(*Offset)) {
    // If there KEYs have been pressed, get the value.
    // otherwise, show 0.
    if (*(KEYPtr + 3)) {
      pretty_print(*(KEYPtr + 3), KEYDevMsg, 4);
      *(KEYPtr + 3) = 0xF;
    } else {
      pretty_print(0, KEYDevMsg, 4);
    }
  }
  // 1. Determine How many bytes to Send:
  //    (a) Find How many Outstanding bytes there are
  BytesToSend = strlen(KEYDevMsg) - (*Offset);
  //    (b) Send the Maximum number of bytes user space can handle.
  BytesToSend = BytesToSend > Length ? Length : BytesToSend;
  // 2. Send out bytes to user space.
  if (BytesToSend > 0) {
    if (copy_to_user(Buffer, &KEYDevMsg[*Offset], BytesToSend) != 0)
      printk(KERN_ERR "Error [KEY]: copy_to_user unsuccessful");
    // Update the File Ptr's Offset to reflect where to read from next read.
    *Offset += BytesToSend;
  }
  return BytesToSend;
}

static ssize_t SW_device_read(struct file *FilP, char *Buffer, size_t Length,
                              loff_t *Offset) {
  size_t BytesToSend;
  // If Offset is 0, we are the beginning of the file
  // read in the SWs
  if (!(*Offset)) {
    pretty_print(*SWPtr, SWDevMsg, 6);
  }
  // 1. Determine How many bytes to Send:
  //    (a) Find How many Outstanding bytes there are.
  BytesToSend = strlen(SWDevMsg) - (*Offset);
  //    (b) Send the Maximum number of bytes user space can handle.
  BytesToSend = BytesToSend > Length ? Length : BytesToSend;

  // 2. Send out bytes to user space.
  if (BytesToSend > 0) {
    if (copy_to_user(Buffer, &SWDevMsg[*Offset], BytesToSend) != 0)
      printk(KERN_ERR "Error [SW]: copy_to_user unsuccessful");
    // Update the File Ptr's Offset to reflect where to read from next read.
    *Offset += BytesToSend;
  }
  // Return the number of bytes sent: zero indicates EOF.
  return BytesToSend;
}

module_init(init_drivers);
module_exit(stop_drivers);

// End of Module.
