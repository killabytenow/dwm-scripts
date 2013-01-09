#!/bin/sh
###############################################################################
# install.sh
#
# This script install DWM Scripts.
#
# -----------------------------------------------------------------------------
# DWM Scripts - wrapper for dwm/dmenu/slock
#   (C) 2007-2013 Gerardo García Peña
#   Programmed by Gerardo García Peña
#
#   This program is free software; you can redistribute it and/or modify it
#   under the terms of the GNU General Public License as published by the Free
#   Software Foundation; either version 2 of the License, or (at your option)
#   any later version.
#
#   This program is distributed in the hope that it will be useful, but WITHOUT
#   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
#   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
#   more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program; if not, write to the Free Software Foundation, Inc., 51
#   Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
#
###############################################################################

set -e

if ! DWM=`which dwm`; then
  echo "You have not installed DWM."
  if [ "$1" != "-f" -a "$1" != "--force" ]; then
      echo "Use -f (--force) to force install."
      exit 0
  fi
fi

echo "Installing files..."
for i in dmenu-wrapper      \
         dwm-config-read    \
         dwm-msg-writer     \
         dwm-screenshot     \
         dwm-screen-lock    \
         dwm-say            \
         dwm-set-background \
         dwm-shutdown       \
         dwm-sudo           \
         dwm-volume-down    \
         dwm-volume-get     \
         dwm-volume-mute    \
         dwm-volume-up      \
         dwm-windowshot     \
         dwm-wrapper        \
         dwm-xterm          ; do
  echo "  '$i'..."
  install -t "/usr/bin/" "$i" 
done

echo "Installing other files..."
if [ -d "/var/lib/menu-xdg/xsessions/" ]; then
  echo "  'X-Debian-WindowManagers-dwm-scripts.desktop'..."
  install -t "/var/lib/menu-xdg/xsessions/" X-Debian-WindowManagers-dwm-scripts.desktop
fi
if [ -d "/usr/share/xsessions/" ]; then
  echo "  'dwm-scripts.desktop'..."
  install -t "/usr/share/xsessions/" dwm-scripts.desktop
fi
if [ -d "/usr/share/icons/" ]; then
  echo "  'dwm-scripts.png'..."
  mkdir -p "/usr/share/icons/"
  install -m 664 -t "/usr/share/icons/" dwm-scripts.png
fi
FIN=""
C=0
F="default-dwm-background.jpg"
while test -z "$FIN"; do
  if [ -e "/usr/share/dwm-scripts/$F" ]; then
    if diff "default-dwm-background.jpg" "/usr/share/dwm-scripts/$F" > /dev/null 2>&1; then
      FIN=1
    else
      C=$(($C+1))
      F="dwm-background-$C.jpg"
    fi
  else
    FIN=1
  fi
done
echo "  Default dwm background as '$F'..."
mkdir -p "/usr/share/dwm-scripts/"
install -m 664 "default-dwm-background.jpg" "/usr/share/dwm-scripts/$F"

echo "Installing config files..."
mkdir -p "/etc/dwm-scripts/"
for i in dwm-config dmenu-config; do
  echo "  '$i'..."
  install -m 664 -t "/etc/dwm-scripts/" "$i"
done

echo "Installing doc files..."
mkdir -p "/usr/share/doc/dwm-scripts/"
for i in LICENSE README.md; do
  echo "  '$i'..."
  install -m 664 -t "/usr/share/doc/dwm-scripts/" "$i"
done

echo "Installation ok."
exit 0
