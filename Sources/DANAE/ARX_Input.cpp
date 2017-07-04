/*
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/
//////////////////////////////////////////////////////////////////////////////////////
//   @@        @@@        @@@                @@                           @@@@@     //
//   @@@       @@@@@@     @@@     @@        @@@@                         @@@  @@@   //
//   @@@       @@@@@@@    @@@    @@@@       @@@@      @@                @@@@        //
//   @@@       @@  @@@@   @@@  @@@@@       @@@@@@     @@@               @@@         //
//  @@@@@      @@  @@@@   @@@ @@@@@        @@@@@@@    @@@            @  @@@         //
//  @@@@@      @@  @@@@  @@@@@@@@         @@@@ @@@    @@@@@         @@ @@@@@@@      //
//  @@ @@@     @@  @@@@  @@@@@@@          @@@  @@@    @@@@@@        @@ @@@@         //
// @@@ @@@    @@@ @@@@   @@@@@            @@@@@@@@@   @@@@@@@      @@@ @@@@         //
// @@@ @@@@   @@@@@@@    @@@@@@           @@@  @@@@   @@@ @@@      @@@ @@@@         //
// @@@@@@@@   @@@@@      @@@@@@@@@@      @@@    @@@   @@@  @@@    @@@  @@@@@        //
// @@@  @@@@  @@@@       @@@  @@@@@@@    @@@    @@@   @@@@  @@@  @@@@  @@@@@        //
//@@@   @@@@  @@@@@      @@@      @@@@@@ @@     @@@   @@@@   @@@@@@@    @@@@@ @@@@@ //
//@@@   @@@@@ @@@@@     @@@@        @@@  @@      @@   @@@@   @@@@@@@    @@@@@@@@@   //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@      @@@@@      //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@       @@        //
//@@@    @@@  @@@ @@@@@                          @@            @@@                  //
//            @@@ @@@                           @@             @@        STUDIOS    //
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
// ARX_Input
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Input interface (uses MERCURY input system)
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////


#include "ARX_Input.h"
#include "Arx_menu2.h" //controls
#include "ARX_Loc.h"
#include "Mercury_dx_input.h"
#include "ARX_Common.h"
#include "Danae.h"
#include "EERIEmath.h"
#include "EERIEDraw.h"
#include "ARX_Time.h"
#include "EERIERenderer.h"
#include "ARX_Interface.h"

#include <stdio.h>
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

extern long LastEERIEMouseButton;
extern long _EERIEMouseXdep;
extern long _EERIEMouseYdep;
extern float ARXDiffTimeMenu;
extern DANAE danaeApp;
extern CRenderApplication* g_pRenderApp;
extern TextureContainer * scursor[];

bool bGLOBAL_DINPUT_MENU = true;
bool bGLOBAL_DINPUT_GAME = true;


DXI_INIT	InputInit;
long ARX_SCID = 0;
long ARX_XBOXPAD = 0;

//-----------------------------------------------------------------------------
extern ARXInputHandler * pInputHandler;
extern CMenuConfig * pMenuConfig;
extern long STOP_KEYBOARD_INPUT;

//-----------------------------------------------------------------------------
BOOL ARX_INPUT_Init(HINSTANCE hInst, HWND hWnd)
{
#ifdef NO_DIRECT_INPUT
	return TRUE;
#endif
	memset((void *)&InputInit, 0, sizeof(DXI_INIT));
	DXI_Init(hInst, &InputInit);

	if (DXI_FAIL == DXI_GetKeyboardInputDevice(hWnd, DXI_KEYBOARD1, DXI_MODE_NONEXCLUSIF_OURMSG)) 
		return FALSE;

	if (DXI_FAIL == DXI_GetMouseInputDevice(hWnd, DXI_MOUSE1, DXI_MODE_NONEXCLUSIF_ALLMSG, 2, 2))
		return FALSE;

	if (DXI_FAIL == DXI_GetSCIDInputDevice(hWnd, DXI_SCID, DXI_MODE_EXCLUSIF_OURMSG, 2, 2))
		ARX_SCID = 0;
	else ARX_SCID = 1;

	if (DXI_FAIL == DXI_SetMouseRelative(DXI_MOUSE1))
		return FALSE;

	//"ThrustMaster FireStorm(TM) Dual Power Gamepad"
	if (DXI_FAIL != DXI_GetJoyInputDevice(hWnd, DXI_JOY1, DXI_MODE_EXCLUSIF_ALLMSG, 13, 4))
	{
		ARX_XBOXPAD = 1;
		DXI_SetRangeJoy(DXI_JOY1, DXI_XAxis, 100);
		DXI_SetRangeJoy(DXI_JOY1, DXI_YAxis, 100);
		DXI_SetRangeJoy(DXI_JOY1, DXI_RzAxis, 100);
		DXI_SetRangeJoy(DXI_JOY1, DXI_Slider, 100);
	}
	else ARX_XBOXPAD = 0;

	return TRUE;
}

void ARX_INPUT_Release()
{
#ifdef NO_DIRECT_INPUT
	return;
#endif
	DXI_Release();
}

//ARX_GAME_IMPULSES GameImpulses;
long GameImpulses[MAX_IMPULSES][MAX_IMPULSES_NB];


void ARX_INPUT_Init_Game_Impulses()
{
	for (long i = 0; i < MAX_IMPULSES; i++)
		for (long j = 0; j < MAX_IMPULSES_NB; j++)
			GameImpulses[i][j] = 0;

	GameImpulses[ARX_INPUT_IMPULSE_MAGIC_MODE][0] = 29;
	GameImpulses[ARX_INPUT_IMPULSE_MAGIC_MODE][1] = 157;
	GameImpulses[ARX_INPUT_IMPULSE_MAGIC_MODE][2] = 0; 
	GameImpulses[ARX_INPUT_IMPULSE_COMBAT_MODE][0] = 28;
	GameImpulses[ARX_INPUT_IMPULSE_COMBAT_MODE][1] = INTERNAL_JOYSTICK_7;
	GameImpulses[ARX_INPUT_IMPULSE_JUMP][0] = 82;
	GameImpulses[ARX_INPUT_IMPULSE_JUMP][1] = INTERNAL_MOUSE_3;
	GameImpulses[ARX_INPUT_IMPULSE_JUMP][2] = INTERNAL_JOYSTICK_1;
	GameImpulses[ARX_INPUT_IMPULSE_STEALTH][0] = 54;
	GameImpulses[ARX_INPUT_IMPULSE_STEALTH][1] = 42;

	GameImpulses[ARX_INPUT_IMPULSE_WALK_FORWARD][0] = 200;
	GameImpulses[ARX_INPUT_IMPULSE_WALK_FORWARD][1] = 0;
	GameImpulses[ARX_INPUT_IMPULSE_WALK_BACKWARD][0] = 208;
	GameImpulses[ARX_INPUT_IMPULSE_WALK_BACKWARD][1] = 0;
	GameImpulses[ARX_INPUT_IMPULSE_STRAFE_LEFT][0] = 203;
	GameImpulses[ARX_INPUT_IMPULSE_STRAFE_LEFT][1] = 0;
	GameImpulses[ARX_INPUT_IMPULSE_STRAFE_RIGHT][0] = 205;
	GameImpulses[ARX_INPUT_IMPULSE_STRAFE_RIGHT][1] = 0;

	GameImpulses[ARX_INPUT_IMPULSE_MOUSE_LOOK][0] = INTERNAL_MOUSE_2;
	GameImpulses[ARX_INPUT_IMPULSE_MOUSE_LOOK][1] = 0;

	GameImpulses[ARX_INPUT_IMPULSE_ACTION][0] = INTERNAL_MOUSE_1;
	GameImpulses[ARX_INPUT_IMPULSE_ACTION][1] = INTERNAL_JOYSTICK_13;

	GameImpulses[ARX_INPUT_IMPULSE_INVENTORY][0] = 23;
	GameImpulses[ARX_INPUT_IMPULSE_INVENTORY][1] = INTERNAL_JOYSTICK_8;

	GameImpulses[ARX_INPUT_IMPULSE_BOOK][0] = 15;
	GameImpulses[ARX_INPUT_IMPULSE_BOOK][1] = INTERNAL_JOYSTICK_6;

	GameImpulses[ARX_INPUT_IMPULSE_LEAN_RIGHT][0] = 49;
	GameImpulses[ARX_INPUT_IMPULSE_LEAN_RIGHT][1] = 0;

	GameImpulses[ARX_INPUT_IMPULSE_LEAN_LEFT][0] = 51;
	GameImpulses[ARX_INPUT_IMPULSE_LEAN_LEFT][1] = 0;

	GameImpulses[ARX_INPUT_IMPULSE_CROUCH][0] = 83; //50;
	GameImpulses[ARX_INPUT_IMPULSE_CROUCH][1] = INTERNAL_JOYSTICK_2;
}
 
BOOL ARX_INPUT_GetSCIDAxis(int * jx, int * jy, int * jz)
{
	if (ARX_SCID)
	{
		DXI_GetSCIDAxis(DXI_SCID, jx, jy, jz);
		return TRUE;
	}
	else
	{
		*jx = 0;
		*jy = 0;
		*jz = 0;
		return FALSE;
	}
}
 
//-----------------------------------------------------------------------------
BOOL ARX_IMPULSE_NowPressed(long ident)
{
	switch (ident)
	{
		case CONTROLS_CUST_MOUSELOOK:
		case CONTROLS_CUST_ACTION:
			break;
		default:
		{
			for (long j = 0; j < 2; j++)
			{
				if (pMenuConfig->sakActionKey[ident].iKey[j] != -1)
				{
					if (pMenuConfig->sakActionKey[ident].iKey[j] & 0x80000000)
					{
						if (pInputHandler->GetMouseButtonNowPressed(pMenuConfig->sakActionKey[ident].iKey[j]&~0x80000000))
							return TRUE;
					}
					else
					{
						if (pMenuConfig->sakActionKey[ident].iKey[j] & 0x40000000)
						{
							if (pMenuConfig->sakActionKey[ident].iKey[j] == 0x40000001)
							{
								if (pInputHandler->iWheelSens < 0) return TRUE;
							}
							else
							{
								if (pInputHandler->iWheelSens > 0) return TRUE;
							}
						}
						else
						{
							bool bCombine = true;

							if (pMenuConfig->sakActionKey[ident].iKey[j] & 0x7FFF0000)
							{
								if (!pInputHandler->IsVirtualKeyPressed((pMenuConfig->sakActionKey[ident].iKey[j] >> 16) & 0xFFFF))
									bCombine = false;
							}

							if (pInputHandler->IsVirtualKeyPressedNowPressed(pMenuConfig->sakActionKey[ident].iKey[j] & 0xFFFF))
								return TRUE & bCombine;
						}
					}
				}
			}
		}
	}

	return FALSE;
}

//-----------------------------------------------------------------------------
static unsigned int uiOneHandedMagicMode = 0;
static unsigned int uiOneHandedStealth = 0;

BOOL ARX_IMPULSE_Pressed(long ident)
{
	switch (ident)
	{
		case CONTROLS_CUST_MOUSELOOK:
		case CONTROLS_CUST_ACTION:
			break;
		default:
		{
			if (pMenuConfig->bOneHanded)
			{
				for (long j = 0; j < 2; j++)
				{
					if (pMenuConfig->sakActionKey[ident].iKey[j] != -1)
					{
						if (pMenuConfig->sakActionKey[ident].iKey[j] & 0x80000000)
						{
							if (pInputHandler->GetMouseButtonRepeat(pMenuConfig->sakActionKey[ident].iKey[j]&~0x80000000))
								return TRUE;
						}
						else
						{
							if (pMenuConfig->sakActionKey[ident].iKey[j] & 0x40000000)
							{
								if (pMenuConfig->sakActionKey[ident].iKey[j] == 0x40000001)
								{
									if (pInputHandler->iWheelSens < 0) return TRUE;
								}
								else
								{
									if (pInputHandler->iWheelSens > 0) return TRUE;
								}
							}
							else
							{
								bool bCombine = true;

								if (pMenuConfig->sakActionKey[ident].iKey[j] & 0x7FFF0000)
								{
									if (!pInputHandler->IsVirtualKeyPressed((pMenuConfig->sakActionKey[ident].iKey[j] >> 16) & 0xFFFF))
										bCombine = false;
								}

								if (pInputHandler->IsVirtualKeyPressed(pMenuConfig->sakActionKey[ident].iKey[j] & 0xFFFF))
								{
									bool bQuit = false;

									switch (ident)
									{
										case CONTROLS_CUST_MAGICMODE:
										{
											if (bCombine)
											{
												if (!uiOneHandedMagicMode)
												{
													uiOneHandedMagicMode = 1;
												}
												else
												{
													if (uiOneHandedMagicMode == 2)
													{
														uiOneHandedMagicMode = 3;
													}
												}

												bQuit = true;
											}
										}
										break;
										case CONTROLS_CUST_STEALTHMODE:
										{
											if (bCombine)
											{
												if (!uiOneHandedStealth)
												{
													uiOneHandedStealth = 1;
												}
												else
												{
													if (uiOneHandedStealth == 2)
													{
														uiOneHandedStealth = 3;
													}
												}

												bQuit = true;
											}
										}
										break;
										default:
										{
											return TRUE & bCombine;
										}
										break;
									}

									if (bQuit)
									{
										break;
									}
								}
								else
								{
									switch (ident)
									{
										case CONTROLS_CUST_MAGICMODE:
										{
											if ((!j) &&
											        (pInputHandler->IsVirtualKeyPressed(pMenuConfig->sakActionKey[ident].iKey[j+1] & 0xFFFF)))
											{
												continue;
											}

											if (uiOneHandedMagicMode == 1)
											{
												uiOneHandedMagicMode = 2;
											}
											else
											{
												if (uiOneHandedMagicMode == 3)
												{
													uiOneHandedMagicMode = 0;
												}
											}
										}
										break;
										case CONTROLS_CUST_STEALTHMODE:
										{
											if ((!j) &&
											        (pInputHandler->IsVirtualKeyPressed(pMenuConfig->sakActionKey[ident].iKey[j+1] & 0xFFFF)))
											{
												continue;
											}

											if (uiOneHandedStealth == 1)
											{
												uiOneHandedStealth = 2;
											}
											else
											{
												if (uiOneHandedStealth == 3)
												{
													uiOneHandedStealth = 0;
												}
											}
										}
										break;
									}
								}
							}
						}
					}
				}

				switch (ident)
				{
					case CONTROLS_CUST_MAGICMODE:

						if ((uiOneHandedMagicMode == 1) || (uiOneHandedMagicMode == 2))
						{
							return TRUE;
						}

						break;
					case CONTROLS_CUST_STEALTHMODE:

						if ((uiOneHandedStealth == 1) || (uiOneHandedStealth == 2))
						{
							return TRUE;
						}

						break;
				}
			}
			else
			{
				for (long j = 0; j < 2; j++)
				{
					if (pMenuConfig->sakActionKey[ident].iKey[j] != -1)
					{
						if (pMenuConfig->sakActionKey[ident].iKey[j] & 0x80000000)
						{
							if (pInputHandler->GetMouseButtonRepeat(pMenuConfig->sakActionKey[ident].iKey[j]&~0x80000000))
								return TRUE;
						}
						else
						{
							if (pMenuConfig->sakActionKey[ident].iKey[j] & 0x40000000)
							{
								if (pMenuConfig->sakActionKey[ident].iKey[j] == 0x40000001)
								{
									if (pInputHandler->iWheelSens < 0) return TRUE;
								}
								else
								{
									if (pInputHandler->iWheelSens > 0) return TRUE;
								}
							}
							else
							{
								bool bCombine = true;

								if (pMenuConfig->sakActionKey[ident].iKey[j] & 0x7FFF0000)
								{
									if (!pInputHandler->IsVirtualKeyPressed((pMenuConfig->sakActionKey[ident].iKey[j] >> 16) & 0xFFFF))
										bCombine = false;
								}

								if (pInputHandler->IsVirtualKeyPressed(pMenuConfig->sakActionKey[ident].iKey[j] & 0xFFFF))
									return TRUE & bCombine;
							}
						}
					}
				}
			}
		}
	}

	return FALSE;
}

//-----------------------------------------------------------------------------
BOOL ARX_IMPULSE_NowUnPressed(long ident)
{
	switch (ident)
	{
		case CONTROLS_CUST_MOUSELOOK:
		case CONTROLS_CUST_ACTION:
			break;
		default:
		{
			for (long j = 0; j < 2; j++)
			{
				if (pMenuConfig->sakActionKey[ident].iKey[j] != -1)
				{
					if (pMenuConfig->sakActionKey[ident].iKey[j] & 0x80000000)
					{
						if (pInputHandler->GetMouseButtonNowUnPressed(pMenuConfig->sakActionKey[ident].iKey[j]&~0x80000000))
							return TRUE;
					}
					else
					{
						bool bCombine = true;

						if (pMenuConfig->sakActionKey[ident].iKey[j] & 0x7FFF0000)
						{
							if (!pInputHandler->IsVirtualKeyPressed((pMenuConfig->sakActionKey[ident].iKey[j] >> 16) & 0xFFFF))
								bCombine = false;
						}

						if (pInputHandler->IsVirtualKeyPressedNowUnPressed(pMenuConfig->sakActionKey[ident].iKey[j] & 0xFFFF))
							return TRUE & bCombine;
					}
				}
			}
		}
	}

	return FALSE;
}

//-----------------------------------------------------------------------------
 
ARXInputHandler::ARXInputHandler()
{
	char temp[256];
	MakeDir(temp, "graph\\interface\\cursors\\cursor00.bmp");
	pTex[0] = D3DTextr_GetSurfaceContainer(temp);
	MakeDir(temp, "graph\\interface\\cursors\\cursor01.bmp");
	pTex[1] = D3DTextr_GetSurfaceContainer(temp);
	MakeDir(temp, "graph\\interface\\cursors\\cursor02.bmp");
	pTex[2] = D3DTextr_GetSurfaceContainer(temp);
	MakeDir(temp, "graph\\interface\\cursors\\cursor03.bmp");
	pTex[3] = D3DTextr_GetSurfaceContainer(temp);
	MakeDir(temp, "graph\\interface\\cursors\\cursor04.bmp");
	pTex[4] = D3DTextr_GetSurfaceContainer(temp);
	MakeDir(temp, "graph\\interface\\cursors\\cursor05.bmp");
	pTex[5] = D3DTextr_GetSurfaceContainer(temp);
	MakeDir(temp, "graph\\interface\\cursors\\cursor06.bmp");
	pTex[6] = D3DTextr_GetSurfaceContainer(temp);
	MakeDir(temp, "graph\\interface\\cursors\\cursor07.bmp");
	pTex[7] = D3DTextr_GetSurfaceContainer(temp);

	SetCursorOff();
	SetSensibility(2);
	iMouseAX = 0;
	iMouseAY = 0;
	iMouseAZ = 0;
	fMouseAXTemp = fMouseAYTemp = 0.f;
	iNbOldCoord = 0;
	iMaxOldCoord = 40;

	bMouseOver = false;

	if (pTex[0])
	{
		fTailleX = (float)pTex[0]->m_dwWidth;
		fTailleY = (float)pTex[0]->m_dwHeight;
	}
	else
	{
		fTailleX = fTailleY = 0.f;
	}

	iNumCursor = 0;
	lFrameDiff = 0;

	bDrawCursor = true;
	bActive = false;

	iWheelSens = 0;
}

ARXInputHandler::~ARXInputHandler()
{

}

void ARXInputHandler::SetCursorOff()
{
	eNumTex = CURSOR_OFF;
}

void ARXInputHandler::SetCursorOn()
{
	eNumTex = CURSOR_ON;
}

void ARXInputHandler::SetMouseOver()
{
	bMouseOver = true;
	SetCursorOn();
}

void ARXInputHandler::SetSensibility(int _iSensibility)
{
	iSensibility = _iSensibility;
}

void ARXInputHandler::ResetAll()
{

}

void ARXInputHandler::GetInput()
{

}

int ARXInputHandler::GetWheelSens(int _iIDbutton)
{
	return 0;
}

bool ARXInputHandler::IsVirtualKeyPressed(int _iVirtualKey)
{
	return false;
}

bool ARXInputHandler::IsVirtualKeyPressedOneTouch(int _iVirtualKey)
{
	return false;
}

bool ARXInputHandler::IsVirtualKeyPressedNowPressed(int _iVirtualKey)
{
	return false;
}

bool ARXInputHandler::IsVirtualKeyPressedNowUnPressed(int _iVirtualKey)
{
	return false;
}

_TCHAR * ARXInputHandler::GetFullNameTouch(int _iVirtualKey)
{
	_TCHAR *pText = (_TCHAR*)malloc(256 * sizeof(_TCHAR));

	if (!pText) return NULL;

	long lParam;

	_TCHAR *pText2 = NULL;

	if ((_iVirtualKey != -1) &&
		(_iVirtualKey&~0xC000FFFF))	//COMBINAISON
	{
		pText2 = GetFullNameTouch((_iVirtualKey >> 16) & 0x3FFF);
	}

	lParam = ((_iVirtualKey) & 0x7F) << 16;

	switch (_iVirtualKey)
	{
	case DIK_HOME:

		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_home"), _T("string"), _T("---"), pText, 256, NULL);
		break;
	case DIK_NEXT:

		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_pagedown"), _T("string"), _T("---"), pText, 256, NULL);
		break;
	case DIK_END:

		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_end"), _T("string"), _T("---"), pText, 256, NULL);
		break;
	case DIK_INSERT:

		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_insert"), _T("string"), _T("---"), pText, 256, NULL);
		break;
	case DIK_DELETE:

		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_delete"), _T("string"), _T("---"), pText, 256, NULL);
		break;
	case DIK_NUMLOCK:

		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_numlock"), _T("string"), _T("---"), pText, 256, NULL);
		break;
	case DIK_DIVIDE:
		_tcscpy(pText, _T("_/_"));
		break;
	case DIK_MULTIPLY:
		_tcscpy(pText, _T("_x_"));
		break;
	case DIK_SYSRQ:
		_tcscpy(pText, _T("?"));
		break;
	case DIK_UP:                  // UpArrow on arrow keypad

		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_up"), _T("string"), _T("---"), pText, 256, NULL);
		break;
	case DIK_PRIOR:               // PgUp on arrow keypad

		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_pageup"), _T("string"), _T("---"), pText, 256, NULL);
		break;
	case DIK_LEFT:                // LeftArrow on arrow keypad

		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_left"), _T("string"), _T("---"), pText, 256, NULL);
		break;
	case DIK_RIGHT:               // RightArrow on arrow keypad

		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_right"), _T("string"), _T("---"), pText, 256, NULL);
		break;
	case DIK_DOWN:                // DownArrow on arrow keypad

		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_down"), _T("string"), _T("---"), pText, 256, NULL);
		break;
	case DIK_BUTTON1:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button0"), _T("string"), _T("b1"), pText, 256, NULL);
		break;
	case DIK_BUTTON2:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button1"), _T("string"), _T("b2"), pText, 256, NULL);
		break;
	case DIK_BUTTON3:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button2"), _T("string"), _T("b3"), pText, 256, NULL);
		break;
	case DIK_BUTTON4:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button3"), _T("string"), _T("b4"), pText, 256, NULL);
		break;
	case DIK_BUTTON5:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button4"), _T("string"), _T("b5"), pText, 256, NULL);
		break;
	case DIK_BUTTON6:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button5"), _T("string"), _T("b6"), pText, 256, NULL);
		break;
	case DIK_BUTTON7:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button6"), _T("string"), _T("b7"), pText, 256, NULL);
		break;
	case DIK_BUTTON8:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button7"), _T("string"), _T("b8"), pText, 256, NULL);
		break;
	case DIK_BUTTON9:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button8"), _T("string"), _T("b9"), pText, 256, NULL);
		break;
	case DIK_BUTTON10:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button9"), _T("string"), _T("b10"), pText, 256, NULL);
		break;
	case DIK_BUTTON11:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button10"), _T("string"), _T("b11"), pText, 256, NULL);
		break;
	case DIK_BUTTON12:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button11"), _T("string"), _T("b12"), pText, 256, NULL);
		break;
	case DIK_BUTTON13:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button12"), _T("string"), _T("b13"), pText, 256, NULL);
		break;
	case DIK_BUTTON14:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button13"), _T("string"), _T("b14"), pText, 256, NULL);
		break;
	case DIK_BUTTON15:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button14"), _T("string"), _T("b15"), pText, 256, NULL);
		break;
	case DIK_BUTTON16:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button15"), _T("string"), _T("b16"), pText, 256, NULL);
		break;
	case DIK_BUTTON17:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button16"), _T("string"), _T("b17"), pText, 256, NULL);
		break;
	case DIK_BUTTON18:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button17"), _T("string"), _T("b18"), pText, 256, NULL);
		break;
	case DIK_BUTTON19:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button18"), _T("string"), _T("b19"), pText, 256, NULL);
		break;
	case DIK_BUTTON20:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button19"), _T("string"), _T("b20"), pText, 256, NULL);
		break;
	case DIK_BUTTON21:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button20"), _T("string"), _T("b21"), pText, 256, NULL);
		break;
	case DIK_BUTTON22:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button21"), _T("string"), _T("b22"), pText, 256, NULL);
		break;
	case DIK_BUTTON23:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button22"), _T("string"), _T("b23"), pText, 256, NULL);
		break;
	case DIK_BUTTON24:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button23"), _T("string"), _T("b24"), pText, 256, NULL);
		break;
	case DIK_BUTTON25:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button24"), _T("string"), _T("b25"), pText, 256, NULL);
		break;
	case DIK_BUTTON26:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button25"), _T("string"), _T("b26"), pText, 256, NULL);
		break;
	case DIK_BUTTON27:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button26"), _T("string"), _T("b27"), pText, 256, NULL);
		break;
	case DIK_BUTTON28:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button27"), _T("string"), _T("b28"), pText, 256, NULL);
		break;
	case DIK_BUTTON29:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button28"), _T("string"), _T("b29"), pText, 256, NULL);
		break;
	case DIK_BUTTON30:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button29"), _T("string"), _T("b30"), pText, 256, NULL);
		break;
	case DIK_BUTTON31:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button30"), _T("string"), _T("b31"), pText, 256, NULL);
		break;
	case DIK_BUTTON32:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_button31"), _T("string"), _T("b32"), pText, 256, NULL);
		break;
	case DIK_WHEELUP:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_wheelup"), _T("string"), _T("w0"), pText, 256, NULL);
		break;
	case DIK_WHEELDOWN:
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_wheeldown"), _T("string"), _T("w1"), pText, 256, NULL);
		break;
	case -1:
		_tcscpy(pText, _T("---"));
		break;
	default:
	{
		char tAnsiText[256];
		GetKeyNameText(lParam, tAnsiText, 256);
		int i = strlen(tAnsiText);

		if (!i)
		{
			_stprintf(pText, _T("Key_%d"), _iVirtualKey);
		}
		else
		{
			MultiByteToWideChar(CP_ACP, 0, tAnsiText, -1, pText, 256);

			if (_iVirtualKey == DIK_LSHIFT)
			{
				_TCHAR tText2[256];
				_TCHAR *pText3;
				PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_left"), _T("string"), _T("---"), tText2, 256, NULL);
				tText2[1] = 0;
				pText3 = (_TCHAR*)malloc((_tcslen(tText2) + _tcslen(pText) + 1) * sizeof(_TCHAR));
				_tcscpy(pText3, tText2);
				_tcscat(pText3, pText);
				free((void*)pText);
				pText = pText3;
			}

			if (_iVirtualKey == DIK_LCONTROL)
			{
				_TCHAR tText2[256];
				_TCHAR *pText3;
				PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_left"), _T("string"), _T("---"), tText2, 256, NULL);
				tText2[1] = 0;
				pText3 = (_TCHAR*)malloc((_tcslen(tText2) + _tcslen(pText) + 1) * sizeof(_TCHAR));
				_tcscpy(pText3, tText2);
				_tcscat(pText3, pText);
				free((void*)pText);
				pText = pText3;
			}

			if (_iVirtualKey == DIK_LALT)
			{
				_TCHAR tText2[256];
				_TCHAR *pText3;
				PAK_UNICODE_GetPrivateProfileString(_T("system_menus_options_input_customize_controls_left"), _T("string"), _T("---"), tText2, 256, NULL);
				tText2[1] = 0;
				pText3 = (_TCHAR*)malloc((_tcslen(tText2) + _tcslen(pText) + 1) * sizeof(_TCHAR));
				_tcscpy(pText3, tText2);
				_tcscat(pText3, pText);
				free((void*)pText);
				pText = pText3;
			}

			if (_iVirtualKey == DIK_NUMPADENTER)
			{
				_TCHAR *pText3;
				pText3 = (_TCHAR*)malloc((_tcslen(pText) + 1 + 1) * sizeof(_TCHAR));
				_tcscpy(pText3, pText);
				_tcscat(pText3, _T("0"));
				free((void*)pText);
				pText = pText3;
			}

			if (_tcslen(pText) > 8)
			{
				pText[8] = 0;
				int iI = 8;

				while (iI--)
				{
					if (pText[iI] == _T(' '))
					{
						pText[iI] = 0;
					}
					else break;
				}
			}
		}
	}
	break;
	}

	if (pText2)
	{
		_TCHAR *pText3 = (_TCHAR*)malloc((_tcslen(pText2) + _tcslen(pText) + 1 + 1) * sizeof(_TCHAR));
		_tcscpy(pText3, pText2);
		_tcscat(pText3, _T("+"));
		_tcscat(pText3, pText);
		free((void*)pText);
		free((void*)pText2);
		pText = pText3;

	}

	return pText;

}

void ARXInputHandler::DrawOneCursor(int _iPosX, int _iPosY, int _iColor)
{

}

void ARXInputHandler::DrawCursor()
{
	if (!bDrawCursor) return;

}

bool ARXInputHandler::GetMouseButton(int _iNumButton)
{
	int native = arxMouseToNative(_iNumButton);
	return((bMouseButton[native]) && (!bOldMouseButton[native]));
}

bool ARXInputHandler::GetMouseButtonRepeat(int _iNumButton)
{
	int native = arxMouseToNative(_iNumButton);
	return(bMouseButton[native]);
}

bool ARXInputHandler::GetMouseButtonNowPressed(int _iNumButton)
{
	int native = arxMouseToNative(_iNumButton);
	return((bMouseButton[native]) && (!bOldMouseButton[native]));
}

bool ARXInputHandler::GetMouseButtonNowUnPressed(int _iNumButton)
{
	int native = arxMouseToNative(_iNumButton);
	return((!bMouseButton[native]) && (bOldMouseButton[native]));
}

bool ARXInputHandler::GetMouseButtonDoubleClick(int _iNumButton, int _iTime)
{
	int native = arxMouseToNative(_iNumButton);
	return ((iMouseTimeSet[native] == 2) && (iMouseTime[native] < _iTime));
}



/************************************************/
/*						SDL						*/
/************************************************/


