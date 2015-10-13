/*
 * G15Interface.cpp
 *
 *  Created on: Nov 27, 2014
 *      Author: frank
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <signal.h>

#include "G15Interface.h"

#include <boost/filesystem.hpp>
using namespace boost::filesystem;

#include <iostream>
using namespace std;


// Define the device types we officially support here.  If you add
// a new device under this library, you need to add it to the *end*
// of this structure list...
#define MAX_DEVICES 11
const libg15_devices_t G15Interface::g15_devices[MAX_DEVICES] =
{
	DEVICE("UNKNOWN", 0x0, 0x0, G15_NONE),
    DEVICE("Logitech Z-10",0x46d,0x0a07,G15_LCD|G15_KEYS|G15_MKEYS|G15_BACKLIGHT_CNTL),
    DEVICE("Logitech G11",0x46d,0xc225,G15_KEYS|G15_MKEYS|G15_BACKLIGHT_CNTL),
    DEVICE("Logitech G13",0x46d,0xc21c,G15_LCD|G15_KEYS|G15_MKEYS|G15_RGB_BKLT_CNTL|G15_IS_G13),
    DEVICE("Logitech G15",0x46d,0xc222,G15_LCD|G15_KEYS|G15_MKEYS|G15_BACKLIGHT_CNTL|G15_CONTRAST_CNTL),
    DEVICE("Logitech G15 v2",0x46d,0xc227,G15_LCD|G15_KEYS|G15_MKEYS|G15_BACKLIGHT_CNTL),
    DEVICE("Logitech G110",0x46d,0xc22b,G15_KEYS|G15_MKEYS|G15_RED_BLUE_BKLT_CNTL),
    DEVICE("Logitech G510s",0x46d,0xc22d,G15_LCD|G15_KEYS|G15_MKEYS|G15_RGB_BKLT_CNTL|G15_DUAL_ENDPOINT), /* without audio activated */
    DEVICE("Logitech G510s",0x46d,0xc22e,G15_LCD|G15_KEYS|G15_MKEYS|G15_RGB_BKLT_CNTL|G15_DUAL_ENDPOINT), /* with audio activated */
    DEVICE("Logitech G710+",0x46d,0xc22e,G15_KEYS|G15_MKEYS|G15_DUAL_ENDPOINT),		 					  /* Hacking this in for now */
    DEVICE("Cisco UC K725-C",0x046d,0xb321,G15_LCD|G15_KEYS|G15_MKEYS|G15_BACKLIGHT_CNTL),				  /* Hacking this in for now */
};

// Handle the control variable for the logging levels for the library...
G15_LOG_LEVEL G15Interface::logLevel;

G15Interface::G15Interface() : _modelIndex(0), _hidDev(NULL), _inputDev(-1)
{
}

// If for any reason, we drop out of scope, we need to clean
// up any instances we've initted.
G15Interface::~G15Interface()
{
	// Just force the interface down...
	close();
}

int G15Interface::log (FILE *fd, unsigned int level, const char *fmt, ...)
{
    if (logLevel >= level)
    {
        fprintf(fd,"libg15-ng: ");
        va_list argp;
        va_start (argp, fmt);
        vfprintf(fd,fmt,argp);
        va_end (argp);
    }

    return 0;
}

