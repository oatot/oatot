/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
===========================================================================
 */
//
// cg_scoreboard -- draw the scoreboard on top of the game screen
#include "cg_local.h"

#define SCOREBOARD_X (0)

#define SB_HEADER 86
#define SB_TOP (SB_HEADER+36)

// Where the status bar starts, so we don't overwrite it
#define SB_STATUSBAR 420

#define SB_NORMAL_HEIGHT 40
#define SB_INTER_HEIGHT 16 // interleaved height
#define SB_TIGHT_HEIGHT 10 // Compressed (for > 12 players) height.

#define SB_MAXCLIENTS_NORMAL ((SB_STATUSBAR - SB_TOP) / SB_NORMAL_HEIGHT)
#define SB_MAXCLIENTS_INTER 14
#define SB_MAXCLIENTS 24

// Normal
#define SB_BOTICON_X (SCOREBOARD_X)
#define SB_HEAD_X (SCOREBOARD_X+16)

#define SB_SCORELINE_X 34

#define MAX_NAME_LEN 15
#define MAX_KDR_LEN 8
#define MAX_DMG_LEN 13

static qboolean localClient; // true if local client has been displayed

/*
=================
CG_DrawScoreboard
=================
 */
static void CG_DrawClientScore(int y, score_t* score, float* color, float fade, int lineHeight) {
    char string[1024];
    char dmg_str[1024];
    char kdr_str[1024];
    const char* info;
    vec3_t headAngles;
    clientInfo_t* ci;
    int iconx, headx;
    if (lineHeight == SB_NORMAL_HEIGHT) {
        lineHeight = SMALLCHAR_HEIGHT;
    }
    // to detect insta
    info = CG_ConfigString(CS_SERVERINFO);
    if (score->client < 0 || score->client >= cgs.maxclients) {
        Com_Printf("Bad score->client: %i\n", score->client);
        return;
    }
    ci = &cgs.clientinfo[score->client];
    iconx = SB_BOTICON_X;
    headx = SB_HEAD_X;
    if (ci->botSkill > 0 && ci->botSkill <= 5) {
        // Clear all the stats for bots.
        // Not useful info, but overlaps because of playing too long.
        score->damageGiven = 0;
        score->damageTaken = 0;
        score->kills = 0;
        score->deaths = 0;
        score->accuracy = 0;
        score->grabs = 0;
        score->captures = 0;
        score->averageSpeed = 0;
    }
    // draw the handicap or bot skill marker (unless player has flag)
    if (cgs.enableBetting && (cgs.gameStage == FORMING_TEAMS)) {
        if (ci->botSkill > 0 && ci->botSkill <= 5) {
            if (cg_drawIcons.integer) {
                CG_DrawPic(iconx, y, 16, lineHeight, cgs.media.botSkillShaders[ ci->botSkill - 1 ]);
            }
        } else {
            if (ci->team != TEAM_SPECTATOR) {
                if (score->ready) {
                    CG_DrawPic(iconx, y, 16, lineHeight, cgs.media.readyShader);
                } else {
                    CG_DrawPic(iconx, y, 16, lineHeight, cgs.media.notReadyShader);
                }
            }
        }
    } else {
        if (ci->powerups & (1 << PW_NEUTRALFLAG)) {
            CG_DrawFlagModel(iconx, y, 16, lineHeight, TEAM_FREE, qfalse);
        } else if (ci->powerups & (1 << PW_REDFLAG)) {
            CG_DrawFlagModel(iconx, y, 16, lineHeight, TEAM_RED, qfalse);
        } else if (ci->powerups & (1 << PW_BLUEFLAG)) {
            CG_DrawFlagModel(iconx, y, 16, lineHeight, TEAM_BLUE, qfalse);
        } else {
            if (ci->botSkill > 0 && ci->botSkill <= 5) {
                if (cg_drawIcons.integer) {
                    CG_DrawPic(iconx, y, 16, lineHeight, cgs.media.botSkillShaders[ ci->botSkill - 1 ]);
                }
            } else {
                // Fav weapon.
                if (score->favWeapon > 0 && score->favWeapon < WP_NUM_WEAPONS) {
                    CG_DrawPic(iconx, y, 16, lineHeight, cg_weapons[score->favWeapon].weaponIcon);
                }
            }
        }
        // draw the wins / losses
        if (cgs.gametype == GT_TOURNAMENT) {
            Com_sprintf(string, sizeof(string), "%i/%i", ci->wins, ci->losses);
            if (ci->handicap < 100 && !ci->botSkill) {
                CG_DrawStringExt(
                    iconx,
                    y + SMALLCHAR_HEIGHT / 2,
                    string,
                    color,
                    qtrue,
                    qtrue,
                    BIGCHAR_WIDTH,
                    lineHeight,
                    0
                );
            } else {
                CG_DrawStringExt(iconx, y, string, color, qtrue, qtrue, BIGCHAR_WIDTH, lineHeight, 0);
            }
        }
    }
    // draw the face
    VectorClear(headAngles);
    headAngles[YAW] = 180;
    CG_DrawHead(headx, y, 16, lineHeight, score->client, headAngles);
#ifdef MISSIONPACK
    // draw the team task
    if (ci->teamTask != TEAMTASK_NONE) {
        if (ci->isDead) {
            CG_DrawPic(headx + 48, y, 16, lineHeight, cgs.media.deathShader);
        } else if (ci->teamTask == TEAMTASK_OFFENSE) {
            CG_DrawPic(headx + 48, y, 16, lineHeight, cgs.media.assaultShader);
        } else if (ci->teamTask == TEAMTASK_DEFENSE) {
            CG_DrawPic(headx + 48, y, 16, lineHeight, cgs.media.defendShader);
        }
    }
#endif
    // highlight your position
    if (score->client == cg.snap->ps.clientNum) {
        float hcolor[4];
        int rank;
        localClient = qtrue;
        if ((cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR) ||
                ((cgs.gametype >= GT_TEAM) &&
                 (cgs.ffa_gt != 1))) {
            // Sago: I think this means that it doesn't matter if two players are tied in team game - only team score counts
            rank = -1;
        } else {
            rank = cg.snap->ps.persistant[PERS_RANK] & ~RANK_TIED_FLAG;
        }
        if (rank == 0) {
            hcolor[0] = 0;
            hcolor[1] = 0;
            hcolor[2] = 0.7f;
        } else if (rank == 1) {
            hcolor[0] = 0.7f;
            hcolor[1] = 0;
            hcolor[2] = 0;
        } else if (rank == 2) {
            hcolor[0] = 0.7f;
            hcolor[1] = 0.7f;
            hcolor[2] = 0;
        } else {
            hcolor[0] = 0.7f;
            hcolor[1] = 0.7f;
            hcolor[2] = 0.7f;
        }
        hcolor[3] = fade * 0.7;
        CG_FillRect(SB_SCORELINE_X, y,
                    640 - SB_SCORELINE_X - SMALLCHAR_WIDTH, lineHeight + 1, hcolor);
    }
    if (score->ping == -1) {
        CG_DrawStringSize(SB_SCORELINE_X, y, SMALLCHAR_WIDTH, lineHeight, " connecting", fade);
        CG_DrawStringCustom(SB_SCORELINE_X + 20 * SMALLCHAR_WIDTH, y, SMALLCHAR_WIDTH, lineHeight, va("%s", ci->name), fade, MAX_NAME_LEN);
    } else if (ci->team == TEAM_SPECTATOR) {
        CG_DrawStringSize(SB_SCORELINE_X, y, SMALLCHAR_WIDTH, lineHeight, " SPECT", fade);
        CG_DrawStringSize(SB_SCORELINE_X + 8 * SMALLCHAR_WIDTH, y, SMALLCHAR_WIDTH, lineHeight, va("%i", score->ping), fade);
        CG_DrawStringSize(SB_SCORELINE_X + 13 * SMALLCHAR_WIDTH, y, SMALLCHAR_WIDTH, lineHeight, va("%i", score->time), fade);
        CG_DrawStringCustom(SB_SCORELINE_X + 20 * SMALLCHAR_WIDTH, y, SMALLCHAR_WIDTH, lineHeight, va("%s", ci->name), fade, MAX_NAME_LEN);
    } else {
        Com_sprintf(kdr_str, sizeof(kdr_str), "^2%i^4/^1%i", score->kills, score->deaths);
        Com_sprintf(dmg_str, sizeof(dmg_str), "^2%.1fK^4/^1%.1fK", score->damageGiven / 1000.0, score->damageTaken / 1000.0);
        CG_DrawStringSize(SB_SCORELINE_X, y, SMALLCHAR_WIDTH, lineHeight, va(" ^5%i", score->score), fade);
        CG_DrawStringSize(SB_SCORELINE_X + 8 * SMALLCHAR_WIDTH, y, SMALLCHAR_WIDTH, lineHeight, va("%i", score->ping), fade);
        CG_DrawStringSize(SB_SCORELINE_X + 13 * SMALLCHAR_WIDTH, y, SMALLCHAR_WIDTH, lineHeight, va("%i", score->time), fade);
        CG_DrawStringCustom(SB_SCORELINE_X + 20 * SMALLCHAR_WIDTH, y, SMALLCHAR_WIDTH, lineHeight, va("%s", ci->name), fade, MAX_NAME_LEN);
        CG_DrawStringSize(SB_SCORELINE_X + 36 * SMALLCHAR_WIDTH, y, SMALLCHAR_WIDTH, lineHeight, va("^6%i%%", score->accuracy), fade);
        CG_DrawStringCustom(SB_SCORELINE_X + 41 * SMALLCHAR_WIDTH, y, SMALLCHAR_WIDTH, lineHeight, va("%s", kdr_str), fade, MAX_KDR_LEN);
        if (!atoi(Info_ValueForKey(info, "g_instantgib"))) {
            CG_DrawStringCustom(SB_SCORELINE_X + 50 * SMALLCHAR_WIDTH, y, SMALLCHAR_WIDTH, lineHeight, va("%s", dmg_str), fade, MAX_DMG_LEN);
            CG_DrawStringSize(SB_SCORELINE_X + 64 * SMALLCHAR_WIDTH, y, SMALLCHAR_WIDTH, lineHeight, va("^3%i^1/^7%i", score->captures, score->grabs), fade);
            CG_DrawStringSize(SB_SCORELINE_X + 69 * SMALLCHAR_WIDTH, y, SMALLCHAR_WIDTH, lineHeight, va("^6%i", score->averageSpeed), fade);
        } else {
            CG_DrawStringSize(SB_SCORELINE_X + 51 * SMALLCHAR_WIDTH, y, SMALLCHAR_WIDTH, lineHeight, va("^3%i^1/^7%i", score->captures, score->grabs), fade);
            CG_DrawStringSize(SB_SCORELINE_X + 56 * SMALLCHAR_WIDTH, y, SMALLCHAR_WIDTH, lineHeight, va("^6%i", score->averageSpeed), fade);
        }
    }
    // add the "ready" marker for intermission exiting
    if (cg.snap->ps.stats[ STAT_CLIENTS_READY ] & (1 << score->client)) {
        CG_DrawBigStringColor(iconx, y, "READY", color);
    } else if (cgs.gametype == GT_LMS) {
        CG_DrawBigStringColor(iconx - 50, y, va("*%i*", ci->isDead), color);
    } else if (ci->isDead) {
        CG_DrawBigStringColor(iconx - 60, y, "DEAD", color);
    }
}

