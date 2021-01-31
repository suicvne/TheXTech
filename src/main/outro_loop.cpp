/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2020 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */


#include <Logger/logger.h>

#include "../globals.h"
#include "../game_main.h"
#include "../control/joystick.h"
#include "../npc.h"
#include "../blocks.h"
#include "../effect.h"
#include "../player.h"
#include "../graphics.h"
#include "../sound.h"
#include "../pseudo_vb.h"
#include "game_info.h"


void DoCredits()
{
    if(GameMenu)
        return;

    int screenH_half = ScreenH / 2;

    CreditOffsetY -= 0.8;
//    if(CreditOffsetY > ScreenH || CreditOffsetY + CreditTotalHeight < 0)
//    {}

    // Closing screen
    if((CreditOffsetY + CreditTotalHeight) < -100)
    {
        if(musicPlaying)
        {
            FadeOutMusic(11000);
            musicPlaying = false;
        }

        CreditChop += 0.4f;
        if(CreditChop >= static_cast<float>(screenH_half))
        {
            CreditChop = static_cast<float>(screenH_half);
            EndCredits++;
            if(EndCredits == screenH_half)
            {
                SetupCredits();
                GameOutroDoQuit = true;
                GameMenu = true;
            }
        }
        else
            EndCredits = 0;
    }

        // Opening screen
    else if(CreditChop > 100 && CreditOffsetY + CreditTotalHeight > 0)
    {
        CreditChop -= 2.0f;
        if(CreditChop < 100)
            CreditChop = 100;

        if(CreditChop < 250 && !musicPlaying)
        {
            if(bgMusic[0] <= 0) // Play default music if no music set in outro level
            {
                musicName = "tmusic";
                PlayMusic("tmusic", 2000);
                musicPlaying = true;
            }
            else // Otherwise, play the music that set by level
                StartMusic(0, 2000);
        }
    }

    if(CreditChop <= 100 || EndCredits > 0)
    {
//        for(A = 1; A <= 2; A++) // Useless loop
//        {
        bool quitKey = false;
        quitKey |= (getKeyState(vbKeyEscape) == KEY_PRESSED);
        quitKey |= (getKeyState(vbKeySpace) == KEY_PRESSED);
        quitKey |= (getKeyState(vbKeyReturn) == KEY_PRESSED);
#ifdef __ANDROID__ // Quit credits on BACK key press
        quitKey |= (getKeyState(SDL_SCANCODE_AC_BACK) == KEY_PRESSED);
#endif
        if(quitKey)
        {
            CreditChop = static_cast<float>(screenH_half);
            EndCredits = 0;
            frmMain.clearBuffer();
            SetupCredits();
            GameMenu = true;
            GameOutroDoQuit = true;
        }
//        }
    }
}

void OutroLoop()
{
    Controls_t blankControls;
    int A = 0;
    int B = 0;
    Location_t tempLocation;
    bool jumpBool = false;
    long long fBlock = 0;
    long long lBlock = 0;
    UpdateControls();

    for(A = 1; A <= numPlayers; A++)
    {
        Player[A].Controls = blankControls;
        Player[A].Controls.Left = true;
        jumpBool = true;
        tempLocation = Player[A].Location;
        tempLocation = Player[A].Location;
        tempLocation.SpeedX = 0;
        tempLocation.SpeedY = 0;
        tempLocation.Y = Player[A].Location.Y + Player[A].Location.Height - 8;
        tempLocation.Height = 16;
        tempLocation.Width = 16;

        if(Player[A].Location.SpeedX > 0)
            tempLocation.X = Player[A].Location.X + Player[A].Location.Width + 20;
        else
            tempLocation.X = Player[A].Location.X - tempLocation.Width - 20;
        fBlock = FirstBlock[long(tempLocation.X / 32) - 1];
        lBlock = LastBlock[long((tempLocation.X + tempLocation.Width) / 32.0) + 1];

        for(B = (int)fBlock; B <= lBlock; B++)
        {
            if(tempLocation.X + tempLocation.Width >= Block[B].Location.X)
            {
                if(tempLocation.X <= Block[B].Location.X + Block[B].Location.Width)
                {
                    if(tempLocation.Y + tempLocation.Height >= Block[B].Location.Y)
                    {
                        if(tempLocation.Y <= Block[B].Location.Y + Block[B].Location.Height)
                        {
                            if(!BlockNoClipping[Block[B].Type] && !Block[B].Invis && !Block[B].Hidden && !(BlockIsSizable[Block[B].Type] && Block[B].Location.Y < Player[A].Location.Y + Player[A].Location.Height - 3))
                                jumpBool = false;
                        }
                    }
                }
            }
            else
            {
                if(BlocksSorted)
                    break;
            }
        }

        if(jumpBool || Player[A].Jump > 0)
            Player[A].Controls.Jump = true;
    }

    UpdateNPCs();
    UpdateBlocks();
    UpdateEffects();
    UpdatePlayer();
    DoCredits();
    UpdateGraphics();
    UpdateSound();
    if(GameOutroDoQuit) // Don't unset the GameOutro before GFX update, otherwise a glitch will happen
    {
        GameOutro = false;
        GameOutroDoQuit = false;
    }
}

