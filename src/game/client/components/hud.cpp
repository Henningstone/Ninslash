#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/shared/config.h>

#include <game/generated/protocol.h>
#include <game/generated/client_data.h>
#include <game/layers.h>
#include <game/client/gameclient.h>
#include <game/client/render.h>
#include <game/client/customstuff.h>

#include <game/weapons.h>
#include <game/buildables.h>

#include "controls.h"
#include "camera.h"
#include "hud.h"
#include "voting.h"
#include "binds.h"

#define RAD 0.017453292519943295769236907684886f

CHud::CHud()
{
	// won't work if zero
	m_AverageFPS = 1.0f;
}

void CHud::OnReset()
{
}

void CHud::RenderGameTimer()
{
	float Half = 300.0f*Graphics()->ScreenAspect()/2.0f;

	if(!(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_SUDDENDEATH))
	{
		char Buf[32];
		int Time = 0;
		if(m_pClient->m_Snap.m_pGameInfoObj->m_TimeLimit && !m_pClient->m_Snap.m_pGameInfoObj->m_WarmupTimer)
		{
			Time = m_pClient->m_Snap.m_pGameInfoObj->m_TimeLimit*60 - ((Client()->GameTick()-m_pClient->m_Snap.m_pGameInfoObj->m_RoundStartTick)/Client()->GameTickSpeed());

			if(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER)
				Time = 0;
		}
		else
			Time = (Client()->GameTick()-m_pClient->m_Snap.m_pGameInfoObj->m_RoundStartTick)/Client()->GameTickSpeed();

		str_format(Buf, sizeof(Buf), "%d:%02d", Time/60, Time%60);
		float FontSize = 10.0f;
		float w = TextRender()->TextWidth(0, FontSize, Buf, -1);
		// last 60 sec red, last 10 sec blink
		if(m_pClient->m_Snap.m_pGameInfoObj->m_TimeLimit && Time <= 60 && !m_pClient->m_Snap.m_pGameInfoObj->m_WarmupTimer)
		{
			float Alpha = Time <= 10 && (2*time_get()/time_freq()) % 2 ? 0.5f : 1.0f;
			TextRender()->TextColor(1.0f, 0.25f, 0.25f, Alpha);
		}
		TextRender()->Text(0, Half-w/2, 2, FontSize, Buf, -1);
		TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
	}
}

void CHud::RenderPauseNotification()
{
	if(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_PAUSED &&
		!(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER))
	{
		const char *pText = Localize("Game paused");
		float FontSize = 20.0f;
		float w = TextRender()->TextWidth(0, FontSize,pText, -1);
		TextRender()->Text(0, 150.0f*Graphics()->ScreenAspect()+-w/2.0f, 50.0f, FontSize, pText, -1);
	}
}

void CHud::RenderSuddenDeath()
{
	if(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_SUDDENDEATH)
	{
		float Half = 300.0f*Graphics()->ScreenAspect()/2.0f;
		const char *pText = Localize("Sudden Death");
		float FontSize = 12.0f;
		float w = TextRender()->TextWidth(0, FontSize, pText, -1);
		TextRender()->Text(0, Half-w/2, 2, FontSize, pText, -1);
	}
}

