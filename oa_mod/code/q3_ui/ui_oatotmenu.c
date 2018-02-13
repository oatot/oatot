#include "ui_local.h"

#define ART_BACK0           "menu/" MENU_OATOT_DIR "/myback_0"
#define ART_BACK1           "menu/" MENU_OATOT_DIR "/myback_1"
#define ART_MAKEBET0        "menu/" MENU_OATOT_DIR "/makebet_0"
#define ART_MAKEBET1        "menu/" MENU_OATOT_DIR "/makebet_1"
#define ART_DISCARDBET0     "menu/" MENU_OATOT_DIR "/discardbet_0"
#define ART_DISCARDBET1     "menu/" MENU_OATOT_DIR "/discardbet_1"
#define ART_EDITBET0        "menu/" MENU_OATOT_DIR "/editbet_0"
#define ART_EDITBET1        "menu/" MENU_OATOT_DIR "/editbet_1"
#define ART_BACKGROUND      "menu/" MENU_ART_DIR   "/addbotframe"

#define ID_BET0          0
#define ID_BET1          1
#define ID_BET2          2
#define ID_BET3          3
#define ID_BET4          4
#define ID_BACK          5
#define ID_MAKEBET       6
#define ID_DISCARDBET    7
#define ID_EDITBET       8

#define SIZE_OF_LIST 5

#define OATOT_MENU_VERTICAL_SPACING 30
#define FIRST_BET_Y 120

t_oatotinfo oatotinfo;

const char* betHorse_items[] = {
    "RED",
    "BLUE",
    NULL
};

const char* betCurrency_items[] = {
    "OAC",
    "BTC",
    NULL
};


typedef struct {
    menuframework_s menu;
    menutext_s      banner;
    menutext_s      info;
    menubitmap_s    back;
    menubitmap_s    makeBet;
    menubitmap_s    discardBet;
    menubitmap_s    editBet;

    menutext_s activeBets[SIZE_OF_LIST];

    int     selected;
} oatotmenu_t;

static oatotmenu_t s_oatotmenu;

/*
=================
GetBalanceLen
=================
*/
int GetBalanceLen(void) {
    int oac_balance = oatotinfo.oac_balance.free_money;
    int btc_balance = oatotinfo.btc_balance.free_money;
    if (oac_balance > btc_balance) {
        return oac_balance / 10 + 2;
    } else {
        return btc_balance / 10 + 2;
    }
}

/*
=================
InitBetFromInput
=================
*/
void InitBetFromInput(activeBet_t* bet, int horse_index, int currency_index, menufield_s amount_field) {
    strcpy(bet->horse, betHorse_items[horse_index]);
    bet->amount = atoi(amount_field.field.buffer);
    strcpy(bet->currency, betCurrency_items[currency_index]);
}

/*
=================
DiscardBet
=================
*/
void DiscardBet(int bet_index) {
    trap_Cmd_ExecuteText(EXEC_APPEND, va("unbet %d\n", bet_index));
}

/*
=================
MakeBet
=================
*/
void MakeBet(activeBet_t bet) {
    trap_Cmd_ExecuteText(
        EXEC_APPEND,
        va("bet %s %d %s\n", bet.horse, bet.amount, bet.currency)
    );
}

/*
=================
CheckBetUpper
=================
*/
qboolean CheckBetUpper(activeBet_t bet) {
    if (!Q_stricmp(bet.currency, "OAC")) {
        if (bet.amount > oatotinfo.oac_balance.free_money) {
            return qfalse;
        }
    } else if (!Q_stricmp(bet.currency, "BTC")) {
        if (bet.amount > oatotinfo.btc_balance.free_money) {
            return qfalse;
        }
    }
    return qtrue;
}

/*
=================
CheckBetLower
=================
*/
qboolean CheckBetLower(activeBet_t bet) {
    return bet.amount > 0;
}

/*
=================
getDefaultBet
=================
*/
static activeBet_t getDefaultBet(int free_money) {
    activeBet_t bet;
    int optimal = free_money / OPTIMAL_BET_AMOUNT_MAGIC_COEFFICIENT;
    strcpy(bet.horse, "red");
    bet.amount = (optimal ? optimal : 1);
    strcpy(bet.currency, "OAC");
    return bet;
}