//Map DIK_* to SDL scancodes
SDL_Scancode DIK2SDL[256];
void initDIK2SDLMap()
{
	for (size_t i = 0; i < 256; ++i)
		DIK2SDL[i] = SDL_SCANCODE_UNKNOWN;

	DIK2SDL[DIK_A] = SDL_SCANCODE_A;
	DIK2SDL[DIK_B] = SDL_SCANCODE_B;
	DIK2SDL[DIK_C] = SDL_SCANCODE_C;
	DIK2SDL[DIK_D] = SDL_SCANCODE_D;
	DIK2SDL[DIK_E] = SDL_SCANCODE_E;
	DIK2SDL[DIK_F] = SDL_SCANCODE_F;
	DIK2SDL[DIK_G] = SDL_SCANCODE_G;
	DIK2SDL[DIK_H] = SDL_SCANCODE_H;
	DIK2SDL[DIK_I] = SDL_SCANCODE_I;
	DIK2SDL[DIK_J] = SDL_SCANCODE_J;
	DIK2SDL[DIK_K] = SDL_SCANCODE_K;
	DIK2SDL[DIK_L] = SDL_SCANCODE_L;
	DIK2SDL[DIK_M] = SDL_SCANCODE_M;
	DIK2SDL[DIK_N] = SDL_SCANCODE_N;
	DIK2SDL[DIK_O] = SDL_SCANCODE_O;
	DIK2SDL[DIK_P] = SDL_SCANCODE_P;
	DIK2SDL[DIK_Q] = SDL_SCANCODE_Q;
	DIK2SDL[DIK_R] = SDL_SCANCODE_R;
	DIK2SDL[DIK_S] = SDL_SCANCODE_S;
	DIK2SDL[DIK_T] = SDL_SCANCODE_T;
	DIK2SDL[DIK_U] = SDL_SCANCODE_U;
	DIK2SDL[DIK_V] = SDL_SCANCODE_V;
	DIK2SDL[DIK_W] = SDL_SCANCODE_W;
	DIK2SDL[DIK_X] = SDL_SCANCODE_X;
	DIK2SDL[DIK_Y] = SDL_SCANCODE_Y;
	DIK2SDL[DIK_Z] = SDL_SCANCODE_Z;
	DIK2SDL[DIK_1] = SDL_SCANCODE_1;
	DIK2SDL[DIK_2] = SDL_SCANCODE_2;
	DIK2SDL[DIK_3] = SDL_SCANCODE_3;
	DIK2SDL[DIK_4] = SDL_SCANCODE_4;
	DIK2SDL[DIK_5] = SDL_SCANCODE_5;
	DIK2SDL[DIK_6] = SDL_SCANCODE_6;
	DIK2SDL[DIK_7] = SDL_SCANCODE_7;
	DIK2SDL[DIK_8] = SDL_SCANCODE_8;
	DIK2SDL[DIK_9] = SDL_SCANCODE_9;
	DIK2SDL[DIK_0] = SDL_SCANCODE_0;
	DIK2SDL[DIK_RETURN] = SDL_SCANCODE_RETURN;
	DIK2SDL[DIK_ESCAPE] = SDL_SCANCODE_ESCAPE;
	DIK2SDL[DIK_BACKSPACE] = SDL_SCANCODE_BACKSPACE;
	DIK2SDL[DIK_TAB] = SDL_SCANCODE_TAB;
	DIK2SDL[DIK_SPACE] = SDL_SCANCODE_SPACE;
	DIK2SDL[DIK_MINUS] = SDL_SCANCODE_MINUS;
	DIK2SDL[DIK_EQUALS] = SDL_SCANCODE_EQUALS;
	DIK2SDL[DIK_LBRACKET] = SDL_SCANCODE_LEFTBRACKET;
	DIK2SDL[DIK_RBRACKET] = SDL_SCANCODE_RIGHTBRACKET;
	DIK2SDL[DIK_BACKSLASH] = SDL_SCANCODE_BACKSLASH;
	DIK2SDL[DIK_SEMICOLON] = SDL_SCANCODE_SEMICOLON;
	DIK2SDL[DIK_APOSTROPHE] = SDL_SCANCODE_APOSTROPHE;
	DIK2SDL[DIK_GRAVE] = SDL_SCANCODE_GRAVE;
	DIK2SDL[DIK_COMMA] = SDL_SCANCODE_COMMA;
	DIK2SDL[DIK_PERIOD] = SDL_SCANCODE_PERIOD;
	DIK2SDL[DIK_SLASH] = SDL_SCANCODE_SLASH;
	DIK2SDL[DIK_CAPSLOCK] = SDL_SCANCODE_CAPSLOCK;
	DIK2SDL[DIK_F1] = SDL_SCANCODE_F1;
	DIK2SDL[DIK_F2] = SDL_SCANCODE_F2;
	DIK2SDL[DIK_F3] = SDL_SCANCODE_F3;
	DIK2SDL[DIK_F4] = SDL_SCANCODE_F4;
	DIK2SDL[DIK_F5] = SDL_SCANCODE_F5;
	DIK2SDL[DIK_F6] = SDL_SCANCODE_F6;
	DIK2SDL[DIK_F7] = SDL_SCANCODE_F7;
	DIK2SDL[DIK_F8] = SDL_SCANCODE_F8;
	DIK2SDL[DIK_F9] = SDL_SCANCODE_F9;
	DIK2SDL[DIK_F10] = SDL_SCANCODE_F10;
	DIK2SDL[DIK_F11] = SDL_SCANCODE_F11;
	DIK2SDL[DIK_F12] = SDL_SCANCODE_F12;
	DIK2SDL[DIK_SCROLL] = SDL_SCANCODE_SCROLLLOCK;
	DIK2SDL[DIK_PAUSE] = SDL_SCANCODE_PAUSE;
	DIK2SDL[DIK_INSERT] = SDL_SCANCODE_INSERT;
	DIK2SDL[DIK_HOME] = SDL_SCANCODE_HOME;
	DIK2SDL[DIK_PGUP] = SDL_SCANCODE_PAGEUP;
	DIK2SDL[DIK_DELETE] = SDL_SCANCODE_DELETE;
	DIK2SDL[DIK_END] = SDL_SCANCODE_END;
	DIK2SDL[DIK_PGDN] = SDL_SCANCODE_PAGEDOWN;
	DIK2SDL[DIK_RIGHT] = SDL_SCANCODE_RIGHT;
	DIK2SDL[DIK_LEFT] = SDL_SCANCODE_LEFT;
	DIK2SDL[DIK_DOWN] = SDL_SCANCODE_DOWN;
	DIK2SDL[DIK_UP] = SDL_SCANCODE_UP;
	DIK2SDL[DIK_NUMLOCK] = SDL_SCANCODE_NUMLOCKCLEAR;
	DIK2SDL[DIK_DIVIDE] = SDL_SCANCODE_KP_DIVIDE;
	DIK2SDL[DIK_MULTIPLY] = SDL_SCANCODE_KP_MULTIPLY;
	DIK2SDL[DIK_NUMPADMINUS] = SDL_SCANCODE_KP_MINUS;
	DIK2SDL[DIK_NUMPADPLUS] = SDL_SCANCODE_KP_PLUS;
	DIK2SDL[DIK_NUMPADENTER] = SDL_SCANCODE_KP_ENTER;
	DIK2SDL[DIK_NUMPAD1] = SDL_SCANCODE_KP_1;
	DIK2SDL[DIK_NUMPAD2] = SDL_SCANCODE_KP_2;
	DIK2SDL[DIK_NUMPAD3] = SDL_SCANCODE_KP_3;
	DIK2SDL[DIK_NUMPAD4] = SDL_SCANCODE_KP_4;
	DIK2SDL[DIK_NUMPAD5] = SDL_SCANCODE_KP_5;
	DIK2SDL[DIK_NUMPAD6] = SDL_SCANCODE_KP_6;
	DIK2SDL[DIK_NUMPAD7] = SDL_SCANCODE_KP_7;
	DIK2SDL[DIK_NUMPAD8] = SDL_SCANCODE_KP_8;
	DIK2SDL[DIK_NUMPAD9] = SDL_SCANCODE_KP_9;
	DIK2SDL[DIK_NUMPAD0] = SDL_SCANCODE_KP_0;
	DIK2SDL[DIK_NUMPADPERIOD] = SDL_SCANCODE_KP_PERIOD;
	DIK2SDL[DIK_NUMPADEQUALS] = SDL_SCANCODE_KP_EQUALS;
	DIK2SDL[DIK_F13] = SDL_SCANCODE_F13;
	DIK2SDL[DIK_F14] = SDL_SCANCODE_F14;
	DIK2SDL[DIK_F15] = SDL_SCANCODE_F15;
	DIK2SDL[DIK_LALT] = SDL_SCANCODE_MENU;
	DIK2SDL[DIK_NUMPADCOMMA] = SDL_SCANCODE_KP_COMMA;
	DIK2SDL[DIK_SYSRQ] = SDL_SCANCODE_SYSREQ;	
	DIK2SDL[DIK_LCONTROL] = SDL_SCANCODE_LCTRL;
	DIK2SDL[DIK_LSHIFT] = SDL_SCANCODE_LSHIFT;
	DIK2SDL[DIK_LALT] = SDL_SCANCODE_LALT;
	DIK2SDL[DIK_RCONTROL] = SDL_SCANCODE_RCTRL;
	DIK2SDL[DIK_RSHIFT] = SDL_SCANCODE_RSHIFT;
	DIK2SDL[DIK_RALT] = SDL_SCANCODE_RALT;
}

