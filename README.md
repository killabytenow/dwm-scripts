===============================================================================
DWM Scripts README
===============================================================================


Copyright
=========

  DWM Scripts - wrapper for dwm/dmenu/slock
    (C) 2007-2013 Gerardo García Peña <killabytenow@gmail.com>
    Programmed by Gerardo García Peña

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the Free
    Software Foundation; either version 2 of the License, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc., 51
    Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA



Installation Instructions
=========================

Required libraries
------------------

  This program does not require any library a part from the ones used yet by
  DWM for being compiled. So you will need at least the same libraries than the
  used by the dwm binaries:

    libx11-dev libxinerama-dev

  You will also need some utilities used by the dwm scripts:

    feh


Patch DWM
---------

  Add this line to the begining of 'config.h':

    #include<X11/XF86keysym.h>

  Change and add the following assignments:

    static const char *dmenucmd[]   = { "dmenu-wrapper",   NULL };
    static const char *termcmd[]    = { "dwm-xterm",       NULL };
    static const char *lockcmd[]    = { "dwm-screen-lock", NULL };
    static const char *volmutecmd[] = { "dwm-volume-mute", NULL };
    static const char *volupcmd[]   = { "dwm-volume-up",   NULL };
    static const char *voldowncmd[] = { "dwm-volume-down", NULL };

  Add following keys to add screenlocker, alt tab and multimedia keys:

    { MODKEY|ControlMask, XK_l,   spawn,      {.v = lockcmd    } },
    { 0, XF86XK_AudioMute,        spawn,      {.v = volmutecmd } },
    { 0, XF86XK_AudioLowerVolume, spawn,      {.v = voldowncmd } },
    { 0, XF86XK_AudioRaiseVolume, spawn,      {.v = volupcmd   } },
    { MODKEY, XK_Tab,             focusstack, {.i = +1         } },

  Don't forget to remove the following line to avoid duplicates:

    { MODKEY,                       XK_Tab,    view,           {0} },

  If you don't have multimedia keys, you can also try:

    { MODKEY|ControlMask, XK_F12,  spawn,      {.v = volmutecmd } },
    { MODKEY|ControlMask, XK_Down, spawn,      {.v = voldowncmd } },
    { MODKEY|ControlMask, XK_Up,   spawn,      {.v = volupcmd   } },

  I recommed seriously to remove this line:

    { MODKEY|ShiftMask,             XK_q,      quit,           {0} },

  Finally, if you don't like forced layouts or tags (depending on application
  name), I recommend deletion of following line:

    { "Firefox", NULL, NULL, 1 << 8, True },

  Or adding others like following:

    { "xeyes",   NULL, NULL, 0, True },

  Another interesting hack is to change the window properties:

        static const char selbordercolor[]  = "#ff0000";
        static const char selbgcolor[]      = "#3465a4";
        static const char selfgcolor[]      = "#ffffff";
        static const unsigned int borderpx  = 2; /* border pixel of windows */

  Now you can install it as always:

    make && sudo make install


Compiling and installing the DWM Scripts
----------------------------------------

   To compile and install the DWM Scripts you only have to execute the
   following line:

     make && sudo make install



Configuring DWM Scripts
=======================

The 'dwm-config' file
---------------------

  The DWM scripts look for their configuration in the following places (in this
  order):

    - ~/.dwm-config
    - ~/etc/dwm-scripts/dwm-config
    - /etc/dwm-scripts/dwm-config
    - /opt/etc/dwm-scripts/dwm-config

  The installation scripts only creates the global configuration at
  '/etc/dwm-scripts/dwm-config', but it can be superseeded by a local user
  configuration.

  In this file you can change any DWM Scripts option at runtime.


