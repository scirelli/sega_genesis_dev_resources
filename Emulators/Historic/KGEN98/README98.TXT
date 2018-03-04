
------------------------------------------------------------------------------
		  KGen 98 V0.4b BETA TEST
                    Sega Genesis / MegaDrive Emulator.
         Written by Steve Snake, Kode and Muzak Ltd, Jan-Feb/May 1998.
------------------------------------------------------------------------------

------------------------------------------------------------------------------

   This is an emulator for the Sega Genesis/MegaDrive, one of my favourite
               consoles of all time, for a Pentium PC in MSDOS.

------------------------------------------------------------------------------

HISTORY:
--------

Version 0.4b : 8th June 1998.

Various compatibility issues have been fixed. Surging Aura, MazinSaga, Dashin'
Desperadoes now work correctly, to name but three.

Improved ScanLines support. (VESA 2.0 ONLY). Should work on more video cards.
Use at your own risk.

Altered EUROPEAN mode. Previous versions only had full European support when
in 50Hz mode. Note that music / sampled sounds will sound faster - This is
due to emulating 1/50th Sec. of European Genesis in 1/60th Sec ! But this will
fix the European games that only worked in 50Hz mode.

Stopped LEFT & RIGHT or UP & DOWN being pressed at the same time. This can
happen on a real Genesis with a worn pad, and programmers are SUPPOSED to make
sure it won't affect their games.... But some people don't listen, apparently.

Improved the File Requester Keyboard Shortcuts. You can now type a few letters
within a short space of time, and it will jump to the first file starting with
those letters. The current search string is displayed at the top of the file
requester. Also, pressing SHIFT and a letter will jump directly to the Disk
Drive of the same letter.

Added 16-Bit Video Mode. (VESA 2.0 ONLY). This was ported from the now defunct
KGen-X, and has been considerably rewritten to cope with the new Raster FX
System. It is slower than the 8-Bit mode, but allows games that try to do more
Palette Raster FX than the 8-Bit mode can cope with to function properly. Note
that there are a few things left to do in this mode - for example, you cannot
currently save a ScreenShot when you are in 16-Bit mode, and the only Joypad
modes that work in this mode are SLOW and SIDEWINDER.


Various small changes to the GUI.

Various cosmetic fixes.


Please note to all the people STILL complaining that the road in Road Rash is
blue - it is NOT if you are using a VESA 2.0 driver, when the Palette Raster
FX kick in.

------------------------------------------------------------------------------

Version 0.3b : 3rd June 1998.

Fixed minor VDP problem.

Added bits to File Requester that I forgot to add last night :-(
Maybe I should get some sleep.....

Yes, I know I forgot to change the version no. in 0.2b ;-) I realised right
after I uploaded it.

Further to the SIDEWINDER driver and Windows 95 - it has been confirmed that
if you press the MODE button four times (so the light comes back on) once in
KGen 98, everything works fine.

I'd just like to point out that a LOT of people have mailed me saying that
ToeJam and Earl doesn't work - So I'd just like to say:

ToeJam and Earl works PERFECTLY - IF you switch to 'FULL' Raster FX mode.

As I said before, some games work better in 'MOST' mode, some in 'FULL' mode,
and others in 'NONE'. It's really up to you to find out - but please, if it's
not working, try another mode !

------------------------------------------------------------------------------

Version 0.2b : 2nd June 1998.

Fixed problem with KGen 98 locking with 8-bit soundcards. Turned out to be a
line that I forgot to delete :-)

Fixed VDP Status register - I don't know why I forgot this, since I had the
same problem way back in KGen 0.32b !

Fixed problem with Joypad modes - again, something that should have been
deleted.

Added Long File Name Support. I had actually written this before, but it got
lost with the changes I made to the GUI / hard drive problem.
Completely forgot to put it back in ! But it's there now.

Also added -L command line option to disable Long Filename Support, incase it
causes problems.... DOS 6.22, for example, reports no error..... I got around
that problem, but other versions of DOS, or other OSses, may still die.



PLEASE NOTE:

A lot of people have been mailing me saying the SideWinder stuff doesn't work.

   IT WILL NOT WORK IF YOU ARE RUNNING FROM WINDOWS 95 WITH THE WINDOWS 95
                         SIDEWINDER DRIVERS ACTIVE !!!

