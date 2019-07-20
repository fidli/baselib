 //https://www.kernel.org/doc/Documentation/input/joystick-api.txt
#ifndef LINUX_JOYSTICK
#define LINUX_JOYSTICK
 
 struct js_event {
     __u32 time;     /* event timestamp in milliseconds */
     __s16 value;    /* value */
     __u8 type;      /* event type */
     __u8 number;    /* axis/button number */
 };
 
#define JS_EVENT_BUTTON         0x01    /* button pressed/released */
#define JS_EVENT_AXIS           0x02    /* joystick moved */
#define JS_EVENT_INIT           0x80    /* initial state of device */
 
 /* function			3rd arg  */
#define JSIOCGAXES	/* get number of axes		char	 */
#define JSIOCGBUTTONS	/* get number of buttons	char	 */
#define JSIOCGVERSION	/* get driver version		int	 */
#define JSIOCGNAME(len) /* get identifier string	char	 */
#define JSIOCSCORR	/* set correction values	&js_corr */
#define JSIOCGCORR	/* get correction values	&js_corr */
 
 
 #endif