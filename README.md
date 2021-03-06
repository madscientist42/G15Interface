G15Interface
===========

WIP of a re-spin of G15Tools using HIDAPI instead of libusb.

This is a Work In Progress of a project to produce a G15Tools-ng (or rather the
basis for that and a bit more...).

It is *NOT* intended to be a drop-in replacement of the G15Tools, the included
lib, or any of that.  It's a functional re-work of the whole thing with an eye
on being quite a bit more sophisticated (The original was a bit dumb in that it
only worked with the first device found and you were done.  If you have a Z10
speaker set, you didn't get it to work or your nifty G510s or G13 wouldn't work
right.  Not really acceptable...in any manner...)  Right now, it's framed in to
be able to list and provide control support for multiple devices on the machine
in question.  (As an aside, this is using HIDAPI- this means it can technically
be made to work with Windows AND OSX, but hasn't been validated there yet...)

What works:

- Macro Record/Selection LEDs
- Macro Record/Selection button detection
- Backlight brightness on a G15, either display or keyboard
- Via the G15 specific interface, control an RGB keyboard as white with the 
  same level settings.
- RGB color selection on a G13 or G510s
- LCD bitmap setting on any G15 class "gamepad" including G13, G15v1, G15v2,
  and G510s
- Keymappings to bitfield for G15, G13, G510s.  This means you get the G-Keys
  feedback for your specific keyboard.  I figured out the clean magic for bit
  shifting the HID report bytes back from the devices into the generic, universal
  mapping for all devices from this library.  Right now, I'm presuming (right or
  wrong) that the G510s ground rules are going to be the same or similar for the
  G710 and G19 keyboard.  I'll know more about this when I can afford to lay hands
  on one of each of them...
- Reset works for G15 and G510s.  Going to claim that one and make it such that
  it's unsupported for the G13.  Besides, it's really a special case- we don't
  *WANT* to reset except for a daemon or the like dying anyhow.
- Blocking of key events going to /dev/input from the G-Keys and special
  keys edge of things on select keyboards.  If the edge we're working with
  presents a /dev/input event interface, we grab it for the duration of the
  interface instance being initialized and open to block junk actions we
  DO NOT WANT.
- The G13 Thumbstick.  Added joystick x/y parameters in an extension and re-name of
  the method call for getting input events out of the API.

What doesn't work, at-all (Yet- it *IS* a WIP...):

- *ANY* of the G19 support- this MAY be done as a separate little
  beastie from this (i.e. I'll refactor this one a bit and make a more
  generic interface than this that provides a common edge to work with,
  etc...)  This depends on how much different it is other than the
  display (which I'll account for with a small refactor if that's the
  only oddball thing...)


Roadmap:

- Bolting this on, at least initially, to a fork of Gnome15, intended
  to bring the power of G15Interface (which is cleaner than the direct
  method they came up with, cleaner than libg15, and cleaner than the 
  kernel modules (which are in mainline, but are viewed as semi-deprecated
  by at least one of the main kernel maintainers (Greg K-H had an at-length
  discussion with the Razer keyboard support project on this subject...he
  dissuaded them to make a driver.  Too many differing devices, much 
  more cleanly supported via libhid like I'm doing with G15 and soon
  G19.
- Adding G19 aupport as hinted at above.
- Add "rendering" in a manner that exposes a /dev/fb framebuffer-like
  interface to the library (See the next remarks for reasons why...)
- Refactor the library as needed to handle being used by a "universal"
  backlight management tool for backlight-only keyboards (G710+, G910)
  and to provide a consistent API for that same tool for OTHER keyboards
  so that I can provide something robust for Steelseries APEX, MSI, etc.)

This is being exposed in Git (Currently on GitHub) with an intent of
publishing early and often.

It _*currently*_ is not really intended for general public consumption (Though
this may change fairly soon...it's nearly ready for this role...) - but
is up for me being able to have multiple backups of the work and history
and to allow someone that is a dev to scoop it up and extend it (Hey, if you
like where I'm taking this and you beat me to the punch in finishing part
or all of the functionality that doesn't work- knock yourself out.  It'll
get it all done that much sooner- and this is just the beginning of something
that is a real solution to a real problem (though a small one) on Linux
and it'll get us moved forward just that much faster doing it.)

Ultimately, this will be merged into a larger compliation that encompasses
this repo's contents and this will be deprecated.  The README will be
updated accordingly when that happens.
