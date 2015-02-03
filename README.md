G15Inteface
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
in question.

What works:

- Macro Record/Selection LEDs
- Macro Record/Selection button detection
- Backlight brightness on a G15.
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

What doesn't work, at-all (Yet- it *IS* a WIP...):

- *ANY* of the G19 support- this MAY be done as a separate little
  beastie from this (i.e. I'll refactor this one a bit and make a more
  generic interface than this that provides a common edge to work with,
  etc...)  This depends on how much different it is other than the
  display (which I'll account for with a small refactor if that's the
  only oddball thing...)
- The thumbstick on the G13.  It's about to be the next thing up
  on the hit-parade as we rework and refactor the getPressedKeys()
  method.  It is expected to be renamed to getDeviceEvent() and
  it'll pass the G13 thumbstick as a separate parameter and it'll
  just be un-set for anything that doesn't present it.


Roadmap:

- Thumbstick support proper.
- Adding "rendering" so that we can frame in versions of the tools that
  used to be offered by G15tools.  I'm still thinking a bit on this.
  I'd like to make it be almost like a simple framebuffer, which isn't
  quite like the old tools were.  I'd like to render into a canvas with
  almost any bit-bashing API you name and tell it to update off of the
  canvas.  From there, I've the basis for reworking libg15render in the
  same manner this has been for the base interface library.  In the end
  we should have the basics available for people to use directly and
  easily in ONE library- and then extend it by a G15daemon equivalent.
- (Not directly with this...because the above two almost complete this
   piece...) Base daemon support- this is good for doing cutesy one-
   shot pieces like the "less" clone to the LCD and for the core for
   something larger.  But...we need something that's at root privs
   level.  Something vetted well.  Something that can do mappings of
   select keys to the input interface subsystem since the G510s doesn't
   do it's media keys "right" under all conditions.  Something that
   can allow us to have different profiles, colors, etc. for each
   user as they're active on the console.


This is being exposed in Git with an intent of publishing early and often.

It *currently* is not really intended for general public consumption- but
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
