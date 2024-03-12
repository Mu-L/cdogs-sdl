/*
	C-Dogs SDL
	A port of the legendary (and fun) action/arcade cdogs.
	Copyright (c) 2024 Cong Xu
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
#include "pause_menu.h"

#include <cdogs/draw/drawtools.h>
#include <cdogs/font.h>

void PauseMenuInit(PauseMenu *pm, EventHandlers *handlers, GraphicsDevice *g)
{
	memset(pm, 0, sizeof *pm);
	MenuSystemInit(&pm->ms, handlers, g, svec2i_zero(), g->cachedConfig.Res);
	// TODO: create pause menu
	pm->handlers = handlers;
	pm->g = g;
	pm->pausingDevice = INPUT_DEVICE_UNSET;
}
void PauseMenuTerminate(PauseMenu *pm)
{
	MenuSystemTerminate(&pm->ms);
}

bool PauseMenuUpdate(
	PauseMenu *pm, const int cmds[MAX_LOCAL_PLAYERS],
	const int lastCmds[MAX_LOCAL_PLAYERS])
{
	int cmdAll = 0;
	int lastCmdAll = 0;
	for (int i = 0; i < MAX_LOCAL_PLAYERS; i++)
	{
		lastCmdAll |= lastCmds[i];
	}

	input_device_e pausingDevice = INPUT_DEVICE_UNSET;
	input_device_e firstPausingDevice = INPUT_DEVICE_UNSET;
	if (GetNumPlayers(PLAYER_ANY, false, true) == 0)
	{
		// If no players, allow default keyboard
		firstPausingDevice = INPUT_DEVICE_KEYBOARD;
		cmdAll |= cmds[0];
	}
	else
	{
		int idx = 0;
		for (int i = 0; i < (int)gPlayerDatas.size; i++, idx++)
		{
			const PlayerData *p = CArrayGet(&gPlayerDatas, i);
			if (!p->IsLocal)
			{
				idx--;
				continue;
			}
			cmdAll |= cmds[idx];
			if (firstPausingDevice == INPUT_DEVICE_UNSET)
			{
				firstPausingDevice = p->inputDevice;
			}

			// Only allow the first player to escape
			// Use keypress otherwise the player will quit immediately
			if (idx == 0 && (cmds[idx] & CMD_ESC) &&
				!(lastCmds[idx] & CMD_ESC))
			{
				pausingDevice = p->inputDevice;
			}
		}
	}
	if (KeyIsPressed(&gEventHandlers.keyboard, SDL_SCANCODE_ESCAPE))
	{
		pausingDevice = INPUT_DEVICE_KEYBOARD;
	}

	// Check if any controllers are unplugged
	pm->controllerUnplugged = false;
	CA_FOREACH(const PlayerData, p, gPlayerDatas)
	if (p->inputDevice == INPUT_DEVICE_UNSET && p->IsLocal)
	{
		pm->controllerUnplugged = true;
		break;
	}
	CA_FOREACH_END()

	// Check if:
	// - escape was pressed, or
	// - window lost focus
	// - controller unplugged
	// If the game is paused, update the pause menu
	// If the game was not paused, enter pause mode
	// If the game was paused and escape was pressed, exit the game
	if (pm->pausingDevice != INPUT_DEVICE_UNSET && AnyButton(lastCmdAll) &&
		!AnyButton(cmdAll))
	{
		// Unpause on any button press
		// TODO: don't do this, update menu system instead
		pm->pausingDevice = INPUT_DEVICE_UNSET;
	}
	else if (pm->controllerUnplugged || gEventHandlers.HasLostFocus)
	{
		// Pause the game
		pm->pausingDevice = firstPausingDevice;
		SoundPlay(&gSoundDevice, StrSound("menu_error"));
	}
	else if (pausingDevice != INPUT_DEVICE_UNSET)
	{
		if (pm->pausingDevice != INPUT_DEVICE_UNSET)
		{
			// Already paused; exit
			// TODO: don't exit, use pause menu
			GameEvent e = GameEventNew(GAME_EVENT_MISSION_END);
			e.u.MissionEnd.IsQuit = true;
			GameEventsEnqueue(&gGameEvents, e);
			// Need to unpause to process the quit
			pm->pausingDevice = INPUT_DEVICE_UNSET;
			pm->controllerUnplugged = false;
			SoundPlay(&gSoundDevice, StrSound("menu_back"));
			// Return true to say we want to quit
			return true;
		}
		else
		{
			// Pause the game
			pm->pausingDevice = pausingDevice;
			SoundPlay(&gSoundDevice, StrSound("menu_back"));
		}
	}
	return false;
}
void PauseMenuDraw(const PauseMenu *pm)
{
	if (!PauseMenuIsShown(pm))
	{
		return;
	}

	// Draw a background overlay
	color_t overlay = colorBlack;
	overlay.a = 128;
	DrawRectangle(
		pm->g, svec2i_zero(), pm->g->cachedConfig.Res, overlay, true);

	if (pm->controllerUnplugged)
	{
		struct vec2i pos = svec2i_scale_divide(
			svec2i_subtract(
				pm->g->cachedConfig.Res,
				FontStrSize(ARROW_LEFT
							"Paused" ARROW_RIGHT
							"\nFoobar\nPlease reconnect controller")),
			2);
		const int x = pos.x;
		FontStr(ARROW_LEFT "Paused" ARROW_RIGHT, pos);

		pos.y += FontH();
		pos = FontStr("Press ", pos);
		char buf[256];
		color_t c = colorWhite;
		InputGetButtonNameColor(pm->pausingDevice, 0, CMD_ESC, buf, &c);
		pos = FontStrMask(buf, pos, c);
		FontStr(" to quit", pos);

		pos.x = x;
		pos.y += FontH();
		FontStr("Please reconnect controller", pos);
	}
	else if (pm->pausingDevice != INPUT_DEVICE_UNSET)
	{
		struct vec2i pos = svec2i_scale_divide(
			svec2i_subtract(
				gGraphicsDevice.cachedConfig.Res,
				FontStrSize("Foo\nPress foo or bar to unpause\nBaz")),
			2);
		const int x = pos.x;
		FontStr(ARROW_LEFT "Paused" ARROW_RIGHT, pos);

		pos.y += FontH();
		pos = FontStr("Press ", pos);
		char buf[256];
		color_t c = colorWhite;
		InputGetButtonNameColor(pm->pausingDevice, 0, CMD_ESC, buf, &c);
		pos = FontStrMask(buf, pos, c);
		FontStr(" again to quit", pos);

		pos.x = x;
		pos.y += FontH();
		pos = FontStr("Press ", pos);
		c = colorWhite;
		InputGetButtonNameColor(pm->pausingDevice, 0, CMD_BUTTON1, buf, &c);
		pos = FontStrMask(buf, pos, c);
		pos = FontStr(" or ", pos);
		c = colorWhite;
		InputGetButtonNameColor(pm->pausingDevice, 0, CMD_BUTTON2, buf, &c);
		pos = FontStrMask(buf, pos, c);
		FontStr(" to unpause", pos);
	}
}

bool PauseMenuIsShown(const PauseMenu *pm)
{
	return pm->pausingDevice != INPUT_DEVICE_UNSET || pm->controllerUnplugged;
}