// Trawl the USB ID space on this machine, looking for valid
// interfaces.  Typically there's only going to be ONE of these,
// but if not, we want the list of ALL of them.  No telling how
// someone can gracefully handle the device list here past one
// device in it yet, but it's poor form to presume there's only
// one device....
//
// We're using the HID API here, because it looks like the original
// and similar APIs were semi-using a HID interface, which didn't
// mix/play well with the HID device needing to be yanked for
// libusb's ability to do this "raw".  If it's HID, talk with
// it via HID.  (Not to mention it seems to be more robust
// and cleaner via HID anyhow...)
G15InterfaceList G15Interface::getAvailableInterfaces()
{
	G15InterfaceList 	retVal;
	G15Interface		curInterface;

	struct hid_device_info *devs, *cur_dev;

	devs = hid_enumerate(0x0, 0x0);		// Look for *ALL* HID devices...
	cur_dev = devs;
	while (cur_dev)
	{
        for (int i = 0; i < MAX_DEVICES; i++)
        {
			if ((cur_dev->vendor_id == g15_devices[i].vendorid && cur_dev->product_id == g15_devices[i].productid))
			{
				// Found one.  See if it's a G510s or similar.  If it's one of that class, we need
				// to see if the interface is an even or an odd one in the position slot.  If even,
				// SKIP it.  It's the bottom, main keyboard edge and won't like us dinking
				// with it and it won't work for us on the G-Keys or the controls...
				if (!(g15_devices[i].caps & G15_DUAL_ENDPOINT) ||
					((g15_devices[i].caps & G15_DUAL_ENDPOINT) && (cur_dev->interface_number % 2)))
				{
					curInterface.setDevPath(cur_dev->path);
					curInterface.setModelIndex(i);
					retVal.push_back(curInterface);

					// FIXME -- DEBUG...
					cout << "Found : " << cur_dev->interface_number << " | " << cur_dev->product_string << " | " << cur_dev->path << endl;
					i = MAX_DEVICES;
				}
			}
        }

		// Step to the next in the linked list of devices found...
		cur_dev = cur_dev->next;
	}
	hid_free_enumeration(devs);

	return retVal;
}


int G15Interface::init()
{
	int retVal = G15_NO_ERROR;
	path devPath(_devPath);

	// Try opening this one...
	_hidDev = hid_open_path(_devPath.c_str());
	if (_hidDev != NULL)
	{
		// Opened...GRAB all input events from it as needed...we own it, lock, stock, and barrel,
		// from this moment forward until close (Have to...can't allow anything else from the
		// regular keyboards (G11, G15, G510s, etc.) since they generate events to
		// the world from the G-keys, etc. that we DO NOT WANT GOING THERE when
		// the user is running with us controlling the interface via this library.
		_inputPath = "/sys/class/hidraw/" + devPath.filename().string() + "/device/input";
		if (!access(_inputPath.c_str(), F_OK))
		{
			// We have an input edge- it's exposed as a full-on keyboard...figure out the
			// rest of the path here...
			path p(_inputPath);
			directory_iterator it(p);
			directory_iterator end_it;
			_inputPath = "";

			// FIXME -- Presume that the first entry is the one that is our path.
			p = path(it->path().string());
			for (it = directory_iterator(p); it != end_it; it++)
			{
				if (it->path().filename().string().substr(0, 5) == "event")
				{
					_inputPath = "/dev/input/" + it->path().filename().string();
					break;
				}
			}
			if (!_inputPath.empty())
			{
				// GRAB IT- we have a device....
				_inputDev = open(_inputPath.c_str(), O_RDONLY);
				if (_inputDev == -1)
				{
					cout << "Error: Failed to open event device!  (This means we're going to get JUNK!)" << endl;
				}
				else
				{
					// Opened...take full control of the inputs from this edge...and just ignore 'em...
					if (ioctl(_inputDev, EVIOCGRAB, 1))
					{
						cout << "Error: Failed to GRAB event device!  (This means we're going to get JUNK!)" << endl;
						cout << "errno = " << errno << endl;
					}
				}
			}
		}

	}
	else
	{
		retVal = G15_ERROR_OPENING_USB_DEVICE;
	}

    return retVal;
}

