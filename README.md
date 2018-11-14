# OSMC-Arduino

This little project is aimed to help people using OSMC (https://osmc.tv/) on Raspberry Pi
One common issue with that is starting up the Raspberry.
OSMC allows the use of the remote control but this is only possible when OSMC is up and running,
when it's OFF there no other way to wake up OSMC than power-off power-on or adding a restart button to RPi.

This project exploits an Arduino board to keep on receiving IR data and waking up OSMC when a specific button
is pressed on the remote control.

The basic idea is that OSMC tells Arduino when it's running, so that Arduino ignores the remote codes,
when OSMC is not running Arduino will intercept the IR codes and when the chosen one is received it will reset
the Raspberry so that OSMC will start.

The Raspberry part of the project exploits WiringPi libs for generating a 1Hz signal on pin 11 (BMC17),
the program blink.c from WiringPi examples is used for that.
The blink program is ran automatically from /etc/rc.local
The following line needs to be added inside that file before "exit 0"
sudo /usr/local/bin/blink &

On Arduino IDE, the IRLib2 set of libraries are needed.
The instruction for installing are available at 
https://github.com/cyborg5/IRLib2
or at
https://learn.adafruit.com/using-an-infrared-library/hardware-needed
In order to sniff the codes from the remote you can start with the example in that page.

Arduino and Raspberry needs to be connected, and you also need an IR receiver chip.
The connections among these objects are as follows
 *            Connections : Arduino Pro Micro    Raspberry Pi   TSOP1838
 *                          ------------------+----------------+------
 *                                  2         |  BCM 18 (12)   | Out
 *                                  3         |  BCM 17 (11)   |
 *                                  4         |      RUN       |
 *                                 Vcc        |  5v Power (2)  | 
 *                                            | 3v3 Power (1)  | Vs
 *                                 GND        |    Ground (6)  |       
 *                                            |    Ground (9)  | GND
 *                          ------------------+----------------+------         
 
 Enjoy OSMC and Arduino!
 
