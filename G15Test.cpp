//============================================================================
// Name        : G15Test.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <unistd.h>
#include <poll.h>	// Cheating to get millisecond resolution sleeps here...

#include "G15Interface.h"

#include <iostream>
using namespace std;

int main()
{
	uint8_t	testBuf[G15_BUFFER_LEN];
	bool 	m1_state = false;
	bool 	m2_state = false;
	bool 	m3_state = false;
	int		led_val = 0;

	// Initialize the HIDAPI...
	hid_init();

    // Initial tests...
    G15InterfaceList test = G15Interface::getAvailableInterfaces();
    cout << "Detected " << test.size() << " supported Logitech interfaces." << endl << endl;
    for (unsigned int i = 0; i < test.size(); i++)
    {
    	cout << test[i].deviceModelName() << " | " << test[i].getCapabilities() << endl;
    }
    cout << endl;

    // Can we init the device properly?
    if (test.size() > 0)
    {
    	cout << "Attempting to init..." << endl;
    	int ret = test[0].init();
    	if(ret == G15_NO_ERROR)
    	{
    		cout << "init succeeded" << endl;
    	}
    	else
    	{
    		cout << "init failed... ERROR = " << ret << endl;
    	}
    }

    // If we're inited...  Can we dink with the device's features?
    if (test[0].isInited())
    {
    	cout << "Attempting to control the macro control button LEDs" << endl;
    	test[0].setLEDs(G15_LED_M1);
    	sleep(1);
    	test[0].setLEDs(G15_LED_M2);
    	sleep(1);
    	test[0].setLEDs(G15_LED_M3);
    	sleep(1);
    	test[0].setLEDs(G15_LED_MR);
    	sleep(1);
    	test[0].setLEDs(G15_LED_M2 | G15_LED_MR);
    	sleep(1);
    	test[0].setLEDs(0);

    	if (!(test[0].getCapabilities() & G15_RGB_BKLT_CNTL))
    	{
        	cout << "Attempting to adjust LCD brightness" << endl;
        	cout << test[0].setLCDBrightness(0);
        	sleep(1);
        	cout << test[0].setLCDBrightness(1);
        	sleep(1);
        	cout << test[0].setLCDBrightness(2);

    		if (test[0].getCapabilities() & G15_CONTRAST_CNTL)
    		{
            	cout << "Attempting to set LCD contrast" << endl;
            	test[0].setLCDContrast(0);
            	sleep(1);
            	test[0].setLCDContrast(1);
            	sleep(1);
            	test[0].setLCDContrast(2);

    		}

        	cout << "Attempting to adjust keyboard brightness" << endl;
        	test[0].setKBBrightness(0);
        	sleep(1);
        	test[0].setKBBrightness(1);
        	sleep(1);
        	test[0].setKBBrightness(2);
    	}
    	else
    	{
    		cout << "Skipping non-RGB mode control tests..." << endl;
    	}

		if (test[0].getCapabilities() & G15_RGB_BKLT_CNTL)
		{
	    	cout << "Attempting color wheel set pass for RGB backlight..." << endl;
	    	for (int i = 0; i < 4; i++)
	    	{
	    		for (int j = 0; j < 8; j++)
	    		{
	    			int k = (j & 1) ? i * 64 : 0;
	    			int l = (j & 2) ? i * 64 : 0;
	    			int m = (j & 4) ? i * 64 : 0;
					test[0].setRGBLEDColor(k, l, m);
					sleep(1);
	    		}
	    	}
		}

		if (test[0].getCapabilities() & G15_LCD)
		{
			cout << "Attempting to set the display with a pattern" << endl;
			for (int i = 0; i < G15_BUFFER_LEN; i++)
			{
				testBuf[i] = i % 256;
			}
			test[0].writeMonoPixmapToLCD(testBuf);
			sleep(3);
		}

		if (test[0].getCapabilities() & G15_KEYS) 	// Pretty much everything has "G-Keys", but...
		{
			cout << "Attempting to query G-Keys, etc." << endl;
			uint64_t g_keys = 0;
			while ((test[0].getPressedKeys(&g_keys, -1) == G15_NO_ERROR) && !(g_keys & G15_KEY_MR))		// We'll key exit off the Macro Record button for this test...
			{
				cout << "Pressed Keys : " << hex << g_keys << endl;

				// Do a few cute things.  Let's set the LEDs based off of state...
				led_val = 0;
				if (g_keys & G15_KEY_M1)
				{
					m1_state = ~m1_state;
					led_val |= (m1_state) ? G15_LED_M1 : 0;
				}
				if (g_keys & G15_KEY_M2)
				{
					m2_state = ~m2_state;
					led_val |= (m2_state) ? G15_LED_M2 : 0;
				}
				if (g_keys & G15_KEY_M3)
				{
					m3_state = ~m3_state;
					led_val |= (m3_state) ? G15_LED_M3 : 0;
				}
				test[0].setLEDs(led_val);
				poll(NULL, 0, 100);
			}
		}


		// Full clean-up of things.  We need to figure out the booby-traps since
    	// libhidapi's test UI seems to do things that leave the keyboard dead
    	// within seconds of the disconnect.  Need to see what they're doing
    	// AND NOT DO IT.  :-D
    	cout << "Resetting the device from the the user HID side..." << endl;
    	test[0].reset();
    }

    // Close our interface down...
    test[0].close();

    // Shut the HIDAPI down...
    hid_exit();

	return 0;
}
