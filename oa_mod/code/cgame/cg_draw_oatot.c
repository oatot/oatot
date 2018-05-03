#include "cg_local.h"

static float* GetGameStageColor(void) {
    if (cgs.gameStage == FORMING_TEAMS) {
        return colorGreen;
    } else if (cgs.gameStage == MAKING_BETS) {
        return colorRed;
    } else if (cgs.gameStage == PLAYING) {
        return colorYellow;
    }
    return NULL;
}

static dictEntry_t currencyToShaderIndex[] = {
    {"OAC", 0},
    {"BTC", 1}
};

static dictEntry_t horseToFlagShaderIndex[] = {
    {"red", 1},
    {"blue", 2}
};

static dictEntry_t currencyToBalanceY[] = {
    {"OAC", 210},
    {"BTC", 255}
};

static dictEntry_t currencyToBetSumY[] = {
    {"OAC", 80}
    // {"BTC", 125} Do not draw BTC here for now.
};

// Space between value and currency coin image.
#define VALUE_SPACE_LEN 15

static int GetValueLength(int amount) {
    int val_len = strlen(va("%d", amount)) * SMALLCHAR_WIDTH;
    int coin_len = CHAR_WIDTH;
    return val_len + VALUE_SPACE_LEN + coin_len;
}

static int GetMaxAmountLength(void) {
    int max_len = 0, len = 0, i = 0;
    for (i = 0; i < cgs.clientinfo[cg.clientNum].betsN; i++) {
        len = strlen(va("%d", cgs.clientinfo[cg.clientNum].activeBets[i].amount));
        if (len > max_len) {
            max_len = len;
        }
    }
    return max_len * SMALLCHAR_WIDTH;
}

static int BetSumXByHorse(const char* horse, int value) {
    // Dirty solution for now.
    // With more horses, diff positions will be needed.
    if (!strcmp(horse, "red")) {
        return 0;
    } else if (!strcmp(horse, "blue")) {
        return 640 - GetValueLength(value);
    } else {
        // No horse.
        return -1;
    }
}

//==============================================================================

/*
=================
CG_DrawValue
=================
 */
static void CG_DrawValue(int x, int y, int amount, int shift, const char* currency) {
    int shader_index;
    char* val_str = va("%d", amount);
    CG_DrawSmallString(x, y, val_str, 1.0F);
    if (shift == -1) {
        shift = strlen(val_str) * SMALLCHAR_WIDTH;
    }
    if (DictFind(currencyToShaderIndex, CURRENCIES_N, currency, &shader_index)) {
        qhandle_t currencyShader = cgs.media.currencyShader[shader_index];
        CG_DrawPic(x + shift + VALUE_SPACE_LEN, y - 10, CHAR_WIDTH, CHAR_WIDTH, currencyShader);
    }
}

/*
=================
CG_DrawBet
=================
 */
static void CG_DrawBet(int x, int y, int shift, activeBet_t bet) {
    int flag_shader_index;
    if (DictFind(horseToFlagShaderIndex, HORSES_N, bet.horse, &flag_shader_index)) {
        CG_DrawPic(x, y - 10, CHAR_WIDTH, CHAR_WIDTH, cgs.media.flagShader[flag_shader_index]);
        CG_DrawValue(x + CHAR_WIDTH + VALUE_SPACE_LEN, y, bet.amount, shift, bet.currency);
    }
}

/*
=================
CG_DrawGameStageInfo
=================
 */
