#include <dlfcn.h>

#include "client.h"
#include "g_local.h"

// g_oatot.c - main oatot functionality is implemented here.

//=====================================

// Call certain RPCs needed.
// (asl backend to give info).

/*
==================
G_GetBalance
==================
*/
int G_GetBalance(gentity_t* ent, balance_t* balances) {
    gclient_t* client = ent->client;
    if (GOaIsNew(client->pers.guid)) {
        return -1;
    } else {
        return GOaMyBalance(client->pers.guid, balances);
    }
}

/*
==================
G_GetActiveBets
==================
*/
int G_GetActiveBets(gentity_t* ent, bet_t* bets) {
    gclient_t* client = ent->client;
    int i = 0;
    int bets_n = 0;
    if (GOaIsNew(client->pers.guid)) {
        return bets_n;
    }
    bets_n = GOaMyActiveBets(client->pers.guid, bets);
    if (bets_n != client->sess.activeBetsNumber) {
        return -1;
    } else if (bets_n < 0 || bets_n > MAX_ACTIVE_BETS_NUMBER) {
        return -1;
    }
    for (i = 0; i < bets_n; i++) {
        client->pers.activeBetsIds[i] = bets[i].betID;
    }
    return bets_n;
}

/*
==================
G_GetCurrencyBalance
==================
*/
qboolean G_GetCurrencyBalance(gentity_t* ent, const char* currency, balance_t* res) {
    int i, balances_n;
    balance_t balances[CURRENCIES_N];
    balances_n = G_GetBalance(ent, balances);
    for (i = 0; i < balances_n; i++) {
        if (!strcmp(currency, balances[i].currency)) {
            *res = balances[i];
            return qtrue;
        }
    }
    return qfalse;
}

//=====================================

// Draw updates (send commands to cgame).

/*
==================
G_UpdateBalance
==================
*/
void G_UpdateBalance(gentity_t* ent) {
    int i;
    balance_t balances[CURRENCIES_N];
    int balances_n = G_GetBalance(ent, balances);
    char cmd_str[MAX_STRING_TOKENS];
    cmd_str[0] = 0;
    strcat(cmd_str, va("updateBalance %d ", balances_n));
    for (i = 0; i < balances_n; i++) {
        strcat(cmd_str, va("%d %d %s ", balances[i].freeMoney, balances[i].moneyOnBets, balances[i].currency));
    }
    trap_SendServerCommand(ent - g_entities, cmd_str);
}

/*
==================
G_UpdateActiveBets
==================
*/
void G_UpdateActiveBets(gentity_t* ent) {
    int i;
    char cmd_str[MAX_STRING_TOKENS];
    bet_t bets[MAX_ACTIVE_BETS_NUMBER];
    int n_bets = G_GetActiveBets(ent, bets);
    cmd_str[0] = 0;
    strcat(cmd_str, va("updateActiveBets \%d ", n_bets));
    for (i = 0; i < n_bets; i++) {
        strcat(cmd_str, va("%s %s %d %d ", bets[i].horse, bets[i].currency, bets[i].amount, bets[i].betID));
    }
    strcat(cmd_str, "\"");
    trap_SendServerCommand(ent - g_entities, cmd_str);
}

/*
==================
G_UpdateActiveBetsSums
==================
*/
void G_UpdateActiveBetsSums(gentity_t* ent) {
    int i;
    char cmd_str[MAX_STRING_TOKENS];
    cmd_str[0] = 0;
    betSum_t betSums[HORSES_N * CURRENCIES_N];
    int sums_n = GOaActiveBetsSums(betSums);
    strcat(cmd_str, va("updateActiveBetsSums %d ", sums_n));
    for (i = 0; i < sums_n; i++) {
        strcat(cmd_str, va("%d %s %s ", betSums[i].amount, betSums[i].currency, betSums[i].horse));
    }
    if (!ent) {
        trap_SendServerCommand(-1, cmd_str);
    } else {
        trap_SendServerCommand(ent - g_entities, cmd_str);
    }
}

//=====================================

// Utility.

/*
==================
G_ReadAndPrintFile
==================
*/
void G_ReadAndPrintFile(gentity_t* ent, fileHandle_t file, int len) {
    char text[MAX_ARENAS_TEXT];
    if (file) {
        trap_FS_Read(&text, len, file);
        text[len] = '\0';
        trap_SendServerCommand(ent - g_entities, va("print \"%s\"", text));
        trap_FS_FCloseFile(file);
    }
}