void CHud::RenderScoreHud()
{
	// render small score hud
	if(!(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER))
	{
		int GameFlags = m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags;
		float Whole = 300*Graphics()->ScreenAspect();
		float StartY = 229.0f;

		if(GameFlags&GAMEFLAG_TEAMS && !(GameFlags&GAMEFLAG_INFECTION) && m_pClient->m_Snap.m_pGameDataObj)
		{
			char aScoreTeam[2][32];
			str_format(aScoreTeam[TEAM_RED], sizeof(aScoreTeam)/2, "%d", m_pClient->m_Snap.m_pGameDataObj->m_TeamscoreRed);
			str_format(aScoreTeam[TEAM_BLUE], sizeof(aScoreTeam)/2, "%d", m_pClient->m_Snap.m_pGameDataObj->m_TeamscoreBlue);
			float aScoreTeamWidth[2] = { TextRender()->TextWidth(0, 14.0f, aScoreTeam[TEAM_RED], -1), TextRender()->TextWidth(0, 14.0f, aScoreTeam[TEAM_BLUE], -1) };
			int FlagCarrier[2] = { m_pClient->m_Snap.m_pGameDataObj->m_FlagCarrierRed, m_pClient->m_Snap.m_pGameDataObj->m_FlagCarrierBlue };
			float ScoreWidthMax = max(max(aScoreTeamWidth[TEAM_RED], aScoreTeamWidth[TEAM_BLUE]), TextRender()->TextWidth(0, 14.0f, "100", -1));
			float Split = 3.0f;
			float ImageSize = GameFlags&GAMEFLAG_FLAGS ? 16.0f : Split;

			for(int t = 0; t < 2; t++)
			{
				// draw box
				Graphics()->BlendNormal();
				Graphics()->TextureSet(-1);
				Graphics()->QuadsBegin();
				
				if (GameFlags&GAMEFLAG_INFECTION)
				{
					if(t == 0)
						Graphics()->SetColor(1.0f, 0.7f, 0.7f, 0.3f);
					else
						Graphics()->SetColor(0.1f, 0.1f, 0.1f, 0.3f);
				}
				else
				{
					if(t == 0)
						Graphics()->SetColor(1.0f, 0.0f, 0.0f, 0.25f);
					else
						Graphics()->SetColor(0.0f, 0.0f, 1.0f, 0.25f);
				}
				
				RenderTools()->DrawRoundRectExt(Whole-ScoreWidthMax-ImageSize-2*Split, StartY+t*20, ScoreWidthMax+ImageSize+2*Split, 18.0f, 5.0f, CUI::CORNER_L);
				Graphics()->QuadsEnd();

				// draw score
				TextRender()->Text(0, Whole-ScoreWidthMax+(ScoreWidthMax-aScoreTeamWidth[t])/2-Split, StartY+t*20, 14.0f, aScoreTeam[t], -1);

				if(GameFlags&GAMEFLAG_FLAGS)
				{
					int BlinkTimer = (m_pClient->m_FlagDropTick[t] != 0 &&
										(Client()->GameTick()-m_pClient->m_FlagDropTick[t])/Client()->GameTickSpeed() >= 25) ? 10 : 20;
					if(FlagCarrier[t] == FLAG_ATSTAND || (FlagCarrier[t] == FLAG_TAKEN && ((Client()->GameTick()/BlinkTimer)&1)))
					{
						// draw flag
						Graphics()->BlendNormal();
						Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
						Graphics()->QuadsBegin();
						RenderTools()->SelectSprite(t==0?SPRITE_FLAG_RED:SPRITE_FLAG_BLUE);
						IGraphics::CQuadItem QuadItem(Whole-ScoreWidthMax-ImageSize, StartY+1.0f+t*20, ImageSize/2, ImageSize);
						Graphics()->QuadsDrawTL(&QuadItem, 1);
						Graphics()->QuadsEnd();
					}
					else if(FlagCarrier[t] >= 0)
					{
						// draw name of the flag holder
						int ID = FlagCarrier[t]%MAX_CLIENTS;
						const char *pName = m_pClient->m_aClients[ID].m_aName;
						float w = TextRender()->TextWidth(0, 8.0f, pName, -1);
						TextRender()->Text(0, min(Whole-w-1.0f, Whole-ScoreWidthMax-ImageSize-2*Split), StartY+(t+1)*20.0f-3.0f, 8.0f, pName, -1);

						// draw tee of the flag holder
						CTeeRenderInfo Info = m_pClient->m_aClients[ID].m_RenderInfo;
						Info.m_Size = 18.0f;
						//RenderTools()->RenderTee(CAnimState::GetIdle(), &Info, EMOTE_NORMAL, vec2(1,0),
						//	vec2(Whole-ScoreWidthMax-Info.m_Size/2-Split, StartY+1.0f+Info.m_Size/2+t*20));
					}
				}
				StartY += 8.0f;
			}
		}
		else
		{
			int Local = -1;
			int aPos[2] = { 1, 2 };
			const CNetObj_PlayerInfo *apPlayerInfo[2] = { 0, 0 };
			int i = 0;
			for(int t = 0; t < 2 && i < MAX_CLIENTS && m_pClient->m_Snap.m_paInfoByScore[i]; ++i)
			{
				if(m_pClient->m_Snap.m_paInfoByScore[i]->m_Team != TEAM_SPECTATORS)
				{
					apPlayerInfo[t] = m_pClient->m_Snap.m_paInfoByScore[i];
					if(apPlayerInfo[t]->m_ClientID == m_pClient->m_Snap.m_LocalClientID)
						Local = t;
					++t;
				}
			}
			// search local player info if not a spectator, nor within top2 scores
			if(Local == -1 && m_pClient->m_Snap.m_pLocalInfo && m_pClient->m_Snap.m_pLocalInfo->m_Team != TEAM_SPECTATORS)
			{
				for(; i < MAX_CLIENTS && m_pClient->m_Snap.m_paInfoByScore[i]; ++i)
				{
					if(m_pClient->m_Snap.m_paInfoByScore[i]->m_Team != TEAM_SPECTATORS)
						++aPos[1];
					if(m_pClient->m_Snap.m_paInfoByScore[i]->m_ClientID == m_pClient->m_Snap.m_LocalClientID)
					{
						apPlayerInfo[1] = m_pClient->m_Snap.m_paInfoByScore[i];
						Local = 1;
						break;
					}
				}
			}
			char aScore[2][32];
			for(int t = 0; t < 2; ++t)
			{
				if(apPlayerInfo[t])
					str_format(aScore[t], sizeof(aScore)/2, "%d", apPlayerInfo[t]->m_Score);
				else
					aScore[t][0] = 0;
			}
			float aScoreWidth[2] = {TextRender()->TextWidth(0, 14.0f, aScore[0], -1), TextRender()->TextWidth(0, 14.0f, aScore[1], -1)};
			float ScoreWidthMax = max(max(aScoreWidth[0], aScoreWidth[1]), TextRender()->TextWidth(0, 14.0f, "10", -1));
			float Split = 3.0f, ImageSize = 16.0f, PosSize = 16.0f;

			for(int t = 0; t < 2; t++)
			{
				// draw box
				Graphics()->BlendNormal();
				Graphics()->TextureSet(-1);
				Graphics()->QuadsBegin();
				if(t == Local)
					Graphics()->SetColor(1.0f, 1.0f, 1.0f, 0.25f);
				else
					Graphics()->SetColor(0.0f, 0.0f, 0.0f, 0.25f);
				RenderTools()->DrawRoundRectExt(Whole-ScoreWidthMax-ImageSize-2*Split-PosSize, StartY+t*20, ScoreWidthMax+ImageSize+2*Split+PosSize, 18.0f, 5.0f, CUI::CORNER_L);
				Graphics()->QuadsEnd();

				// draw score
				TextRender()->Text(0, Whole-ScoreWidthMax+(ScoreWidthMax-aScoreWidth[t])/2-Split, StartY+t*20, 14.0f, aScore[t], -1);

				if(apPlayerInfo[t])
 				{
					// draw name
					int ID = apPlayerInfo[t]->m_ClientID;
					const char *pName = m_pClient->m_aClients[ID].m_aName;
					float w = TextRender()->TextWidth(0, 8.0f, pName, -1);
					TextRender()->Text(0, min(Whole-w-1.0f, Whole-ScoreWidthMax-ImageSize-2*Split-PosSize), StartY+(t+1)*20.0f-3.0f, 8.0f, pName, -1);

					// draw tee
					CTeeRenderInfo Info = m_pClient->m_aClients[ID].m_RenderInfo;
 					Info.m_Size = 18.0f;
 					//RenderTools()->RenderTee(CAnimState::GetIdle(), &Info, EMOTE_NORMAL, vec2(1,0),
					RenderTools()->RenderPortrait(&Info,
						vec2(Whole-ScoreWidthMax-Info.m_Size/2-Split, StartY+1.0f+Info.m_Size/2+t*20 + 16), 0);
						
 					//	vec2(Whole-ScoreWidthMax-Info.m_Size/2-Split, StartY+1.0f+Info.m_Size/2+t*20));
					
				}

				// draw position
				char aBuf[32];
				str_format(aBuf, sizeof(aBuf), "%d.", aPos[t]);
				TextRender()->Text(0, Whole-ScoreWidthMax-ImageSize-Split-PosSize, StartY+2.0f+t*20, 10.0f, aBuf, -1);

				StartY += 8.0f;
			}
		}
	}
}