int G15Interface::reset(void)
{
	int retVal = G15_ERROR_UNSUPPORTED;

	// If we don't have keys or if we have them and are a G13, this won't work
	// right.  So, we don't support it if we meet either criteria...
	if ((getCapabilities() & G15_KEYS) && !(getCapabilities() & G15_IS_G13))
	{
		// Issue a reset request against the feature report.  This
		// has the effect of hotplugging the device on the caller
		// so we're going to close() for them.  The caller will
		// need to re-enumerate their list to properly talk to
		// the device at that point in time.
		if (_hidDev != NULL)
		{
			_buf[0] = 2;
			_buf[1] = 64;
			_buf[2] = 0;
			_buf[3] = 0;
			retVal = hid_send_feature_report(_hidDev, _buf, 4);		// Reset logo if supported...
			_buf[0] = 1;
			_buf[1] = 0;
			retVal = hid_send_feature_report(_hidDev, _buf, 2);		// Smack the reset button if supported...
			close();
		}
	}

	return retVal;
}

void G15Interface::close()
{
	if (_hidDev != NULL)
	{
		// Just in case...release here...
		ioctl(_inputDev, EVIOCGRAB, 0);
		::close(_inputDev);
		hid_close(_hidDev);
		_hidDev = NULL;
	}
}

int G15Interface::setLCDBrightness(unsigned int level)
{
	int retVal = G15_ERROR_UNSUPPORTED;

	// This is only supported if you've got an LCD and *NOT* RGB support
	// (It appears that if you've got RGB support, you get the option
	//  of even finer-grained (4 steps on each of R, G, and B for the LEDS)
	// if that's the case.  This means the G510s and the G710+ both are
	// controlled slightly differently by this codebase...)
	if ((getCapabilities() & G15_LCD) && (getCapabilities() && G15_BACKLIGHT_CNTL))
	{
		retVal = G15_ERROR_OPENING_USB_DEVICE;
		if (_hidDev != NULL)
		{
			// Set up the feature report...
			_buf[0] = 2;
			_buf[1] = 2;
			_buf[3] = 0;
			switch(level)
			{
			    case 1 :
			        _buf[2] = 0x10;
			        break;
			    case 2 :
			    	_buf[2] = 0x20;
			        break;
			    default:
			    	_buf[2] = 0x00;
			}

			if (hid_send_feature_report(_hidDev, _buf, 4) < 0)
			{
				retVal = G15_ERROR_WRITING_USB_DEVICE;
			}
		}
	}

	return retVal;
}


int G15Interface::setLCDContrast(unsigned int level)
{
	int retVal = G15_ERROR_UNSUPPORTED;

	if ((getCapabilities() & G15_LCD) && (getCapabilities() && G15_CONTRAST_CNTL))
	{
		retVal = G15_ERROR_OPENING_USB_DEVICE;
		if (_hidDev != NULL)
		{
			// Set up the feature report...
			_buf[0] = 2;
			_buf[1] = 32;
			_buf[2] = 129;
		    switch(level)
		    {
		        case 1:
		        	_buf[3] = 22;
		            break;
		        case 2:
		        	_buf[3] = 26;
		            break;
		        default:
		        	_buf[3] = 18;
		        	break;
		    }

			if (hid_send_feature_report(_hidDev, _buf, 4) < 0)
			{
				retVal = G15_ERROR_WRITING_USB_DEVICE;
			}
		}
	}

	return retVal;
}

int G15Interface::setKBBrightness(unsigned int level)
{
	int retVal = G15_ERROR_UNSUPPORTED;

	// This is only supported if you've got keys and *NOT* RGB support
	// (It appears that if you've got RGB support, you get the option
	//  of even finer-grained (4 steps on each of R, G, and B for the LEDS)
	// if that's the case.  This means the G510s and the G13 are controlled
	// slightly different by the codebase here.)
	if (getCapabilities() && G15_BACKLIGHT_CNTL)
	{
		retVal = G15_ERROR_OPENING_USB_DEVICE;
		if (_hidDev != NULL)
		{
			// Set up the output command...
			_buf[0] = 2;
			_buf[1] = 1;
			_buf[3] = 0;
			switch(level)
			{
			    case 1 :
			        _buf[2] = 0x1;
			        break;
			    case 2 :
			    	_buf[2] = 0x2;
			        break;
			    default:
			    	_buf[2] = 0x0;
			    	break;
			}

			if (hid_send_feature_report(_hidDev, _buf, 4) < 0)
			{
				retVal = G15_ERROR_WRITING_USB_DEVICE;
			}
		}
	}

	return retVal;
}