//=====================================

// Utility functions for Cmds.

void printConsoleMessage(gentity_t* ent, const char* message) {
    trap_SendServerCommand(ent - g_entities, "print \"^3============\n\"");
    trap_SendServerCommand(ent - g_entities, message);
    trap_SendServerCommand(ent - g_entities, "print \"^3============\n\"");
}

void StrToUpper(char* str) {
    int i;
    for (i = 0; i < strlen(str); i++) {
        str[i] = toupper(str[i]);
    }
}

void printCurrencySummary(gentity_t* ent, currencySummary_t summary) {
    char summary_str[MAX_STRING_TOKENS];
    char total_bet_str[MAX_STRING_TOKENS];
    char total_prize_str[MAX_STRING_TOKENS];
    char total_lost_str[MAX_STRING_TOKENS];
    char bets_won_str[MAX_STRING_TOKENS];
    char bets_lost_str[MAX_STRING_TOKENS];
    summary_str[0] = 0;
    strcat(summary_str, "print \"");
    strcat(summary_str, "^6Your ^3");
    strcat(summary_str, summary.currency);
    strcat(summary_str, " ^6summary:\n");
    Q_snprintf(total_bet_str, MAX_STRING_TOKENS, "^6Total bet: %d %s\n", summary.totalBet, summary.currency);
    strcat(summary_str, total_bet_str);
    Q_snprintf(total_prize_str, MAX_STRING_TOKENS, "^2Total prize: %d %s\n", summary.totalPrize, summary.currency);
    strcat(summary_str, total_prize_str);
    Q_snprintf(total_lost_str, MAX_STRING_TOKENS, "^1Total lost: %d %s\n", summary.totalLost, summary.currency);
    strcat(summary_str, total_lost_str);
    Q_snprintf(bets_won_str, MAX_STRING_TOKENS, "^2Bets won: %d\n", summary.betsWon);
    strcat(summary_str, bets_won_str);
    Q_snprintf(bets_lost_str, MAX_STRING_TOKENS, "^1Bets lost: %d\n", summary.betsLost);
    strcat(summary_str, bets_lost_str);
    strcat(summary_str, "\n\"");
    printConsoleMessage(ent, summary_str);
}

//=====================================

// Console Commands.

/*
==================
Cmd_Bet_f
==================
*/
void Cmd_Bet_f(gentity_t* ent) {
    int money, max_money = 0;
    char arg1[MAX_STRING_TOKENS];
    char arg2[MAX_STRING_TOKENS];
    char arg3[MAX_STRING_TOKENS];
    balance_t balance;
    gclient_t* client = ent->client;
    if (g_gameStage.integer != MAKING_BETS) {
        trap_SendServerCommand(ent - g_entities, "print \"^1Betting not allowed now.\n\"");
        return;
    }
    if (client) {
        int bets_n = client->sess.activeBetsNumber;
        if (bets_n < MAX_ACTIVE_BETS_NUMBER) {
            trap_Argv(1, arg1, sizeof(arg1));
            trap_Argv(2, arg2, sizeof(arg2));
            trap_Argv(3, arg3, sizeof(arg3));
            // --> "red", "blue", ...
            Q_StrToLower(arg1);
            // --> "OAC", "BTC", ...
            StrToUpper(arg3);
            // Check arguments.
            if (!SearchStr(horses, HORSES_N, arg1)) {
                trap_SendServerCommand(ent - g_entities, "print \"^1Invalid horse.\n\"");
                return;
            }
            if (!SearchStr(currencies, CURRENCIES_N, arg3)) {
                trap_SendServerCommand(ent - g_entities, "print \"^1Invalid currency.\n\"");
                return;
            }
            money = atoi(arg2);
            if (G_GetCurrencyBalance(ent, arg3, &balance)) {
                max_money = balance.freeMoney;
            }
            if (money <= 0 || money > max_money) {
                trap_SendServerCommand(ent - g_entities, "print \"^1Insufficient amount of money.\n\"");
                return;
            }
            bet_t bet;
            memcpy(bet.horse, arg1, sizeof(arg1));
            memcpy(bet.currency, arg3, sizeof(arg3));
            bet.amount = money;
            GOaMyBet(client->pers.guid, bet);
            ent->client->sess.activeBetsNumber += 1;
            G_UpdateBalance(ent);
            G_UpdateActiveBets(ent);
            G_UpdateActiveBetsSums(0);
            trap_SendServerCommand(ent - g_entities, "print \"^2Your bet is made.\n\"");
        } else {
            trap_SendServerCommand(ent - g_entities, "print \"^1You can't make so many bets, sorry!\n\"");
        }
    } else {
        trap_SendServerCommand(ent - g_entities, "print \"^1You aren't a client!\n\"");
    }
}

