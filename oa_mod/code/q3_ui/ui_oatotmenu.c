#include "ui_local.h"

#define ART_BACK0           "menu/" MENU_OATOT_DIR "/myback_0"
#define ART_BACK1           "menu/" MENU_OATOT_DIR "/myback_1"
#define ART_MAKEBET0        "menu/" MENU_OATOT_DIR "/makebet_0"
#define ART_MAKEBET1        "menu/" MENU_OATOT_DIR "/makebet_1"
#define ART_DISCARDBET0     "menu/" MENU_OATOT_DIR "/discardbet_0"
#define ART_DISCARDBET1     "menu/" MENU_OATOT_DIR "/discardbet_1"
#define ART_BACKGROUND      "menu/" MENU_ART_DIR   "/addbotframe"

#define ID_BET0_HORSE           0
#define ID_BET0_AMOUNT          1
#define ID_BET0_CURRENCY        2
#define ID_BET1_HORSE           3
#define ID_BET1_AMOUNT          4
#define ID_BET1_CURRENCY        5
#define ID_BET2_HORSE           6
#define ID_BET2_AMOUNT          7
#define ID_BET2_CURRENCY        8
#define ID_BET3_HORSE           9
#define ID_BET3_AMOUNT          10
#define ID_BET3_CURRENCY        11
#define ID_BET4_HORSE           12
#define ID_BET4_AMOUNT          13
#define ID_BET4_CURRENCY        14
#define ID_BACK                 15
#define ID_MAKEBET              16
#define ID_DISCARDBET           17

#define SIZE_OF_LIST 5

