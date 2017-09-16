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

void G_oatot_ChangeGameStage_Closure(
    const Oatot__OaChangeGameStageResponse* message,
    void* closure_data
) {
    ((RPC_result*) closure_data)->result = (ProtobufCMessage*) message;
    (((RPC_result*) closure_data)->done) = qtrue;
}

void G_oatot_MakeBet_Closure(
    const Oatot__OaMyBidResponse* message,
    void* closure_data
) {
    ((RPC_result*) closure_data)->result = (ProtobufCMessage*) message;
    (((RPC_result*) closure_data)->done) = qtrue;
}

void G_oatot_CloseBids_Closure(
    const Oatot__OaCloseBidsResponse* message,
    void* closure_data
) {
    ((RPC_result*) closure_data)->result = (ProtobufCMessage*) message;
    (((RPC_result*) closure_data)->done) = qtrue;
}

void G_oatot_CloseBidsByIncident_Closure(
    const Oatot__OaCloseBidsByIncidentResponse* message,
    void* closure_data
) {
    ((RPC_result*) closure_data)->result = (ProtobufCMessage*) message;
    (((RPC_result*) closure_data)->done) = qtrue;
}

void G_oatot_GetPastBids_Closure(
    const Oatot__OaMyPastBidsResponse* message,
    void* closure_data
) {
    ((RPC_result*) closure_data)->result = (ProtobufCMessage*) message;
    (((RPC_result*) closure_data)->done) = qtrue;
}

void G_oatot_GetBidsSummary_Closure(
    const Oatot__OaMyBidsSummaryResponse* message,
    void* closure_data
) {
    ((RPC_result*) closure_data)->result = (ProtobufCMessage*) message;
    (((RPC_result*) closure_data)->done) = qtrue;
}

betSum_t G_oatot_getActiveBidsSums( const char* horse ) {
    betSum_t bet_sum;
    bet_sum.oac_amount = 1000;
    bet_sum.btc_amount = 1000;
    return bet_sum;
}

int G_oatot_getBalance( const char* cl_guid, const char* currency ) {
    // TODO dummy, no implementation yet.
    return 999999;
}

int G_oatot_getActiveBids( const char* cl_guid, Oatot__Bid* bids_arr ) {
    return 0;
}