/*
==================
Cmd_Unbet_f
==================
*/
void Cmd_Unbet_f(gentity_t* ent) {
    int bet_id;
    char arg1[MAX_STRING_TOKENS];
    gclient_t* client = ent->client;
    if (g_gameStage.integer != MAKING_BETS) {
        trap_SendServerCommand(ent - g_entities, "print \"^1You can't unbet anything now.\n\"");
        return;
    }
    if (client) {
        // Check argument.
        trap_Argv(1, arg1, sizeof(arg1));
        if (atoi(arg1) < 0 || atoi(arg1) >= client->sess.activeBetsNumber) {
            trap_SendServerCommand(ent - g_entities, "print \"^1Invalid bet ID.\n\"");
            return;
        }
        bet_id = client->pers.activeBetsIds[atoi(arg1)];
        GOaDiscardBet(client->pers.guid, bet_id);
        ent->client->sess.activeBetsNumber -= 1;
        G_UpdateBalance(ent);
        G_UpdateActiveBets(ent);
        G_UpdateActiveBetsSums(0);
        trap_SendServerCommand(ent - g_entities, "print \"^2Bet was discarded.\n\"");
    } else {
        trap_SendServerCommand(ent - g_entities, "print \"^1You aren't a client!\n\"");
    }
}

/*
==================
Cmd_PastBets_f
get last BETS_NUMBER_IN_HISTORY_PAGE bets,
specifying "elder" as arg shows the page before.
==================
*/
void Cmd_PastBets_f(gentity_t* ent) {
    int i, bets_n;
    char bet_str[MAX_STRING_TOKENS];
    fullbet_t past_bets[BETS_NUMBER_IN_HISTORY_PAGE];
    gclient_t* client = ent->client;
    if (client) {
        if (trap_Argc() == 1 || !client->pers.nextPageUsed) {
            bets_n = GOaMyPastBets(client->pers.guid, "", client->pers.nextPage, past_bets);
            client->pers.nextPageUsed = qtrue;
        } else {
            bets_n = GOaMyPastBets(client->pers.guid, client->pers.nextPage, client->pers.nextPage, past_bets);
        }
        trap_SendServerCommand(ent - g_entities, "print \"^6Bets list:\n\"");
        for (i = 0; i < bets_n; i++) {
            fullbet_t bet = past_bets[i];
            bet_str[0] = 0;
            strcat(bet_str, "print \"");
            if (!strcmp(bet.openBet.horse, "red")) {
                strcat(bet_str, "^6*^7On ^1Red^6*^7 ");
            } else {
                strcat(bet_str, "^6*^7On ^5Blue^6*^7 ");
            }
            strcat(bet_str, ((bet.prize > 0) ? "^6Result: ^2Win^7 " : "^1Defeat^7 "));
            strcat(bet_str, va("^6Amount:^7 %d ^3%s^6 ", bet.openBet.amount, bet.openBet.currency));
            strcat(bet_str, va("^6Prize:^7 %d ^3%s^6  ", bet.prize, bet.openBet.currency));
            // TODO: implement time storage in backend.
            //strcat( bet_str, bet.openBet.openTime );
            strcat(bet_str, "\n\"");
            printConsoleMessage(ent, bet_str);
        }
    } else {
        trap_SendServerCommand(ent - g_entities, "print \"^1You aren't a client!\n\"");
    }
}