#define OATOT_MENU_VERTICAL_SPACING 20

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
    menubitmap_s    back;
    menubitmap_s    makeBet;
    menubitmap_s    discardBet;

    menulist_s betHorses[SIZE_OF_LIST];
    menufield_s betAmounts[SIZE_OF_LIST];
    menulist_s betCurrencies[SIZE_OF_LIST];

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
        return oac_balance / 10;
    } else {
        return btc_balance / 10;
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
void DiscardBet(activeBet_t bet) {
    trap_Cmd_ExecuteText(EXEC_APPEND, va("unbet %d", bet.id));
}

/*
=================
MakeBet
=================
*/
void MakeBet(activeBet_t bet) {
    trap_Cmd_ExecuteText(
        EXEC_APPEND,
        va("bet %s %d %s", bet.horse, bet.amount, bet.currency)
    );
}

/*
=================
CheckBetUpper
=================
*/
qboolean CheckBetUpper(activeBet_t bet) {
    if (!strcmp(bet.currency, "OAC")) {
        if (bet.amount > oatotinfo.oac_balance.free_money) {
            return qfalse;
        }
    } else if (!strcmp(bet.currency, "BTC")) {
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
GetSelectedBetIndex
=================
*/
static int GetSelectedBetIndex(void) {
    return s_oatotmenu.selected / 3;
}

/*
=================
GetSelectedBet
=================
*/
static void GetSelectedBet(activeBet_t* bet) {
    int bet_index;
    bet_index = GetSelectedBetIndex();
    if (bet_index >= 0 && bet_index < oatotinfo.bets_n) {
        InitBetFromInput(
            bet,
            s_oatotmenu.betHorses[bet_index].curvalue,
            s_oatotmenu.betCurrencies[bet_index].curvalue,
            s_oatotmenu.betAmounts[bet_index]
        );
    }
}

/*
=================
Bet_Event
=================
*/
static void Bet_Event(void* ptr, int event) {
    activeBet_t bet;
    GetSelectedBet(&bet);
    if (s_oatotmenu.selected != ((menucommon_s*)ptr)->id) {
        s_oatotmenu.selected = ((menucommon_s*)ptr)->id;
    }
    if (event != QM_ACTIVATED) {
        return;
    }
    if (CheckBetUpper(bet) && CheckBetLower(bet)) {
        // Discard old bet.
        DiscardBet(bet);
        // Make new bet with new input data.
        MakeBet(bet);
    } else {
        // Invalid input (amount).
        // We don't make a bet and set previous values instead.
        UI_OatotMenuInternal();
    }
}

/*
=================
OatotMenu_Event
=================
*/
static void OatotMenu_Event(void* ptr, int event) {
    activeBet_t bet;
    GetSelectedBet(&bet);
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
        UI_BetMenu();
        break;
    case ID_DISCARDBET:
        DiscardBet(bet);
        break;
    }
}

/*
=================
UI_OatotMenu_Draw
=================
*/
static void UI_OatotMenu_Draw(void) {
    UI_DrawBannerString(320, 16, "YOUR ACTIVE BETS", UI_CENTER, color_white);
    UI_DrawNamedPic(320 - 275, 240 - 166, 550, 332, ART_BACKGROUND);
    // standard menu drawing
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
    trap_R_RegisterShaderNoMip(ART_BACKGROUND);
}

static void setBetHorse(menulist_s* menu, int y, int id, const char* horse) {
    menu->generic.type        = MTYPE_SPINCONTROL;
    menu->generic.flags       = QMF_PULSEIFFOCUS | QMF_SMALLFONT;
    menu->generic.x           = 170;
    menu->generic.y           = y;
    menu->generic.id          = id;
    menu->generic.name        = "Horse: ";
    menu->generic.callback    = Bet_Event;
    menu->itemnames           = betHorse_items;
    if (!strcmp(horse, "red")) {
        menu->curvalue = 0;
    } else if (!strcmp(horse, "blue")) {
        menu->curvalue = 1;
    }
}

static void setBetAmount(menufield_s* menu, int y, int id, int amount) {
    menu->generic.type        = MTYPE_FIELD;
    menu->generic.flags       = QMF_NUMBERSONLY | QMF_PULSEIFFOCUS | QMF_SMALLFONT;
    menu->generic.x           = 320;
    menu->generic.y           = y;
    menu->generic.id          = id;
    menu->generic.name        = "Amount: ";
    menu->generic.callback    = Bet_Event;
    menu->field.maxchars      = GetBalanceLen();
    Q_strncpyz(menu->field.buffer, va("%d", amount), sizeof(menu->field.buffer));
}

static void setBetCurrency(menulist_s* menu, int y, int id, const char* currency) {
    menu->generic.type        = MTYPE_SPINCONTROL;
    menu->generic.flags       = QMF_PULSEIFFOCUS | QMF_SMALLFONT;
    menu->generic.x           = 470;
    menu->generic.y           = y;
    menu->generic.id          = id;
    menu->generic.name        = "Currency: ";
    menu->generic.callback    = Bet_Event;
    menu->itemnames           = betCurrency_items;
    if (!strcmp(currency, "OAC")) {
        menu->curvalue = 0;
    } else if (!strcmp(currency, "BTC")) {
        menu->curvalue = 1;
    }
}

/*
=================
UI_OatotMenuInternal
 *Used then forcing a redraw
=================
*/
void UI_OatotMenuInternal(void) {
    int y, i;
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
    // Initialize horse, amount and currency menu components.
    y = 98;
    for (i = 0; i < oatotinfo.bets_n; i++) {
        setBetHorse(&s_oatotmenu.betHorses[i], y, i * 3, oatotinfo.bets[i].horse);
        setBetAmount(&s_oatotmenu.betAmounts[i], y, i * 3 + 1, oatotinfo.bets[i].amount);
        setBetCurrency(&s_oatotmenu.betCurrencies[i], y, i * 3 + 2, oatotinfo.bets[i].currency);
        y += OATOT_MENU_VERTICAL_SPACING;
    }
    // Button back.
    s_oatotmenu.back.generic.type       = MTYPE_BITMAP;
    s_oatotmenu.back.generic.name       = ART_BACK0;
    s_oatotmenu.back.generic.flags      = QMF_LEFT_JUSTIFY | QMF_PULSEIFFOCUS;
    s_oatotmenu.back.generic.id         = ID_BACK;
    s_oatotmenu.back.generic.callback   = OatotMenu_Event;
    s_oatotmenu.back.generic.x          = 220 - 90;
    s_oatotmenu.back.generic.y          = 410 - 45;
    s_oatotmenu.back.width              = 90;
    s_oatotmenu.back.height             = 45;
    s_oatotmenu.back.focuspic           = ART_BACK1;
    // Button makeBet.
    s_oatotmenu.makeBet.generic.type        = MTYPE_BITMAP;
    s_oatotmenu.makeBet.generic.name        = ART_MAKEBET0;
    s_oatotmenu.makeBet.generic.flags       = QMF_LEFT_JUSTIFY | QMF_PULSEIFFOCUS;
    s_oatotmenu.makeBet.generic.id          = ID_MAKEBET;
    s_oatotmenu.makeBet.generic.callback    = OatotMenu_Event;
    s_oatotmenu.makeBet.generic.x           = 220 + BUTTON_HORIZONTAL_SPACING - 90;
    s_oatotmenu.makeBet.generic.y           = 410 - 45;
    s_oatotmenu.makeBet.width               = 90;
    s_oatotmenu.makeBet.height              = 45;
    s_oatotmenu.makeBet.focuspic            = ART_MAKEBET1;
    // Button discardBet.
    s_oatotmenu.discardBet.generic.type         = MTYPE_BITMAP;
    s_oatotmenu.discardBet.generic.name         = ART_DISCARDBET0;
    s_oatotmenu.discardBet.generic.flags        = QMF_LEFT_JUSTIFY | QMF_PULSEIFFOCUS;
    s_oatotmenu.discardBet.generic.id           = ID_DISCARDBET;
    s_oatotmenu.discardBet.generic.callback     = OatotMenu_Event;
    s_oatotmenu.discardBet.generic.x            = 220 + BUTTON_HORIZONTAL_SPACING * 2 - 90;
    s_oatotmenu.discardBet.generic.y            = 410 - 45;
    s_oatotmenu.discardBet.width                = 90;
    s_oatotmenu.discardBet.height               = 45;
    s_oatotmenu.discardBet.focuspic             = ART_DISCARDBET1;
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
    trap_Cmd_ExecuteText(EXEC_APPEND, "getActiveBets");
    UI_OatotMenuInternal();
    // We need to initialize the bets list or it will be impossible to click on the items.
    for (i = 0; i < oatotinfo.bets_n; i++) {
        //Q_strncpyz(mappage.mapname[i],"----",5);
    }
    trap_Cvar_Set("cl_paused", "0");   // We cannot send server commands while paused!
    Menu_AddItem(&s_oatotmenu.menu, (void*) &s_oatotmenu.banner);
    Menu_AddItem(&s_oatotmenu.menu, (void*) &s_oatotmenu.back);
    Menu_AddItem(&s_oatotmenu.menu, (void*) &s_oatotmenu.makeBet);
    Menu_AddItem(&s_oatotmenu.menu, (void*) &s_oatotmenu.discardBet);
    for (i = 0; i < oatotinfo.bets_n; i++) {
        Menu_AddItem(&s_oatotmenu.menu, (void*) &s_oatotmenu.betHorses[i]);
        Menu_AddItem(&s_oatotmenu.menu, (void*) &s_oatotmenu.betAmounts[i]);
        Menu_AddItem(&s_oatotmenu.menu, (void*) &s_oatotmenu.betCurrencies[i]);
    }
    UI_PushMenu(&s_oatotmenu.menu);
}