void CHud::RenderWarmupTimer()
{
	// render warmup timer
	if(m_pClient->m_Snap.m_pGameInfoObj->m_WarmupTimer)
	{
		char Buf[256];
		float FontSize = 20.0f;
		float w = TextRender()->TextWidth(0, FontSize, Localize("Warmup"), -1);
		TextRender()->Text(0, 150*Graphics()->ScreenAspect()+-w/2, 50, FontSize, Localize("Warmup"), -1);

		int Seconds = m_pClient->m_Snap.m_pGameInfoObj->m_WarmupTimer/SERVER_TICK_SPEED;
		if(Seconds < 5)
			str_format(Buf, sizeof(Buf), "%d.%d", Seconds, (m_pClient->m_Snap.m_pGameInfoObj->m_WarmupTimer*10/SERVER_TICK_SPEED)%10);
		else
			str_format(Buf, sizeof(Buf), "%d", Seconds);
		w = TextRender()->TextWidth(0, FontSize, Buf, -1);
		TextRender()->Text(0, 150*Graphics()->ScreenAspect()+-w/2, 75, FontSize, Buf, -1);
	}
}

void CHud::MapscreenToGroup(float CenterX, float CenterY, CMapItemGroup *pGroup)
{
	float Points[4];
	RenderTools()->MapscreenToWorld(CenterX, CenterY, pGroup->m_ParallaxX/100.0f, pGroup->m_ParallaxY/100.0f,
		pGroup->m_OffsetX, pGroup->m_OffsetY, Graphics()->ScreenAspect(), 1.0f, Points);
	Graphics()->MapScreen(Points[0], Points[1], Points[2], Points[3]);
}