/*
==================
Cmd_BetsSummary_f
==================
*/
void Cmd_BetsSummary_f(gentity_t* ent) {
    currencySummary_t summaries[CURRENCIES_N];
    int summaries_n, i;
    gclient_t* client = ent->client;
    if (client) {
        summaries_n = GOaMyBetsSummary(client->pers.guid, summaries);
        for (i = 0; i < summaries_n; i++) {
            printCurrencySummary(ent, summaries[i]);
        }
    } else {
        trap_SendServerCommand(ent - g_entities, "print \"^1You aren't a client!\n\"");
    }
}

/*
==================
Cmd_Ready_f
==================
*/
void Cmd_Ready_f(gentity_t* ent) {
    int new_val;
    char new_val_str[MAX_CVAR_VALUE_STRING];
    gclient_t* client = ent->client;
    if (g_gameStage.integer == MAKING_BETS) {
        trap_SendServerCommand(ent - g_entities, "print \"^2You can make bets now.\n\"");
        return;
    }
    if (g_gameStage.integer == PLAYING) {
        trap_SendServerCommand(ent - g_entities, "print \"^1Too slow, they are already playing.\n\"");
        return;
    }
    if (client) {
        if (client->sess.sessionTeam == TEAM_SPECTATOR) {
            trap_SendServerCommand(ent - g_entities, "print \"^1Specs can't ^2/ready.\n\"");
            return;
        } else if (!client->pers.ready) {
            client->pers.ready = qtrue;
            new_val = g_readyN.integer + 1;
            Q_snprintf(new_val_str, MAX_CVAR_VALUE_STRING, "%d", new_val);
            trap_Cvar_Set("g_readyN", new_val_str);
            G_UpdateCvars();
            trap_SendServerCommand(-1, va("cp \"%s" S_COLOR_GREEN " is ready.\n\"",
                                          client->pers.netname));
            trap_SendServerCommand(ent - g_entities, "cp \"^2You are ready.\n\"");
            if (checkForRestart()) {
                trap_Cvar_Set("g_readyToBet", "1");
                trap_SendConsoleCommand(EXEC_APPEND, "map_restart\n");
            }
        } else if (client->pers.ready) {
            client->pers.ready = qfalse;
            new_val = g_readyN.integer - 1;
            Q_snprintf(new_val_str, MAX_CVAR_VALUE_STRING, "%d", new_val);
            trap_Cvar_Set("g_readyN", new_val_str);
            G_UpdateCvars();
            trap_SendServerCommand(-1, va("cp \"%s" S_COLOR_RED " is not ready.\n\"",
                                          client->pers.netname));
            trap_SendServerCommand(ent - g_entities, "cp \"^1You are not ready.\n\"");
        }
    } else {
        trap_SendServerCommand(ent - g_entities, "print \"^1You aren't a client!\n\"");
    }
}

/*
==================
Cmd_Help_f
==================
*/
void Cmd_Help_f(gentity_t* ent) {
    int len, len_common;
    fileHandle_t file0, file1;
    gclient_t* client = ent->client;
    if (client) {
        len_common = trap_FS_FOpenFile("texts/help_message_common.txt", &file0, FS_READ);
        if (g_gameStage.integer == FORMING_TEAMS) {
            len = trap_FS_FOpenFile("texts/help_message_forming_teams.txt", &file1, FS_READ);
        } else if (g_gameStage.integer == MAKING_BETS) {
            len = trap_FS_FOpenFile("texts/help_message_making_bets.txt", &file1, FS_READ);
        } else {
            len = trap_FS_FOpenFile("texts/help_message_playing.txt", &file1, FS_READ);
        }
        G_ReadAndPrintFile(ent, file1, len);
        G_ReadAndPrintFile(ent, file0, len_common);
    } else {
        trap_SendServerCommand(ent - g_entities, "print \"^1You aren't a client!\n\"");
    }
}