The Windows 95 drivers for DOS make the PC 'believe' that it has a standard 4
button Joypad connected. This is BAD - it means you can only use four buttons,
and you can only use one SideWinder.

To get around this, you should either:

a) Run it from pure DOS :-)
b) Disable the Windows SideWinder drivers (remove them from 'game devices')
c) I've been told that pressing the 'mode' button a couple of times when in
   KGen will cause the Windows drivers to temporarily stop reading. I can't
   try this cos I don't have the windows drivers installed :-)

Note that if option C does work, you MUST do it with only one sidewinder
plugged in, once that one is working, you can plug another one in.

------------------------------------------------------------------------------

Version 0.1b : 1st June 1998.

This is the first release of KGen 98.

KGen 98 is practically an entirely new Emulator. Around 90% of the code was
rewritten for this release. It is for this reason that I decided to change the
name - since there is every chance that things that worked before now do not,
and vice versa. Also, since more and more people have PCs that are faster than
mine :-( the focus of this project has now changed from speed to accuracy.

I also recommend that you keep KGen 0.34b - hence the renaming of the files.

Most of what you see here was written and ready for release before the end of
February 1998. The release was delayed due to interrest from Sega. Since I
have not been able to get a reply from them for a long time, I decided to tidy
it up and release it. If there are any objections from Sega, please contact me
and it will be removed.

WELL, WHAT'S THE STORY, THEN ?
------------------------------

Changes from KGen 0.34b -> KGen 98 0.1b are as follows:

Command line options have changed - see below for more information.

Complete re-write of the VDP emulation.

        All Raster Effects are now implemented.
        Shadow / Hilight mode is now 100% accurate.
        Most games that had severe graphics problems are now fixed.
        (See below for more information on this.)

Complete re-write of the YM2612 FM Sound Emulation.

        Waves calculated with a much higher degree of accuracy.
        Support for 44.1Khz 16-bit Stereo Sound. (SB16 and above only.)
        Mixer values now restored on exit.

Complete re-write of the PSG Sound Emulation.

        Any previous problems should no longer occur.
        Support for the undocumented features also.

Complete re-write of the Control System.

        Better support for Sega "Fighting Pad" 6 Button Controller.
        New Joypad modes : Fast, Medium and Slow for both 4 and 6 button pads.
        Support for one or two Microsoft SideWinder pads.

Major changes to the 68000 and Z80 emulation.

Near-Instant start-up time.

Complete re-write of the SnapShot system.

        Load SnapShot still included for old SnapShot / Genecyst saves.
        Save SnapShot removed.
        SnapShot Slot system added.
        Saves / Loads are now done from within the game using F5 / F8.

Added ScanLines video mode (VESA 2.0 Only)

Added Game Genie Support.

Parts of the GUI rewritten / improved.

Loads more parts re-written to get all the new systems working together.

Many, Many, Many bugs removed (most of it rewritten !)

------------------------------------------------------------------------------

KGen 98 was written entirely by me, Steve Snake, in 100% Assembler. Most of it
has been written in under two months, with only a few "glue" routines taken
from the original KGen. I had planned on a complete GUI overhaul, but I really
can't be bothered now, since I lost a lot of code / graphics when a HD died
recently.

In running this program you accept that I take no responsibility for any
damage done to your computer, your hard drive, or you for that matter. This
version has hardly been tested at all, and although it SHOULD be stable, I
have no idea just how stable or otherwise it is.

And now, it's the part of the file I borrow from KGen, and modify where
appropriate...... Please READ IT, however, because there are a LOT of major,
important changes and stuff. And that.

------------------------------------------------------------------------------

GUI MENU SYSTEM
---------------

At any point during emulation, you can hit the ESC key to enter the GUI Menu
System. The GUI is FULLY Keyboard driven, as this is what I, and many others
who have mailed me, prefer. If you don't like it then don't use it, because it
ISN'T going to change. No, it STILL isn't.

However, for those of you who LIKE things that are easy to use, here is some
information on it.

Up and down cursor keys are used to hilight a specific option, and RETURN to
select an option, throughout. During the File Menus you can also use PAGE UP,
PAGE DOWN, HOME and END to move through the file list. You can now also use
Keyboard shortcuts - pressing 'T', for example, will jump to the first file
starting with 'T'.

Most of it should be obvious.

LOAD ROM        : Loads an SMD, BIN or .1 file, just as with the command line.
LOAD SNAPSHOT   : Loads a KGen 0.34b SnapShot or Genecyst Save State file.

CONTROLLER MENU : Allows enabling / disabling / configuring joypads etc.
GAME GENIE MENU : Allows editing / selecting Game Genie codes.
VERSION MENU    : Allows setting of Hardware Version, or Auto-Detect.
VIDEO MENU      : Allows selection of various Video-related options.

CHECKSUMS       : Disable checksums if a game won't run with them on.

HELP PAGE       : Help Page telling you the extra available Key commands.

EXIT            : Returns to DOS.

------------------------------------------------------------------------------

CONTROLLER MENU:
----------------

You can cycle through joypad modes for FAST, MEDIUM and SLOW, for both 4 and 6
button Joypads. Choose the option that works best on your Joypad - the faster,
the better. You can also select SIDEWINDER if you have one or two Microsoft
SideWinder gamepads connected.

You should always calibrate your joypad when starting KGen, or when changing
the reading mode. Note that the SideWinder driver does not need calibrating,
and the buttons are fixed, as follows:

SIDEWINDER	GENESIS

A	A
B	B
C	C
X	X
Y	Y
Z	Z
START	START
MACRO	START

The SideWinder driver is my own, it works fine on all the PCs here.
Any problems, you know what to do.

------------------------------------------------------------------------------

GAME GENIE MENU:
----------------

From here, you can enter both Game Genie codes and Hex codes. Game Genie codes
are in the format ABCD-EFGH (the '-' is optional). Hex codes are in the format
AAAAAA:DDDD (24 bit Hex address, and 16 bit Hex Data). When the code is
enabled, the 16 bit data generated by the code will be written to the 24 bit
address.

Along with the code, you can enter a descriptive name to remind you what the
code does. Here is an example:

GAME: Alien 3
CODE: AJBT-AAEY
NAME: START ON LEVEL 2

Select "Add Code", and a cursor will appear in the CODE box. Type the code in,
and use cursor down to move to the NAME box. Type in your codes name here.
Pressing <RETURN> will add the code to the list, if it is valid.

Note that if no NAME is entered the CODE value will be used.

codes can be saved to a file (called GAMENAME.PAT) that will be reloaded next
time the game is loaded. Just select "Save Codes".

There are lots of sites on the internet containing Game Genie codes for most
of the popular games. You can cut and paste in any text editor the entire list
of codes for a game into it's .PAT file, saving you a lot of typing.

The .PAT file has one code per line, in the format:

CODE  NAME

The .PAT file is named as such to maintain compatibility with Genecyst

To Enable / Disable codes, select "Toggle Codes". You can now scroll up and
down through the code list. Hitting <RETURN> on a code will enable or disable
it. Pressing 'X' will exit Toggle mode.

Enabling a code will automatically disable any other codes that write to the
same address.

To Delete codes, select "Delete Codes". The controls are as for "Toggle Codes"

------------------------------------------------------------------------------

GAME GENIE NOTES:
-----------------

At any time during the game, keys F9 & F10 can be used to Activate/Deactivate
the Game Genie. This is useful for games that crash due to a checksum error
when codes are enabled. Deactivate the Genie, hit <TAB> to reset the genesis,
then Activate the Genie when the game is running. This saves you having to
manually switch off and then on again all your active codes.

Note that the Genie is automatically Deactivated when a ROM is loaded, and
Activated when you enter the Game Genie menu.

------------------------------------------------------------------------------

VIDEO MENU:
-----------

From the Video menu, you can now select the following:

VSYNC 		ON or OFF

50Hz MODE *		ON or OFF

SCANLINES *		ON or OFF (VESA 2.0 ONLY)

SHADOW/HILITE		ON or OFF

RASTER FX		NONE, MOST or FULL

H-INTERRUPTS		ON or OFF


* 50Hz Mode and ScanLines mode may or may not be supported by your graphics
card. Use at your own risk! They are both OFF by default.

50Hz mode is only selected when it is switched ON and EUROPEAN version is
selected.

As mentioned before, both Shadow/Hilite mode and Raster FX are now 100%
emulated. On the downside, they can now be a LOT slower than in previous
versions. For this reason, I have given you the option to switch them off if
you wish.

For Raster FX, most games will operate properly in the MOST mode. There are a
few, however (E.G. Gunstar Heroes) that require the FULL mode. The FULL mode
can be a lot slower than the MOST mode, especially if a game is doing a lot of
video memory writes during frame time, when it doesn't really matter when they
are written. Also, there are some games that can overload FULL mode so much
that it cannot cope, resulting in corrupted graphics. I will try to come up
with a better solution, so that this doesn't happen. But if it does happen,
MOST mode will fix it, and give you a significant speedup too.

There shouldn't be any need to switch H-Interrupts OFF, but the option is left
incase you want to.

Note that if you are going to be using FULL Raster FX, and / or 44Khz Stereo
Sound, it is recommended that VSYNC be switched OFF, since flying interrupts
can cause the VSYNC signal to be missed, resulting in unneccessary slowdown
while KGen 98 waits for another one.

------------------------------------------------------------------------------

SNAPSHOTS
---------

Snapshots are now saved by simply hitting the F5 key during play, and reloaded
by hitting F8. You can have up to Ten SnapShot files per game, saving disk
space :-)

