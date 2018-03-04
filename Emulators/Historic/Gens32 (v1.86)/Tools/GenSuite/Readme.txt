GenRomSuite version 2.8.2 by Tom
~~~~~~~~~~~~~~~~~~~~~~~

GenRomSuite is a tiny "suite" of tools to work with Sega Genesis / MegaDrive ROMs & emulators. You can do this with this app:

+ Edit binary-ROMs header info, like game name, country codes, etc... Also it can fix ROM checksum if it is wrong.
+ ROM converter: You can convert your ROMs between the BIN (raw) format and the SMD (interleaved) format. Also you can do batch conversions.
+ Emulator launcher: It support up to six emulators (Gens, Kega/Fusion, Ages, Xega, DGen and a custom emulator). You can also select a ROM and launch it into emulator.
+ IPS ROM Patcher: Apply IPS patchs to your ROMs. Support flat and RLE-packed patches. Also it can apply extra data sets, if necessary.

Another functions of this app:

* An useful statusbar that reports you about status and errors in the program.
* Language files support, for easy program translation.
* XP visual-styles support.
* An "Outlook-style" toolbar with shortcuts to tools. Simply click on a icon, and the tool will open.

For execute application, first unzip all files into a folder, including language files (this app include english and spanish translations into .IDI files). Then open the app folder, and doubleclick "GenSuite.exe". If you are running this app for first time, you must select an lang file for start (default selection is english, but you can change this option if you have the necessary .IDI files)

Starting with version 1.7.55, this program will be distributed with an easy-to-use setup program.

Version history
===============
See "history.txt" for details

Application notes
=================
- This app was developed under Visual Basic 6, so you need the VB6 runtimes. If you are using Win98SE or later, don't worry, that versions come with that libraries, if not, you can donwload an installer from http://activex.microsoft.com/controls/vb6/vbrun60.cab (~1 MB) .
- If you use a DOS-based emulator, you MUST point it in the "Setup" window, for allow that emu for find the ROM.
- You can change app language from File-->Select language...
- If you use an emulator that does not support commandline ROM loading (like retroDrive), you can setup GenRomSuite for "send the keys" to emulator. Simply find the shortcut key that opens the "Load/Open ROM" dialogbox (like Ctrl+O), and in the Setup window point that keys.
- GenRomSuite support commandline switches (only one per execution):
	GenSuite.exe [/c] [/h ROM_filename] [/l ROM_filename] [/p]
	/c	Launch ROM converter
	/h rom	Launch Header editor, optionally loading a ROM
	/l rom	Launch Emulator launcher, optionally with a ROM
	/p	Launch IPS patcher
- App settings are saved under "GENSUITE.ini", into app's folder
- You can create your own translations, simply create a copy of an included lang (.IDI) files. Please do not change variable names, and respect "%x" symbols, that are replaced at run-time:

Symbol--->Replaced by:
  -> "%r"=ROM filename
  -> "%p"=Executable filename
  -> "%v"=Total RAM
  -> "%f"=Free RAM

If you change or delete these sybmols you can cause that the program fail.
If you want to make a translation and you wish that it can be include it with the application, then send me the translation (you can found my e-mail later in this doc...).
Don't forget to include your name in the variable "lblTransBy=", it will appear in the "About..." window of the program.

- Bugs? Serious errors? Report that to my e-mail (specially that "VB runtime errors")
- Sources are available, so if you want to improve program, download it!. Now released under GNU General Public License.

Contact me:
===========
My e-mail addres is tomman@cantv.net.
you can report bugs, errors, and send translations and comments

Acknowledgements:
=================
 *To Felipe XnaK (http://www.classicgaming.com/launchtool, felipe@ComPorts.com), the creator of "The Complete Documentation About Genesis ROM Format", an excellent document, that helped me to create this program.
 *To Genesis Collective (http://www.genesiscollective.com), for their excellent Sega Genesis documents.
 *To emulator's makers (Michel Gerritse, Steve Snake, Stéphane Dallongeville, Quinntesson, ...), por sus excelentes aplicaciones. Mantengan ese esfuerzo!
 *To Sega Emulation Forums members, they always helped me...
 *To Zophar's Domain, if i publish something, they help reporting and publishing that. Thanks!.
 *To fataku for test my applications under XP.
 *To VB creators and programmers, not to bill....
 *To Microsoft VB newsgroup users (microsoft.public.es.vb), that helped me to understand about API's usage...(special thanks to Morgan and Saga)
 *To VBnet (http://vbnet.mvps.org/), and AllAPI (http://www.mentalis.org/allapi) by their excellent source code for VB.
 *To ZeroSoft (http://zerosoft.zophar.net/), for their excellent IPS format document.
 *To Fabian and his PIII 866 with XP and VB6, that helped me to trace and fix some bugs under NT-based platforms.
 *to my mother, she bought my computer...
 *to anybody else that i've not named here (my english is a litte crappy...). thanks

=GenRomSuite 2.8 rv.2==05/03/2005==05:43p======100% MADE IN VENEZUELA=================