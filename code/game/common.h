#ifndef TOWER_DEFENSE_COMMON_H
#define TOWER_DEFENSE_COMMON_H

#include "wrappers/GuiGen.h"
#include "wrappers/SuiGen.h"
#include "wrappers/gui_key_type.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <fstream>
#include <sstream>

#include <math.h>
#include <stdlib.h>

#include "game_utils/Preloader.h"
#include "utils/smart_pointer.h"

#include "game_utils/game_runner_interface.h"

#define FULL_VERSION
#define PC_VERSION
//#define KEYBOARD_CONTROLS

#ifdef KEYBOARD_CONTROLS
//#define JOYSTICK_TUTORIAL
#endif

#ifdef PC_VERSION
//#define FULL_SCREEN_VERSION
//#define LOADING_SCREEN
#endif

//#define SMALL_SCREEN_VERSION

#ifndef FULL_VERSION
#define TRIAL_VERSION
#endif

#ifdef SMALL_SCREEN_VERSION
#define FLIGHT_POWER_MODE
#endif

//#define UNDERLINE_UNIT_TEXT

extern int nSlimeMax;

using namespace Gui;

typedef Gui::GraphicalInterface<Index> Graphic;
typedef Gui::SoundInterface<Index> Soundic;

extern MessageWriter *pWr;

#endif
