/*
 * status.c: A plugin for the Video Disk Recorder
 *           based on the status plugin by kls
 *
 * See the README file for copyright information and how to reach the author.
 *
 */
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>



#include <vdr/plugin.h>
#include <vdr/status.h>


static const char *VERSION        = "0.0.4";
static const char *DESCRIPTION    = "Samsung LED display plugin";
static const char *MAINMENUENTRY  = NULL;

// ---

class cSmtledTest : public cStatus {
private:
  int CH_Hi;
  int CH_Lo;
  int reccount;
protected:
  virtual void SetLED(char * cr);
  virtual void ChannelSwitch(const cDevice *Device, int ChannelNumber);
  virtual void Recording(const cDevice *Device, const char *Name, const char *FileName, bool On);
public:
  cSmtledTest(void);
  };

void cSmtledTest::SetLED(char* cw) 
{
      int fd,n; /* File descriptor for the port */
      fd = open("/dev/ttyS1", O_RDWR | O_NOCTTY | O_NDELAY);
      if (fd == -1)
      {
       /*
        * Could not open the port.
        */

        perror("open_port: Unable to open /dev/ttyS1 - ");
      }
      else {
       // fcntl(fd, F_SETFL, 0);
        n = write(fd, cw, 5);
        if (n < 0)
          fputs("write() failed!\n", stderr);
         close(fd);
        }

}

void cSmtledTest::ChannelSwitch(const cDevice *Device, int ChannelNumber)
{
   if ( !Device->IsPrimaryDevice() || (ChannelNumber == 0) ) {
     dsyslog("status: cSmtledTest::ChannelSwitch ignore device %d channel %d",
                 Device->CardIndex(), ChannelNumber);
     return;
   }
 
  int chhi,chlo;
  chhi=(ChannelNumber/100)%100; 
  chlo=ChannelNumber%100;
  cSmtledTest::CH_Lo=chlo;
  cSmtledTest::CH_Hi=chhi;
  char cw[5];
  cw[0]=0x20;
  cw[1]=0x40;
  cw[2]=0x00;
  cw[2]=cw[2]+(chhi/10)<<4;
  cw[2]=cw[2]+(chhi%10);
  cw[3]=0x00;
  cw[3]=cw[3]+(chlo/10)<<4;
  cw[3]=cw[3]+(chlo%10);
  cw[4]='\n';
  SetLED(cw);

}

void cSmtledTest::Recording(const cDevice *Device, const char *Name, const char *FileName, bool On)
{
//  dsyslog("status: cStatusTest::Recording  %d %s %s %d", Device->CardIndex(), Name, FileName, On);
  char cw[5];

  if (On==1) {
     int chhi,chlo;
     chhi=cSmtledTest::CH_Hi;
     chlo=cSmtledTest::CH_Lo;

     cw[0]=0x20;
     cw[1]=0x80;
     cw[2]=0x00;
     cw[3]=0x00;
     cw[4]='\n';
     SetLED(cw); //error led setzen

     reccount++; //Anzahl der aktiven Aufnahmen erhoehen

     cw[0]=0x20;
     cw[1]=0x40;
     cw[2]=0x00;
     cw[2]=cw[2]+(chhi/10)<<4;
     cw[2]=cw[2]+(chhi%10);
     cw[3]=0x00;
     cw[3]=cw[3]+(chlo/10)<<4;
     cw[3]=cw[3]+(chlo%10);
     cw[4]='\n';
     SetLED(cw); //channel setzen


  } else {
     reccount--; //Anzahl der aktiven Aufnahmen um eins reduzieren
     if (reccount==0) { //wenn keine Aufnahme mehr aktiv --> LED aus
       cw[0]=0x20;
       cw[1]=0x82;
       cw[2]=0x00;
       cw[3]=0x00;
       cw[4]='\n';
       SetLED(cw); //error led ausschalten 
     }

  }
}
cSmtledTest::cSmtledTest(void)
{
     reccount=0;
}


// ---

class cPluginSmtled : public cPlugin {
private:
  // Add any member variables or functions you may need here.
  cSmtledTest *statusTest;
public:
  cPluginSmtled(void);
  virtual ~cPluginSmtled();
  virtual const char *Version(void) { return VERSION; }
  virtual const char *Description(void) { return DESCRIPTION; }
  virtual const char *CommandLineHelp(void);
  virtual bool ProcessArgs(int argc, char *argv[]);
  virtual bool Start(void);
  virtual void Housekeeping(void);
  virtual const char *MainMenuEntry(void) { return MAINMENUENTRY; }
  virtual cOsdObject *MainMenuAction(void);
  virtual cMenuSetupPage *SetupMenu(void);
  virtual bool SetupParse(const char *Name, const char *Value);
  };

cPluginSmtled::cPluginSmtled(void)
{
  // Initialize any member variables here.
  // DON'T DO ANYTHING ELSE THAT MAY HAVE SIDE EFFECTS, REQUIRE GLOBAL
  // VDR OBJECTS TO EXIST OR PRODUCE ANY OUTPUT!
  statusTest = NULL;
}

cPluginSmtled::~cPluginSmtled()
{
  // Clean up after yourself!
  delete statusTest;
}

const char *cPluginSmtled::CommandLineHelp(void)
{
  // Return a string that describes all known command line options.
  return NULL;
}

bool cPluginSmtled::ProcessArgs(int argc, char *argv[])
{
  // Implement command line argument processing here if applicable.
  return true;
}

bool cPluginSmtled::Start(void)
{
  // Start any background activities the plugin shall perform.
  statusTest = new cSmtledTest;
  return true;
}

void cPluginSmtled::Housekeeping(void)
{
  // Perform any cleanup or other regular tasks.
}

cOsdObject *cPluginSmtled::MainMenuAction(void)
{
  // Perform the action when selected from the main VDR menu.
  return NULL;
}

cMenuSetupPage *cPluginSmtled::SetupMenu(void)
{
  // Return a setup menu in case the plugin supports one.
  return NULL;
}

bool cPluginSmtled::SetupParse(const char *Name, const char *Value)
{
  // Parse your own setup parameters and store their values.
  return false;
}

VDRPLUGINCREATOR(cPluginSmtled); // Don't touch this!
