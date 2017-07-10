# oatot
OpenArena tote

## Idea

Make betting (with both virtual currency or [BTC][BTC-link]) possible in [OA][OA-link].

## Motivation

Just for fun and to keep the game I love alive.

## OpenArena mod

*New console commands*:

Basic stuff you can call by typing `/<command>` in OA game console.

```
 - bet              <horse>[red,blue] <amount> <currency>[BTC,OAC]
 - unbet            <bet_ID>
 - pastBids         <page_index>
 - bidsSummary
 - readyToBet
 - finishedBetting
 - listPlayersBids
 - help
```

*Game stages*

Game process is splitted into the following stages, the only way to switch between them
is to use `readyToBet` and `finishedBetting` commands or by finishing the match
in some way.

 - **FORMING_TEAMS**<br>
    Available commands: `pastBids, bidsSummary, readyToBet, help`.
    Players are able to switch teams or spec and disconnect. No additional restrictions
    comparing to normal game, but scores (both flags and personal) aren't counted.
    You can't make any bets yet, teams aren't formed.

 - **MAKING_BETS**<br>
    Available commands: `bet, unbet, pastBids, bidsSummary, finishedBetting, listPlayersBids, help`.
    Teams are fixed, you can't switch. If someone disconnects, `map_restart` is called.
    You can now make and discard your bets. Still no scores though.

 - **PLAYING**<br>
    Available commands: `pastBids, bidsSummary, listPlayersBids, help`.
    Betting is finished. Teams are still locked, but the game has started already,
    so score is now counted.

## Architecture of the project

```
OaMod <-----------------> ModProxy <-----------------> Backend
        FUSE/FileSocket                    gRPC
                                 (documented in api.proto)
```

[BTC-link]: https://en.wikipedia.org/wiki/Bitcoin
[OA-link]: https://en.wikipedia.org/wiki/OpenArena
