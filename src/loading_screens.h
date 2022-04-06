/*
	C-Dogs SDL
	A port of the legendary (and fun) action/arcade cdogs.
	Copyright (c) 2022 Cong Xu
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	Redistributions of source code must retain the above copyright notice, this
	list of conditions and the following disclaimer.
	Redistributions in binary form must reproduce the above copyright notice,
	this list of conditions and the following disclaimer in the documentation
	and/or other materials provided with the distribution.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
*/
#pragma once

#include "game_loop.h"
#include <cdogs/draw/draw_buffer.h>
#include <cdogs/grafx.h>
#include <cdogs/map.h>
#include <cdogs/pic.h>

typedef struct
{
	const Pic *logo;
	GraphicsDevice *g;
	Map m;
	float showPct;
	CArray tileIndices;
	DrawBuffer db;
	Mix_Chunk *sndTick;
	Mix_Chunk *sndComplete;
} LoadingScreen;

extern LoadingScreen gLoadingScreen;

void LoadingScreenInit(LoadingScreen *l, GraphicsDevice *g);
void LoadingScreenTerminate(LoadingScreen *l);

// Reload tile textures, required when the map changes
void LoadingScreenReload(LoadingScreen *l);
void LoadingScreenDraw(
	LoadingScreen *l, const char *loadingText, const float showPct);

GameLoopData *ScreenLoading(
	const char *loadingText, const bool ascending, GameLoopData *nextLoop);