static void CG_DrawGameStageInfo(void) {
    if (cgs.gameStage == FORMING_TEAMS) {
        CG_DrawBigString(320 - 7 * BIGCHAR_WIDTH, 50, "^2FORMING TEAMS", 1.0F);
        CG_DrawSmallString(320 - 24 * SMALLCHAR_WIDTH, 100, "Type ^2/help ^7to get help, ^2/ready ^7to start betting.", 1.0F);
    } else if (cgs.gameStage == MAKING_BETS) {
        CG_DrawPic(270, 0, 100, 100, cgs.media.lockShader);
        CG_DrawBigString(320 - 6 * BIGCHAR_WIDTH, 50, "^1MAKING BETS", 1.0F);
        CG_DrawSmallString(320 - 20 * SMALLCHAR_WIDTH, 100, "Press ^1ESC --> OATOT MENU ^7to make a bet.", 1.0F);
    } else if (cgs.gameStage == PLAYING) {
        CG_DrawPic(270, 0, 100, 100, cgs.media.lockShader);
        CG_DrawBigString(320 - 4 * BIGCHAR_WIDTH, 50, "^3PLAYING", 1.0F);
        CG_DrawSmallString(320 - 12 * SMALLCHAR_WIDTH, 100, "Type ^2/help ^7to get help.", 1.0F);
    }
}

/*
=================
CG_DrawOatotTimer
=================
 */
static void CG_DrawOatotTimer(void) {
    int msec, mins, seconds, tens;
    msec = cgs.levelStartTime + cgs.makingBetsTime * 60000 - cg.time;
    seconds = msec / 1000;
    mins = seconds / 60;
    tens = seconds / 10;
    seconds -= tens * 10;
    if (cgs.gameStage == MAKING_BETS) {
        CG_DrawSmallString(
            320 - 17 * SMALLCHAR_WIDTH,
            120,
            va("^3The match will be started in: ^1%i:%i%i", mins, tens, seconds),
            1.0F
        );
    }
}

/*
=================
CG_DrawBalanceBar
=================
 */
void CG_DrawBalanceBar(int left_side) {
    CG_DrawRect(left_side, 160, 640 - left_side, 125, 2, GetGameStageColor());
}

/*
=================
CG_DrawBalance
=================
 */
void CG_DrawBalance(void) {
    char* currency;
    int i, value, value_x, value_y, left_side, string_pos;
    int max_value = 0;
    int min_x = 640;
    for (i = 0; i < CURRENCIES_N; i++) {
        // Some data needed for drawing.
        value = cgs.clientinfo[cg.clientNum].balances[i].freeMoney;
        value_x = 640 - GetValueLength(value);
        currency = cgs.clientinfo[cg.clientNum].balances[i].currency;
        // Draw actual value for this currency.
        if (DictFind(currencyToBalanceY, CURRENCIES_N, currency, &value_y)) {
            CG_DrawValue(value_x, value_y, value, -1, currency);
        }
        // For Balance Bar and Balace string positions.
        if (value > max_value) {
            max_value = value;
        }
        if (value_x < min_x) {
            min_x = value_x;
        }
    }
    // Balance heading string position.
    string_pos = 640 - (GetValueLength(max_value) / 2) - 3 * SMALLCHAR_WIDTH;
    if ((string_pos + 7 * SMALLCHAR_WIDTH) > 640) {
        // Adjust if needed.
        string_pos = 640 - 8 * SMALLCHAR_WIDTH;
    }
    // The left side of the whole balance bar.
    left_side = string_pos - 5;
    if (min_x < left_side) {
        // Adjust if needed.
        left_side = min_x - 5;
    }
    // Draw balance bar and balance heading.
    CG_DrawBalanceBar(left_side);
    CG_DrawSmallStringColor(string_pos, 170, "^2Balance ", GetGameStageColor());
}

/*
=================
CG_DrawActiveBetsSumsHorses
=================
 */
static void CG_DrawActiveBetsSumsHorses(dictEntry_t* max_amounts) {
    // TODO: think what can be done with more than 2 horses.
    int red_amount, blue_amount, red_str_pos, blue_str_pos;
    DictFind(max_amounts, HORSES_N, "red", &red_amount);
    DictFind(max_amounts, HORSES_N, "blue", &blue_amount);
    red_str_pos = (GetValueLength(red_amount) / 2) - 3 * SMALLCHAR_WIDTH;
    blue_str_pos = 640 - (GetValueLength(blue_amount) / 2) - 3 * SMALLCHAR_WIDTH;
    if (red_str_pos < 0) {
        red_str_pos = 0;
    }
    if ((blue_str_pos + 7 * SMALLCHAR_WIDTH) > 640) {
        blue_str_pos = 640 - 7 * SMALLCHAR_WIDTH;
    }
    CG_DrawSmallString(blue_str_pos, 50, "^4On Blue", 1.0F);
    CG_DrawSmallString(red_str_pos, 50, "^1On Red", 1.0F);
}