void CHud::RenderFps()
{
	if(g_Config.m_ClShowfps)
	{
		// calculate avg. fps
		float FPS = 1.0f / Client()->RenderFrameTime();
		m_AverageFPS = (m_AverageFPS*(1.0f-(1.0f/m_AverageFPS))) + (FPS*(1.0f/m_AverageFPS));
		char Buf[512];
		str_format(Buf, sizeof(Buf), "%d", (int)m_AverageFPS);
		TextRender()->Text(0, m_Width-10-TextRender()->TextWidth(0,12,Buf,-1), 5, 12, Buf, -1);
	}
}

void CHud::RenderConnectionWarning()
{
	if(Client()->ConnectionProblems())
	{
		const char *pText = Localize("Connection Problems...");
		float w = TextRender()->TextWidth(0, 24, pText, -1);
		TextRender()->Text(0, 150*Graphics()->ScreenAspect()-w/2, 50, 24, pText, -1);
	}
}

void CHud::RenderTeambalanceWarning()
{
	if (m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_INFECTION)
		return;
	
	// render prompt about team-balance
	bool Flash = time_get()/(time_freq()/2)%2 == 0;
	if(m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_TEAMS)
	{
		int TeamDiff = m_pClient->m_Snap.m_aTeamSize[TEAM_RED]-m_pClient->m_Snap.m_aTeamSize[TEAM_BLUE];
		if (g_Config.m_ClWarningTeambalance && (TeamDiff >= 2 || TeamDiff <= -2))
		{
			const char *pText = Localize("Please balance teams!");
			if(Flash)
				TextRender()->TextColor(1,1,0.5f,1);
			else
				TextRender()->TextColor(0.7f,0.7f,0.2f,1.0f);
			TextRender()->Text(0x0, 5, 50, 6, pText, -1);
			TextRender()->TextColor(1,1,1,1);
		}
	}
}


void CHud::RenderVoting()
{
	if(!m_pClient->m_pVoting->IsVoting() || Client()->State() == IClient::STATE_DEMOPLAYBACK)
		return;

	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0,0,0,0.40f);
	RenderTools()->DrawRoundRect(-10, 60-2, 100+10+4+5, 46, 5.0f);
	Graphics()->QuadsEnd();

	TextRender()->TextColor(1,1,1,1);

	CTextCursor Cursor;
	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), Localize("%ds left"), m_pClient->m_pVoting->SecondsLeft());
	float tw = TextRender()->TextWidth(0x0, 6, aBuf, -1);
	TextRender()->SetCursor(&Cursor, 5.0f+100.0f-tw, 60.0f, 6.0f, TEXTFLAG_RENDER);
	TextRender()->TextEx(&Cursor, aBuf, -1);

	TextRender()->SetCursor(&Cursor, 5.0f, 60.0f, 6.0f, TEXTFLAG_RENDER);
	Cursor.m_LineWidth = 100.0f-tw;
	Cursor.m_MaxLines = 3;
	TextRender()->TextEx(&Cursor, m_pClient->m_pVoting->VoteDescription(), -1);

	// reason
	str_format(aBuf, sizeof(aBuf), "%s %s", Localize("Reason:"), m_pClient->m_pVoting->VoteReason());
	TextRender()->SetCursor(&Cursor, 5.0f, 79.0f, 6.0f, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
	Cursor.m_LineWidth = 100.0f;
	TextRender()->TextEx(&Cursor, aBuf, -1);

	CUIRect Base = {5, 88, 100, 4};
	m_pClient->m_pVoting->RenderBars(Base, false);

	const char *pYesKey = m_pClient->m_pBinds->GetKey("vote yes");
	const char *pNoKey = m_pClient->m_pBinds->GetKey("vote no");
	str_format(aBuf, sizeof(aBuf), "%s - %s", pYesKey, Localize("Vote yes"));
	Base.y += Base.h+1;
	UI()->DoLabel(&Base, aBuf, 6.0f, -1);

	str_format(aBuf, sizeof(aBuf), "%s - %s", Localize("Vote no"), pNoKey);
	UI()->DoLabel(&Base, aBuf, 6.0f, 1);
}

