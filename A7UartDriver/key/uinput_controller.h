#ifndef UINPUT_CONTROLLER_H
#define UINPUT_CONTROLLER_H
#include <linux/input.h>
#include <linux/uinput.h>

class UinputController
{
public:
	UinputController();
	void Init();
	void Close();
    void SendKeyStroke(__u16 keycode);
    void send_key_press(__u16 keycode);
    void send_key_release(__u16 keycode);

private:
	void send_key_event(__u16 keycode, __s32 value);
	void syn_report();

private:
	int uinp_fd ;
	struct uinput_user_dev uinp;// uInput device structure
    struct input_event event; // Input device structure
};

#endif
