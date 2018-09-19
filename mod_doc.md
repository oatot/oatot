*New console commands*:

Basic stuff you can call by typing `/<command>` in OA game console.

```
 - bet              <horse>[red,blue] <amount> <currency>[BTC,OAC]
 - unbet            <bet_ID>
 - pastBets
 - pastBets         elder
 - betsSummary
 - ready
 - help
 - shareBalance
 - shareBalance     <currency>[BTC,OAC]
 - timeout
 - timein
```

*New Cvars*

**Server-side:**<br>
 - `g_delagProjectiles`<br>
    <= 0 for old behavior (no projectiles delag);
    == 1 for less old behavior (50 msec);
    == 2 accurate to 1 server snap (usually similar to 50 msec);
    otherwise: add client ping to the nudge, clamped at g_delagProjectiles.
    100 by default (and therefore, the last option).
 - `g_enableBetting`<br>
    1 to enable *all* the betting features, 1 by default.
 - `g_backendAddr`<br>
    The address (IP:port) string of oatot backend.
    This Cvar has the same defaults as backend's `grpc-addr` flag.
 - `g_makingBetsTime`<br>
    The duration of MAKING_BETS in mins, 2 by default.
 - `g_easyItemPickup`<br>
    1 for easy item pickup (high items), 1 by default.
 - `g_scoreboardDefaultSeason`<br>
    Season which will be set as default scoreboard season on clients, 1 by default.
    0 - no season, 1 - winter, 2 - spring, 3 - summer, 4 - autumn.
 - `g_allowTimeouts`<br>
    Allows all players to use `/timeout` cmd, 1 by default.
 - `g_afterTimeoutTime`<br>
    Time to wait after `/timein` cmd (in seconds).

**Client-side:**<br>
 - `cg_ambientSounds`<br>
    0 to disable ambient sounds, 1 by default.
 - `cg_scoreboardEffects`<br>
    1 to enable additional scoreboard effects, 0 by default.
 - `cg_scoreboardSeason`<br>
    Scoreboard season, `g_scoreboardDefaultSeason` (server-side Cvar) by default.
    0 - no season, 1 - winter, 2 - spring, 3 - summer, 4 - autumn.
    Set to -1 in order to set server defaults again.
    Will be forced updated when default changes on the server.
 - `cg_scoreboardAggressive`<br>
    1 to enable aggressive scoreboard effects. Incompatible with `cg_scoreboardSeason != 0`.
    0 by default.
 - `cg_hudFlagStyle`<br>
    0 to disable new flag status HUD.
    1 by default.
 - `cg_forceTeamModels`
    0 by default, 1 to activate red/blue color switch.
    with 0, the switch is based on enemy/team Cvars.
 - `cg_enemyModel` "smarine/pm" by default.
 - `cg_teamModel` "major/pm" by default.
 - `cg_redTeamModel` "major/pm" by default.
 - `cg_blueTeamModel` "skelebot/pm" by default.
 - `cg_deadBodyDarken` "1" by default.
 - `cg_deadBodyColor` "0x323232FF" by default.
 - `cg_teamLegsColor` "white" by default.
 - `cg_teamTorsoColor` "white" by default.
 - `cg_teamHeadColor` "white" by default.
 - `cg_redLegsColor` "red" by default.
 - `cg_redTorsoColor` "red" by default.
 - `cg_redHeadColor` "red" by default.
 - `cg_blueLegsColor` "blue" by default.
 - `cg_blueTorsoColor` "blue by default.
 - `cg_blueHeadColor` "blue" by default.
 - `cg_enemyLegsColor` "yellow" by default.
 - `cg_enemyTorsoColor` "yellow" by default.
 - `cg_enemyHeadColor` "yellow" by default.
