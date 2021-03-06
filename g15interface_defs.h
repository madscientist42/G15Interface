/*
 * g15interface_defs.h
 *
 *  Created on: Dec 5, 2016
 *      Author: frank
 */

#ifndef G15INTERFACE_DEFS_H_
#define G15INTERFACE_DEFS_H_

#define G15_KEY_READ_LENGTH 9

// Helper macro for defining the support table...
#define DEVICE(name, vendorid, productid, caps) { \
    name, \
    vendorid, \
    productid, \
    caps \
}

// The supported devices list structure...
typedef struct
{
  const char 	*name;			// Name used by the detect code for the device we're detecting against...
  uint32_t 		vendorid;		// USB vendor ID
  uint32_t		productid;		// USB product ID
  uint32_t		caps; 			// capability bitfield, e.g. G15_LCD|G15_KEYS;
} libg15_devices_t;

// Define our error types...
typedef enum
{
	G15_NO_ERROR,
	G15_ERROR_OPENING_USB_DEVICE,
	G15_ERROR_WRITING_USB_DEVICE,
	G15_ERROR_READING_USB_DEVICE,
	G15_ERROR_TIMEOUT,
	G15_ERROR_TRY_AGAIN,
	G15_ERROR_WRITING_PIXMAP,
	G15_ERROR_WRITING_BUFFER,
	G15_ERROR_UNSUPPORTED
} G15_ERROR;


// Define logging levels...
typedef enum
{
	G15_LOG_NONE,
	G15_LOG_INFO,
	G15_LOG_WARN
} G15_LOG_LEVEL;


// Define some constant values.  It's somewhat cleaner using
// a user specified enum value than a #define and doesn't eat
// resources like using a const <foo> value to do it.

// Capability bits.
typedef enum
{
	G15_NONE				= 0,
	G15_LCD 				= 1<<0,
	G15_RGB_LCD				= 1<<1,
	G15_KEYS 				= 1<<2,
	G15_MKEYS				= 1<<3,
	G15_BACKLIGHT_CNTL 		= 1<<4,
	G15_RED_BLUE_BKLT_CNTL  = 1<<5,
	G15_RGB_BKLT_CNTL		= 1<<6,
	G15_CONTRAST_CNTL		= 1<<7,
	G15_DEVICE_5BYTE_RETURN = 1<<8,
	G15_DUAL_ENDPOINT		= 1<<9,
	G19_LCD					= 1<<10,		// Not supported YET- got to get a G19 in hand...
	G15_IS_G13				= 1<<11			// Is a special-case override for the G13 for READ ops and a few other things...
} G15_CAPABILITY_BITS;

// LCD size values...
typedef enum
{
	G15_LCD_OFFSET = 32,
	G15_LCD_HEIGHT = 43,
	G15_LCD_WIDTH = 160
} G15_LCD_SIZES;

// Contrast settings for the LCD
typedef enum
{
	G15_CONTRAST_LOW=0,
	G15_CONTRAST_MEDIUM,
	G15_CONTRAST_HIGH
} G15_CONTRAST;

// Brightness settings for the LCD
typedef enum
{
	G15_BRIGHTNESS_DARK=0,
	G15_BRIGHTNESS_MEDIUM,
	G15_BRIGHTNESS_BRIGHT
} G15_BRIGHTNESS;

// Buffer length.
typedef enum
{
	G15_BUFFER_LEN = 0x03e0
} G15_BUFFER;

// Macro key highlight LED values.
typedef enum
{
	G15_LED_M1 = 1<<3,
	G15_LED_M2 = 1<<2,
	G15_LED_M3 = 1<<1,
	G15_LED_MR = 1<<0
} G15_LED;