int G15Interface::setLEDs(unsigned int leds)
{
	int retVal = G15_ERROR_UNSUPPORTED;

	// Since this is a macro key backlight function...
	if (getCapabilities() & G15_MKEYS)
	{
		retVal = G15_ERROR_OPENING_USB_DEVICE;
		if (_hidDev != NULL)
		{
			if (getCapabilities() & G15_RGB_BKLT_CNTL)
			{
				// For now, we're going to presume the behavior is 1:1 this, even
				// if the HID reports are differing for the G13 and the G510s (If we
				// find more deltas here, we'll adjust accordingly...)
				if (getCapabilities() & G15_IS_G13)
				{
					leds = ReverseBitsInByte(leds) >> 4;
					_buf[0] = 5;
					_buf[1] = (unsigned char)leds << 4 | (unsigned char)leds;
					_buf[2] = 0;
					_buf[3] = 0;
					_buf[4] = 0;
				}
				else
				{
					_buf[0] = 4;
					_buf[1] = (unsigned char)leds << 4;
				}
				if (hid_send_feature_report(_hidDev, _buf, (getCapabilities() & G15_IS_G13) ? 5 : 2) < 0)
				{
					retVal = G15_ERROR_WRITING_USB_DEVICE;
				}
			}
			else
			{
				// This is a bit screwy, but it's the inverse of the reversed bit order
				// that sets the G15 style macro buttons here via HID...so...
				_buf[0] = 2;
				_buf[1] = 4;
				_buf[2] = ~(uint8_t)ReverseBitsInByte((uint8_t) leds << 4);;
				_buf[3] = 0;
				if (hid_send_feature_report(_hidDev, _buf, 4) < 0)
				{
					retVal = G15_ERROR_WRITING_USB_DEVICE;
				}
			}
		}
	}

	return retVal;
}

// The LED levels are relative (there's only 4 brightnesses.) so it will take it
// and set the actual level proportionate to the slot it lands in.
int G15Interface::setRGBLEDColor(unsigned char r, unsigned char g, unsigned char b)
{
	int retVal = G15_ERROR_UNSUPPORTED;

	// Since this is a function of at least having keys and RGB backlight
	// support...
	if ((getCapabilities() & G15_KEYS) && (getCapabilities() & G15_RGB_BKLT_CNTL))
	{
		retVal = G15_ERROR_OPENING_USB_DEVICE;
		if (_hidDev != NULL)
		{
			// Set up the feature report...
			_buf[0] = (getCapabilities() & G15_IS_G13) ? 7 : 5;
			_buf[1] = r;
			_buf[2] = g;
			_buf[3] = b;
			_buf[4] = 1;

			int rc = hid_send_feature_report(_hidDev, _buf, (getCapabilities() & G15_IS_G13) ? 5 : 4);
			if(rc < 0)
			{
				retVal = G15_ERROR_WRITING_USB_DEVICE;
			}
		}
	}

	return retVal;
}