void CG_DrawDate(int x, int y) {
    CG_DrawSmallString(x, y, va("^3%s", cgs.timestamp), 1.0F);
}

void CG_DrawScoreboardEffect(int x, int y, qhandle_t scoreboard_effect0, qhandle_t scoreboard_effect1) {
    int shift = 24;
    int type = 0;
    for (; x < 640; x += shift) {
        if (type == 0) {
            CG_DrawPic(x, y, 24, 24, scoreboard_effect0);
            type += 1;
        } else {
            CG_DrawPic(x, y, 24, 24, scoreboard_effect1);
            type -= 1;
        }
    }
}

void CG_DrawScoreboardEffects(int x, int y) {
    if (cg_scoreboardEffects.integer) {
        if (cg_scoreboardAggressive.integer) {
            CG_DrawScoreboardEffect(x, y, cgs.media.fireShader, cgs.media.fireShader);
        } else {
            // Draw season.
            switch (cg_scoreboardSeason.integer) {
                case 1:
                    CG_DrawScoreboardEffect(x, y, cgs.media.winterShader0, cgs.media.winterShader1);
                    break;
                case 2:
                    CG_DrawScoreboardEffect(x, y, cgs.media.springShader0, cgs.media.springShader1);
                    break;
                case 3:
                    CG_DrawScoreboardEffect(x, y, cgs.media.summerShader0, cgs.media.summerShader1);
                    break;
                case 4:
                    CG_DrawScoreboardEffect(x, y, cgs.media.autumnShader0, cgs.media.autumnShader1);
                    break;
            }
        }
    }
}

