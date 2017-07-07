/*
Interactions with `mod_proxy`.
*/

#include "g_oatot.h"

int G_oatot_getBalance( const char* cl_guid, const char* currency ) {
    // TODO dummy, no implementation yet.
    return 999999;
}

int G_oatot_getPastBids( const char* cl_guid, fullbid_t* bids_arr, int page_index ) {
    // TODO dummy, no implementation yet.
    qtime_t open_time, close_time;
    bid_t test_b;
    fullbid_t test;
    trap_RealTime( &open_time );
    trap_RealTime( &close_time );
    test_b.discarded = qfalse;
    test_b.horse = "Red";
    test_b.currency = "OAC";
    test_b.amount = 10;
    test_b.openTime = open_time;
    test.open_bid = test_b;
    test.winner = "Red";
    test.prize = 20;
    test.closeTime = close_time;
    bids_arr[0] = test;
    return 1;
}