The File is selected by hitting the numbers 0-9 before saving / loading a
SnapShot.

------------------------------------------------------------------------------

EXTRA KEY COMMANDS:
-------------------

F1-F4 : Set FrameSkip.
TAB   : Reset Genesis.
F12   : Save ScreenShot.
+/-   : Alter Volume Level. (Keypad)
F5/F8 : Save / Load SnapShot.
0-9   : Select SnapShot Slot 0-9.
F9/F10: Enable / Disable the Game Genie.

------------------------------------------------------------------------------

COMMAND LINE.
-------------

    The command line interface has been improved, but essentially isn't much
different to previous versions. It is now also unneccessary, so you can type:

KGEN98

    This will run the emulator and jump straight into the Main Menu.

Note, however, that the Sound options are ONLY available from the command line


KGEN98 /? will display the command line options, which are:

KGEN98 filename.??? -J -U -E -A -Fn -D -L -8 -4 -S

where ??? is either SMD, BIN, or .1 (Multi-part SMD files)

Any other extension will assume a BINARY file.

NOTE: the filename no longer needs to come before any options, and can be
ommited completely.

Options are: -J -U -E (select Japanese, American (USA) or European Machine)

             -A       (attempt to Auto-Detect the Machine Version)

             -Fn      (Frame Skip)

 > CHANGED   -D       (Disable Sound - NOTE: DOES NOT DISABLE THE Z80 !)

 > ADDED     -L       (Disable Long File Name Support)

             -8       (Force 8-Bit sound output on SB16 or above.)

             -4       (Select 44.1Khz Sound. Defaults to 22.05Khz)

             -S       (Select Stereo Sound (SB16 or above))