/*
=================
Bet_Event
=================
*/
static void Bet_Event(void* ptr, int event) {
    if (event != QM_ACTIVATED) {
        return;
    }
    if (s_oatotmenu.selected != ((menucommon_s*)ptr)->id) {
        s_oatotmenu.selected = ((menucommon_s*)ptr)->id;
    }
}

/*
=================
MakeBet_StatusBar
=================
*/
static void MakeBet_StatusBar(void* self) {
    UI_DrawString(320, 320, "Click to make new bet.", UI_CENTER | UI_SMALLFONT, colorGreen);
}

/*
=================
DiscardBet_StatusBar
=================
*/
static void DiscardBet_StatusBar(void* self) {
    UI_DrawString(320, 320, "Click to discard the bet which is currently selected.", UI_CENTER | UI_SMALLFONT, colorGreen);
}

/*
=================
EditBet_StatusBar
=================
*/
static void EditBet_StatusBar(void* self) {
    UI_DrawString(320, 320, "Click to edit the bet which is currently selected.", UI_CENTER | UI_SMALLFONT, colorGreen);
}

/*
=================
OatotMenu_Event
=================
*/
static void OatotMenu_Event(void* ptr, int event) {
    activeBet_t selected_bet = oatotinfo.bets[s_oatotmenu.selected];
    if (event != QM_ACTIVATED) {
        return;
    }
    switch (((menucommon_s*)ptr)->id) {
    case ID_BACK:
        if (event != QM_ACTIVATED) {
            return;
        }
        UI_PopMenu();
        break;
    case ID_MAKEBET:
        UI_BetMenu(getDefaultBet(oatotinfo.oac_balance.free_money));
        break;
    case ID_DISCARDBET:
        DiscardBet(s_oatotmenu.selected);
        UI_OatotMenuInternal();
        UI_ForceMenuOff();
        break;
    case ID_EDITBET:
        DiscardBet(s_oatotmenu.selected);
        UI_OatotMenuInternal();
        UI_BetMenu(selected_bet);
        break;
    }
}

/*
=================
UI_OatotMenu_Draw
=================
*/
static void UI_OatotMenu_Draw(void) {
    int x, y;
    UI_DrawBannerString(320, 16, "YOUR ACTIVE BETS", UI_CENTER | UI_SMALLFONT, color_white);
    UI_DrawNamedPic(320 - 330, 240 - 166, 660, 332, ART_BACKGROUND);
    if (s_oatotmenu.selected >= 0 && s_oatotmenu.selected < oatotinfo.bets_n) {
        // Draw current bet selection.
        if (s_oatotmenu.activeBets[s_oatotmenu.selected].generic.id < oatotinfo.bets_n) {
            // Bet actually exists.
            y = FIRST_BET_Y + s_oatotmenu.selected * OATOT_MENU_VERTICAL_SPACING;
            x = 220;
            UI_DrawRect(x, y, 250, OATOT_MENU_VERTICAL_SPACING, colorGreen);
        }
    }
    // Standard menu drawing.
    Menu_Draw(&s_oatotmenu.menu);
}

/*
=================
OatotMenu_Cache
=================
*/
static void OatotMenu_Cache(void) {
    trap_R_RegisterShaderNoMip(ART_BACK0);
    trap_R_RegisterShaderNoMip(ART_BACK1);
    trap_R_RegisterShaderNoMip(ART_MAKEBET0);
    trap_R_RegisterShaderNoMip(ART_MAKEBET1);
    trap_R_RegisterShaderNoMip(ART_DISCARDBET0);
    trap_R_RegisterShaderNoMip(ART_DISCARDBET1);
    trap_R_RegisterShaderNoMip(ART_EDITBET0);
    trap_R_RegisterShaderNoMip(ART_EDITBET1);
    trap_R_RegisterShaderNoMip(ART_BACKGROUND);
}

