/*
Interactions with `mod_proxy`.
*/

#include "g_oatot.h"

void waitForRPC( qboolean* done ) {
    while ( !(*done) ) {
        protobuf_c_rpc_dispatch_run( protobuf_c_rpc_dispatch_default() );
    }
}

void G_oatot_IsNew_Closure(
    const Oatot__OaIsNewResponse* message,
    void* closure_data
) {
    ((RPC_result*) closure_data)->result = (ProtobufCMessage*) message;
    (((RPC_result*) closure_data)->done) = qtrue;
}

void G_oatot_Register_Closure(
    const Oatot__OaRegisterResponse* message,
    void* closure_data
) {
    ((RPC_result*) closure_data)->result = (ProtobufCMessage*) message;
    (((RPC_result*) closure_data)->done) = qtrue;
}

void G_oatot_DiscardBet_Closure(
    const Oatot__OaDiscardBetResponse* message,
    void* closure_data
) {
    ((RPC_result*) closure_data)->result = (ProtobufCMessage*) message;
    (((RPC_result*) closure_data)->done) = qtrue;
}

void G_oatot_TransferMoney_Closure(
    const Oatot__OaTransferMoneyResponse* message,
    void* closure_data
) {
    ((RPC_result*) closure_data)->result = (ProtobufCMessage*) message;
    (((RPC_result*) closure_data)->done) = qtrue;
}

void G_oatot_discardBet( const char* cl_guid, int bet_id ) {
}

void G_oatot_transferMoney( const char* cl_guid, int amount ) {
}

betSum_t G_oatot_getActiveBidsSums( const char* horse ) {
    betSum_t bet_sum;
    bet_sum.oac_amount = 1000;
    bet_sum.btc_amount = 1000;
    return bet_sum;
}

void G_oatot_changeGameStage( gameStage_t new_stage) {
}

qboolean G_oatot_isNew( const char* cl_guid ) {
    return qfalse;
}

void G_oatot_register( const char* cl_guid ) {
}

int G_oatot_getBalance( const char* cl_guid, const char* currency ) {
    // TODO dummy, no implementation yet.
    return 999999;
}

void G_oatot_makeBet( const char* cl_guid, bid_t bet ) {
}

void G_oatot_closeBids( const char* winner ) {
}

void G_oatot_closeBidsByIncident( void ) {
}

int G_oatot_getActiveBids( const char* cl_guid, bid_t* bids_arr ) {
    return 0;
}

int G_oatot_getPastBids( const char* cl_guid, fullbid_t* bids_arr, const char* page, char* next_page ) {
    // TODO dummy, no implementation yet.
    qtime_t open_time, close_time;
    bid_t test_b;
    fullbid_t test;
    trap_RealTime( &open_time );
    trap_RealTime( &close_time );
    test_b.horse = "Red";
    test_b.currency = "OAC";
    test_b.amount = 10;
    test_b.openTime = open_time;
    test.open_bid = test_b;
    test.winner = "Red";
    test.prize = 20;
    test.closeTime = close_time;
    bids_arr[0] = test;
    next_page = "aaaa";
    return 1;
}

bidsSummary_t G_oatot_getBidsSummary( const char* cl_guid ) {
    // TODO dummy, no implementation yet.
    bidsSummary_t test_summary;
    currencySummary_t btc_summary;
    currencySummary_t oac_summary;
    btc_summary.total_bet = 0;
    btc_summary.total_prize = 0;
    btc_summary.total_lost = 0;
    btc_summary.bets_won = 0;
    btc_summary.bets_lost = 0;
    oac_summary.total_bet = 5;
    oac_summary.total_prize = 5;
    oac_summary.total_lost = 0;
    oac_summary.bets_won = 5;
    oac_summary.bets_lost = 0;
    test_summary.btc_summary = btc_summary;
    test_summary.oac_summary = oac_summary;
    return test_summary;
}