FrameSkip is a value from 1 to 4 (default=1) that causes KGen to only draw the
2nd, 3rd or 4th frames, for use on slower PCs. If you have a P166 or above you
shouldn't need this. If you have a P200 you should NEVER need this!

Read the section on the GUI for more information on Frame Skipping.

There is currently no way to disable the Z80. It's fast anyway! :-)
Note also that sound can only be disabled from the command line.

------------------------------------------------------------------------------

TECHNICAL INFO
--------------

This version currently emulates:

     * Motorola 68000 CPU
     * ZiLog Z80A CPU
     * Scroll Screens A,B and Sprites all fully prioritised and masked
     * Fully Working Window.
     * All Scrolling modes (H-Screen, H-Cell, H-Line, V-Screen, V-Cell)
     * DMA
     * V Interrupts
     * H Interrupts
     * All standard Raster Effects
     * Palette Raster Effects (VESA 2.0 ONLY)
     * Shadow / Hilight mode
     * Complete YM2612 FM Emulation + DAC
     * Stereo Sound.
     * Joypad (both ports, keyboard, joypad, or Microsoft SideWinder Joypad)
     * Battery Backup Save / Load support
     * PSG Emulation (all documented and undocumented features)
     * Sega 6 button Joypads
     * Game Genie Cheat Cartridge

Future versions (if, indeed, there ARE any future versions) will emulate:

     * Interlace Mode

------------------------------------------------------------------------------

