/*****************************************************************************
 * dwm-msg-writer
 *
 * This program writes, each 10 seconds, the current time and
 * battery status in the user dwm-wrapper pipe.
 *
 * ----------------------------------------------------------------------------
 * DWM Scripts - wrapper for dwm/dmenu/slock
 *   (C) 2007-2013 Gerardo García Peña
 *   Programmed by Gerardo García Peña
 *
 *   This program is free software; you can redistribute it
 *   and/or modify it under the terms of the GNU General
 *   Public License as published by the Free Software
 *   Foundation; either version 2 of the License, or (at your
 *   option) any later version.
 *
 *   This program is distributed in the hope that it will be
 *   useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the GNU General Public License
 *   for more details.
 *
 *   You should have received a copy of the GNU General
 *   Public License along with this program; if not, write to
 *   the Free Software Foundation, Inc., 51 Franklin Street,
 *   Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <X11/Xlib.h>
#include <X11/XKBlib.h>

void battery_status_fini(void);
int battery_status_get();
int battery_status_init(void);

/* X-Window global vars */
Display *dpy;
Window rootwin;

/* output flags */
int stdout_mode = 0;
int debug_mode = 0;

/* finalization routine */
void killemall(void)
{
  battery_status_fini();
  if(dpy)
    XCloseDisplay(dpy);
  exit(0);
}

/* initialize and check x keyboard extensions */
int init_xkb_extension(int *xkbev, int *xkberr)
{
  int code;
  int maj = XkbMajorVersion;
  int min = XkbMinorVersion;
    
  if(!XkbLibraryVersion(&maj, &min))
    return 0;
  if(!XkbQueryExtension(dpy, &code, xkbev, xkberr, &maj, &min))
    return 0;

  return -1;
}

/*****************************************************************************
 * BATTERY INFO
 *
 * Some info ripped from 'batterymon-clone' project:
 *
 *   > Working with /sys/class/power_supply/ battery interface:
 *   >
 *   > /present:
 *   >   0 or 1
 *   > /status:
 *   >   "Unknown", "Charging", "Discharging", "Not charging", "Full"
 *   >   If the battery was charged during the system boot and never changed
 *   >   state, then probably it will be reported as "Unknown".
 *   >   All other states are pretty self-explanatory.
 *   >   Similar to "charging state:" from /proc/acpi/battery/BAT0/state
 *   > /power_now:
 *   >   Power being supplied to or by the battery, measured in microwatts.
 *   >   The same as "present rate:" from /proc/acpi/battery/BAT0/state
 *   > /current_now:
 *   >   Outdated interface, replaced by power_now.
 *   > /energy_now:
 *   >   Remaining energy stored in the battery, measured in microWh.
 *   >   The same as "remaining capacity:" from /proc/acpi/battery/BAT0/state
 *   > /energy_full_design:
 *   >   Measured in microWh.
 *   >   The same as "design capacity:" from /proc/acpi/battery/BAT0/info
 *   > /energy_full:
 *   >   This is the value that should be used when calculating the capacity.
 *   >   Measured in microWh.
 *   >   The same as "last full capacity:" from /proc/acpi/battery/BAT0/info
 *   >
 *   > http://lxr.linux.no/linux+v3.7.1/Documentation/power/power_supply_class.txt
 *   > http://git.kernel.org/?p=linux/kernel/git/torvalds/linux-2.6.git;a=commit;h=7faa144a518c456e2057918f030f50100144ccc6
 *   > http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=532000
 *
 *****************************************************************************/

#define SYSB_BASEDIR  "/sys/class/power_supply"
#define SYSB_MAX_BATS 20

struct battstat {
  char * name;
  int    status;
#   define BATTSTAT_ST_NOT_PRESENT -1
#   define BATTSTAT_ST_UNKNOWN     0
#   define BATTSTAT_ST_NOTCHARGING 1
#   define BATTSTAT_ST_CHARGING    2
#   define BATTSTAT_ST_DISCHARGING 3
#   define BATTSTAT_ST_FULL        4
#   define BATTSTAT_ST_NONE        5

  int    power_now;   /* Power being supplied to or by the battery (microwatts) */
  int    energy_now;  /* Remaining energy stored in the battery (microWh)       */
  int    energy_full; /* Current capacity (microWh)                             */

