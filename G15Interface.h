/*
 * G15Interface.h
 *
 *  Created on: Nov 27, 2014
 *      Author: frank
 */

#ifndef G15INTERFACE_H_
#define G15INTERFACE_H_

#include <stdint.h>
#include <hidapi/hidapi.h>

#include <vector>
using std::vector;
#include <string>
using std::string;

#include "g15interface_defs.h"


// Set aside a vector type for us to get a list of the interfaces
// we've got available on the PC we're running on.
class G15Interface;
typedef vector<G15Interface> G15InterfaceList;

// This is the main class- it handles the interface rules for
// each category of device we have hooked up.  The library
// through this C++ class object, doesn't *CARE* which one
// of the Logitech devices you're dinking with at a given time
// (and shouldn't...the daemon might (but it probably shouldn't
// EITHER...)) so you can have differing feeds going to differing
// devices.  Like the media player going to a Z10 speaker,
// and something like weather going to the keyboard...  It shouldn't
// presume something silly like first device except as a default
// for the tools apps.
class G15Interface
{
public:
	G15Interface();
	virtual ~G15Interface();

	// Static helper method to nab the list of current interfaces
	// that we support....
	static G15InterfaceList getAvailableInterfaces();

	// enable or disable debugging
	static void libg15Debug(G15_LOG_LEVEL level);

	// Initialize the current interface -- If we get an ENODEV reply somewhere
	// along the lines for *ANY* of these methods, you need to drop the instance
	// of this interface like a hot potato and re-call getAvailableInterfaces
	// and re-run init() on whatever you're supposed to be working with.
	int init(void);

	// Are we inited?
	bool isInited(void) { return (_hidDev != NULL); };

	// Reset the device...you'll need/want to discard the instance
	// of this object you're holding at that point because it's
	// not assured it'll be in the same place in the HID dev
	// order once you've DONE this.
	int reset(void);

	// Close down the session to the interface.  Needs re-init if
	// this is called.
	void close(void);

	// Return the detected model string from the "getAvailableInterfaces"
	// call.  This should be the "means" for a caller to know what to
	// re-grab for if it's lost it's device on a hotplug out and back in.
	string deviceModelName(void) { return string(g15_devices[_modelIndex].name); };

	// Return the device's capability bits list, so that a developer
	// can know they're dinking with something that may not have, say
	// the macro LEDs, for example.
	uint32_t getCapabilities(void) { return g15_devices[_modelIndex].caps; };

	// Here's the operations you can *potentially* do to the keyboard.
	// (You may be barred from doing it due to lack of capabilities.)
	int writeMonoPixmapToLCD(unsigned char const *data);
	int setLEDs(unsigned int leds);
	int setLCDBrightness(unsigned int level);
	int setLCDContrast(unsigned int level);
	int setKBBrightness(unsigned int level);
	int setRGBLEDColor(unsigned char r, unsigned char g, unsigned char b);

	// Some feedback for the dev using this would be helpful...
	string getDevicePath() { return _devPath; };

	/* Please be warned
	* the g15 sends two different usb msgs for each key press
	* but only one of these two is used here. Since we do not want to wait
	* longer than timeout we will return on any msg recieved. in the good
	* case you will get G15_NO_ERROR and ORd keys in pressed_keys
	* in the bad case you will get G15_ERROR_TRY_AGAIN -> try again
	*/
	int getDeviceEvent(uint64_t *pressed_keys, int *xjoy, int *yjoy, unsigned int timeout);


private:
	static const libg15_devices_t 	g15_devices[];
	static G15_LOG_LEVEL 			logLevel;

	uint32_t 			_modelIndex;
	string 				_devPath;
	hid_device			*_hidDev;
	uint8_t				_buf[G15_BUFFER_LEN];

	static int log (FILE *fd, unsigned int level, const char *fmt, ...);
	void setModelIndex(uint32_t modelIndex) { _modelIndex = modelIndex; };
	void setDevPath(const char *path) { _devPath = string(path); };
	void dumpPixmapIntoLCDFormat(unsigned char *lcd_buffer, unsigned char const *data);
	void processKeyEvent8Byte(uint64_t *pressed_keys, int *xjoy, int *yjoy, unsigned char *buffer);
	void processKeyEvent5Byte(uint64_t *pressed_keys, unsigned char *buffer);
	void processKeyEvent2Byte(uint64_t *pressed_keys, unsigned char *buffer);

	uint8_t ReverseBitsInByte(uint8_t v)
	{
		return (v * 0x0202020202ULL & 0x010884422010ULL) % 1023;
	}
};


#endif /* G15INTERFACE_H_ */