void AddCredit(std::string newCredit)
{
    numCredits += 1;
    if(numCredits > maxCreditsLines)
    {
        numCredits = maxCreditsLines;
        pLogWarning("Can't add more credits lines: max limit has been excited ({0} linex maximum)", maxCreditsLines);
        return;
    }
    Credit[numCredits].Text = newCredit;
}

void SetupCredits()
{
    int A = 0;
    numCredits = 0;

    AddCredit(g_gameInfo.title);

    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
#ifdef ENABLE_OLD_CREDITS
    AddCredit("Created By:");
#else
    if(!g_gameInfo.creditsGame.empty())
    {
        AddCredit("Game credits:");
        for(auto &s : g_gameInfo.creditsGame)
            AddCredit(s);
        AddCredit("");
        AddCredit("");
        AddCredit("Engine credits:");
        AddCredit("");
    }

    AddCredit("Original VB6 code By:");
#endif
    AddCredit("");
    AddCredit("Andrew Spinks");
    AddCredit("'Redigit'");
    AddCredit("");
    AddCredit("");
#ifndef ENABLE_OLD_CREDITS
    AddCredit("C++ port By:");
    AddCredit("");
    AddCredit("Vitaly Novichkov");
    AddCredit("'Wohlstand'");
    AddCredit("");
    AddCredit("");
#endif

    if(!WorldCredits[1].empty())
    {
        AddCredit("Level Design:");
        AddCredit("");
        for(A = 1; A <= maxWorldCredits; A++)
        {
            if(WorldCredits[A].empty())
                break;
            AddCredit(WorldCredits[A]);
        }
        AddCredit("");
        AddCredit("");
        AddCredit("");
    }

    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("Custom Sprites:");
    AddCredit("");
    AddCredit("Blue");
    AddCredit("Iceman404");
    AddCredit("LuigiFan");
    AddCredit("NameUser");
    AddCredit("Redigit");
    AddCredit("Valtteri");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("Special Thanks:");
#ifndef ENABLE_OLD_CREDITS
    AddCredit("");
    AddCredit("Kevsoft");
    AddCredit("Rednaxela");
    AddCredit("Aero");
    AddCredit("Kley");
    AddCredit("ShadowYoshi (Joey)");
    AddCredit("ZeZeinzer"); // Android testing, touchscreen controller button pictures
    AddCredit("LucyZocker"); // Android testing
#endif
    AddCredit("");
    AddCredit("4matsy");
    AddCredit("AndyDark");
    AddCredit("Bikcmp");
    AddCredit("Blue");
    AddCredit("Captain Obvious");
    AddCredit("CaptainTrek");
    AddCredit("Chase");
    AddCredit("Coldwin");
    AddCredit("CrystalMike");
    AddCredit("DarkMatt");
    AddCredit("FallingSnow");
    AddCredit("Garro");
    AddCredit("Knuckles96");
    AddCredit("Kuribo");
    AddCredit("Kyasarin");
    AddCredit("Luminous");
    AddCredit("m4sterbr0s");
    AddCredit("NameUser");
    AddCredit("Namyrr");
    AddCredit("Qig");
    AddCredit("Quill");
    AddCredit("Red_Yoshi");
    AddCredit("Spitfire");
    AddCredit("Valtteri");
    AddCredit("Vandarx");
    AddCredit("Zephyr");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
#ifndef ENABLE_OLD_CREDITS
    AddCredit("'It just works!'");
    AddCredit("");
    AddCredit("Todd Howard");
#else
    AddCredit("'He has delivered us from the power");
    AddCredit("of darkness and conveyed us into");
    AddCredit("the kingdom of the Son of His love.'");
    AddCredit("");
    AddCredit("Colossians 1:13");
#endif
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit("");
    AddCredit(g_gameInfo.creditsHomePage);

    CreditOffsetY = (ScreenH + 40);
    CreditTotalHeight = 32.0;

    for(A = 1; A <= numCredits; A++)
    {
        auto &cr = Credit[A];
        cr.Location.Width = cr.Text.size() * 18;
        cr.Location.Height = 16;
        cr.Location.X = (double(ScreenW) / 2) - (cr.Location.Width / 2.0);
        cr.Location.Y = 32 * A;
        CreditTotalHeight += 32.0;
    }
}
