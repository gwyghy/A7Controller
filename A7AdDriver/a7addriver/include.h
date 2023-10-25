#ifndef INCLUDE_H
#define INCLUDE_H

#include <pthread.h>
#include <errno.h>
#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <ucontext.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <math.h>
#include <termio.h>

#include "ya/ya.h"
#include "ydshrdlib.h"
#include "analog/libmcp3204.h"
#include "analog/analog.h"
#include "adc/adc.h"


#endif // INCLUDE_H