OTHER NOTES:
------------

KGen will probably run on a 486 with 8Mb of RAM, but as a Pentium is the only
PC that could emulate a Genesis PROPERLY, it has been *highly* optimised for
a pentium.

KGen uses VESA 2.0 support (320x240x8bit) if it is available.
THIS IS HIGHLY RECOMMENDED - if you don't have a VESA 2.0 extender, visit
http://www.scitechsoft.com and download UNIVBE (Scitech Display Doctor).

Without the VESA 2.0 mode, KGen resorts to a custom video mode, which can be
MUCH slower, depending on your PC / Graphics card. Also, several features are
unavailable.

Other than that, KGen runs on any Pentium PC with 8Mb of RAM or greater. P166
or above is HIGHLY RECOMMENDED! With a decent graphics card, UNIVBE, and the
-F4 option, it will run reasonably on a P75.

For Sound Emulation, KGen requires a SoundBlaster Pro (or higher) or 100%
HARDWARE compatible (no software driven!) sound card. If your sound card is
only SoundBlaster compatible (and not PRO) it won't work, so don't ask me why!

You can buy a SoundBlaster 16 compatible for around 10 UK Pounds now, so stop
moaning!

It currently looks for the BLASTER environment. If you don't have one set up,
then you'll have to do so. Later versions will auto-detect.

KGen uses the PMODE/W DOS extender by Charles Scheffold and Thomas Pytel.

------------------------------------------------------------------------------

CONTROLS:
---------

Control is via Keyboard, via re-defining, or using the defaults:

CURSOR KEYS:     Move
A                Button A
S                Button B
D                Button C
RETURN           Start
Q                Button X
W                Button Y
E                Button Z
]                Mode

or JoyPad, which is reconfigureable. Joypads / Joysticks with less than four
buttons should work, you will need to use the keys for the undefined buttons.

------------------------------------------------------------------------------

PROBLEMS:
---------

There are probably a few bugs in my 68000 and Z80A emulators. The 68000 code
was written in about 36 hours, and the Z80A in only three hours, so you'd
expect there to be something wrong somewhere! I do not know of any specific
bugs at the moment.

The YM2612 emulation is still not perfect, but is as close as you'll get for a
while.

------------------------------------------------------------------------------

FINALLY:
--------

I do not condone piracy. I have been a games programmer for the last 12 years
myself, and I know we don't get paid enough :-)

Do NOT ask me for ROM images. I have a BIG collection of cartridges, which are
currently "living with" Andrew 22, but I WILL NOT tell you how to copy them,
or give any out. If you can't find any, tough.

KGen is "AbsolutelyFreeWare" (tm) - I hope you get a lot of enjoyment out of
it. Feel free to distribute it wherever you like, AS LONG AS THIS FILE IS WITH
IT.

Do NOT bug me if something doesn't work. Eventually, 99% of games should work.
But until then, just accept what you've got and be glad that it's free.

------------------------------------------------------------------------------

THANK YOU:
-----------

Thanks to Alain Jourgensen / cEvin Key / Nivek Ogre / D.R. Goettel (R.I.P) and
Sascha K. for making my life worth living.

Thanks to Charles Scheffold and Thomas Pytel for releasing PMODE/W and letting
me use it for free.

Thanks to Dave W. for allowing me to link to his site when mine was about to
die!

Thanks to Nicolas Choukroun @ EmulNews for supporting KML, and for hosting
The KML Empire.

------------------------------------------------------------------------------

THAT'S ALL FOLKS!
-----------------

KGen-X, the MMX version, has, for the time being at least, been scrapped. I
did a lot of work on this at the end of last year, and since the standard
version is using so many optimisation tricks that just get in the way with
MMX, the speed gained by MMX was hardly noticable. KGen98 is now way ahead of
KGen-X, and I'm not convinced another re-write is going to be worthwhile.

Future versions of KGen 98 may or may not be released. That depends on a lot
of things. This doesn't necessarily mean the end, however...... That's all I
want to say for now, since I don't want loads of Emails bugging me about other
versions.

------------------------------------------------------------------------------

snake@toodarkpark.demon.co.uk

                            Snakes' Pit   : http://www.toodarkpark.demon.co.uk
                           The KML Empire : http://www.emulnews.com/kml-empire

------------------------------------------------------------------------------



