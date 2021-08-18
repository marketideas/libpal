#pragma once

#include "palPoco.h"


#define FREEANDNIL(x) \
  if (x!=NULL) { delete(x); x=NULL;}
  

namespace PAL_NAMESPACE {

enum {
    CL_NORMAL,
    CL_RED,
    CL_YELLOW,
    CL_GREEN,
    CL_CYAN,
    CL_BLUE,
    CL_PURPLE,
    CL_WHITE
};

//#define CL_NORMAL 0x00
//#define CL_RED 0x01
//#define CL_YELLOW 0x02
//#define CL_GREEN 0x03
//#define CL_CYAN 0x04
//#define CL_BLUE 0x05
//#define CL_PURPLE 0x06
//#define CL_WHITE 0x07

#define BG_NORMAL 0x00
#define BG_RED 0x10
#define BG_YELLOW 0x20
#define BG_GREEN 0x30
#define BG_CYAN 0x40
#define BG_BLUE 0x50
#define BG_PURPLE 0x60
#define BG_WHITE 0x70

#define CL_BOLD 0x08
#define BG_BOLD 0x80

std::string getEnv(std::string name, std::string dflt);
std::string getConfig(std::string key, std::string dflt);
bool toBool(std::string str);
bool getBool(std::string key, bool dflt);
int getInt(std::string key, int dflt);
uint64_t GetTickCount();
std::string SubstEnvironment(std::string instr);
std::string getLibVersion(void);
std::string getPocoVersion(void);
int isDebug(void);
uint8_t SetColor(uint8_t icolor);

std::string getTimeAsString(void);

bool changeOwner(std::string filename, std::string userstr, std::string groupstr);
bool changePermissions(std::string filename, uint32_t permissions);
}