SDL_Scancode dikToScancode(int dik)
{
	if (dik > 255 || dik < 0) return SDL_SCANCODE_UNKNOWN;

	//SDL_GetScancodeFromName(SDL_GetKeyName((SDL_Keycode)iKeyScanCode[dik]))
	return DIK2SDL[dik];
}

ARXInputHandlerSDL::ARXInputHandlerSDL() : ARXInputHandler()
{
	initDIK2SDLMap();

	for (int i = ARXMOUSE_BUTTON0; i <= ARXMOUSE_BUTTON31; i++)
	{
		iOldMouseButton[i] = 0;
		iMouseTime[i] = 0;
		iMouseTimeSet[i] = 0;
		bMouseButton[i] = bOldMouseButton[i] = false;
		iOldNumClick[i] = iOldNumUnClick[i] = 0;
	}
}



void ARXInputHandlerSDL::ResetAll()
{
	for (int i = ARXMOUSE_BUTTON0; i <= ARXMOUSE_BUTTON31; i++)
	{
		iOldMouseButton[i] = 0;
		iMouseTime[i] = 0;
		iMouseTimeSet[i] = 0;
		bMouseButton[i] = bOldMouseButton[i] = false;
		iOldNumClick[i] = iOldNumUnClick[i] = 0;
	}

	iKeyId = -1;
	bTouch = false;

	for (int i = 0; i < 256; i++)
	{
		iOneTouch[i] = 0;
	}

	EERIEMouseButton = LastEERIEMouseButton = 0;

	iWheelSens = 0;
}

