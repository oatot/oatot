# List of changes to consider by Commie

Things to consider

* Seperate the logic of actual betting from the gamecode as much as possible.
  So it is trivial to add support for betting for other mods, like Aftershock.

* Add some GUI-way to place bets. Some people might not be comfortable with
  using console-commands.

* Lower the amount of time and complexity dedicated to betting. Having to do
  `/readyToBet` `/finishedbetting` takes away from the time spent playing the
  game. Maybe simply having a normal `/ready`, then a 2 minute countdown where
  people can place their bets. I think it is important that betting is more of a
  "side thing", if it should catch on and be popular. People probably come to
  play or spectate, rather than to type commands and spend their OA coins. Even
  though spending OA-coins can be a fun side-thing!

* Remove 1000 free OAC. Make the only way to gain OAC by playing the game. That
  way people can't generate new qkeys to bet against themselves and gain free
  money. With time, the amount of coins in circulation will increase anyways.

  But it is probably good to keep it, for testing purposes. But when
  beta-testing is done, reset every coin earned, and start from 0 without free
  OAC.

* Store the amount of coins in some data-base that can be shared between
  servers.
  This way you could run a CASUAL server without actual betting, but players
  who play there still get money which can be used to bet. People also mostly
  start out on CASUAL servers, so they will probably have accumulated some money
  before they find out betting is even a thing. This should remove the need
  for 1000 free OAC.

* Make it possible to have a coefficient on the amount of money people gain on
  various servers. So playing on a more CASUAL server (with different game
  mechanics like free-RJ), rewards less coins for the same score compared to
  other more competitive servers.

* Punish people who disconnect, without reconnecting, so they loose OAC. To
  prevent sabotage. This seems especially important if you keep the behavior
  where bets get cancelled when someone disconnects. Maybe even have the
  punishment be proportional to the stake of the game, and use this money to
  repay people who bet on the side where the player left.

* Sometimes people disconnect because their internet goes down, or laptop runs
  out of battery. Don't cancel the match immediately. Give them some time to
  reconnect. Maybe pause the match, for one minute, and cancel if the player
  has not reconnected in that time.

* When the match is over. Give the players on the winning side a percentage
  of the coins used to bet on the match. This way, they will probably take the
  match more seriously

* Maybe make it possible for players to give away and trade coins? But this
  creates more opporunities for match-fixing. But it should be considered.
  Note that in theory people would still be able to give away coins by starting
  matches and loosing on purpose.
