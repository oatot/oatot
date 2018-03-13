#include "ui_local.h"

#define ART_BACK0 "menu/" MENU_OATOT_DIR "/myback_0"
#define ART_BACK1 "menu/" MENU_OATOT_DIR "/myback_1"
#define ART_OK0 "menu/" MENU_OATOT_DIR "/ok_0"
#define ART_OK1 "menu/" MENU_OATOT_DIR "/ok_1"
#define ART_BACKGROUND "menu/" MENU_ART_DIR "/addbotframe"

#define ID_HORSE 0
#define ID_AMOUNT 1
#define ID_CURRENCY 2
#define ID_BACK 3
#define ID_OK 4

typedef struct {
    menuframework_s menu;
    menutext_s banner;
    menubitmap_s back;
    menubitmap_s ok;

    menulist_s betHorse;
    menufield_s betAmount;
    menulist_s betCurrency;
} betmenu_t;

static betmenu_t s_betmenu;

/*
=================
GetCurrentBet
=================
*/
static void GetCurrentBet(activeBet_t* bet) {
    InitBetFromInput(bet, s_betmenu.betHorse.curvalue, s_betmenu.betCurrency.curvalue, s_betmenu.betAmount);
}

/*
=================
CheckAndNormalize
=================
*/
static qboolean CheckAndNormalize(activeBet_t bet) {
    balance_t balance;
    if (!CheckBetLower(bet)) {
        Q_strncpyz(s_betmenu.betAmount.field.buffer, va("%d", 0), sizeof(s_betmenu.betAmount.field.buffer));
        return qfalse;
    } else if (!CheckBetUpper(bet)) {
        if (GetBalanceByCurrency(bet.currency, oatotinfo.balances, &balance)) {
            Q_strncpyz(
                s_betmenu.betAmount.field.buffer,
                va("%d", balance.freeMoney),
                sizeof(s_betmenu.betAmount.field.buffer)
            );
            return qfalse;
        }
    }
    return qtrue;
}

/*
=================
BetMenu_Event
=================
*/
static void BetMenu_Event(void* ptr, int event) {
    activeBet_t current_bet;
    GetCurrentBet(&current_bet);
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
    case ID_OK:
        if (CheckAndNormalize(current_bet)) {
            MakeBet(current_bet);
            UI_ForceMenuOff();
        }
        break;
    default:
        CheckAndNormalize(current_bet);
        break;
    }
}

/*
=================
UI_BetMenu_Draw
=================
*/
static void UI_BetMenu_Draw(void) {
    UI_DrawBannerString(320, 80, "MAKE BET", UI_CENTER | UI_SMALLFONT, color_white);
    UI_DrawNamedPic(320 - 233, 240 - 166, 466, 332, ART_BACKGROUND);
    // Standard menu drawing.
    Menu_Draw(&s_betmenu.menu);
}

/*
=================
BetMenu_Cache
=================
*/
static void BetMenu_Cache(void) {
    trap_R_RegisterShaderNoMip(ART_BACK0);
    trap_R_RegisterShaderNoMip(ART_BACK1);
    trap_R_RegisterShaderNoMip(ART_OK0);
    trap_R_RegisterShaderNoMip(ART_OK1);
    trap_R_RegisterShaderNoMip(ART_BACKGROUND);
}

