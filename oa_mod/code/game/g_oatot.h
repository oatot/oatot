/*
Declarations for interactions with `mod_proxy`.
*/

#ifndef _G_OATOT_H
#define _G_OATOT_H

#include "g_local.h"

int G_oatot_getBalance( const char* cl_guid, const char* currency );
int G_oatot_getPastBids( const char* cl_guid, fullbid_t* bids_arr, int page_index );

#endif /* ifndef _G_OATOT_H */
