/*
Declarations for interactions with `mod_proxy`.
*/

#ifndef _G_OATOT_H
#define _G_OATOT_H

#include "generated/api.pb-c.h"

#include "g_local.h"

void waitForRPC( qboolean* done );

typedef struct {
    ProtobufCMessage* result;
    qboolean done;
} RPC_result;

void G_oatot_IsNew_Closure(
    const Oatot__OaIsNewResponse* message,
    void* closure_data
);

void G_oatot_Register_Closure(
    const Oatot__OaRegisterResponse* message,
    void* closure_data
);

void G_oatot_DiscardBet_Closure(
    const Oatot__OaDiscardBetResponse* message,
    void* closure_data
);

// "done" means is properly used within a mod already.

void G_oatot_discardBet( const char* cl_guid, int bet_id ); // done
void G_oatot_transferMoney( const char* cl_guid, int amount ); // done
betSum_t G_oatot_getActiveBidsSums( const char* horse ); // postponed
void G_oatot_changeGameStage( gameStage_t new_stage ); // done
qboolean G_oatot_isNew( const char* cl_guid ); // done
void G_oatot_register( const char* cl_guid ); // done
int G_oatot_getBalance( const char* cl_guid, const char* currency ); // postponed
void G_oatot_makeBet( const char* cl_guid, bid_t bet ); // done
void G_oatot_closeBids( const char* winner ); // done
void G_oatot_closeBidsByIncident( void ); // done
int G_oatot_getActiveBids( const char* cl_guid, bid_t* bids_arr ); // postponed
int G_oatot_getPastBids( const char* cl_guid, fullbid_t* bids_arr, const char* page, char* next_page ); // done
bidsSummary_t G_oatot_getBidsSummary( const char* cl_guid ); // done

#endif /* ifndef _G_OATOT_H */