void ARXInputHandlerSDL::GetInput()
{
	int iDTime;

	Uint32 mask = SDL_GetMouseState(&iMouseAX, &iMouseAY);
	EERIEMouseX = iMouseAX;
	EERIEMouseY = iMouseAY;

	bOldMouseButton[SDL_BUTTON_LEFT] = bMouseButton[SDL_BUTTON_LEFT];
	bOldMouseButton[SDL_BUTTON_MIDDLE] = bMouseButton[SDL_BUTTON_MIDDLE];
	bOldMouseButton[SDL_BUTTON_RIGHT] = bMouseButton[SDL_BUTTON_RIGHT];

	ARX_CHECK_INT(ARX_TIME_Get(false));
	const int iArxTime = ARX_CLEAN_WARN_CAST_INT(ARX_TIME_Get(false));

	const bool masks[] = { 
		mask & SDL_BUTTON_LMASK,
		mask & SDL_BUTTON_MMASK,
		mask & SDL_BUTTON_RMASK
	};
	for (int i = 0; i <= 0; i++)
	{
		int iNumClick = 0;
		int iNumUnClick = 0;
		//DXI_MouseButtonCountClick(DXI_MOUSE1, i, &iNumClick, &iNumUnClick);

		iOldNumClick[i] += iNumClick + iNumUnClick;

		if ((!bMouseButton[i]) && (iOldNumClick[i] == iNumUnClick))
		{
			iNumUnClick = iOldNumClick[i] = 0;
		}

		bOldMouseButton[i] = bMouseButton[i];

		if (bMouseButton[i])
		{
			if (iOldNumClick[i])
			{
				bMouseButton[i] = false;
			}
		}
		else
		{
			if (iOldNumClick[i])
			{
				bMouseButton[i] = true;
			}
		}

		if (iOldNumClick[i]) iOldNumClick[i]--;

		iDTime = 0;
		if (masks[i]) iDTime = ARX_TIME_Get(false);

		if (iDTime)
		{
			iMouseTime[i] = iDTime;
			iMouseTimeSet[i] = 2;
		}
		else
		{
			if ((iMouseTimeSet[i] > 0) &&
				((ARX_TIME_Get(false) - iMouseTime[i]) > 300)
				)
			{
				iMouseTime[i] = 0;
				iMouseTimeSet[i] = 0;
			}

			if (masks[i])
			{
				switch (iMouseTimeSet[i])
				{
				case 0:
					iMouseTime[i] = iArxTime;
					iMouseTimeSet[i]++;
					break;
				case 1:
					iMouseTime[i] = iArxTime - iMouseTime[i];
					iMouseTimeSet[i]++;
					break;
				}
			}
		}
	}

	iWheelSens = GetWheelSens(DXI_MOUSE1);

	bMouseButton[SDL_BUTTON_LEFT] = mask & SDL_BUTTON_LMASK;
	bMouseButton[SDL_BUTTON_MIDDLE] = mask & SDL_BUTTON_MMASK;
	bMouseButton[SDL_BUTTON_RIGHT] = mask & SDL_BUTTON_RMASK;

	bMouseMove = ((iMouseAX != DANAEMouse.x) || (iMouseAY != DANAEMouse.y));
	iMouseAX = DANAEMouse.x;
	iMouseAY = DANAEMouse.y;
	iMouseAZ = 0;

	keystate = SDL_GetKeyboardState(0);

	SDL_Scancode scancode;
	for (int i = 0; i < 256; ++i)
	{
		scancode = dikToScancode(i);

		if (keystate[scancode])
		{
			if (iOneTouch[scancode] < 2)
			{
				iOneTouch[scancode]++;
			}
		}
		else
		{
			if (iOneTouch[scancode] > 0)
			{
				iOneTouch[scancode]--;
			}
		}
	}
}

