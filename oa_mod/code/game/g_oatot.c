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

void G_oatot_GetActiveBidsSums_Closure(
    const Oatot__OaActiveBidsSumsResponse* message,
    void* closure_data
) {
    ((RPC_result*) closure_data)->result = (ProtobufCMessage*) message;
    (((RPC_result*) closure_data)->done) = qtrue;
}

void G_oatot_GetBalance_Closure(
    const Oatot__OaMyBalanceResponse* message,
    void* closure_data
) {
    ((RPC_result*) closure_data)->result = (ProtobufCMessage*) message;
    (((RPC_result*) closure_data)->done) = qtrue;
}

void G_oatot_GetActiveBids_Closure(
    const Oatot__OaMyActiveBidsResponse* message,
    void* closure_data
) {
    ((RPC_result*) closure_data)->result = (ProtobufCMessage*) message;
    (((RPC_result*) closure_data)->done) = qtrue;
}
