# oatot
OpenArena tote

## Idea

Make betting (with both virtual currency or [BTC][BTC-link]) possible in [OA][OA-link].

## Motivation

Just for fun and to keep the game I love alive.

## Architecture of the project

```
OaMod <-----------> gRPC client (Go) <-------------> Backend (Go)
           Cgo                             gRPC
                                  (documented in api.proto)
```

## How to build and run OA mod

You will need a C-compiler (better gcc or clang), GNU make, Go, grpc-go.

```
cd /path/to/oa_mod/linux_scripts/
./build_oa_server
./run_oa_server <port> server.cfg
```
Make sure that `server.cfg` is in `~/.openarena/baseoa` folder
and you have `BINNAME` set to your openarena server binary.

## How to build and run backend

You will need: Go, grpc-go.

```
cd /path/to/backend/otservice/
go build
./otservice -database '/path/to/storage_file' -start-money 0
```

## OpenArena mod

Modified OAX.

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
```

*Update note:*<br>
`/bet` and `/unbet` commands are outdated, there is super cool UI menu for this purpose instead.
*Don't worry, /bet and /unbet are still there for quality people! :p*

*Game stages*

Game process is splitted into the following stages, the only way to switch between them
is to use `ready` command or by finishing the match
in some way.

 - **FORMING_TEAMS**<br>
    Available commands: `pastBets, betsSummary, ready, help, shareBalance`.
    Players are able to switch teams or spec and disconnect. No additional restrictions
    comparing to normal game, but scores (both flags and personal) aren't counted.
    You can't make any bets yet, teams aren't formed.

 - **MAKING_BETS**<br>
    Available commands: `bet, unbet, pastBets, betsSummary, help, shareBalance`.
    Teams are fixed, you can't switch. If someone disconnects, `map_restart` is called.
    You can now make and discard your bets. Still no scores though.
    Next stage will be started in `g_makingBetsTime`.

 - **PLAYING**<br>
    Available commands: `pastBets, betsSummary, help, shareBalance`.
    Betting is finished. Teams are still locked, but the game has started already,
    so score is now counted.

*New Cvars*

**Server-side:**<br>
 - `g_makingBetsTime`<br>
    The duration of MAKING_BETS in mins, 2 by default.
 - `g_easyItemPickup`<br>
    1 for easy item pickup (high items), 1 by default.
 - `g_scoreboardDefaultSeason`<br>
    Season which will be set as default scoreboard season on clients, 1 by default.
    0 - no season, 1 - winter, 2 - spring, 3 - summer, 4 - autumn.

**Client-side:**<br>
 - `cg_scoreboardEffects`<br>
    0 to disable all additional scoreboard effects, 1 by default.
 - `cg_scoreboardSeason`<br>
    Scoreboard season, `g_scoreboardDefaultSeason` (server-side Cvar) by default.
    0 - no season, 1 - winter, 2 - spring, 3 - summer, 4 - autumn.
    Set to -1 in order to set server defaults again.
 - `cg_scoreboardAggressive`<br>
    1 to enable aggressive scoreboard effects. Incompatible with `cg_scoreboardSeason != 0`.
    0 by default.

*How to earn OaCoins?*

 - Every new player is sponsored with 1000 OAC for first sign up.
 - Your score after every single match played in oatot mod is transferred to your
    *prize top-up* (1 score point is equal to one OAC for now).
 - Obviously - by making bets and winning them! :)<br>

*Future solutions*
 - Mining.
 - Make OaCoins visible items on the map so you can collect them while playing.

[BTC-link]: https://en.wikipedia.org/wiki/Bitcoin
[OA-link]: https://en.wikipedia.org/wiki/OpenArena
