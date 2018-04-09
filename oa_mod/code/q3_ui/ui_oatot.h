#ifndef _UI_OATOT_H
#define _UI_OATOT_H

#include "../qcommon/q_shared.h"
#include "../ui/ui_public.h"
#include "../ui/ui_shared.h"
#include "ui_local.h"

extern qboolean enableBetting;

// For ui_ingame.c
#define ID_OATOT 21

#define MAX_BETSTRING_LENGTH 40

typedef struct {
    int balancesN;
    balance_t balances[CURRENCIES_N]; // Is needed to limit bet amount menu field.
    int betsN; // The actual number of active bets.
    activeBet_t bets[MAX_ACTIVE_BETS_NUMBER]; // Active bets.
    char betStrings[MAX_ACTIVE_BETS_NUMBER][MAX_BETSTRING_LENGTH];
} t_oatotinfo;

extern t_oatotinfo oatotinfo;

// ui_ingame_oatot.c
extern void UI_InitOatotField(menutext_s* oatot_text, int y);

// ui_oatotmenu.c
extern void UI_OatotMenu(void);
extern void UI_OatotMenuInternal(void);

// ui_betmenu.c
extern void UI_BetMenu(activeBet_t bet, qboolean edit_mode);
extern void UI_BetMenuInternal(activeBet_t bet, qboolean edit_mode);

#endif /* ifndef _UI_OATOT_H */