static void setBet(menutext_s* menu, int y, int id, char* text) {
    menu->generic.type        = MTYPE_PTEXT;
    menu->color               = color_red;
    menu->generic.flags       = QMF_PULSEIFFOCUS | QMF_CENTER_JUSTIFY;
    menu->generic.x           = 320;
    menu->generic.y           = y;
    menu->generic.id          = id;
    menu->generic.callback    = Bet_Event;
    if (id < oatotinfo.bets_n) {
        // Bet actually exists.
        menu->color           = color_orange;
    } else {
        // Bet doesn't exist, let's hide it.
        menu->generic.flags |= QMF_INACTIVE | QMF_GRAYED;
    }
    menu->style               = UI_CENTER | UI_SMALLFONT;
    menu->string              = text;
}

/*
=================
UI_OatotMenuInternal
 *Used then forcing a redraw
=================
*/
void UI_OatotMenuInternal(void) {
    char info[MAX_INFO_STRING];
    int y, i, game_stage;
    trap_GetConfigString(CS_SERVERINFO, info, MAX_INFO_STRING);
    game_stage = atoi(Info_ValueForKey(info, "g_gameStage"));
    trap_Cmd_ExecuteText(EXEC_APPEND, "getBalance OAC\n");
    trap_Cmd_ExecuteText(EXEC_APPEND, "getBalance BTC\n");
    // Menu.
    s_oatotmenu.menu.wrapAround = qtrue;
    s_oatotmenu.menu.fullscreen = qfalse;
    s_oatotmenu.menu.draw = UI_OatotMenu_Draw;
    // Banner.
    s_oatotmenu.banner.generic.type   = MTYPE_BTEXT;
    s_oatotmenu.banner.generic.x      = 320;
    s_oatotmenu.banner.generic.y      = 16;
    s_oatotmenu.banner.string         = "YOUR ACTIVE BETS";
    s_oatotmenu.banner.color          = color_white;
    s_oatotmenu.banner.style          = UI_CENTER;
    // Info.
    s_oatotmenu.info.generic.type     = MTYPE_TEXT;
    if (game_stage != FORMING_TEAMS) {
        s_oatotmenu.info.generic.flags = QMF_HIDDEN;
    }
    s_oatotmenu.info.generic.x        = 320;
    s_oatotmenu.info.generic.y        = 170;
    s_oatotmenu.info.string           = "Betting is not yet started.";
    s_oatotmenu.info.color            = color_orange;
    s_oatotmenu.info.style            = UI_CENTER | UI_SMALLFONT;
    // Initialize horse, amount and currency menu components.
    y = FIRST_BET_Y;
    for (i = 0; i < SIZE_OF_LIST; i++) {
        setBet(&s_oatotmenu.activeBets[i], y, i, oatotinfo.betStrings[i]);
        y += OATOT_MENU_VERTICAL_SPACING;
    }
    // Button back.
    s_oatotmenu.back.generic.type       = MTYPE_BITMAP;
    s_oatotmenu.back.generic.name       = ART_BACK0;
    s_oatotmenu.back.generic.flags      = QMF_LEFT_JUSTIFY | QMF_PULSEIFFOCUS;
    s_oatotmenu.back.generic.id         = ID_BACK;
    s_oatotmenu.back.generic.callback   = OatotMenu_Event;
    s_oatotmenu.back.generic.x          = 120 - 90;
    s_oatotmenu.back.generic.y          = 410 - 45;
    s_oatotmenu.back.width              = 90;
    s_oatotmenu.back.height             = 45;
    s_oatotmenu.back.focuspic           = ART_BACK1;
    // Button makeBet.
    s_oatotmenu.makeBet.generic.type        = MTYPE_BITMAP;
    s_oatotmenu.makeBet.generic.name        = ART_MAKEBET0;
    if (game_stage == MAKING_BETS) {
        s_oatotmenu.makeBet.generic.flags   = QMF_LEFT_JUSTIFY | QMF_PULSEIFFOCUS;
    } else {
        s_oatotmenu.makeBet.generic.flags   = QMF_GRAYED;
    }
    s_oatotmenu.makeBet.generic.id          = ID_MAKEBET;
    s_oatotmenu.makeBet.generic.callback    = OatotMenu_Event;
    s_oatotmenu.makeBet.generic.statusbar   = MakeBet_StatusBar;
    s_oatotmenu.makeBet.generic.x           = 120 + BUTTON_HORIZONTAL_SPACING - 125;
    s_oatotmenu.makeBet.generic.y           = 410 - 45;
    s_oatotmenu.makeBet.width               = 125;
    s_oatotmenu.makeBet.height              = 45;
    s_oatotmenu.makeBet.focuspic            = ART_MAKEBET1;
    // Button discardBet.
    s_oatotmenu.discardBet.generic.type         = MTYPE_BITMAP;
    s_oatotmenu.discardBet.generic.name         = ART_DISCARDBET0;
    if (game_stage == MAKING_BETS) {
        s_oatotmenu.discardBet.generic.flags    = QMF_LEFT_JUSTIFY | QMF_PULSEIFFOCUS;
    } else {
        s_oatotmenu.discardBet.generic.flags    = QMF_GRAYED;
    }
    s_oatotmenu.discardBet.generic.id           = ID_DISCARDBET;
    s_oatotmenu.discardBet.generic.callback     = OatotMenu_Event;
    s_oatotmenu.discardBet.generic.statusbar    = DiscardBet_StatusBar;
    s_oatotmenu.discardBet.generic.x            = 120 + BUTTON_HORIZONTAL_SPACING * 2 - 125;
    s_oatotmenu.discardBet.generic.y            = 410 - 45;
    s_oatotmenu.discardBet.width                = 125;
    s_oatotmenu.discardBet.height               = 45;
    s_oatotmenu.discardBet.focuspic             = ART_DISCARDBET1;
    // Button editBet.
    s_oatotmenu.editBet.generic.type         = MTYPE_BITMAP;
    s_oatotmenu.editBet.generic.name         = ART_EDITBET0;
    if (game_stage == MAKING_BETS) {
        s_oatotmenu.editBet.generic.flags    = QMF_LEFT_JUSTIFY | QMF_PULSEIFFOCUS;
    } else {
        s_oatotmenu.editBet.generic.flags    = QMF_GRAYED;
    }
    s_oatotmenu.editBet.generic.id           = ID_EDITBET;
    s_oatotmenu.editBet.generic.callback     = OatotMenu_Event;
    s_oatotmenu.editBet.generic.statusbar    = EditBet_StatusBar;
    s_oatotmenu.editBet.generic.x            = 120 + BUTTON_HORIZONTAL_SPACING * 3 - 125;
    s_oatotmenu.editBet.generic.y            = 410 - 45;
    s_oatotmenu.editBet.width                = 125;
    s_oatotmenu.editBet.height               = 45;
    s_oatotmenu.editBet.focuspic             = ART_EDITBET1;
}

