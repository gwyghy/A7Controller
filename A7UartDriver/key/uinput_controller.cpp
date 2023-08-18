#include "uinput_controller.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <iostream>
using namespace std;

UinputController::UinputController()
{
	uinp_fd = -1;
}

void UinputController::Init()
{
	uinp_fd = open("/dev/uinput",O_WRONLY | O_NDELAY);
    if(uinp_fd == -1)
    {
		//LOG
		printf("Unable to open /dev/uinput/n");
		return;
	}
	memset(&uinp,0,sizeof(uinp));
    strncpy(uinp.name,"Uart keyboard",UINPUT_MAX_NAME_SIZE);
	uinp.id.version = 4;
	uinp.id.bustype = BUS_USB;

	// Setup the uinput device
	ioctl(uinp_fd, UI_SET_EVBIT, EV_KEY);
    ioctl(uinp_fd ,UI_SET_EVBIT,EV_REP);
    for (int i=0; i < 256; i++) {
      		ioctl(uinp_fd, UI_SET_KEYBIT, i); 
   	} 

   	/* Create input device into input sub-system */ 
   	if( write(uinp_fd, &uinp, sizeof(uinp)) < 0)
   	{
   		//LOG
   	}

   	if(ioctl(uinp_fd, UI_DEV_CREATE))
   	{
   		//LOG 
   		printf("Unable to create UINPUT device.");
   	}

}

void UinputController::Close()
{
    /* Destroy the input device */
    ioctl(uinp_fd, UI_DEV_DESTROY);
    /* Close the UINPUT device */
    close(uinp_fd);
}

void UinputController::SendKeyStroke(__u16 keycode)
{
	send_key_press(keycode);
	send_key_release(keycode);
}

void UinputController::send_key_press(__u16 keycode){
	send_key_event(keycode,1);
}
void UinputController::send_key_release(__u16 keycode){
	send_key_event(keycode,0);
}
void UinputController::send_key_event(__u16 keycode, __s32 value)
{
	memset(&event , 0, sizeof(event));
	gettimeofday(&event.time,NULL);
	event.type = EV_KEY;
	event.code = keycode;
	event.value = value;
	if(write(uinp_fd, &event,sizeof(event))<0 )
	{
		//LOG sen key failed
        printf(" send key failed");
	}


    syn_report();// event  depart
}
void UinputController::syn_report()
{
	event.type = EV_SYN;
    event.code = SYN_REPORT;
	event.value = 0;
	if(write(uinp_fd, &event,sizeof(event))<0 )
	{
		//LOG sen key failed
        printf(" send report failed");
    }

}
