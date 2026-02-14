#ifndef TOWER_DEFENSE_DRAGON_MACROS_H
#define TOWER_DEFENSE_DRAGON_MACROS_H

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

#endif