  int    rem_time;
  double charge_level;
};

struct enstat {
  int             enabled;

  int             on_ac;
  int             rem_time;
  int             rem_time_charging;
  int             rem_time_discharging;
  double          charge_level;
  int             status;

  int             batn;
  struct battstat batt[SYSB_MAX_BATS];

  char           *tpath;
} energy;

int read_str(char *b, char *f, char *s, int ss)
{
  int fd;
  char *a;

  sprintf(energy.tpath, "%s/%s/%s", SYSB_BASEDIR, b, f);

  /* read file */
  if((fd = open(energy.tpath, O_RDONLY)) < 0)
  {
    fprintf(stderr, "Cannot open '%s': %s", f, strerror(errno));
    return -1;
  }
  if(read(fd, s, ss) < 0)
  {
    fprintf(stderr, "Cannot read '%s': %s", f, strerror(errno));
    return -1;
  }
  close(fd);

  s[ss-1] = '\0';
  for(a = s; *a && *a != '\n'; a++)
    ;
  *a = '\0';

  return 0;
}

int read_int(char *b, char *f, int *n)
{
  char buff[256];

  if(read_str(b, f, buff, sizeof(buff)) < 0)
    return -1;

  /* read value */
  sscanf(buff, "%d", n);

  return 0;
}

int battery_status_init(void)
{
  DIR *d;
  struct dirent *e;
  int maxlen;

  energy.enabled = 0;
  if((d = opendir(SYSB_BASEDIR)) == NULL)
  {
    fprintf(stderr, "Cannot open '"SYSB_BASEDIR "' directory: %s", strerror(errno));
    return -1;
  }

  energy.batn = 0;
  maxlen = 5;
  while((e = readdir(d)) != NULL)
  {
    if(!strncmp("BAT", e->d_name, 3))
    {
      if(debug_mode)
        fprintf(stderr, "DEBUG: Found battery '%s'.\n", e->d_name);
      if(energy.batn < SYSB_MAX_BATS)
      {
        if((energy.batt[energy.batn].name = strdup(e->d_name)) == NULL)
        {
          fprintf(stderr, "Cannot dup string '%s': %s", e->d_name, strerror(errno));
          exit(1);
        }
        if(strlen(e->d_name) > maxlen)
          maxlen = strlen(e->d_name);
        energy.batn++;
      } else
        fprintf(stderr, "Cannot manage more than %d batteries - ignoring battery '%s'.", SYSB_MAX_BATS, e->d_name);
    }
  }
  if(debug_mode)
    fprintf(stderr, "DEBUG: Found %d batteries.\n", energy.batn);

  maxlen += strlen(SYSB_BASEDIR) + 2 + 256;

  if((energy.tpath = malloc(maxlen)) == NULL)
  {
    fprintf(stderr, "Cannot malloc a string of %d bytes: %s", maxlen, strerror(errno));
    return -1;
  }

  *energy.tpath = '\0';
  closedir(d);

  energy.enabled = (energy.batn > 0);

  return 0;
}

void battery_status_fini(void)
{
  energy.enabled = 0;
  if(energy.tpath != NULL)
    free(energy.tpath);
}