void CHud::RenderCursor()
{
	if(!m_pClient->m_Snap.m_pLocalCharacter || Client()->State() == IClient::STATE_DEMOPLAYBACK)
		return;

	MapscreenToGroup(m_pClient->m_pCamera->m_Center.x, m_pClient->m_pCamera->m_Center.y, Layers()->GameGroup());
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
	Graphics()->QuadsBegin();

	// render cursor
	RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[m_pClient->m_Snap.m_pLocalCharacter->m_Weapon%NUM_WEAPONS].m_pSpriteCursor);
	float CursorSize = 64;
	RenderTools()->DrawSprite(m_pClient->m_pControls->m_TargetPos.x, m_pClient->m_pControls->m_TargetPos.y, CursorSize);
	Graphics()->QuadsEnd();
}

void CHud::RenderHealthAndAmmo(const CNetObj_Character *pCharacter)
{
	if(!pCharacter)
		return;

	//mapscreen_to_group(gacenter_x, center_y, layers_game_group());

	float x = 30; // 16
	float y = 5;

	
	CustomStuff()->m_SelectedGroup = pCharacter->m_SelectedGroup+1;
	
	// render ammo count
	// render gui stuff
	if (aCustomWeapon[pCharacter->m_Weapon%NUM_WEAPONS].m_MaxAmmo > 0)
	{
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_PICKUPS].m_Id);

		Graphics()->QuadsBegin();

		// if weaponstage is active, put a "glow" around the stage ammo
		RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[pCharacter->m_Weapon%NUM_WEAPONS].m_pSpritePickup);
		IGraphics::CQuadItem Array[30];
		int i;
		
		float d = 30.0f / aCustomWeapon[pCharacter->m_Weapon%NUM_WEAPONS].m_MaxAmmo;
		
		for (i = 0; i < min(pCharacter->m_AmmoCount, 30); i++)
			Array[i] = IGraphics::CQuadItem(x+i*4*d,y+12, 6,12);
		Graphics()->QuadsDrawTL(Array, i);
		Graphics()->QuadsEnd();
	}

	
	// new health bar
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_HP].m_Id);
	Graphics()->QuadsBegin();
	
	vec2 HpSize = vec2(120, 12);
	
	{ // hp fill
		float f = min(pCharacter->m_Health, 10) / 10.0f;
		Graphics()->SetColor(1, 0, 0, 1);
		Graphics()->QuadsSetSubsetFree(0, 0.5f, 1*f, 0.5f, 0, 1, 1*f, 1);

		IGraphics::CFreeformItem FreeFormItem(
			x, y,
			x+f*HpSize.x, y,
			x, y+HpSize.y,
			x+f*HpSize.x, y+HpSize.y);

		Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
	}
	
	{ // hp frame
		Graphics()->SetColor(1, 1, 1, 1);
		Graphics()->QuadsSetSubsetFree(0, 0, 1, 0, 0, 0.5f, 1, 0.5f); // nice way to pick a sprite

		IGraphics::CFreeformItem FreeFormItem(
			x, y,
			x+HpSize.x, y,
			x, y+HpSize.y,
			x+HpSize.x, y+HpSize.y);

		Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
	}
	
	Graphics()->QuadsEnd();
	
	
	// new jetpack meter
	x = -1;
	y = -1;
	
	vec2 FrameSize = vec2(32, 32);
	int Fuel = pCharacter->m_JetpackPower;
	
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_CIRCULAR].m_Id);
	Graphics()->QuadsBegin();
	// fuel
	if (Fuel >= 50)
	{
		float a = (100-Fuel)*3.6f * RAD / 2;
		float f = 1-abs(cos(a));
	
		vec2 p0 = vec2(f, f*0.5f); // f*0.5f
		vec2 p1 = vec2(1, p0.y);
		vec2 p2 = vec2(0, 0.5f);
		vec2 p3 = vec2(1, 0.5f);
		
		Graphics()->SetColor(1, Fuel*0.01f, 0, 1);
		Graphics()->QuadsSetSubsetFree(	p0.x, p0.y,
										p1.x, p1.y,
										p2.x, p2.y,
										p3.x, p3.y);
		{
		IGraphics::CFreeformItem FreeFormItem(
			x+FrameSize.x/2+FrameSize.x/2*f, y+FrameSize.y/2*f,
			x+FrameSize.x, y+FrameSize.y/2*f,
			x+FrameSize.x/2, y+FrameSize.y/2,
			x+FrameSize.x, y+FrameSize.y/2);

		Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
		}
		
		// full lower
		Graphics()->QuadsSetSubsetFree(0, 0.5f, 1, 0.5f, 0, 1, 1, 1);

		{
		IGraphics::CFreeformItem FreeFormItem(
			x+FrameSize.x/2, y+FrameSize.y/2,
			x+FrameSize.x, y+FrameSize.y/2,
			x+FrameSize.x/2, y+FrameSize.y,
			x+FrameSize.x, y+FrameSize.y);
			
		Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
		}
	}
	else if (Fuel > 0)
	{
		// only lower part
		float a = Fuel*3.6f * RAD / 2;
		float f = abs(cos(a));
	
		vec2 p0 = vec2(0, 0.5f);
		vec2 p1 = vec2(1-f, 0.5f + f*0.5f);
		vec2 p2 = vec2(0, 1);
		vec2 p3 = vec2(p1.x, 1);
		
		Graphics()->SetColor(1, Fuel*0.01f, 0, 1);
		Graphics()->QuadsSetSubsetFree(	p0.x, p0.y,
										p1.x, p1.y,
										p2.x, p2.y,
										p3.x, p3.y);
										
		{
		IGraphics::CFreeformItem FreeFormItem(
			x+FrameSize.x/2, y+FrameSize.y/2,
			x+FrameSize.x-FrameSize.x/2*f, y+FrameSize.y/2+FrameSize.y/2*f,
			x+FrameSize.x/2, y+FrameSize.y,
			x+FrameSize.x-FrameSize.x/2*f, y+FrameSize.y);

		Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
		}
	}
	Graphics()->QuadsEnd();
	
	

	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_FUEL].m_Id);
	Graphics()->QuadsBegin();
	{ // frame
		Graphics()->SetColor(1, 1, 1, 1);
		Graphics()->QuadsSetSubsetFree(0, 0, 1, 0, 0, 1, 1, 1);

		IGraphics::CFreeformItem FreeFormItem(
			x, y,
			x+FrameSize.x, y,
			x, y+FrameSize.y,
			x+FrameSize.x, y+FrameSize.y);

		Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
	}
	Graphics()->QuadsEnd();

	

	x = 30; // 16
	y = 5;
	

	
	// buildings
	if (m_pClient->m_pControls->m_BuildMode)
	{
		float Size = 0.2f;
		y += 20;
		x += 12;
		
		for (int i = 0; i < NUM_KITS; i++)
		{
			if (i != 0)
				x += 180*Size;
			
			int Cost = BuildableCost[i];
			
			// draw building
			Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
			Graphics()->QuadsBegin();
	
			Graphics()->SetColor(1, 1, 1, 1);
			
			RenderTools()->SelectSprite(SPRITE_KIT_BARREL+i);
			RenderTools()->DrawSprite(x, y, 20);

			Graphics()->QuadsEnd();
			
			
			if (m_pClient->m_pControls->m_SelectedBuilding == i+1)
			{
				Graphics()->ShaderBegin(SHADER_GRAYSCALE, 0.0f);
				Graphics()->QuadsBegin();
				
				if (Cost > CustomStuff()->m_LocalKits)
					Graphics()->SetColor(1, 0, 0, 0.7f);
				else
					Graphics()->SetColor(0, 1, 0, 0.7f);
				
				RenderTools()->SelectSprite(SPRITE_KIT_BARREL+i);
				RenderTools()->DrawSprite(x, y, 20);
				
				Graphics()->QuadsEnd();
				Graphics()->ShaderEnd();
			}
			
			// draw cost
			char aBuf[8];
			str_format(aBuf, sizeof(aBuf), "%d", Cost);
			
			if (Cost > CustomStuff()->m_LocalKits)
				TextRender()->TextColor(1, 0, 0, 1);
			else
				TextRender()->TextColor(0, 1, 0, 1);
			
			TextRender()->Text(0, x+6, y+2, 6, aBuf, -1);
			TextRender()->TextColor(1, 1, 1, 1);
			
			Graphics()->TextureSet(g_pData->m_aImages[IMAGE_WEAPONS].m_Id);
			Graphics()->QuadsBegin();
			Graphics()->SetColor(1, 1, 1, 1.0f);
			RenderTools()->SelectSprite(SPRITE_PICKUP_KIT);
			RenderTools()->DrawSprite(x+14, y+6.5f, 8);
			Graphics()->QuadsEnd();
		}
		
		x = 164;
		y = 16;
		
		// back to weapon helper
		int iw = clamp(CustomStuff()->m_LatestWeapon, 0, NUM_WEAPONS-1);
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_WEAPONS].m_Id);
		Graphics()->QuadsBegin();

		Graphics()->SetColor(1, 1, 1, 1);

		RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[iw].m_pSpriteBody);
		RenderTools()->DrawSprite(x, y, 24);
		Graphics()->QuadsEnd();
		

		TextRender()->TextColor(0.2f, 0.7f, 0.2f, 1);
		TextRender()->Text(0, x+9, y+1, 8, m_pClient->m_pBinds->GetKey("+build"), -1);
		TextRender()->TextColor(1, 1, 1, 1);
		
		x = 194;
		y = 16;
		
		// number of contruction kits
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_WEAPONS].m_Id);
		Graphics()->QuadsBegin();
		Graphics()->SetColor(1, 1, 1, 1.0f);
		RenderTools()->SelectSprite(SPRITE_PICKUP_KIT);
		RenderTools()->DrawSprite(x, y, 20);
		Graphics()->QuadsEnd();
		
		char aBuf[8];
		str_format(aBuf, sizeof(aBuf), "%d", clamp(CustomStuff()->m_LocalKits ,0, 9));
		TextRender()->TextColor(0.2f, 0.7f, 0.2f, 1);
		TextRender()->Text(0, x+7, y+2, 8, aBuf, -1);
		TextRender()->TextColor(1, 1, 1, 1);

		return;
	}
	
	
	y += 31;
	
	
	// weapons
	float Size = 0.2f;
	int iw = pCharacter->m_WeaponGroup1;

	if (m_pClient->m_pControls->m_SignalWeapon >= 0)
	{
		CustomStuff()->m_WeaponSignalTimer = 1.0f;
		CustomStuff()->m_WeaponSignal = m_pClient->m_pControls->m_SignalWeapon;
		m_pClient->m_pControls->m_SignalWeapon = -1;
	}
	
	for (int i = 1; i < NUM_WEAPONS; i++)
	{
		if (i != 1)
			x += 40*Size;
		
		bool Got = true;
		
		int w = CustomStuff()->m_LocalWeapons;
		if (!(w & (1<<(i))))
			Got = false;
		
		if (CustomStuff()->m_WeaponpickTimer > 0.0f)
		{
			int pw = clamp(CustomStuff()->m_WeaponpickWeapon, 0, NUM_WEAPONS-1);
			if (i == pw)
			{
				Graphics()->QuadsBegin();
				float a = sin(CustomStuff()->m_WeaponpickTimer*pi)*sin(CustomStuff()->m_WeaponpickTimer*pi);
				
				Graphics()->SetColor(1, 1, 1, a);
				
				RenderTools()->SelectSprite(SPRITE_WEAPON_PICKUP);
				RenderTools()->DrawSprite(x, y, 32);
				Graphics()->QuadsEnd();
			}
		}

		if (!Got)
			Graphics()->ShaderBegin(SHADER_GRAYSCALE, 0.5f);
		
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_WEAPONS].m_Id);
		Graphics()->QuadsBegin();
		
		if (!Got)
			Graphics()->SetColor(1, 1, 1, 0.5f);
		else
			Graphics()->SetColor(1, 1, 1, 1);
		
		RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[i].m_pSpriteBody);
		RenderTools()->DrawSprite(x, y, g_pData->m_Weapons.m_aId[i].m_VisualSize * Size);

		Graphics()->QuadsEnd();
		Graphics()->ShaderEnd();

		// can't pick a weapon
		if (CustomStuff()->m_WeaponSignalTimer > 0.0f)
		{
			int pw = CustomStuff()->m_WeaponSignal;
			if (i == pw)
			{
				Graphics()->ShaderBegin(SHADER_GRAYSCALE, 0.0f);
				Graphics()->QuadsBegin();
				
				Graphics()->SetColor(1.0f, 0.0f, 0.0f, CustomStuff()->m_WeaponSignalTimer * 0.75f);
				
				RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[i].m_pSpriteBody);
				RenderTools()->DrawSprite(x, y, g_pData->m_Weapons.m_aId[i].m_VisualSize * Size);
				
				Graphics()->QuadsEnd();
				Graphics()->ShaderEnd();
			}
		}
		
		// green layer for selected weapon
		if (iw == i)
		{
			Graphics()->ShaderBegin(SHADER_GRAYSCALE, 0.0f);
			Graphics()->QuadsBegin();
			
			Graphics()->SetColor(0.0f, 1.0f, 0.0f, 0.5f);
			
			RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[i].m_pSpriteBody);
			RenderTools()->DrawSprite(x, y, g_pData->m_Weapons.m_aId[i].m_VisualSize * Size);
			
			Graphics()->QuadsEnd();
			Graphics()->ShaderEnd();
			
			CustomStuff()->m_LatestWeapon = i;
		}
		
		x += 40*Size;
	}

	x = 164; y = 16;
	
	// tool / build helper
	if (!m_pClient->IsLocalUndead())
	{
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_WEAPONS].m_Id);
		Graphics()->QuadsBegin();

		Graphics()->SetColor(1, 1, 1, 1);

		RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[WEAPON_TOOL].m_pSpriteBody);
		RenderTools()->DrawSprite(x, y, 24);
		Graphics()->QuadsEnd();
		

		TextRender()->TextColor(0.2f, 0.7f, 0.2f, 1);
		TextRender()->Text(0, x+9, y+1, 8, m_pClient->m_pBinds->GetKey("+build"), -1);
	
		x = 194;
		y = 16;
			
		// number of contruction kits
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_WEAPONS].m_Id);
		Graphics()->QuadsBegin();
		Graphics()->SetColor(1, 1, 1, 1.0f);
		RenderTools()->SelectSprite(SPRITE_PICKUP_KIT);
		RenderTools()->DrawSprite(x, y, 20);
		Graphics()->QuadsEnd();


		char aBuf[8];
		str_format(aBuf, sizeof(aBuf), "%d", clamp(CustomStuff()->m_LocalKits ,0, 9));
		TextRender()->TextColor(0.2f, 0.7f, 0.2f, 1);
		TextRender()->Text(0, x+7, y+2, 8, aBuf, -1);
		TextRender()->TextColor(1, 1, 1, 1);
	}
}