void ARXInputHandlerSDL::DrawCursor()
{
	if (!bDrawCursor) return;

	DrawOneCursor(iMouseAX, iMouseAY, 0);

	ARX_CHECK_LONG(ARXDiffTimeMenu);
	lFrameDiff += ARX_CLEAN_WARN_CAST_LONG(ARXDiffTimeMenu);


	if (lFrameDiff > 70)
	{
		if (bMouseOver)
		{
			if (iNumCursor < 4)
			{
				iNumCursor++;
			}
			else
			{
				if (iNumCursor > 4)
				{
					iNumCursor--;
				}
			}

			SetCursorOff();
			bMouseOver = false;
		}
		else
		{
			if (iNumCursor > 0)
			{
				iNumCursor++;

				if (iNumCursor > 7) iNumCursor = 0;
			}
		}

		lFrameDiff = 0;
	}

}

bool ARXInputHandlerSDL::IsVirtualKeyPressed(int _iVirtualKey)
{
	SDL_Scancode scancode = dikToScancode(_iVirtualKey);
	return (keystate[scancode]);
}

bool ARXInputHandlerSDL::IsVirtualKeyPressedOneTouch(int _iVirtualKey)
{
	SDL_Scancode scancode = dikToScancode(_iVirtualKey);
	return (keystate[scancode] && iOneTouch[scancode]);
}