/*
=================
CG_TeamScoreboard
=================
 */
static int CG_TeamScoreboard(int y, team_t team, float fade, int maxClients, int lineHeight) {
    int i;
    score_t* score;
    float color[4];
    int count;
    clientInfo_t* ci;
    color[0] = color[1] = color[2] = 1.0;
    color[3] = fade;
    count = 0;
    for (i = 0; i < cg.numScores && count < maxClients; i++) {
        score = &cg.scores[i];
        ci = &cgs.clientinfo[ score->client ];
        if (team != ci->team) {
            continue;
        }
        CG_DrawClientScore(y + lineHeight * count, score, color, fade, lineHeight);
        count++;
    }
    return count;
}

/*
=================
CG_DrawScoreboard

Draw the normal in-game scoreboard
=================
 */
qboolean CG_DrawOldScoreboard(void) {
    int x, y, w, i, n1, n2;
    float fade;
    float* fadeColor;
    char* s;
    const char* info;
    int maxClients;
    int lineHeight;
    int topBorderSize, bottomBorderSize;
    // to detect insta
    info = CG_ConfigString(CS_SERVERINFO);
    // don't draw amuthing if the menu or console is up
    if (cg_paused.integer) {
        cg.deferredPlayerLoading = 0;
        return qfalse;
    }
    if (cgs.gametype == GT_SINGLE_PLAYER && cg.predictedPlayerState.pm_type == PM_INTERMISSION) {
        cg.deferredPlayerLoading = 0;
        return qfalse;
    }
    // don't draw scoreboard during death while warmup up
    if (cg.warmup && !cg.showScores) {
        return qfalse;
    }
    if (cg.showScores || cg.predictedPlayerState.pm_type == PM_DEAD ||
            cg.predictedPlayerState.pm_type == PM_INTERMISSION) {
        fade = 1.0;
        fadeColor = colorWhite;
    } else {
        fadeColor = CG_FadeColor(cg.scoreFadeTime, FADE_TIME);
        if (!fadeColor) {
            // next time scoreboard comes up, don't print killer
            cg.deferredPlayerLoading = 0;
            cg.killerName[0] = 0;
            return qfalse;
        }
        fade = *fadeColor;
    }
    // fragged by ... line
    if (cg.killerName[0]) {
        s = va("Fragged by %s", cg.killerName);
        w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;
        x = (SCREEN_WIDTH - w) / 2;
        y = 40;
        CG_DrawBigString(x, y, s, fade);
    }
    // current rank
    if (cgs.gametype < GT_TEAM || cgs.ffa_gt == 1) {
        if (cg.snap->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR) {
            s = va("%s place with %i",
                   CG_PlaceString(cg.snap->ps.persistant[PERS_RANK] + 1),
                   cg.snap->ps.persistant[PERS_SCORE]);
            w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;
            x = (SCREEN_WIDTH - w) / 2;
            y = 60;
            CG_DrawBigString(x, y, s, fade);
        }
    } else {
        if (cg.teamScores[0] == cg.teamScores[1]) {
            s = va("Teams are tied at %i", cg.teamScores[0]);
        } else if (cg.teamScores[0] >= cg.teamScores[1]) {
            s = va("Red leads %i to %i", cg.teamScores[0], cg.teamScores[1]);
        } else {
            s = va("Blue leads %i to %i", cg.teamScores[1], cg.teamScores[0]);
        }
        w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;
        x = (SCREEN_WIDTH - w) / 2;
        y = 60;
        CG_DrawBigString(x, y, s, fade);
    }
    // scoreboard
    y = SB_HEADER;
    if (!atoi(Info_ValueForKey(info, "g_instantgib"))) {
        CG_DrawSmallString(SB_SCORELINE_X, y, " ^1Score  ^1Ping ^1Time   ^1Name            ^1Acc  ^1K/D      ^1Dmg           ^1Caps Speed", 1.0F);
        CG_DrawScoreboardEffects(0, y + 14);
    } else {
        CG_DrawSmallString(SB_SCORELINE_X, y, " ^1Score  ^1Ping ^1Time   ^1Name            ^1Acc  ^1K/D      ^1Caps Speed", 1.0F);
        CG_DrawScoreboardEffects(0, y + 14);
    }
    y = SB_TOP;
    if (cg.numScores > SB_MAXCLIENTS_INTER) {
        // If there are more than SB_MAXCLIENTS_INTER, use the compressed scores.
        maxClients = SB_MAXCLIENTS;
        lineHeight = SB_TIGHT_HEIGHT;
        topBorderSize = 8;
        bottomBorderSize = 16;
    } else if (cg.numScores > SB_MAXCLIENTS_NORMAL) {
        // If there are more than SB_MAXCLIENTS_NORMAL, use the interleaved scores.
        maxClients = SB_MAXCLIENTS_INTER;
        lineHeight = SB_INTER_HEIGHT;
        topBorderSize = 8;
        bottomBorderSize = 16;
    } else {
        maxClients = SB_MAXCLIENTS_NORMAL;
        lineHeight = SB_NORMAL_HEIGHT;
        topBorderSize = 16;
        bottomBorderSize = 16;
    }
    localClient = qfalse;
    if (cgs.gametype >= GT_TEAM && cgs.ffa_gt != 1) {
        //
        // teamplay scoreboard
        //
        y += lineHeight / 2;
        if (cg.teamScores[0] >= cg.teamScores[1]) {
            n1 = CG_TeamScoreboard(y, TEAM_RED, fade, maxClients, lineHeight);
            CG_DrawTeamBackground(0, y - topBorderSize, 640, n1 * lineHeight + bottomBorderSize, 0.33f, TEAM_RED);
            y += (n1 * lineHeight) + BIGCHAR_HEIGHT;
            maxClients -= n1;
            n2 = CG_TeamScoreboard(y, TEAM_BLUE, fade, maxClients, lineHeight);
            CG_DrawTeamBackground(0, y - topBorderSize, 640, n2 * lineHeight + bottomBorderSize, 0.33f, TEAM_BLUE);
            y += (n2 * lineHeight) + BIGCHAR_HEIGHT;
            maxClients -= n2;
        } else {
            n1 = CG_TeamScoreboard(y, TEAM_BLUE, fade, maxClients, lineHeight);
            CG_DrawTeamBackground(0, y - topBorderSize, 640, n1 * lineHeight + bottomBorderSize, 0.33f, TEAM_BLUE);
            y += (n1 * lineHeight) + BIGCHAR_HEIGHT;
            maxClients -= n1;
            n2 = CG_TeamScoreboard(y, TEAM_RED, fade, maxClients, lineHeight);
            CG_DrawTeamBackground(0, y - topBorderSize, 640, n2 * lineHeight + bottomBorderSize, 0.33f, TEAM_RED);
            y += (n2 * lineHeight) + BIGCHAR_HEIGHT;
            maxClients -= n2;
        }
        n1 = CG_TeamScoreboard(y, TEAM_SPECTATOR, fade, maxClients, lineHeight);
        y += (n1 * lineHeight) + BIGCHAR_HEIGHT;
        CG_DrawScoreboardEffects(0, y - BIGCHAR_WIDTH + 6);
        CG_DrawDate(400, y - BIGCHAR_HEIGHT + 40);
    } else {
        //
        // free for all scoreboard
        //
        n1 = CG_TeamScoreboard(y, TEAM_FREE, fade, maxClients, lineHeight);
        y += (n1 * lineHeight) + BIGCHAR_HEIGHT;
        n2 = CG_TeamScoreboard(y, TEAM_SPECTATOR, fade, maxClients - n1, lineHeight);
        y += (n2 * lineHeight) + BIGCHAR_HEIGHT;
    }
    if (!localClient) {
        // draw local client at the bottom
        for (i = 0; i < cg.numScores; i++) {
            if (cg.scores[i].client == cg.snap->ps.clientNum) {
                CG_DrawClientScore(y, &cg.scores[i], fadeColor, fade, lineHeight);
                break;
            }
        }
    }
    // load any models that have been deferred
    if (++cg.deferredPlayerLoading > 10) {
        CG_LoadDeferredPlayers();
    }
    return qtrue;
}

