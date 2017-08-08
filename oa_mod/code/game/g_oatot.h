/*
Declarations for interactions with `mod_proxy`.
*/

#ifndef _G_OATOT_H
#define _G_OATOT_H

#include "g_local.h"

// "done" means is properly used within a mod already.

void G_oatot_discardBet( const char* cl_guid, int bet_id ); // done
void G_oatot_transferMoney( const char* cl_guid, int amount );
betSum_t G_oatot_getActiveBidsSums( const char* horse ); // postponed
void G_oatot_changeGameStage( gameStage_t new_stage ); // done
qboolean G_oatot_isNew( const char* cl_guid ); // done
void G_oatot_register( const char* cl_guid ); // done
int G_oatot_getBalance( const char* cl_guid, const char* currency ); // postponed
void G_oatot_makeBet( const char* cl_guid, bid_t bet ); // done
void G_oatot_closeBids( const char* winner );
void G_oatot_closeBidsByIncident( void );
int G_oatot_getActiveBids( const char* cl_guid, bid_t* bids_arr ); // postponed
int G_oatot_getPastBids( const char* cl_guid, fullbid_t* bids_arr, const char* page, char* next_page ); // done
bidsSummary_t G_oatot_getBidsSummary( const char* cl_guid ); // done

#endif /* ifndef _G_OATOT_H */