int battery_status_get(void)
{
  int i, j;
  char buff[256];

  if(!energy.enabled)
    return -1;

  /* get AC state */
  if(read_int("AC", "online", &energy.on_ac) < 0)
    return -1;

  /* reset globals counters */
  energy.rem_time_charging    = -1;
  energy.rem_time_discharging = -1;
  energy.charge_level         = -1;
  energy.status               = BATTSTAT_ST_NONE;

  /* desist if no batteries */
  if(energy.batn <= 0)
    return 0;

  /* get batteries state ... */
  for(i = 0; i < energy.batn; i++)
  {
    /* read values */
    if(read_int(energy.batt[i].name, "present", &j) < 0)
      return -1;

    if(!j)
    {
      if(debug_mode)
        fprintf(stderr, "DEBUG: battery '%s' not present!\n", energy.batt[i].name);

      energy.batt[i].status = BATTSTAT_ST_NOT_PRESENT;
      energy.batt[i].power_now = 0;
      energy.batt[i].energy_now = 0;
      energy.batt[i].energy_full = 0;
    } else {
      if(read_str(energy.batt[i].name, "status", buff, sizeof(buff)) < 0)
        return -1;

      /* update status */
      if(!strcasecmp(buff, "Unknown"))      { energy.batt[i].status = BATTSTAT_ST_UNKNOWN;     } else
      if(!strcasecmp(buff, "Not charging")) { energy.batt[i].status = BATTSTAT_ST_NOTCHARGING; } else
      if(!strcasecmp(buff, "Charging"))     { energy.batt[i].status = BATTSTAT_ST_CHARGING;    } else
      if(!strcasecmp(buff, "Discharging"))  { energy.batt[i].status = BATTSTAT_ST_DISCHARGING; } else
      if(!strcasecmp(buff, "Full"))         { energy.batt[i].status = BATTSTAT_ST_FULL;        } else
      {
        fprintf(stderr, "Unknown battery state '%s' in batt '%s'.", buff, energy.batt[i].name);
        return -1;
      }

      /* read current and capacity indicators */
      if(read_int(energy.batt[i].name, "power_now",   &energy.batt[i].power_now  ) < 0
      || read_int(energy.batt[i].name, "energy_now",  &energy.batt[i].energy_now ) < 0
      || read_int(energy.batt[i].name, "energy_full", &energy.batt[i].energy_full) < 0)
        return -1;

      /* calculate %charge */
      energy.batt[i].charge_level =
        energy.batt[i].status == BATTSTAT_ST_FULL
          ? 100
          : energy.batt[i].energy_full > 0
              ? (((double) energy.batt[i].energy_now * 100.0) / ((double) energy.batt[i].energy_full))
              : 0.0;

      /* calculate remaining time */
      energy.batt[i].rem_time =
        energy.batt[i].power_now <= 0
          ? 0
          : energy.batt[i].status == BATTSTAT_ST_CHARGING
              ? (int) ((((double) energy.batt[i].energy_full - energy.batt[i].energy_now) * 3600.0) / ((double) energy.batt[i].power_now))
              : energy.batt[i].status == BATTSTAT_ST_DISCHARGING
                  ? (int) ((((double) energy.batt[i].energy_now) * 3600.0) / ((double) energy.batt[i].power_now))
                  : -1;

      if(debug_mode)
        fprintf(stderr,
                "DEBUG: battery [status=%s/%d,energy_now=%d,energy_full=%d,power_now=%d,level=%.2f,rem_time=%d/%d:%02d]\n",
                buff, energy.batt[i].status,
                energy.batt[i].energy_now,
                energy.batt[i].energy_full,
                energy.batt[i].power_now,
                energy.batt[i].charge_level,
                energy.batt[i].rem_time,
                (energy.batt[i].rem_time / 60) / 60,
                (energy.batt[i].rem_time / 60) % 60);

      /* update globals */
      if(energy.batt[i].charge_level > 0)
        energy.charge_level += energy.batt[i].charge_level;
      if(energy.batt[i].status == BATTSTAT_ST_CHARGING)
        energy.rem_time_charging += energy.batt[i].rem_time;
      if(energy.batt[i].status == BATTSTAT_ST_DISCHARGING)
        energy.rem_time_discharging += energy.batt[i].rem_time;
      if(energy.status > energy.batt[i].status)
        energy.status = energy.batt[i].status;
    }
  }

  /* fix global */
  energy.charge_level = energy.charge_level / ((double) energy.batn);
  energy.rem_time =
    energy.status == BATTSTAT_ST_CHARGING    ? energy.rem_time_charging
  : energy.status == BATTSTAT_ST_DISCHARGING ? energy.rem_time_discharging
  : 0;

  return 0;
}