//================================================================================

/*
================
CG_CenterGiantLine
================
 */
static void CG_CenterGiantLine(float y, const char* string) {
    float x;
    vec4_t color;
    color[0] = 1;
    color[1] = 1;
    color[2] = 1;
    color[3] = 1;
    x = 0.5 * (640 - GIANT_WIDTH * CG_DrawStrlen(string));
    CG_DrawStringExt(x, y, string, color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0);
}

/*
=================
CG_DrawTourneyScoreboard

Draw the oversize scoreboard for tournements
=================
 */
void CG_DrawOldTourneyScoreboard(void) {
    const char* s;
    vec4_t color;
    int min, tens, ones;
    clientInfo_t* ci;
    int y;
    int i;
    // request more scores regularly
    if (cg.scoresRequestTime + 2000 < cg.time) {
        cg.scoresRequestTime = cg.time;
        trap_SendClientCommand("score");
    }
    // draw the dialog background
    color[0] = color[1] = color[2] = 0;
    color[3] = 1;
    CG_FillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, color);
    color[0] = 1;
    color[1] = 1;
    color[2] = 1;
    color[3] = 1;
    // print the mesage of the day
    s = CG_ConfigString(CS_MOTD);
    if (!s[0]) {
        s = "Scoreboard";
    }
    // print optional title
    CG_CenterGiantLine(8, s);
    // print server time
    ones = cg.time / 1000;
    min = ones / 60;
    ones %= 60;
    tens = ones / 10;
    ones %= 10;
    s = va("%i:%i%i", min, tens, ones);
    CG_CenterGiantLine(64, s);
    // print the two scores
    y = 160;
    if (cgs.gametype >= GT_TEAM && cgs.ffa_gt != 1) {
        //
        // teamplay scoreboard
        //
        CG_DrawStringExt(8, y, "Red Team", color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0);
        s = va("%i", cg.teamScores[0]);
        CG_DrawStringExt(632 - GIANT_WIDTH * strlen(s), y, s, color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0);
        y += 64;
        CG_DrawStringExt(8, y, "Blue Team", color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0);
        s = va("%i", cg.teamScores[1]);
        CG_DrawStringExt(632 - GIANT_WIDTH * strlen(s), y, s, color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0);
    } else {
        //
        // free for all scoreboard
        //
        for (i = 0; i < MAX_CLIENTS; i++) {
            ci = &cgs.clientinfo[i];
            if (!ci->infoValid) {
                continue;
            }
            if (ci->team != TEAM_FREE) {
                continue;
            }
            CG_DrawStringExt(8, y, ci->name, color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0);
            s = va("%i", ci->score);
            CG_DrawStringExt(632 - GIANT_WIDTH * strlen(s), y, s, color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0);
            y += 64;
        }
    }
}