/*
==================
Cmd_ShareBalance_f
==================
*/
void Cmd_ShareBalance_f(gentity_t* ent) {
    char arg1[MAX_STRING_TOKENS];
    char balance_str[MAX_STRING_TOKENS];
    balance_str[0] = 0;
    gclient_t* client = ent->client;
    balance_t balance;
    balance_t balances[CURRENCIES_N];
    int i, amount, balances_n;
    if (client) {
        if (trap_Argc() == 1) {
            strcat(balance_str, va("^5%s ^5has ", client->pers.netname));
            balances_n = G_GetBalance(ent, balances);
            for (i = 0; i < balances_n; i++) {
                if (i == balances_n - 1) {
                    strcat(balance_str, va("^3%d %s ", balances[i].freeMoney, balances[i].currency));
                } else {
                    strcat(balance_str, va("^3%d %s^5, ", balances[i].freeMoney, balances[i].currency));
                }
            }
            trap_SendServerCommand(-1, va("print \"%s ^6:p\n\"", balance_str));
        } else {
            trap_Argv(1, arg1, sizeof(arg1));
            // --> "OAC", "BTC", ...
            StrToUpper(arg1);
            // Check argument.
            if (!SearchStr(currencies, CURRENCIES_N, arg1)) {
                trap_SendServerCommand(ent - g_entities, "print \"^1Invalid currency.\n\"");
                return;
            }
            if (G_GetCurrencyBalance(ent, arg1, &balance)) {
                amount = balance.freeMoney;
                trap_SendServerCommand(-1, va("print \"^5%s ^5has ^3%d %s ^6:p\n\"", client->pers.netname, amount, arg1));
            }
        }
    } else {
        trap_SendServerCommand(ent - g_entities, "print \"^1You aren't a client!\n\"");
    }
}

/*
==================
Cmd_UpdateBalance_f
==================
*/
void Cmd_UpdateBalance_f(gentity_t* ent) {
    gclient_t* client = ent->client;
    if (client) {
        G_UpdateBalance(ent);
    } else {
        trap_SendServerCommand(ent - g_entities, "print \"^1You aren't a client!\n\"");
    }
}

/*
==================
Cmd_UpdateActiveBets_f
==================
*/
void Cmd_UpdateActiveBets_f(gentity_t* ent) {
    gclient_t* client = ent->client;
    if (client) {
        G_UpdateActiveBets(ent);
    } else {
        trap_SendServerCommand(ent - g_entities, "print \"^1You aren't a client!\n\"");
    }
}

/*
==================
Cmd_UpdateActiveBetsSums_f
==================
*/
void Cmd_UpdateActiveBetsSums_f(gentity_t* ent) {
    gclient_t* client = ent->client;
    if (client) {
        G_UpdateActiveBetsSums(ent);
    } else {
        trap_SendServerCommand(ent - g_entities, "print \"^1You aren't a client!\n\"");
    }
}

//=====================================

// Game stage logic functions.

// Server-wide.

// Returns true for PLAYING game stage, false for other.
// If betting is disabled, always returns true.
// This is used to figure out if scoring or match ending is needed.
qboolean isMatchTime(void) {
    if (g_enableBetting.integer) {
        if (g_gameStage.integer != PLAYING) {
            return qfalse;
        }
    }
    return qtrue;
}

// For G_UpdateGameStage().
qboolean needToUpdateGameStage(void) {
    if (g_gameStage.integer == FORMING_TEAMS) {
        if (g_readyToBet.integer) {
            return qtrue;
        }
    }
    if (g_gameStage.integer == MAKING_BETS) {
        if (g_betsMade.integer) {
            return qtrue;
        }
    }
    return qfalse;
}

// Check if the majority is now ready to bet.
qboolean checkForRestart(void) {
    if (g_gameStage.integer == FORMING_TEAMS) {
        int red_players = G_CountHumanPlayers(TEAM_RED);
        int blue_players = G_CountHumanPlayers(TEAM_BLUE);
        if ((red_players >= 1) && (blue_players >= 1)) {
            // At least 1v1 is needed to start.
            if (g_readyN.integer > ((red_players + blue_players) / 2)) {
                return qtrue;
            }
        }
    }
    return qfalse;
}

// For Go module separated shared library.
// Survive dlclose(qagame.so) initiated by openarena-server executable.
// We also link with this Go .so to use its header.
void G_LoadGoClientSo(void) {
    void* handle = dlopen("go-client/libgoclient.so", RTLD_NOW);
    if (!handle) {
        G_Printf("dlopen: %s\n", dlerror());
        return;
    }
}

// Load shared object for go-client, init Go client for gRPC.
void G_OatotInit(void) {
    // oatot: load shared object for Go client.
    G_LoadGoClientSo();
    // oatot: tell the backend that we exist and initialize Go client.
    GInitializeClient();
}