bool ARXInputHandlerSDL::IsVirtualKeyPressedNowPressed(int _iVirtualKey)
{
	SDL_Scancode scancode = dikToScancode(_iVirtualKey);
	return (keystate[scancode] && iOneTouch[scancode]);
}

bool ARXInputHandlerSDL::IsVirtualKeyPressedNowUnPressed(int _iVirtualKey)
{
	SDL_Scancode scancode = dikToScancode(_iVirtualKey);
	return (!keystate[scancode] && iOneTouch[scancode]);
}

int ARXInputHandlerSDL::GetWheelSens(int _iIDbutton)
{
	return iWheelSens;
}

void ARXInputHandlerSDL::DrawOneCursor(int x, int y, int z)
{
	g_pRenderApp->renderer->DrawQuad(ARX_CLEAN_WARN_CAST_FLOAT(x), ARX_CLEAN_WARN_CAST_FLOAT(y),
		INTERFACE_RATIO_DWORD(scursor[iNumCursor]->m_dwWidth),
		INTERFACE_RATIO_DWORD(scursor[iNumCursor]->m_dwHeight),
		0.00000001f,
		scursor[iNumCursor], 0, 0xFFFFFFFF);
}

int ARXInputHandlerSDL::arxMouseToNative(int button)
{
	switch (button)
	{
	case ARXMOUSE_BUTTON0:
		return SDL_BUTTON_LEFT;
		break;
	case ARXMOUSE_BUTTON1:
		return SDL_BUTTON_RIGHT;
		break;
	case ARXMOUSE_BUTTON2:
		return SDL_BUTTON_MIDDLE;
		break;
	}

	return SDL_BUTTON_LEFT;
}

