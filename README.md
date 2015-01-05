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

What doesn't work (Yet- it *IS* a WIP...):

- *ANY* of the G19 support- this MAY be done as a separate little
  beastie from this (i.e. I'll refactor this one a bit and make a more
  generic interface than this that provides a common edge to work with,
  etc...)  This depends on how much different it is other than the
  display (which I'll account for with a small refactor if that's the
  only oddball thing...)
- Keeping the Key events from reaching the input edge without this lib's
  say-so (If you're controlling the G15/G510s, you don't want the keyboard
  events from the G-keys (Which do come out against /dev/input, etc.) going
  out to everything.  You want to intercept them and hand them out as read
  G-keys, be able to intercept the keystrokes as normal mapped key-events
  so you can do key-macro recording with a daemon upstream of this, etc.)
- Contrast setting on a G15v1.  This is more because I don't have one in
  hand than anything else.  (Patches welcome- donations of a G15v1 would
  be even moreso...)  I don't see this being much of an issue, overall.
  this is because the feature was only on the V1 of the series, and
  it strikes me as limited in the scope of the in-progress framework.

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