/* main program (and loop) */
int main(int argc, char **argv)
{
  char s[1024];
  char batstat[1024];
  int i;
  struct tm ts;
  time_t t;
  int screen;
  int xkb_ext = -1;
  int xkbev, xkberr;

  /* install exit() handler */
  if(atexit(killemall))
  {
    fprintf(stderr, "Cannot install exit handler.\n");
    exit(1);
  }

  if(argc > 1)
  {
    for(i = 1; i < argc; i++)
    {
      if(argv[i][0] != '-'
      || !argv[i][1]
      || argv[i][2])
      {
        fprintf(stderr, "Unrecognized option '%s'.\n", argv[i]);
        exit(1);
      }
      switch(argv[i][1])
      {
        case 'd':
          fprintf(stderr, "debug mode.\n");
          debug_mode = 1;
          break;

        case 'c':
          fprintf(stderr, "stdout mode -- no print to X.\n");
          stdout_mode = 1;
          break;

        default:
          fprintf(stderr, "Unrecognized option '%s'.\n", argv[i]);
          exit(1);
      }
    }
  
  }

  /* Get X11 root window and screen */
  if(!(dpy = XOpenDisplay(0)))
  {
    fprintf(stderr, "Cannot open display\n");
    exit(1);
  }
  screen = DefaultScreen(dpy);
  rootwin = RootWindow(dpy, screen);

  /* Initialize ACPI */
  battery_status_init();

  /* keyboard sexy leds initialization (this sentence has no sense) */
  if (!init_xkb_extension(&xkbev, &xkberr))
  {
    fprintf(stderr, "warning: no sexy kbd extension found.\n");
    xkb_ext = 0;
  } else {
    if(!XkbSelectEvents(dpy, XkbUseCoreKbd, XkbIndicatorStateNotifyMask, XkbIndicatorStateNotifyMask))
    {
      fprintf(stderr, "warning: no sexy kbd event filter.\n");
      xkb_ext = 0;
    }
  }

  /* main loop */
  while(1)
  {
    /* read ac/battery state */
    battery_status_get();

    /* print report */
    *batstat = '\0';
    if(energy.batn > 0)
    {
      if(energy.on_ac)
        strcat(batstat, "AC");
      if(energy.batn > 1)
      {
        char s = '(';
        for(i = 0; i < energy.batn; i++)
        {
          sprintf(batstat + strlen(batstat),
                  "%c%s%.0f%%",
                  s,
                  energy.batt[i].status == BATTSTAT_ST_NOTCHARGING ? "!" :
                  energy.batt[i].status == BATTSTAT_ST_CHARGING    ? "+" :
                  energy.batt[i].status == BATTSTAT_ST_DISCHARGING ? "-" :
                  energy.batt[i].status == BATTSTAT_ST_FULL        ? ""  : "?",
                  energy.batt[i].charge_level);
          s = ',';
        }
        strcat(batstat, ")");
      }

      switch(energy.status)
      {
        case BATTSTAT_ST_NOTCHARGING: strcat(batstat, " NOT charging "); break;
        case BATTSTAT_ST_DISCHARGING:                                    break;
        case BATTSTAT_ST_CHARGING:    strcat(batstat, " charging ");     break;
        case BATTSTAT_ST_FULL:        strcat(batstat, " full ");         break;
        default:
        case BATTSTAT_ST_UNKNOWN:     strcat(batstat, " unknown ");      break;
      }

      if(energy.rem_time > 0)
        sprintf(batstat + strlen(batstat), "%.0f%%, %d:%02d",
                energy.charge_level,
                (energy.rem_time / 60) / 60,
                (energy.rem_time / 60) % 60);
      else
        sprintf(batstat + strlen(batstat), "%.0f%%",
                energy.charge_level);
    }

    /* what time is it? [TIME TO DIE MOTHERFUCKER!!!] */
    time(&t);
    localtime_r(&t, &ts);

    /* format msg */
    sprintf(s, "%02d:%02d %02d/%02d/%d ",
            ts.tm_hour, ts.tm_min,
            ts.tm_mday, ts.tm_mon + 1, ts.tm_year + 1900);

    /* add ac/battery status report */
    if(strlen(batstat) > 0)
    {
      strcat(s, "[");
      strcat(s, batstat);
      strcat(s, "] ");
    }

    /* get key status */
    if(xkb_ext)
    {
      unsigned int state;
      char *t = s + strlen(s);

      XkbGetIndicatorState(dpy, XkbUseCoreKbd, &state);
      *t++ = state & (1 << 0) ? 'M' : '_';
      *t++ = state & (1 << 1) ? 'N' : '_';
      *t++ = state & (1 << 2) ? 'S' : '_';
      *t = '\0';
    }

    /* update dwm title */
    if(stdout_mode)
    {
      printf("%s\n", s);
    } else {
      XStoreName(dpy, rootwin, s);
      XFlush(dpy);
    }

    /* wait a little before repeat */
    sleep(1);
  }
}

