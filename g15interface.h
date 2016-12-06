/*
 * g15interface.h
 *
 *  Created on: Dec 5, 2016
 *      Author: frank
 *
 * This is named the same as the G15Interface.h file but it's to provide a
 * similar-to libg15.so type interface so Python, etc. can use the file.
 *
 * This is intended to largely replace Gnome15's G15 level interfaces and possibly
 * even the G19 based one if this is possible.  If not, there'll be a similar
 * API and edge called G19Interface/g19interface that leverages HID support
 * so that we don't have to un-bind the keyboard.  Gnome15 was less stable
 * and more convoluted as a result of needing a crapload of differing drivers
 * using libusb to UN-Bind the device from the stock USB support to issue
 * raw URBs to the devices, etc.  HID reports are cleaner and while you still
 * need permissions, I don't bind up the keyboard so that if the Gnome15 daemon
 * hangs for some reason, you wedge up all input from the device.
 */

#ifndef G15INTERFACE_C_H_
#define G15INTERFACE_C_H_

#include <stdint.h>
#include <hidapi/hidapi.h>
#include "G15Interface.h"

#ifndef __cplusplus
extern "C" {
#endif //__cplusplus

void getAvailableInterfaces(void);

int numberAvailableInterfaces(void);

// enable or disable debugging
void libg15Debug(int intf, G15_LOG_LEVEL level);

// Initialize the current interface -- If we get an ENODEV reply somewhere
// along the lines for *ANY* of these methods, you need to drop the instance
// of this interface like a hot potato and re-call getAvailableInterfaces
// and re-run init() on whatever you're supposed to be working with.
int init(int intf);

// Are we inited?
int isInited(int intf);

// Reset the device...you'll need/want to discard the instance
// of this object you're holding at that point because it's
// not assured it'll be in the same place in the HID dev
// order once you've DONE this.
int reset(int intf);

// Close down the session to the interface.  Needs re-init if
// this is called.
void close(int intf);

// Return the detected model string from the "getAvailableInterfaces"
// call.  This should be the "means" for a caller to know what to
// re-grab for if it's lost it's device on a hotplug out and back in.
const char * deviceModelName(int intf);

// Return the device's capability bits list, so that a developer
// can know they're dinking with something that may not have, say
// the macro LEDs, for example.
uint32_t getCapabilities(int intf);

// Here's the operations you can *potentially* do to the keyboard.
// (You may be barred from doing it due to lack of capabilities.)
int writeMonoPixmapToLCD(int intf, unsigned char const *data);
int setLEDs(int intf, unsigned int leds);
int setLCDBrightness(int intf, unsigned int level);
int setLCDContrast(int intf, unsigned int level);
int setKBBrightness(int intf, unsigned int level);
int setRGBLEDColor(int intf, unsigned char r, unsigned char g, unsigned char b);

// Some feedback for the dev using this would be helpful...
const char * getDevicePath(int intf);

/* Please be warned
* the g15 sends two different usb msgs for each key press
* but only one of these two is used here. Since we do not want to wait
* longer than timeout we will return on any msg recieved. in the good
* case you will get G15_NO_ERROR and ORd keys in pressed_keys
* in the bad case you will get G15_ERROR_TRY_AGAIN -> try again
*/
int getDeviceEvent(int intf, uint64_t *pressed_keys, int *xjoy, int *yjoy, unsigned int timeout);

#ifndef __cplusplus
}
#endif //__cplusplus

#endif /* G15INTERFACE_C_H_ */
