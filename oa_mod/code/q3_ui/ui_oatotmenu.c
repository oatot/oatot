#include "ui_local.h"

#define ART_BACK0           "menu/" MENU_ART_DIR "/myback_0"
#define ART_BACK1           "menu/" MENU_ART_DIR "/myback_1"
#define ART_MAKEBET0        "menu/" MENU_ART_DIR "/makebet_0"
#define ART_MAKEBET1        "menu/" MENU_ART_DIR "/makebet_1"
#define ART_DISCARDBET0     "menu/" MENU_ART_DIR "/discardbet_0"
#define ART_DISCARDBET1     "menu/" MENU_ART_DIR "/discardbet_1"
#define ART_BACKGROUND      "menu/" MENU_ART_DIR "/addbotframe"

#define ID_BID0_HORSE           0
#define ID_BID0_AMOUNT          1
#define ID_BID0_CURRENCY        2
#define ID_BID1_HORSE           3
#define ID_BID1_AMOUNT          4
#define ID_BID1_CURRENCY        5
#define ID_BID2_HORSE           6
#define ID_BID2_AMOUNT          7
#define ID_BID2_CURRENCY        8
#define ID_BID3_HORSE           9
#define ID_BID3_AMOUNT          10
#define ID_BID3_CURRENCY        11
#define ID_BID4_HORSE           12
#define ID_BID4_AMOUNT          13
#define ID_BID4_CURRENCY        14
#define ID_BACK                 15
#define ID_MAKEBET              16
#define ID_DISCARDBET           17

#define SIZE_OF_LIST 5

#define OATOT_MENU_VERTICAL_SPACING 20
#define BUTTON_HORIZONTAL_SPACING 100

t_oatotinfo oatotinfo;

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

static const char* betHorse_items[] = {
    "RED",
    "BLUE",
    NULL
};

static const char* betCurrency_items[] = {
    "OAC",
    "BTC",
    NULL
};

