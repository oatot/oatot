#include "ui_local.h"
#include "ui_oatot.h"

/*
=================
UI_InitOatotField
=================
*/
void UI_InitOatotField(menutext_s* oatot_text, int y) {
    oatot_text->generic.type = MTYPE_PTEXT;
    oatot_text->generic.flags = QMF_CENTER_JUSTIFY | QMF_PULSEIFFOCUS;
    oatot_text->generic.x = 320;
    oatot_text->generic.y = y;
    oatot_text->generic.id = ID_OATOT;
    oatot_text->generic.callback = InGame_Event;
    oatot_text->string = "OATOT MENU";
    oatot_text->color = color_red;
    oatot_text->style = UI_CENTER | UI_DROPSHADOW;
}
