#include "swappy.h"

#define CONFIG_LINE_SIZE_DEFAULT 5
#define CONFIG_TEXT_FONT_DEFAULT "sans-serif"
#define CONFIG_TEXT_SIZE_DEFAULT 20
#define CONFIG_SHOW_PANEL_DEFAULT false
#define CONFIG_SAVE_FILENAME_FORMAT_DEFAULT "swappy-%Y%m%d_%H%M%S.png"
#define CONFIG_PAINT_MODE_DEFAULT SWAPPY_PAINT_MODE_BRUSH
#define CONFIG_EARLY_EXIT_DEFAULT false
#define CONFIG_FILL_SHAPE_DEFAULT false
#define CONFIG_AUTO_SAVE_DEFAULT false
#define CONFIG_CUSTOM_COLOR_DEFAULT "rgba(193,125,17,1)"

void config_load(struct swappy_state *state);
void config_free(struct swappy_state *state);