// Munged **directly** from the original libg15 code.  This is the "correct" magic, regardless
// of the wrapping layer (since even Gnome15 uses libg15 for it's "direct" mode driver)
// to get a pixmap into the right format for the LCD.  We use this in the next method
// call to format this into a data stream for G15-class (not to be confused with G19)
// LCD displays.
//
// FCE (11-28-14)
void G15Interface::dumpPixmapIntoLCDFormat(unsigned char *lcd_buffer, unsigned char const *data)
{
/*

  For a set of bytes (A, B, C, etc.) the bits representing pixels will appear on the LCD like this:

	A0 B0 C0
	A1 B1 C1
	A2 B2 C2
	A3 B3 C3 ... and across for G15_LCD_WIDTH bytes
	A4 B4 C4
	A5 B5 C5
	A6 B6 C6
	A7 B7 C7

	A0
	A1  <- second 8-pixel-high row starts straight after the last byte on
	A2     the previous row
	A3
	A4
	A5
	A6
	A7
	A8

	A0
	...
	A0
	...
	A0
	...
	A0
	A1 <- only the first three bits are shown on the bottom row (the last three
	A2    pixels of the 43-pixel high display.)


*/

    unsigned int output_offset = G15_LCD_OFFSET;
    unsigned int base_offset = 0;
    unsigned int curr_row = 0;
    unsigned int curr_col = 0;

    /* Five 8-pixel rows + a little 3-pixel row.  This formula will calculate
       the minimum number of bytes required to hold a complete column.  (It
       basically divides by eight and rounds up the result to the nearest byte,
       but at compile time.
      */

#define G15_LCD_HEIGHT_IN_BYTES  ((G15_LCD_HEIGHT + ((8 - (G15_LCD_HEIGHT % 8)) % 8)) / 8)

    for (curr_row = 0; curr_row < G15_LCD_HEIGHT_IN_BYTES; ++curr_row)
    {
        for (curr_col = 0; curr_col < G15_LCD_WIDTH; ++curr_col)
        {
            unsigned int bit = curr_col % 8;
		/* Copy a 1x8 column of pixels across from the source image to the LCD buffer. */

            lcd_buffer[output_offset] =
			(((data[base_offset                        ] << bit) & 0x80) >> 7) |
			(((data[base_offset +  G15_LCD_WIDTH/8     ] << bit) & 0x80) >> 6) |
			(((data[base_offset + (G15_LCD_WIDTH/8 * 2)] << bit) & 0x80) >> 5) |
			(((data[base_offset + (G15_LCD_WIDTH/8 * 3)] << bit) & 0x80) >> 4) |
			(((data[base_offset + (G15_LCD_WIDTH/8 * 4)] << bit) & 0x80) >> 3) |
			(((data[base_offset + (G15_LCD_WIDTH/8 * 5)] << bit) & 0x80) >> 2) |
			(((data[base_offset + (G15_LCD_WIDTH/8 * 6)] << bit) & 0x80) >> 1) |
			(((data[base_offset + (G15_LCD_WIDTH/8 * 7)] << bit) & 0x80) >> 0);
            ++output_offset;
            if (bit == 7)
              base_offset++;
        }
	/* Jump down seven pixel-rows in the source image, since we've just
	   done a row of eight pixels in one pass (and we counted one pixel-row
  	   while we were going, so now we skip the next seven pixel-rows.) */
	base_offset += G15_LCD_WIDTH - (G15_LCD_WIDTH / 8);
    }
}


int G15Interface::writeMonoPixmapToLCD(unsigned char const *data)
{
    int retVal = G15_ERROR_UNSUPPORTED;

    if (getCapabilities() & G15_LCD)
    {
		/* The pixmap conversion function will overwrite everything after G15_LCD_OFFSET, so we only need to blank
		the buffer up to this point.  (Even though the keyboard only cares about bytes 0-23.) */
		memset(_buf, 0, G15_LCD_OFFSET);  /* G15_BUFFER_LEN); */
		dumpPixmapIntoLCDFormat(_buf, data);

		/* the keyboard needs this magic byte */
		_buf[0] = 0x03;

		// This is a HID based variant of the support for the GXXX series keyboards and the like.
		// As such, we're going to presume that we can shove this out all at once and immediately,
		// no waiting.   The HID layer should handle this appropriately and not generate issues
		// for us.
		int rc = hid_write(_hidDev, _buf, G15_BUFFER_LEN);
		if(rc < 0)
		{
			retVal = G15_ERROR_WRITING_PIXMAP;
		}
    }

    return retVal;
}


