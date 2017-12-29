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
cp /path/to/openarena-server .
./build_and_run_oa_server
```

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
 - pastBids
 - pastBids         elder
 - bidsSummary
 - readyToBet
 - finishedBetting
 - help
 - shareBalance
 - shareBalance     <currency>[BTC,OAC]
```

*Game stages*

Game process is splitted into the following stages, the only way to switch between them
is to use `readyToBet` and `finishedBetting` commands or by finishing the match
in some way.

 - **FORMING_TEAMS**<br>
    Available commands: `pastBids, bidsSummary, readyToBet, help, shareBalance`.
    Players are able to switch teams or spec and disconnect. No additional restrictions
    comparing to normal game, but scores (both flags and personal) aren't counted.
    You can't make any bets yet, teams aren't formed.

 - **MAKING_BETS**<br>
    Available commands: `bet, unbet, pastBids, bidsSummary, finishedBetting, help, shareBalance`.
    Teams are fixed, you can't switch. If someone disconnects, `map_restart` is called.
    You can now make and discard your bets. Still no scores though.

 - **PLAYING**<br>
    Available commands: `pastBids, bidsSummary, help, shareBalance`.
    Betting is finished. Teams are still locked, but the game has started already,
    so score is now counted.

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
