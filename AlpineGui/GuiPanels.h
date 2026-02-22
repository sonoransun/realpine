/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once

#include <AppState.h>

class AsyncRpcClient;


void  drawConnectionBar (AppState & state, AsyncRpcClient & client);

void  drawMainTabs (AppState & state, AsyncRpcClient & client);

void  drawLogPanel (AppState & state);