/*
 * Provide for a G15's and G510s's dominant G-Keys event type.
 */
void G15Interface::processKeyEvent5Byte(uint64_t *pressed_keys, unsigned char *buffer)
{
    *pressed_keys = 0;

    log(stderr,G15_LOG_WARN,"Keyboard: %x, %x, %x, %x, %x\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4]);

    // The header byte is a determinant of which type of event source it is (hence mapping)
    if (buffer[0] == 0x02)
    {
    	// G15/G15v2...it's...a bit convoluted...
    	*pressed_keys |= (buffer[1] & 0x3f);										// First block is fine.
    	*pressed_keys |= ((buffer[1] & 0xc0) >> 6) << G15_MAPPING_1;				// Second block comes from the high-bits, goes to M1/M2.
    	*pressed_keys |= (buffer[2] & 0x01) ? G15_KEY_LIGHT_ON : G15_KEY_LIGHT_OFF;	// Light's the first bit.
    	*pressed_keys |= (buffer[2] & 0x80) ? G15_KEY_L1 : 0;						// L1 key's the last bit.
    	*pressed_keys |= ((buffer[2] & 0x1f) >> 1) << G15_MAPPING_2;				// The L2->L5 keys are relatively sane.
    	*pressed_keys |= ((buffer[2] & 0x60) >> 5) << G15_MAPPING_3;				// The M3/MR keys are also so.
    }

    if (buffer[0] == 0x03)
    {
    	// G510s...at least a little simpler...G-keys are sane.  The Macro keys are sane.
    	*pressed_keys |= buffer[1];
    	*pressed_keys |= buffer[2] << G510s_MAPPING_1;
    	*pressed_keys |= (buffer[3] & 0x03) << G510s_MAPPING_2;
     	*pressed_keys |= ((buffer[3] & 0xf0) >> 4) << G510s_MAPPING_3; 				// Macro keys block.
     	*pressed_keys |= buffer[4] << G510s_MAPPING_4;								// LCD control keys block.
    }
}



// The Media Keys are not fully presented on the G510s.  They're handed to us as two-
// byte event packets via the HID edge...
void G15Interface::processKeyEvent2Byte(uint64_t *pressed_keys, unsigned char *buffer)
{
	*pressed_keys = 0;

	log(stderr,G15_LOG_WARN,"Keyboard: %x, %x, %x, %x\n",buffer[0],buffer[1],buffer[2],buffer[3]);

	if (buffer[0] == 0x02)
	{
		*pressed_keys |= uint64_t(buffer[1]) << G510s_MAPPING_5;
	}

	if (buffer[0] == 0x04)
	{
		if (buffer[1] == 0x00)
			*pressed_keys |= G15_KEY_LIGHT_ON;

		if (buffer[1] == 0x04)
			*pressed_keys |= G15_KEY_LIGHT_OFF;
	}
}

// The G13 generates 8-byte events solely.  The G510s does an...odd mix of 2, 5, and 8 byte
// events.  This is to handle the 8-byters where they come.
void G15Interface::processKeyEvent8Byte(uint64_t *pressed_keys, int *xjoy, int *yjoy, unsigned char *buffer)
{
	*pressed_keys = 0;

	log(stderr,G15_LOG_WARN,"Keyboard: %x, %x, %x, %x, %x, %x, %x, %x\n",
			buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);

	if (buffer[0] == 0x01)
	{
		// For now, we're doing just the G13 via the 8-byte event edge.  The G510s needs some work
		// and still presents via the 5-byte event interface in a relatively sane mapping- so we're
		// not using the other for now...
		if (getCapabilities() & G15_IS_G13)
		{
			// First two bytes after header are +/- 128 X/Y values for the joystick on the G13.
			*xjoy = (int)((signed char)buffer[1]);
			*yjoy = (int)((signed char)buffer[2]);

			// Fourth byte is the first set of 8 G-keys on the G13- one-to-one with our mapping.
			*pressed_keys = buffer[3];

			// Fifth byte is the second set of 8 G-keys on the G13- one-to-one with our mapping.
			*pressed_keys |= (buffer[4] << G13_MAPPING_1);

			// Sixth byte has the final set of G-keys, combined with the backlight switch on/off
			// bit...  SO, slightly more thought and ops with this one...
			*pressed_keys |= ((buffer[5] & 0x7f) << G13_MAPPING_2);
			*pressed_keys |= (buffer[5] & 0x80) ? G15_KEY_LIGHT_ON : G15_KEY_LIGHT_OFF;

			// Seventh byte has the display keypad bits and all but the MR record button...
			*pressed_keys |= (buffer[6] << G13_MAPPING_3);

			// Eighth byte has the MR button and the thumbstick's buttons as the first three bytes...
			*pressed_keys |= (uint64_t(buffer[7] & 0x07) << G13_MAPPING_4);
		}
	}
}

/*
 * This is the re-worked interface (In a keep it simple mindset...)- we just added joystick
 * events to the call- and either we give you something for the the thumbstick...or not.
 * It only has bearing with an 8-byte HID event anyhow- with the first two bytes being
 * the values passed back.
 */
int G15Interface::getDeviceEvent(uint64_t *pressed_keys, int *xjoy, int *yjoy, unsigned int timeout)
{
	unsigned char buffer[G15_KEY_READ_LENGTH];
	int ret = 0;

	// Clear the joystick values...
	*xjoy = *yjoy = 0;

	// If the device doesn't support keys, just error out as unsupported...
	int retVal = (getCapabilities() & G15_KEYS) ? G15_ERROR_TRY_AGAIN : G15_ERROR_UNSUPPORTED;

	// This is to catch retryable failures.  Some devices push spurious crap out the
	// HID edge (why are you generating TWO events per keypress on some devices, Logitech!?)
	// so we need the ability to get the desired keys off without passing bad junk back
	// up to the caller.
	//
	// (01-04-15)
	//
	// FIXME -- we really need to see if we can reconcile the G13's thumstick and anytime
	//			the HID edge hands us the whole keyboard properly.
	while (retVal == G15_ERROR_TRY_AGAIN)
	{
		ret = hid_read_timeout(_hidDev, buffer, sizeof(buffer), timeout);
		switch(ret)
		{
		case 2:
			processKeyEvent2Byte(pressed_keys, buffer);
			if (*pressed_keys)
			{
				retVal = G15_NO_ERROR;
			}
			break;

		case 5:
			processKeyEvent5Byte(pressed_keys, buffer);
			{
				retVal = G15_NO_ERROR;
			}
			break;

		case 7:
			// Catch the 7-byte entry and just quietly tell the world to try again...
			break;

		case 8:
			// G13's seem to be using this out of the HID interface.  We have a bit
			// different behavior from the G510s and the G15v2 I have in hand off
			// of the HID interface.  It's self-consistent, so I'm forging forward
			// with it all.  HID's going to be a better solution, ultimately, for
			// this stuff as it's how pretty much every other "magic" gamer keyboard
			// actually seems to work.
			processKeyEvent8Byte(pressed_keys, xjoy, yjoy, buffer);
			if ((getCapabilities() & G15_IS_G13) && (*pressed_keys))
			{
				retVal = G15_NO_ERROR;
			}
			// Quietly do a retry if it's a G510s for now...don't have those mappings.  (FIXME -- See if we can TURN THAT OFF!)
			break;

		case 0:
			// Timeout...if the caller gives us one- if not, we should
			// never really see this...
			retVal = G15_ERROR_TIMEOUT;
			break;

		default:
			cout << "read op got " << ret << "bytes - ";
			for (int i = 0; i < ret; i++)
			{
				cout << std::hex << int(buffer[i]) << " ";
			}
			cout << endl;
			retVal = G15_ERROR_READING_USB_DEVICE;
			break;
		}
	}

	return retVal;
}