/*
=================
GetBalanceLen
=================
*/
static int GetBalanceLen(void) {
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
GetSelectedBetIndex
=================
*/
static int GetSelectedBetIndex(void) {
    return s_oatotmenu.selected / 3;
}

/*
=================
InitBetFromInput
=================
*/
static void InitBetFromInput(activeBet_t* bet, int horse_index, int currency_index, menufield_s amount_field) {
    strcpy(bet->horse, betHorse_items[horse_index]);
    bet->amount = atoi(amount_field.field.buffer);
    strcpy(bet->currency, betCurrency_items[currency_index]);
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
DiscardBet
=================
*/
static void DiscardBet(activeBet_t bet) {
    trap_Cmd_ExecuteText(EXEC_APPEND, va("unbet %d", bet.id));
}

/*
=================
MakeBet
=================
*/
static void MakeBet(activeBet_t bet) {
    trap_Cmd_ExecuteText(
        EXEC_APPEND,
        va("bet %s %d %s", bet.horse, bet.amount, bet.currency)
    );
}

/*
=================
CheckBet
=================
*/
static qboolean CheckBet(activeBet_t bet) {
    if (!strcmp(bet.currency, "OAC")) {
        if (bet.amount > oatotinfo.oac_balance.free_money || bet.amount <= 0) {
            return qfalse;
        }
    } else if (!strcmp(bet.currency, "BTC")) {
        if (bet.amount > oatotinfo.btc_balance.free_money || bet.amount <= 0) {
            return qfalse;
        }
    }
    return qtrue;
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
    if (CheckBet(bet)) {
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
        //UI_BetMenu();
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
    UI_DrawBannerString(320, 16, "OATOT MENU", UI_CENTER, color_white);
    UI_DrawNamedPic(320 - 233, 240 - 166, 466, 332, ART_BACKGROUND);
    // standard menu drawing
    Menu_Draw(&s_oatotmenu.menu);
}

/*
=================
OatotMenu_DrawSpin
=================
*/
static void OatotMenu_SetDrawSpin(void* self, menulist_s* item, int* style, float* color) {
    qboolean focus;
    item = (menulist_s*) self;
    focus = (item->generic.parent->cursor == item->generic.menuPosition);
    *style = UI_LEFT | UI_SMALLFONT;
    color = text_color_normal;
    if (focus) {
        *style |= UI_PULSE;
        color = text_color_highlight;
    }
}

/*
=================
OatotMenu_DrawHorse
=================
*/
static void OatotMenu_DrawHorse(void* self) {
    menulist_s* item;
    qboolean    focus;
    int*        style;
    float*      color;
    OatotMenu_SetDrawSpin(self, item, style, color);
    UI_DrawProportionalString(item->generic.x, item->generic.y, "Horse", *style, color);
    UI_DrawProportionalString(item->generic.x + 64, item->generic.y + PROP_HEIGHT, betHorse_items[item->curvalue], *style, color);
}

/*
=================
OatotMenu_DrawAmount
=================
*/
static void OatotMenu_DrawAmount(void* self) {
    menufield_s*     f;
    qboolean        focus;
    int             style;
    char*            txt;
    char            c;
    float*           color;
    int             basex, x, y;
    f = (menufield_s*)self;
    basex = f->generic.x;
    y = f->generic.y;
    focus = (f->generic.parent->cursor == f->generic.menuPosition);
    style = UI_LEFT | UI_SMALLFONT;
    color = text_color_normal;
    if (focus) {
        style |= UI_PULSE;
        color = text_color_highlight;
    }
    UI_DrawProportionalString(basex, y, "Amount", style, color);
    // draw the actual amount
    basex += 64;
    y += PROP_HEIGHT;
    txt = f->field.buffer;
    color = g_color_table[ColorIndex(COLOR_WHITE)];
    x = basex;
    while ((c = *txt) != 0) {
        UI_DrawChar(x, y, c, style, color);
        txt++;
        x += SMALLCHAR_WIDTH;
    }
    // draw cursor if we have focus
    if (focus) {
        if (trap_Key_GetOverstrikeMode()) {
            c = 11;
        } else {
            c = 10;
        }
        style &= ~UI_PULSE;
        style |= UI_BLINK;
        UI_DrawChar(basex + f->field.cursor * SMALLCHAR_WIDTH, y, c, style, color_white);
    }
}

/*
=================
OatotMenu_DrawCurrency
=================
*/
static void OatotMenu_DrawCurrency(void* self) {
    menulist_s* item;
    qboolean    focus;
    int*        style;
    float*      color;
    OatotMenu_SetDrawSpin(self, item, style, color);
    UI_DrawProportionalString(item->generic.x, item->generic.y, "Currency", *style, color);
    UI_DrawProportionalString(item->generic.x + 64, item->generic.y + PROP_HEIGHT, betCurrency_items[item->curvalue], *style, color);
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
    menu->generic.flags       = QMF_NODEFAULTINIT;
    menu->generic.x           = 280 - 100;
    menu->generic.y           = y;
    menu->generic.left        = 280 - 100;
    menu->generic.right       = 280;
    menu->generic.top         = y - 8;
    menu->generic.bottom      = y + 2 * PROP_HEIGHT;
    menu->generic.id          = id;
    menu->generic.callback    = Bet_Event;
    menu->generic.ownerdraw   = OatotMenu_DrawHorse;
    menu->numitems            = 2;
    if (!strcmp(horse, "red")) {
        menu->curvalue = 0;
    } else if (!strcmp(horse, "blue")) {
        menu->curvalue = 1;
    }
}

static void setBetAmount(menufield_s* menu, int y, int id, int amount) {
    menu->generic.type        = MTYPE_FIELD;
    menu->generic.flags       = QMF_NODEFAULTINIT;
    menu->generic.x           = 390 - 100;
    menu->generic.y           = y;
    menu->generic.left        = 390 - 100;
    menu->generic.right       = 390;
    menu->generic.top         = y - 8;
    menu->generic.bottom      = y + 2 * PROP_HEIGHT;
    menu->generic.id          = id;
    menu->generic.callback    = Bet_Event;
    menu->generic.ownerdraw   = OatotMenu_DrawAmount;
    menu->field.maxchars      = GetBalanceLen();
    Q_strncpyz(menu->field.buffer, va("%d", amount), sizeof(menu->field.buffer));
}

static void setBetCurrency(menulist_s* menu, int y, int id, const char* currency) {
    menu->generic.type        = MTYPE_SPINCONTROL;
    menu->generic.flags       = QMF_NODEFAULTINIT;
    menu->generic.x           = 500 - 100;
    menu->generic.y           = y;
    menu->generic.left        = 500 - 100;
    menu->generic.right       = 500;
    menu->generic.top         = y - 8;
    menu->generic.bottom      = y + 2 * PROP_HEIGHT;
    menu->generic.id          = id;
    menu->generic.callback    = Bet_Event;
    menu->generic.ownerdraw   = OatotMenu_DrawCurrency;
    menu->numitems            = 2;
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
    s_oatotmenu.banner.string         = "YOUR ACTIVE BIDS";
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
    s_oatotmenu.back.generic.x          = 220 - 128;
    s_oatotmenu.back.generic.y          = 410 - 64;
    s_oatotmenu.back.width              = 128;
    s_oatotmenu.back.height             = 64;
    s_oatotmenu.back.focuspic           = ART_BACK1;
    // Button makeBet.
    s_oatotmenu.makeBet.generic.type        = MTYPE_BITMAP;
    s_oatotmenu.makeBet.generic.name        = ART_MAKEBET0;
    s_oatotmenu.makeBet.generic.flags       = QMF_LEFT_JUSTIFY | QMF_PULSEIFFOCUS;
    s_oatotmenu.makeBet.generic.id          = ID_MAKEBET;
    s_oatotmenu.makeBet.generic.callback    = OatotMenu_Event;
    s_oatotmenu.makeBet.generic.x           = 220 + BUTTON_HORIZONTAL_SPACING - 128;
    s_oatotmenu.makeBet.generic.y           = 410 - 64;
    s_oatotmenu.makeBet.width               = 128;
    s_oatotmenu.makeBet.height              = 64;
    s_oatotmenu.makeBet.focuspic            = ART_MAKEBET1;
    // Button discardBet.
    s_oatotmenu.discardBet.generic.type         = MTYPE_BITMAP;
    s_oatotmenu.discardBet.generic.name         = ART_DISCARDBET0;
    s_oatotmenu.discardBet.generic.flags        = QMF_LEFT_JUSTIFY | QMF_PULSEIFFOCUS;
    s_oatotmenu.discardBet.generic.id           = ID_DISCARDBET;
    s_oatotmenu.discardBet.generic.callback     = OatotMenu_Event;
    s_oatotmenu.discardBet.generic.x            = 220 + BUTTON_HORIZONTAL_SPACING * 2 - 128;
    s_oatotmenu.discardBet.generic.y            = 410 - 64;
    s_oatotmenu.discardBet.width                = 128;
    s_oatotmenu.discardBet.height               = 64;
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
