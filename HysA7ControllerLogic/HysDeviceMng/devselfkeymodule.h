#ifndef DEVSELFKEYMODULE_H
#define DEVSELFKEYMODULE_H

#include <sys/types.h>
#include "devglobaltype.h"

/*******/
#define     KEY_NUMBER_MAX  30

/**键盘模块的具体下发数据**/
typedef struct
{
    u_int16_t KeyPressedStatus[KEY_NUMBER_MAX];//ID
    u_int16_t KeyUpStatus[KEY_NUMBER_MAX];//ID
}KEY_CRTL_TYPE;

#endif // DEVSELFKEYMODULE_H