void CHud::RenderSpectatorHud()
{
	// draw the box
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0.0f, 0.0f, 0.0f, 0.4f);
	RenderTools()->DrawRoundRectExt(m_Width-180.0f, m_Height-15.0f, 180.0f, 15.0f, 5.0f, CUI::CORNER_TL);
	Graphics()->QuadsEnd();

	// draw the text
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "%s: %s", Localize("Spectate"), m_pClient->m_Snap.m_SpecInfo.m_SpectatorID != SPEC_FREEVIEW ?
		m_pClient->m_aClients[m_pClient->m_Snap.m_SpecInfo.m_SpectatorID].m_aName : Localize("Free-View"));
	TextRender()->Text(0, m_Width-174.0f, m_Height-13.0f, 8.0f, aBuf, -1);
}

void CHud::OnRender()
{
	if(!m_pClient->m_Snap.m_pGameInfoObj)
		return;

	m_Width = 300.0f*Graphics()->ScreenAspect();
	m_Height = 300.0f;
	Graphics()->MapScreen(0.0f, 0.0f, m_Width, m_Height);

	if(g_Config.m_ClShowhud)
	{
		if(m_pClient->m_Snap.m_pLocalCharacter && !(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER))
			RenderHealthAndAmmo(m_pClient->m_Snap.m_pLocalCharacter);
		else if(m_pClient->m_Snap.m_SpecInfo.m_Active)
		{
			if(m_pClient->m_Snap.m_SpecInfo.m_SpectatorID != SPEC_FREEVIEW)
				RenderHealthAndAmmo(&m_pClient->m_Snap.m_aCharacters[m_pClient->m_Snap.m_SpecInfo.m_SpectatorID].m_Cur);
			RenderSpectatorHud();
		}

		RenderGameTimer();
		RenderPauseNotification();
		RenderSuddenDeath();
		RenderScoreHud();
		RenderWarmupTimer();
		RenderFps();
		if(Client()->State() != IClient::STATE_DEMOPLAYBACK)
			RenderConnectionWarning();
		RenderTeambalanceWarning();
		RenderVoting();
	}
	RenderCursor();
}
