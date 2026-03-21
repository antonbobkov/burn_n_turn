#ifndef TOWER_DEFENSE_GAME_CONFIG_H
#define TOWER_DEFENSE_GAME_CONFIG_H

/** Runtime feature flags, read from config.txt at startup via ReadGameConfig().
 *
 * Keys:
 *
 *   VERSION         — "full" or "trial"
 *   PLATFORM        — "pc" or "mobile"
 *   CONTROLS        — "mouse", "keyboard", or "joystick"
 *   SCREEN_SIZE     — "normal" or "small"
 *   LOADING_SCREEN  — "true" or "false"
 *   DEBUG_UNIT_TEXT — "true" or "false"
 */
class GameConfig {
 public:
  GameConfig(bool is_full_version, bool is_pc_version,
             bool is_keyboard_controls, bool is_joystick_tutorial,
             bool is_small_screen_version, bool is_loading_screen,
             bool is_underline_unit_text);

  bool IsFullVersion()        const { return is_full_version_; }
  bool IsPcVersion()          const { return is_pc_version_; }
  bool IsKeyboardControls()   const { return is_keyboard_controls_; }
  bool IsJoystickTutorial()   const { return is_joystick_tutorial_; }
  bool IsSmallScreenVersion() const { return is_small_screen_version_; }
  bool IsLoadingScreen()      const { return is_loading_screen_; }
  bool IsUnderlineUnitText()  const { return is_underline_unit_text_; }

  bool IsTrialVersion()    const { return !is_full_version_; }
  bool IsFlightPowerMode() const { return is_small_screen_version_; }

 private:
  bool is_full_version_;
  bool is_pc_version_;
  bool is_keyboard_controls_;
  bool is_joystick_tutorial_;
  bool is_small_screen_version_;
  bool is_loading_screen_;
  bool is_underline_unit_text_;
};

class ConfigurationFile;
GameConfig ReadGameConfig(ConfigurationFile const &cfg);

#endif