/*
=================
CG_DrawActiveBetsSums
=================
 */
void CG_DrawActiveBetsSums(void) {
    int i, value, value_y, max_value_index;
    char* currency;
    char* horse;
    // Max amount for given horse.
    dictEntry_t max_amounts[HORSES_N];
    // Initialize max_amounts.
    for (i = 0; i < HORSES_N; i++) {
        max_amounts[i].value = 0;
        max_amounts[i].key = horseToFlagShaderIndex[i].key;
    }
    for (i = 0; i < CURRENCIES_N * HORSES_N; i++) {
        value = cgs.betSums[i].amount;
        currency = cgs.betSums[i].currency;
        horse = cgs.betSums[i].horse;
        // Max value for the given horse.
        max_value_index = DictFindIndex(max_amounts, HORSES_N, horse);
        if (max_value_index != -1) {
            if (value > max_amounts[max_value_index].value) {
                // Need to update max value for the given horse.
                max_amounts[max_value_index].value = value;
            }
        }
        // CURRENCIES_N - 1 because no BTC yet.
        if (DictFind(currencyToBetSumY, CURRENCIES_N - 1, currency, &value_y)) {
            // Draw value for current bet sum.
            CG_DrawValue(
                BetSumXByHorse(horse, value),
                value_y,
                value,
                -1,
                currency
            );
        }
    }
    CG_DrawActiveBetsSumsHorses(max_amounts);
}

/*
=================
CG_DrawActiveBets
=================
 */
void CG_DrawActiveBets(void) {
    int i = 0;
    int init_y = 190;
    int shift = GetMaxAmountLength();
    int max_len = 20 + shift + CHAR_WIDTH * 2 + VALUE_SPACE_LEN * 2;
    int bets_n = cgs.clientinfo[cg.clientNum].betsN;
    for (i = 0; i < bets_n; i++) {
        if (bets_n > 1) {
            CG_DrawRect(0, init_y + 40 * i - 15, max_len, 40, 2, GetGameStageColor());
        }
        CG_DrawSmallString(3, init_y + 40 * i, va("^2%d", i), 1.0F);
        CG_DrawBet(20, init_y + 40 * i, shift, cgs.clientinfo[cg.clientNum].activeBets[i]);
    }
}

/*
=================
CG_DrawResults
=================
 */
void CG_DrawResults(int prize, int balance_change) {
    // TODO: is only implemented for OAC.
    if (balance_change > 0) {
        if (prize > 0) {
            CG_CenterPrint(va("^6Congrats! You won ^3%d OAC^6!\n^2Score prize is ^3%d OAC^6.", balance_change, prize), SCREEN_HEIGHT * 0.50, BIGCHAR_WIDTH);
        } else {
            CG_CenterPrint(va("^6Congrats! You won ^3%d OAC^6!", balance_change), SCREEN_HEIGHT * 0.50, BIGCHAR_WIDTH);
        }
    } else {
        if (prize > 0) {
            CG_CenterPrint(va("^6Maybe next time...\n^2Score prize is ^3%d OAC^6.", prize), SCREEN_HEIGHT * 0.50, BIGCHAR_WIDTH);
        } else {
            CG_CenterPrint("^6Maybe next time...", SCREEN_HEIGHT * 0.50, BIGCHAR_WIDTH);
        }
    }
}

/*
=================
CG_DrawOatotStuff
=================
 */
void CG_DrawOatotStuff(void) {
    CG_DrawGameStageInfo();
    CG_DrawOatotTimer();
    CG_DrawBalance();
    CG_DrawActiveBetsSums();
    CG_DrawActiveBets();
}