// Game stage update logic.
void G_UpdateGameStage(void) {
    int next_game_stage;
    char next_game_stage_str[MAX_CVAR_VALUE_STRING];
    // oatot game stages changing logic.
    if (g_rageQuit.integer == 1) {
        // Rage quit.
        trap_Cvar_Set("g_gameStage", "0");
        GOaChangeGameStage(FORMING_TEAMS);
    } else if (needToUpdateGameStage() || (g_gameStage.integer == PLAYING)) {
        // Normal stage change or map change.
        next_game_stage = (g_gameStage.integer + 1) % 3;
        Q_snprintf(next_game_stage_str, MAX_CVAR_VALUE_STRING, "%d", next_game_stage);
        trap_Cvar_Set("g_gameStage", next_game_stage_str);
        GOaChangeGameStage(next_game_stage);
    } else if (g_gameStage.integer == MAKING_BETS) {
        // Was callvoted.
        trap_Cvar_Set("g_gameStage", "0");
        GOaChangeGameStage(FORMING_TEAMS);
    }
    trap_Cvar_Set("g_readyN", "0");
    trap_Cvar_Set("g_rageQuit", "0");
    trap_Cvar_Set("g_betsMade", "0");
    trap_Cvar_Set("g_readyToBet", "0");
    G_UpdateCvars();
    if (g_gameStage.integer == FORMING_TEAMS) {
        // At this stage, no bets should be active,
        // but if the map was callvot'ed, it is still possible,
        // hence let's clear them.
        GOaCloseBetsByIncident();
    }
}

// End of match: close bets, score prize, show the results.

void getClientsBalances(int* money) {
    gclient_t* cl;
    balance_t balance;
    int i;
    for (i = 0; i < g_maxclients.integer; i++) {
        cl = level.clients + i;
        if (!GOaIsNew(cl->pers.guid) && (cl->pers.connected == CON_CONNECTED)) {
            if (G_GetCurrencyBalance(g_entities + i, "OAC", &balance)) {
                money[i] = balance.freeMoney;
            }
        }
    }
}

void transferPrizeMoney(int* balances_before, int* balances_after, char* winner) {
    gclient_t* cl;
    int i, score, change;
    // Amount of "prize" is equal to player score.
    for (i = 0; i < g_maxclients.integer; i++) {
        cl = level.clients + i;
        if (g_gameStage.integer == PLAYING) {
            if (!GOaIsNew(cl->pers.guid) && (cl->pers.connected == CON_CONNECTED)) {
                score = cl->ps.persistant[PERS_SCORE];
                change = balances_after[i] - balances_before[i];
                if (Q_strequal(winner, "red") && (cl->sess.sessionTeam == TEAM_RED)) {
                    if (cl->sess.sessionTeam != TEAM_SPECTATOR) {
                        GOaTransferMoney(cl->pers.guid, score, "OAC");
                        trap_SendServerCommand(i, va("showResults %d %d\n", score, change));
                    } else if (cl->sess.activeBetsNumber != 0) {
                        trap_SendServerCommand(i, va("showResults 0 %d\n", change));
                    }
                } else if (Q_strequal(winner, "blue") && (cl->sess.sessionTeam == TEAM_BLUE)) {
                    if (cl->sess.sessionTeam != TEAM_SPECTATOR) {
                        GOaTransferMoney(cl->pers.guid, score, "OAC");
                        trap_SendServerCommand(i, va("showResults %d %d\n", score, change));
                    } else if (cl->sess.activeBetsNumber != 0) {
                        trap_SendServerCommand(i, va("showResults 0 %d\n", change));
                    }
                } else if (cl->sess.activeBetsNumber != 0) {
                    trap_SendServerCommand(i, va("showResults 0 %d\n", change));
                }
            }
        }
    }
}

void endOfMatchLogic(char* winner) {
    // TODO: only OAC is supported here.
    int balances_before[MAX_GENTITIES];
    int balances_after[MAX_GENTITIES];
    getClientsBalances(balances_before);
    GOaCloseBets(winner);
    getClientsBalances(balances_after);
    transferPrizeMoney(balances_before, balances_after, winner);
}

