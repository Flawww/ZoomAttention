# Zoom Attention Spoofer
Zoom assigns an attentativeness score for each attendee of a meeting, which is measured based of the time you spent "focused" on the Zoom application while someone is sharing their screen.

See: https://support.zoom.us/hc/en-us/articles/115000538083-Attendee-attention-tracking

![Attentativeness score](https://i.imgur.com/fXC8ATu.png)
![Debug Console](https://i.imgur.com/y2OoojH.png)

## How it works
Zoom makes use of simple Windows-API calls (GetForegroundWindow, GetActiveWindow, GetFocus) to check which window is currently focused.
This is easily exploitable by hooking these functions (With hooking method of your choice, this project uses detour hooking) and returning the "correct" window handle, ie. the one of the zoom meeting.

## Todo
* Test if it works for break-out groups, if not - implement this.
* Reverse engineer Zoom further
	- Look into zVideoUI.dll to find a more precise way of accomplishing this - realistically only one of the above mentioned functions is needed to bypass their checks but I have not had to time to go through and test each one individually.
	- Reverse engineer their networking: is it possible to spoof attendee-list and/or add "extra" users of your choosing?
	
