
About
===
UsbSafety is a little tool i wrote to help me with some common problems i had
at work: people annoying me to "recover" the files stored on their pendrives.

Features
===
UsbSafety performs automatically these actions when you plug in a pendrive:
	- Unhides files and directories (except trusted ones)
	- Deletes known malware files (e.g. autorun.exe or regsvr.exe)
	- Deletes fake directory duplicates (e.g. Photos.exe)
	- Deletes suspicious links (e.g. Music.lnk pointing to a malware)
	- Deletes autorun.ini
	- Deletes fake media files (e.g. Image.jpg.exe or Song.mp3.exe)

Installing to startup folder
===
Windows Vista / Seven:
	Click the Start button, click All Programs, right-click the Startup folder, and then click Open.
	Drag usbsafety.exe into the Startup folder.

Windows XP (method 1):
	Click the Start button, click Programs, right-click the Startup folder, and then click open.
	Drag usbsafety.exe into the Startup folder.
Windows XP (method 2):
	Drag usbsafety.exe into C:\Documents and Settings\_USERNAME_\Start Menu\Programs\Startup

Uninstalling
===
Just remove usbsafety.exe from the Startup folder.

Compiling
===
Open the UsbSafety C++ builder project and hit compile.