// For MAKING_BETS stage, prints info about time left,
// calls restart when time is up.
void checkOatotStageUpdate(void) {
    int duration = level.time - level.startTime;
    if (g_gameStage.integer == MAKING_BETS) {
        if (duration > (g_makingBetsTime.integer * 60000)) {
            // Time is up.
            trap_Cvar_Set("g_betsMade", "1");
            trap_SendConsoleCommand(EXEC_APPEND, "map_restart\n");
        }
        if ((g_makingBetsTime.integer * 60000 - duration) < 30000) {
            // 30 seconds left.
            if (!level.timeWarningPrinted) {
                level.timeWarningPrinted = qtrue;
                trap_SendServerCommand(-1, "cp \"^130 seconds before the start!!!\"");
            }
        }
        if (duration > 5000) {
            // Wait 5 secs before printing info to make sure everyone is able to see it.
            if (!level.betsGreetingPrinted) {
                // Info which is printed once at the beginning.
                level.betsGreetingPrinted = qtrue;
                // min / min(s) logic.
                if (g_makingBetsTime.integer == 1) {
                    trap_SendServerCommand(-1, va("cp \"^2%d min to make bets & warm up :) \"", g_makingBetsTime.integer));
                } else {
                    trap_SendServerCommand(-1, va("cp \"^2%d mins to make bets & warm up :) \"", g_makingBetsTime.integer));
                }
            }
        }
    }
}

//======================================
// For given client.

// Is called on client disconnect:
// rage-quit check, ready count decrease.
void G_OatotClientDisconnect(gentity_t* ent, team_t team) {
    int new_val;
    char new_val_str[MAX_CVAR_VALUE_STRING];
    if (g_gameStage.integer == FORMING_TEAMS) {
        if (ent->client->pers.ready) {
            new_val = g_readyN.integer - 1;
            Q_snprintf(new_val_str, MAX_CVAR_VALUE_STRING, "%d", new_val);
            trap_Cvar_Set("g_readyN", new_val_str);
        }
    }
    G_UpdateCvars();
    if (checkForRestart()) {
        // The majority is ready now.
        trap_Cvar_Set("g_readyToBet", "1");
        trap_SendConsoleCommand(EXEC_APPEND, "map_restart\n");
    }
    if (g_gameStage.integer != FORMING_TEAMS) {
        if (team != TEAM_SPECTATOR) {
            if (!level.intermissiontime) {
                // Quitting not during the FORMING_TEAMS stage isn't allowed, auto-restart.
                trap_SendServerCommand(-1, "cp \"^1We got rage-quitter! Restart!\n\"");
                trap_Cvar_Set("g_rageQuit", "1");
                GOaCloseBetsByIncident();
                trap_SendConsoleCommand(EXEC_APPEND, "map_restart\n");
            }
        }
    }
}

// Register the client on backend.
void G_OatotRegister(char* guid) {
    if (GOaIsNew(guid)) {
        GOaRegister(guid);
    }
}

// Initialize active bets for given client.
void G_OatotInitClientActiveBets(gclient_t* client) {
    bet_t bets[MAX_ACTIVE_BETS_NUMBER];
    if (!GOaIsNew(client->pers.guid)) {
        client->sess.activeBetsNumber = GOaMyActiveBets(client->pers.guid, bets);
    } else {
        client->sess.activeBetsNumber = 0;
    }
}

// Decrease ready counter in case player gone to spec.
void G_DecreaseReadyNForSpec(gclient_t* client) {
    int new_val;
    char new_val_str[MAX_CVAR_VALUE_STRING];
    if (g_gameStage.integer == FORMING_TEAMS) {
        if (client->pers.ready) {
            client->pers.ready = qfalse;
            new_val = g_readyN.integer - 1;
            Q_snprintf(new_val_str, MAX_CVAR_VALUE_STRING, "%d", new_val);
            trap_Cvar_Set("g_readyN", new_val_str);
            G_UpdateCvars();
        }
    }
}

// False if not FORMING_TEAMS and not spectator.
qboolean G_AllowToSwitchTeam(gclient_t* client, char* s) {
    if (g_gameStage.integer != FORMING_TEAMS) {
        if (!((client->sess.sessionTeam == TEAM_SPECTATOR) && (Q_strequal(s, "spectator") || Q_strequal(s, "s")))) {
            return qfalse;
        }
    }
    return qtrue;
}