/************************************************/
/*					DirectInput					*/
/************************************************/
ARXInputHandlerDI::ARXInputHandlerDI() : ARXInputHandler()
{
	for (int i = ARXMOUSE_BUTTON0; i <= ARXMOUSE_BUTTON31; i++)
	{
		iOldMouseButton[i] = 0;
		iMouseTime[i] = 0;
		iMouseTimeSet[i] = 0;
		bMouseButton[i] = bOldMouseButton[i] = false;
		iOldNumClick[i] = iOldNumUnClick[i] = 0;
	}

	// PreCompute le ScanCode
	bTouch = false;
	HKL layout = GetKeyboardLayout(0);

	for (int iI = 0; iI < 256; iI++)
	{
		iKeyScanCode[iI] = MapVirtualKeyEx(iI, 0, layout);
		iOneTouch[iI] = 0;
	}
}

void ARXInputHandlerDI::ResetAll()
{
	for (int i = ARXMOUSE_BUTTON0; i <= ARXMOUSE_BUTTON31; i++)
	{
		iOldMouseButton[i] = 0;
		iMouseTime[i] = 0;
		iMouseTimeSet[i] = 0;
		bMouseButton[i] = bOldMouseButton[i] = false;
		iOldNumClick[i] = iOldNumUnClick[i] = 0;
	}

	iKeyId = -1;
	bTouch = false;

	for (int i = 0; i < 256; i++)
	{
		iOneTouch[i] = 0;
	}

	EERIEMouseButton = LastEERIEMouseButton = 0;

	iWheelSens = 0;
}

void ARXInputHandlerDI::GetInput()
{
	int iDTime;

	DXI_ExecuteAllDevices(false);
	iKeyId = DXI_GetKeyIDPressed(DXI_KEYBOARD1);
	bTouch = (iKeyId >= 0) ? true : false;

	for (int i = 0; i < 256; i++)
	{
		if (IsVirtualKeyPressed(i))
		{
			switch (i)
			{
			case DIK_LSHIFT:
			case DIK_RSHIFT:
			case DIK_LCONTROL:
			case DIK_RCONTROL:
			case DIK_LALT:
			case DIK_RALT:

				if (i != iKeyId)
					iKeyId |= (i << 16);

				break;
			}

			if (iOneTouch[i] < 2)
			{
				iOneTouch[i]++;
			}
		}
		else
		{
			if (iOneTouch[i] > 0)
			{
				iOneTouch[i]--;
			}
		}
	}

	if (bTouch)	//priorité des touches
	{
		switch (iKeyId)
		{
		case DIK_LSHIFT:
		case DIK_RSHIFT:
		case DIK_LCONTROL:
		case DIK_RCONTROL:
		case DIK_LALT:
		case DIK_RALT:
		{
			bool bFound = false;

			for (int i = 0; i < 256; i++)
			{
				if (bFound)
				{
					break;
				}

				switch (i & 0xFFFF)
				{
				case DIK_LSHIFT:
				case DIK_RSHIFT:
				case DIK_LCONTROL:
				case DIK_RCONTROL:
				case DIK_LALT:
				case DIK_RALT:
					continue;
				default:
				{
					if (iOneTouch[i])
					{
						bFound = true;
						iKeyId &= ~0xFFFF;
						iKeyId |= i;
					}
				}
				break;
				}
			}
		}
		}
	}


	ARX_CHECK_INT(ARX_TIME_Get(false));
	const int iArxTime = ARX_CLEAN_WARN_CAST_INT(ARX_TIME_Get(false));


	for (int i = ARXMOUSE_BUTTON0; i <= ARXMOUSE_BUTTON31; i++)
	{
		int iNumClick;
		int iNumUnClick;
		DXI_MouseButtonCountClick(DXI_MOUSE1, i, &iNumClick, &iNumUnClick);

		iOldNumClick[i] += iNumClick + iNumUnClick;

		if ((!bMouseButton[i]) && (iOldNumClick[i] == iNumUnClick))
		{
			iNumUnClick = iOldNumClick[i] = 0;
		}

		bOldMouseButton[i] = bMouseButton[i];

		if (bMouseButton[i])
		{
			if (iOldNumClick[i])
			{
				bMouseButton[i] = false;
			}
		}
		else
		{
			if (iOldNumClick[i])
			{
				bMouseButton[i] = true;
			}
		}

		if (iOldNumClick[i]) iOldNumClick[i]--;

		iDTime = 0;
		DXI_MouseButtonPressed(DXI_MOUSE1, i, &iDTime);

		if (iDTime)
		{
			iMouseTime[i] = iDTime;
			iMouseTimeSet[i] = 2;
		}
		else
		{
			if ((iMouseTimeSet[i] > 0) &&
				((ARX_TIME_Get(false) - iMouseTime[i]) > 300)
				)
			{
				iMouseTime[i] = 0;
				iMouseTimeSet[i] = 0;
			}

			if (GetMouseButtonNowPressed(i))
			{
				switch (iMouseTimeSet[i])
				{
				case 0:
					iMouseTime[i] = iArxTime;
					iMouseTimeSet[i]++;
					break;
				case 1:
					iMouseTime[i] = iArxTime - iMouseTime[i];
					iMouseTimeSet[i]++;
					break;
				}
			}
		}
	}

	iWheelSens = GetWheelSens(DXI_MOUSE1);

	if ((danaeApp.m_pFramework->m_bIsFullscreen) &&
		(bGLOBAL_DINPUT_MENU))
	{
		float fDX = 0.f;
		float fDY = 0.f;
		iMouseRX = iMouseRY = iMouseRZ = 0;

		if (DXI_GetAxeMouseXYZ(DXI_MOUSE1, &iMouseRX, &iMouseRY, &iMouseRZ))
		{
			float fSensMax = 1.f / 6.f;
			float fSensMin = 2.f;
			float fSens = ((fSensMax - fSensMin) * ((float)iSensibility) / 10.f) + fSensMin;
			fSens = pow(.7f, fSens) * 2.f;

			fDX = ((float)iMouseRX) * fSens * (((float)DANAESIZX) / 640.f);
			fDY = ((float)iMouseRY) * fSens * (((float)DANAESIZY) / 480.f);
			fMouseAXTemp += fDX;
			fMouseAYTemp += fDY;


			ARX_CHECK_INT(fMouseAXTemp);
			ARX_CHECK_INT(fMouseAYTemp);
			iMouseAX = ARX_CLEAN_WARN_CAST_INT(fMouseAXTemp);
			iMouseAY = ARX_CLEAN_WARN_CAST_INT(fMouseAYTemp);

			iMouseAZ += iMouseRZ;


			if (iMouseAX < 0)
			{
				iMouseAX = 0;
				fMouseAXTemp = 0.f;
			}


			ARX_CHECK_NOT_NEG(iMouseAX);

			if (ARX_CAST_ULONG(iMouseAX) >= danaeApp.m_pFramework->m_dwRenderWidth)
			{

				iMouseAX = danaeApp.m_pFramework->m_dwRenderWidth - 1;
				fMouseAXTemp = ARX_CLEAN_WARN_CAST_FLOAT(iMouseAX);
			}

			if (iMouseAY < 0)
			{
				fMouseAYTemp = 0.f;
				iMouseAY = 0;
			}


			ARX_CHECK_NOT_NEG(iMouseAY);

			if (ARX_CAST_ULONG(iMouseAY) >= danaeApp.m_pFramework->m_dwRenderHeight)
			{

				iMouseAY = danaeApp.m_pFramework->m_dwRenderHeight - 1;
				fMouseAYTemp = ARX_CLEAN_WARN_CAST_FLOAT(iMouseAY);
			}



			bMouseMove = true;
		}
		else
		{
			bMouseMove = false;
		}

		if (bGLOBAL_DINPUT_GAME)
		{
			_EERIEMouseXdep = (int)fDX;
			_EERIEMouseYdep = (int)fDY;
			EERIEMouseX = iMouseAX;
			EERIEMouseY = iMouseAY;
		}
	}
	else
	{
		bMouseMove = ((iMouseAX != DANAEMouse.x) || (iMouseAY != DANAEMouse.y));
		iMouseAX = DANAEMouse.x;
		iMouseAY = DANAEMouse.y;
		iMouseAZ = 0;
	}

	int iDx;
	int iDy;

	if (pTex[eNumTex])
	{
		iDx = pTex[eNumTex]->m_dwWidth >> 1;
		iDy = pTex[eNumTex]->m_dwHeight >> 1;
	}
	else
	{
		iDx = 0;
		iDy = 0;
	}

	iOldCoord[iNbOldCoord].x = iMouseAX + iDx;
	iOldCoord[iNbOldCoord].y = iMouseAY + iDy;
	iNbOldCoord++;

	if (iNbOldCoord >= iMaxOldCoord)
	{
		iNbOldCoord = iMaxOldCoord - 1;
		memmove((void*)iOldCoord, (void*)(iOldCoord + 1), sizeof(EERIE_2DI)*iNbOldCoord);
	}

}

