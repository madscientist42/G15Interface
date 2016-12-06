/*
 * g15interface.cpp
 *
 *  Created on: Nov 27, 2014
 *      Author: frank
 *
 * This is named the same as the G15Interface.cpp file but it's to provide a
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
 *
 */

#include <vector>
using std::vector;

#include "G15Interface.h"
#include "g15interface.h"

/*
 * Static, global instance list of the available interfaces.  Nasty, I will own, but
 * in this instance, since we're wrapping the enhanced C++ framework for a somewhat
 * simpler C based one that's based off of, but not fully identical to libg15, we're
 * going to run with it all the same.
 */
G15InterfaceList glInterfaceList;

/*
 * This is called to prime the pump on interfaces available.
 *
 */
void getAvailableInterfaces(void)
{
	glInterfaceList = G15Interface::getAvailableInterfaces();
}

int numberAvailableInterfaces(void)
{
	return glInterfaceList.size();
}

/* We're zero based on interfaces, etc... */
int isValidInterface(int intf)
{
	return (intf > 0) && (intf < glInterfaceList.size());
}

// enable or disable debugging output for a given interface object.
void libg15Debug(int intf, G15_LOG_LEVEL level)
{
	if (isValidInterface(intf))
	{
		glInterfaceList[intf].libg15Debug(level);
	}
}

// Initialize the current interface -- If we get an ENODEV reply somewhere
// along the lines for *ANY* of these function, you need to drop the instance
// of this interface like a hot potato and re-call getAvailableInterfaces
// and re-run init() on whatever you're supposed to be working with.
int init(int intf)
{
	if (isValidInterface(intf))
	{
		return glInterfaceList[intf].init();
	}
	else
	{
		return -ENODEV;
	}
}

// Are we inited?
int isInited(int intf)
{
	if (isValidInterface(intf))
	{
		return glInterfaceList[intf].isInited();
	}
	else
	{
		return -ENODEV;
	}
};

int reset(int intf)
{
	if (isValidInterface(intf))
	{
		return glInterfaceList[intf].reset();
	}
	else
	{
		return -ENODEV;
	}
}

void close(int intf)
{
	if (isValidInterface(intf))
	{
		glInterfaceList[intf].close();
	}
}

const char * deviceModelName(int intf)
{
	if (isValidInterface(intf))
	{
		return glInterfaceList[intf].deviceModelName().c_str();
	}
	else
	{
		/* This shouldn't happen, but let's be thorough... */
		return "NONE";
	}
}

uint32_t getCapabilities(int intf)
{
	if (isValidInterface(intf))
	{
		return glInterfaceList[intf].getCapabilities();
	}
	else
	{
		return 0;
	}
}

int writeMonoPixmapToLCD(int intf, unsigned char const *data)
{
	if (isValidInterface(intf))
	{
		return glInterfaceList[intf].writeMonoPixmapToLCD(data);
	}
	else
	{
		return -ENODEV;
	}
}

int setLEDs(int intf, unsigned int leds)
{
	if (isValidInterface(intf))
	{
		return glInterfaceList[intf].setLEDs(leds);
	}
	else
	{
		return -ENODEV;
	}
}

int setLCDBrightness(int intf, unsigned int level)
{
	if (isValidInterface(intf))
	{
		return glInterfaceList[intf].setLCDBrightness(level);
	}
	else
	{
		return -ENODEV;
	}
}

int setLCDContrast(int intf, unsigned int level)
{
	if (isValidInterface(intf))
	{
		return glInterfaceList[intf].setLCDContrast(level);
	}
	else
	{
		return -ENODEV;
	}
}

int setKBBrightness(int intf, unsigned int level)
{
	if (isValidInterface(intf))
	{
		return glInterfaceList[intf].setKBBrightness(level);
	}
	else
	{
		return -ENODEV;
	}
}

int setRGBLEDColor(int intf, unsigned char r, unsigned char g, unsigned char b)
{
	if (isValidInterface(intf))
	{
		return glInterfaceList[intf].setRGBLEDColor(r, g, b);
	}
	else
	{
		return -ENODEV;
	}
}

const char * getDevicePath(int intf)
{
	if (isValidInterface(intf))
	{
		return glInterfaceList[intf].getDevicePath().c_str();
	}
	else
	{
		/* This shouldn't happen, but let's be thorough... */
		return "NONE";
	}
}

int getDeviceEvent(int intf, uint64_t *pressed_keys, int *xjoy, int *yjoy, unsigned int timeout)
{
	if (isValidInterface(intf))
	{
		return glInterfaceList[intf].getDeviceEvent(pressed_keys, xjoy, yjoy, timeout);
	}
	else
	{
		return -ENODEV;
	}
}