/*
=================
UI_BetMenuInternal
 *Used then forcing a redraw
=================
*/
void UI_BetMenuInternal(activeBet_t bet, qboolean edit_mode) {
    trap_Cmd_ExecuteText(EXEC_APPEND, "getBalance\n");
    // Menu.
    s_betmenu.menu.wrapAround = qtrue;
    s_betmenu.menu.fullscreen = qfalse;
    s_betmenu.menu.draw = UI_BetMenu_Draw;
    // Banner.
    s_betmenu.banner.generic.type = MTYPE_BTEXT;
    s_betmenu.banner.generic.x = 320;
    s_betmenu.banner.generic.y = 80;
    s_betmenu.banner.string = "MAKE BET";
    s_betmenu.banner.color = color_white;
    s_betmenu.banner.style = UI_CENTER | UI_SMALLFONT;
    // Initialize horse, amount and currency menu components.
    // Horse.
    s_betmenu.betHorse.generic.type = MTYPE_SPINCONTROL;
    s_betmenu.betHorse.generic.flags = QMF_PULSEIFFOCUS | QMF_SMALLFONT;
    s_betmenu.betHorse.generic.x = 320;
    s_betmenu.betHorse.generic.y = 150;
    s_betmenu.betHorse.generic.id = ID_HORSE;
    s_betmenu.betHorse.generic.name = "Horse: ";
    s_betmenu.betHorse.generic.callback = BetMenu_Event;
    s_betmenu.betHorse.itemnames = betHorse_items;
    if (!Q_stricmp(bet.horse, "red")) {
        s_betmenu.betHorse.curvalue = 0;
    } else if (!Q_stricmp(bet.horse, "blue")) {
        s_betmenu.betHorse.curvalue = 1;
    }
    // Amount.
    s_betmenu.betAmount.generic.type = MTYPE_FIELD;
    s_betmenu.betAmount.generic.flags = QMF_NUMBERSONLY | QMF_PULSEIFFOCUS | QMF_SMALLFONT;
    s_betmenu.betAmount.generic.x = 320;
    s_betmenu.betAmount.generic.y = 200;
    s_betmenu.betAmount.generic.id = ID_AMOUNT;
    s_betmenu.betAmount.generic.name = "Amount: ";
    s_betmenu.betAmount.generic.callback = BetMenu_Event;
    s_betmenu.betAmount.field.widthInChars = GetBalanceLen();
    // Currency.
    s_betmenu.betCurrency.generic.type = MTYPE_SPINCONTROL;
    s_betmenu.betCurrency.generic.flags = QMF_PULSEIFFOCUS | QMF_SMALLFONT;
    s_betmenu.betCurrency.generic.x = 320;
    s_betmenu.betCurrency.generic.y = 250;
    s_betmenu.betCurrency.generic.id = ID_CURRENCY;
    s_betmenu.betCurrency.generic.name = "Currency: ";
    s_betmenu.betCurrency.generic.callback = BetMenu_Event;
    s_betmenu.betCurrency.itemnames = betCurrency_items;
    if (!Q_stricmp(bet.horse, "oac")) {
        s_betmenu.betCurrency.curvalue = 0;
    } else if (!Q_stricmp(bet.horse, "btc")) {
        s_betmenu.betCurrency.curvalue = 1;
    }
    // Button back.
    s_betmenu.back.generic.type = MTYPE_BITMAP;
    s_betmenu.back.generic.name = ART_BACK0;
    if (edit_mode) {
        // When editing the bet, we discard it first.
        // So pressing back would have the effect of discarding bet completely.
        // Since we have discardBet button for that, it doesn't make much sense.
        // Therefore, let's just discard back option when in editing mode.
        // If they don't want to change anything, pressing OK will create same bet again.
        s_betmenu.back.generic.flags = QMF_GRAYED;
    } else {
        s_betmenu.back.generic.flags = QMF_LEFT_JUSTIFY | QMF_PULSEIFFOCUS;
    }
    s_betmenu.back.generic.id = ID_BACK;
    s_betmenu.back.generic.callback = BetMenu_Event;
    s_betmenu.back.generic.x = 220 - 90;
    s_betmenu.back.generic.y = 410 - 45;
    s_betmenu.back.width = 90;
    s_betmenu.back.height = 45;
    s_betmenu.back.focuspic = ART_BACK1;
    // Button ok.
    s_betmenu.ok.generic.type = MTYPE_BITMAP;
    s_betmenu.ok.generic.name = ART_OK0;
    s_betmenu.ok.generic.flags = QMF_LEFT_JUSTIFY | QMF_PULSEIFFOCUS;
    s_betmenu.ok.generic.id = ID_OK;
    s_betmenu.ok.generic.callback = BetMenu_Event;
    s_betmenu.ok.generic.x = 220 + BUTTON_HORIZONTAL_SPACING - 90;
    s_betmenu.ok.generic.y = 410 - 45;
    s_betmenu.ok.width = 90;
    s_betmenu.ok.height = 45;
    s_betmenu.ok.focuspic = ART_OK1;
}

/*
=================
UI_BetMenu
 *Called from outside
=================
*/
void UI_BetMenu(activeBet_t bet, qboolean edit_mode) {
    BetMenu_Cache();
    memset(&s_betmenu, 0, sizeof(betmenu_t));
    UI_BetMenuInternal(bet, edit_mode);
    trap_Cvar_Set("cl_paused", "0"); // We cannot send server commands while paused!
    Menu_AddItem(&s_betmenu.menu, (void*) &s_betmenu.banner);
    Menu_AddItem(&s_betmenu.menu, (void*) &s_betmenu.back);
    Menu_AddItem(&s_betmenu.menu, (void*) &s_betmenu.ok);
    Menu_AddItem(&s_betmenu.menu, (void*) &s_betmenu.betHorse);
    Menu_AddItem(&s_betmenu.menu, (void*) &s_betmenu.betAmount);
    Menu_AddItem(&s_betmenu.menu, (void*) &s_betmenu.betCurrency);
    // Set amount.
    Q_strncpyz(
        s_betmenu.betAmount.field.buffer,
        va("%d", bet.amount),
        sizeof(s_betmenu.betAmount.field.buffer)
    );
    UI_PushMenu(&s_betmenu.menu);
}