static bool ComputePer(EERIE_2DI *_psPoint1, EERIE_2DI *_psPoint2, D3DTLVERTEX *_psd3dv1, D3DTLVERTEX *_psd3dv2, float _fSize)
{
	EERIE_2D sTemp;
	float fTemp;

	sTemp.x = (float)(_psPoint2->x - _psPoint1->x);
	sTemp.y = (float)(_psPoint2->y - _psPoint1->y);
	fTemp = sTemp.x;
	sTemp.x = -sTemp.y;
	sTemp.y = fTemp;
	float fMag = (float)sqrt(sTemp.x*sTemp.x + sTemp.y*sTemp.y);

	if (fMag < EEdef_EPSILON)
	{
		return false;
	}

	fMag = _fSize / fMag;

	_psd3dv1->sx = (sTemp.x*fMag);
	_psd3dv1->sy = (sTemp.y*fMag);
	_psd3dv2->sx = ((float)_psPoint1->x) - _psd3dv1->sx;
	_psd3dv2->sy = ((float)_psPoint1->y) - _psd3dv1->sy;
	_psd3dv1->sx += ((float)_psPoint1->x);
	_psd3dv1->sy += ((float)_psPoint1->y);

	return true;
}

//-----------------------------------------------------------------------------

static void DrawLine2D(EERIE_2DI *_psPoint1, int _iNbPt, float _fSize, float _fRed, float _fGreen, float _fBlue)
{
	_iNbPt--;

	if (!_iNbPt) return;

	float fSize = _fSize / _iNbPt;
	float fTaille = fSize;

	float fDColorRed = _fRed / _iNbPt;
	float fColorRed = fDColorRed;
	float fDColorGreen = _fGreen / _iNbPt;
	float fColorGreen = fDColorGreen;
	float fDColorBlue = _fBlue / _iNbPt;
	float fColorBlue = fDColorBlue;

	g_pRenderApp->renderer->SetBlendFunc(EERIEBlendType::DstColor, EERIEBlendType::OneMinusDstColor);
	SETTC(GDevice, NULL);
	SETALPHABLEND(GDevice, true);

	D3DTLVERTEX v[4];
	v[0].sz = v[1].sz = v[2].sz = v[3].sz = 0.f;
	v[0].rhw = v[1].rhw = v[2].rhw = v[3].rhw = 0.999999f;

	EERIE_2DI *psOldPoint = _psPoint1++;
	v[0].color = v[2].color = D3DRGBA(fColorRed, fColorGreen, fColorBlue, 1.f);

	if (!ComputePer(psOldPoint, _psPoint1, &v[0], &v[2], fTaille))
	{
		v[0].sx = v[2].sx = (float)psOldPoint->x;
		v[0].sy = v[2].sy = (float)psOldPoint->y;
	}

	_iNbPt--;

	while (_iNbPt--)
	{
		fTaille += fSize;
		fColorRed += fDColorRed;
		fColorGreen += fDColorGreen;
		fColorBlue += fDColorBlue;

		if (ComputePer(psOldPoint, _psPoint1 + 1, &v[1], &v[3], fTaille))
		{
			v[1].color = v[3].color = D3DRGBA(fColorRed, fColorGreen, fColorBlue, 1.f);
			g_pRenderApp->renderer->DrawPrim(EERIEPrimType::TriangleStrip, D3DFVF_TLVERTEX | D3DFVF_DIFFUSE, v, 4, 0);

			v[0].sx = v[1].sx;
			v[0].sy = v[1].sy;
			v[0].color = v[1].color;
			v[2].sx = v[3].sx;
			v[2].sy = v[3].sy;
			v[2].color = v[3].color;
		}

		psOldPoint = _psPoint1++;
	}

	fTaille += fSize;
	fColorRed += fDColorRed;
	fColorGreen += fDColorGreen;
	fColorBlue += fDColorBlue;

	if (ComputePer(_psPoint1, psOldPoint, &v[1], &v[3], fTaille))
	{
		v[1].color = v[3].color = D3DRGBA(fColorRed, fColorGreen, fColorBlue, 1.f);
		g_pRenderApp->renderer->DrawPrim(EERIEPrimType::TriangleStrip, D3DFVF_TLVERTEX | D3DFVF_DIFFUSE, v, 4, 0);
	}

	SETALPHABLEND(GDevice, false);
}
void ARXInputHandlerDI::DrawCursor()
{
	if (!bDrawCursor) return;

	GDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, true);
	DrawLine2D(iOldCoord, iMaxOldCoord, 10.f, .725f, .619f, 0.56f);

	if (pTex[iNumCursor]) SETTC(GDevice, pTex[iNumCursor]);
	else SETTC(GDevice, NULL);

	SETALPHABLEND(GDevice, false);

	GDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, false);
	DrawOneCursor(iMouseAX, iMouseAY, -1);

	danaeApp.EnableZBuffer();


	ARX_CHECK_LONG(ARXDiffTimeMenu);
	lFrameDiff += ARX_CLEAN_WARN_CAST_LONG(ARXDiffTimeMenu);


	if (lFrameDiff > 70)
	{
		if (bMouseOver)
		{
			if (iNumCursor < 4)
			{
				iNumCursor++;
			}
			else
			{
				if (iNumCursor > 4)
				{
					iNumCursor--;
				}
			}

			SetCursorOff();
			bMouseOver = false;
		}
		else
		{
			if (iNumCursor > 0)
			{
				iNumCursor++;

				if (iNumCursor > 7) iNumCursor = 0;
			}
		}

		lFrameDiff = 0;
	}

	GDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, false);
}

bool ARXInputHandlerDI::IsVirtualKeyPressed(int _iVirtualKey)
{
	return DXI_KeyPressed(DXI_KEYBOARD1, _iVirtualKey) ? true : false;
}

bool ARXInputHandlerDI::IsVirtualKeyPressedOneTouch(int _iVirtualKey)
{
	return((DXI_KeyPressed(DXI_KEYBOARD1, _iVirtualKey)) &&
		(iOneTouch[_iVirtualKey] == 1));
}

bool ARXInputHandlerDI::IsVirtualKeyPressedNowPressed(int _iVirtualKey)
{
	return((DXI_KeyPressed(DXI_KEYBOARD1, _iVirtualKey)) &&
		(iOneTouch[_iVirtualKey] == 1));
}

bool ARXInputHandlerDI::IsVirtualKeyPressedNowUnPressed(int _iVirtualKey)
{
	return((!DXI_KeyPressed(DXI_KEYBOARD1, _iVirtualKey)) &&
		(iOneTouch[_iVirtualKey] == 1));
}

int ARXInputHandlerDI::GetWheelSens(int _iIDbutton)
{
	int iX, iY, iZ = 0;
	DXI_GetAxeMouseXYZ(_iIDbutton, &iX, &iY, &iZ);

	return iZ;
}

void ARXInputHandlerDI::DrawOneCursor(int _iPosX, int _iPosY, int _iColor)
{
	GDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_POINT);
	GDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_POINT);
	SETTEXTUREWRAPMODE(GDevice, D3DTADDRESS_CLAMP);

	g_pRenderApp->renderer->DrawQuad(ARX_CLEAN_WARN_CAST_FLOAT(_iPosX), ARX_CLEAN_WARN_CAST_FLOAT(_iPosY),

		INTERFACE_RATIO_DWORD(scursor[iNumCursor]->m_dwWidth),
		INTERFACE_RATIO_DWORD(scursor[iNumCursor]->m_dwHeight),

		0.00000001f,
		scursor[iNumCursor], 0, D3DCOLORWHITE);
	GDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFP_LINEAR);
	GDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFP_LINEAR);
	SETTEXTUREWRAPMODE(GDevice, D3DTADDRESS_WRAP);
}

int ARXInputHandlerDI::arxMouseToNative(int button)
{
	return button;
}