typedef enum : uint64_t
{
	// G-keys...
	G15_KEY_G1  = uint64_t(1)<<0,
	G15_KEY_G2  = uint64_t(1)<<1,
	G15_KEY_G3  = uint64_t(1)<<2,
	G15_KEY_G4  = uint64_t(1)<<3,
	G15_KEY_G5  = uint64_t(1)<<4,
	G15_KEY_G6  = uint64_t(1)<<5,
	G15_KEY_G7  = uint64_t(1)<<6,
	G15_KEY_G8  = uint64_t(1)<<7,
	G15_KEY_G9  = uint64_t(1)<<8,
	G15_KEY_G10 = uint64_t(1)<<9,
	G15_KEY_G11 = uint64_t(1)<<10,
	G15_KEY_G12 = uint64_t(1)<<11,
	G15_KEY_G13 = uint64_t(1)<<12,
	G15_KEY_G14 = uint64_t(1)<<13,
	G15_KEY_G15 = uint64_t(1)<<14,
	G15_KEY_G16 = uint64_t(1)<<15,
	G15_KEY_G17 = uint64_t(1)<<16,
	G15_KEY_G18 = uint64_t(1)<<17,
	G15_KEY_G19 = uint64_t(1)<<18,
	G15_KEY_G20 = uint64_t(1)<<19,
	G15_KEY_G21 = uint64_t(1)<<20,
	G15_KEY_G22 = uint64_t(1)<<21,

	// LCD-pad control keys...
	G15_KEY_L1  = uint64_t(1)<<22,
	G15_KEY_L2  = uint64_t(1)<<23,
	G15_KEY_L3  = uint64_t(1)<<24,
	G15_KEY_L4  = uint64_t(1)<<25,
	G15_KEY_L5  = uint64_t(1)<<26,

	// Macro-bank control keys...
	G15_KEY_M1  = uint64_t(1)<<27,
	G15_KEY_M2  = uint64_t(1)<<28,
	G15_KEY_M3  = uint64_t(1)<<29,
	G15_KEY_MR  = uint64_t(1)<<30,

	// Thumbstick control keys...
	G15_KEY_JOY_1 = uint64_t(1)<<31,
	G15_KEY_JOY_2 = uint64_t(1)<<32,

	// Media player control button events...
	G15_KEY_FWD = uint64_t(1)<<33,
	G15_KEY_REW = uint64_t(1)<<34,
	G15_KEY_PLAY = uint64_t(1)<<35,
	G15_KEY_STOP = uint64_t(1)<<36,
	G15_KEY_MUTE = uint64_t(1)<<37,
	G15_KEY_VOL_UP = uint64_t(1)<<38,
	G15_KEY_VOL_DOWN = uint64_t(1)<<39,

	// Backlight hardware control switch events...
	G15_KEY_LIGHT_ON = uint64_t(1)<<40,
	G15_KEY_LIGHT_OFF = uint64_t(1)<<41,

} G15_KEY;


// We're doing this because the bits map properly to our mappings
// for the G13 keypad- but they're not byte aligned except for the G-keys.
typedef enum
{
	G13_MAPPING_1 = 8,		// First bit-shift.  Corresponds to the second block of G-buttons.
	G13_MAPPING_2 = 16,		// Second bit-shift.  Corresponds to the last block of G-buttons.
	G13_MAPPING_3 = 22, 	// Third bit-shift.  Corresponds to the starting bit of our LCD control buttons.
	G13_MAPPING_4 = 30,		// Fourth bit-shift.  Corresponds to the MR key's position in the bitfield.
} G13_MAPPING_OFFSETS;

// The same story here, because it's a bit more convoluted, yet fewer bits...
typedef enum
{
	G15_MAPPING_1 = 27,		// First bit-shift.  Move the block of bits to the M1 position.
	G15_MAPPING_2 = 23, 	// Second bit-shift.  Move the specified block to the L2 position.
	G15_MAPPING_3 = 29,		// Third bit-shift.  Move the specified block to the M3 position.
} G15_MAPPING_OFFSETS;

typedef enum
{
	G510s_MAPPING_1 = 8,	// First bit-shift.  Corresponds to the second block of G-buttons.
	G510s_MAPPING_2 = 16,	// Second bit-shift.  Corresponds to the last block of G-buttons (all of TWO...)
	G510s_MAPPING_3 = 27,	// Third bit-shift.   Corresponds to the M1->MR block's position.
	G510s_MAPPING_4 = 22,	// Fourth bit-shift.  Corresponds to the L1->L5 LCD button block's position.
	G510s_MAPPING_5 = 33,   // Fifth bit-shift.  Corresponds to the media-key events.
} G510s_MAPPING_OFFSETS;

#endif /* G15INTERFACE_DEFS_H_ */