Your list of applications in dmenu
----------------------------------

  The 'dmenu' program is also wrapped by the 'dmenu-wrapper' script. It shows a
  list of predefined programs and allows to launch arbitrary programs (if the
  user input is not found in the list of alternatives, it tries to execute it).

  The list of options presented by 'dmenu' are configured in the 'dmenu' config
  file. This file will be searched in the following places (in this order):

    - ~/.dmenu
    - ~/etc/dwm-scripts/dmenu
    - /etc/dwm-scripts/dmenu
    - /opt/etc/dwm-scripts/dmenu

  This is an example configuration file:

    # Select your favourite programs. If a program needs administration                   
    # privileges (su or sudo), add an exclamation mark before.                            
        aterm                                                                         
        firefox                                                                       
        gedit                                                                         
        gimp                                                                          
        gnome-calculator                                                              
        google-chrome                                                                 
        icedove                                                                       
        nautilus --no-desktop                                                         
       !poweroff                                                                      
       !reboot                                                                        
        oocalc                                                                        
        ooimpress                                                                     
        oowriter                                                                      
        dwm-screenshot                                                                
        dwm-windowshot                                                                
        dwm-logout
        thunderbird                                                                   
        transmission                                                                  
        virtualbox                                                                    
        webscarab                                                                     
       !wireshark                                                                     
        xcalc                                                                         
        xclock                                                                        
        xterm

  The lines prefixed with the sharp character (#) are comments and will be
  ignored.

  Please note that the entries prefixed with the exclamation mark will be
  executed in a 'sudo' or 'su' environment.

  Also, if a program of the list is not found in the current path, it will be
  not shown in the dmenu list.


Choose your xterm
-----------------

  The dwm-xterm script launches an xterm window.
  
  By default this script will try to launch your default x-terminal emulator
  configured by the 'x-terminal-emulator' launcher. This is a generic launcher
  present in Debian distributions which points to your default x-term
  application. The selected x-term application can be changed using the
  'update-alternatives' utility.

  If the 'x-terminal-emulator' launcher is not present (i.e. it is not a Debian
  distro) it will try to launch the classical 'xterm' emulator.

  It is also possible to specify your hardcoded preferred x-terminal in the
  'dwm-config' file defining.


Change your background
----------------------

  The current background is stored in your home directory with name
  .dwm-background. If you want you can set the extension (for instance
  .dwm-background.jpg), but it is optional. If none is supplied, then the
  default background (in /usr/share/dwm/default-dwm-background) will be used.

  If your background is name .dwm-background-center.jpg, the background will be
  centered. You can also try with 'seamless', 'tile' and 'scale' dispositions.
  By default, is none is specified, the background will be scaled.
  
  You can update (or refresh) your background at any time calling the
  'dwm-set-background' script.

  NOTE: You must install 'feh' (a light image viewer and cataloguer) to make
  this script work. In Debian is so easy as executing 'apt-get install feh' as
  root user.


Setup keys
----------

  In some laptops you may need to create a file ~/.Xmodmap for assigning
  keycodes to Xkeysims. Normally it is not needed and everything works flawless
  without doing anything, but in some old distros or laptops you may have to
  create a .Xmodmap file.

  The format of the file is (in case of Dell Lattitude D600 and Compaq NC6400):

    keycode 160 = XF86AudioMute
    keycode 174 = XF86AudioLowerVolume
    keycode 176 = XF86AudioRaiseVolume

  You can discover keycodes with 'xev', a program that prints X window events.
  We attach a demo .Xmodmap file with this program.

  Currently following keys are recognized:

    XF86AudioMute        - Audio mute
    XF86AudioLowerVolume - Decrements audio volume by 5%
    XF86AudioRaiseVolume - Increments audio volume by 5%

  If you don't have multimedia keys you can configure other keybindings like:

    { 0,                  XK_F12,  spawn, "exec dwm-volume-mute" }, \
    { MODKEY|ControlMask, XK_Down, spawn, "exec dwm-volume-down" }, \
    { MODKEY|ControlMask, XK_Up,   spawn, "exec dwm-volume-up"   }, \


Setup Xsessions
---------------

  You must prepare your desktop to use DWM Scripts. There are two files that
  need to be created:

    /usr/share/xsessions/dwm.desktop
    /var/lib/menu-xdg/xsessions/X-Debian-WindowManagers-dwm.desktop
  
  These two files are already included in this package and installed by the
  'install.sh' script automatically.

