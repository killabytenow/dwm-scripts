###############################################################################
# makefile
#
# Build and install script.
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

LIBS=-lX11

all: dwm-msg-writer

.PHONY: clean install

dwm-msg-writer: dwm-msg-writer.c makefile
	gcc -Wall $(ACPI) -o $@ $< $(LIBS)

install: all
	./install.sh

clean:
	rm -f -- dwm-msg-writer