/*
=================
UI_OatotMenu
 *Called from outside
=================
*/
void UI_OatotMenu(void) {
    int i;
    OatotMenu_Cache();
    memset(&s_oatotmenu, 0, sizeof(oatotmenu_t));
    trap_Cmd_ExecuteText(EXEC_APPEND, "getActiveBets\n");
    trap_Cvar_Set("cl_paused", "0");   // We cannot send server commands while paused!
    UI_OatotMenuInternal();
    Menu_AddItem(&s_oatotmenu.menu, (void*) &s_oatotmenu.banner);
    Menu_AddItem(&s_oatotmenu.menu, (void*) &s_oatotmenu.info);
    Menu_AddItem(&s_oatotmenu.menu, (void*) &s_oatotmenu.back);
    Menu_AddItem(&s_oatotmenu.menu, (void*) &s_oatotmenu.makeBet);
    Menu_AddItem(&s_oatotmenu.menu, (void*) &s_oatotmenu.discardBet);
    Menu_AddItem(&s_oatotmenu.menu, (void*) &s_oatotmenu.editBet);
    for (i = 0; i < SIZE_OF_LIST; i++) {
        Menu_AddItem(&s_oatotmenu.menu, (void*) &s_oatotmenu.activeBets[i]);
    }
    UI_PushMenu(&s_oatotmenu.menu);
}
