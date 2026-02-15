#include "core.h"
#include "game_utils/draw_utils.h"
#include "dragon_constants.h"
#include "dragon_macros.h"
#include "game.h"
#include "game_utils/sound_utils.h"
#include "game_utils/image_sequence.h"
#include "utils/smart_pointer.h"
#include "wrappers/font_writer.h"
#include "wrappers/color.h"

MessageWriter *pWr = 0;

int nSlimeMax = 100;

const std::string sFullScreenPath = "fullscreen.txt";
