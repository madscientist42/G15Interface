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

What sort-of works:

- "Reset" is known to sort-of work on the G15v1 and G510s.  It's a series of
  HID reports that appear to do the tasks in question.  But, I can't for-sure
  say that it's working fully as desired or not.  The desire is to effectively
  have the device disconnect itself from the USB bus and hotplug it right back
  in so that you have a clean slate once you disconnect the controlling app
  and not leave the device hosed up on you.  One of the big problems with
  the original library (and all the apps that used it, including Gnome15...)
  is that if the controlling daemon, whether it was G15Daemon or Gnome15's
  service, died you'd have the G510s lock up, needing you to hotplug it to have
  it come back and the daemon service to control it again.  Again...not
  terribly acceptable.  I *THINK* I have a handle on this, but I don't seem
  to have a magic...yet...for the G13.  Without it...I can't really claim
  that the others are properly working because I don't have a solid handle
  on this one...yet.  Ultimately, I'd like to couple the reset to the unwind
  so that under almost all conditions of a crash with this library (which
  I hope never happens, but...  >:-D) this will force the reset calls before
  dying.

What doesn't work, at-all (Yet- it *IS* a WIP...):

- *ANY* of the G19 support- this MAY be done as a separate little
  beastie from this (i.e. I'll refactor this one a bit and make a more
  generic interface than this that provides a common edge to work with,
  etc...)  This depends on how much different it is other than the
  display (which I'll account for with a small refactor if that's the
  only oddball thing...)
- The thumbstick on the G13.  I'm...unsure...right at the moment how to
  treat it.  Mapping's already clear off of it's input report events
  (Second and Third bytes of the 8-byte event off of the G13 is the
  X and Y of the Thumbstick.)- but I need to determine what to do
  with the WORD size of data.  Make it available as a re-worked
  get keys where it's there if I've got it, it's just "empty" if
  I don't?  Dunno right now.  Got to think on it.  It's got to
  make some sense and be processable by a daemon to produce a mouse
  or joystick type event from the library's output for it.
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
