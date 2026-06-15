SET(SOURCES ui_comp_button2.c
    ui_comp.c
    ui_Screen1.c
    ui.c
    ui_comp_hook.c
    ui_helpers.c
    ui_events.c
    ui_img_background_png.c
    ui_img_on_png.c
    ui_img_off_png.c)

add_library(ui ${SOURCES})
