#include "game_config.h"
#include "../utils/configuration_file.h"

GameConfig::GameConfig(bool is_full_version, bool is_pc_version,
                       bool is_keyboard_controls, bool is_joystick_tutorial,
                       bool is_small_screen_version, bool is_loading_screen,
                       bool is_underline_unit_text)
    : is_full_version_(is_full_version),
      is_pc_version_(is_pc_version),
      is_keyboard_controls_(is_keyboard_controls),
      is_joystick_tutorial_(is_joystick_tutorial),
      is_small_screen_version_(is_small_screen_version),
      is_loading_screen_(is_loading_screen),
      is_underline_unit_text_(is_underline_unit_text) {}

GameConfig ReadGameConfig(ConfigurationFile const &cfg) {
  std::string controls = cfg.GetEntry("CONTROLS");
  return GameConfig(
      /*is_full_version=*/        cfg.GetEntry("VERSION")         != "trial",
      /*is_pc_version=*/          cfg.GetEntry("PLATFORM")        != "mobile",
      /*is_keyboard_controls=*/   controls == "keyboard",
      /*is_joystick_tutorial=*/   controls == "joystick",
      /*is_small_screen_version=*/cfg.GetEntry("SCREEN_SIZE")     == "small",
      /*is_loading_screen=*/      cfg.GetEntry("LOADING_SCREEN")  == "true",
      /*is_underline_unit_text=*/ cfg.GetEntry("DEBUG_UNIT_TEXT") == "true");
}
