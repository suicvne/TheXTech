/*
 * A2xTech - A platform game engine ported from old source code for VB6
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

#include "game_main.h"
#include <SDL2/SDL.h>
#include <AppPath/app_path.h>
#include <tclap/CmdLine.h>
#include <Utils/strings.h>

static void strToPlayerSetup(int player, const std::string &setupString)
{
    if(setupString.empty())
        return; // Do nothing

    std::vector<std::string> keys;

    auto &p = testPlayer[player];

    Strings::split(keys, setupString, ";");
    for(auto &k : keys)
    {
        if(k.empty())
            continue;
        if(k[0] == 'c') // Character
            p.Character = int(SDL_strtol(k.substr(1).c_str(), nullptr, 10));
        else if(k[0] == 's') // State
            p.State = int(SDL_strtol(k.substr(1).c_str(), nullptr, 10));
        else if(k[0] == 'm') // Mounts
            p.Mount = int(SDL_strtol(k.substr(1).c_str(), nullptr, 10));
        else if(k[0] == 't') // Mount types
            p.MountType = int(SDL_strtol(k.substr(1).c_str(), nullptr, 10));
    }

    if(p.Character < 1)
        p.Character = 1;
    if(p.Character > 5)
        p.Character = 5;

    if(p.State < 1)
        p.State = 1;
    if(p.State > 7)
        p.State = 7;


    switch(p.Mount)
    {
    default:
    case 0:
        p.Mount = 0;
        p.MountType = 0;
        break; // Rejected aliens
    case 1: case 2: case 3:
        break; // Allowed
    }

    switch(p.Mount)
    {
    case 1:
        if(p.MountType < 1 || p.MountType > 3) // Socks
            p.MountType = 1;
        break;
    default:
        break;
    case 3:
        if(p.MountType < 1 || p.MountType > 8) // Cat Llamas
            p.MountType = 1;
        break;
    }
}

extern "C"
int main(int argc, char**argv)
{
    CmdLineSetup_t setup;

    AppPathManager::initAppPath();
    AppPath = AppPathManager::userAppDirSTD();

    OpenConfig_preSetup();

    setup.renderType = CmdLineSetup_t::RenderType(RenderMode);

    testPlayer.fill(Player_t());

    try
    {
        // Define the command line object.
        TCLAP::CmdLine  cmd("A2xTech\n"
                            "Copyright (c) 2020-2020 Vitaly Novichkov <admin@wohlnet.ru>\n"
                            "This program is distributed under the MIT license\n", ' ', "1.3");

        TCLAP::SwitchArg switchFrameSkip("f", "frameskip", "Enable frame skipping mode", false);
        TCLAP::SwitchArg switchNoSound("s", "no-sound", "Disable sound", false);
        TCLAP::SwitchArg switchNoPause("p", "never-pause", "Never pause game when window losts a focus", false);
        TCLAP::ValueArg<std::string> renderType("r", "render", "Render mode: sw (software), hw (hardware), vsync (hardware with vsync)",
                                                false, "",
                                                "render type",
                                                cmd);

        TCLAP::ValueArg<std::string> testLevel("l", "leveltest", "Start a level test of given file",
                                                false, "",
                                                "file path",
                                                cmd);

        TCLAP::ValueArg<unsigned int> numPlayers("n", "num-players", "Count of players",
                                                    false, 1u,
                                                   "number 1 or 2",
                                                   cmd);

        TCLAP::SwitchArg switchBattleMode("b", "battle", "Test level in battle mode", false);

        TCLAP::ValueArg<std::string> playerCharacter1("1", "player1", "Setup of playable character for player 1",
                                                        false, "",
                                                       "с1;s2;m0;t0 : c - character, s - state, m - mount, t - mount type",
                                                       cmd);

        TCLAP::ValueArg<std::string> playerCharacter2("2", "player2", "Setup of playable character for player 1",
                                                        false, "",
                                                       "с1;s2;m0;t0 : c - character, s - state, m - mount, t - mount type",
                                                       cmd);

        TCLAP::SwitchArg switchTestGodMode("g", "god-mode", "Enable god mode in level testing", false);
        TCLAP::SwitchArg switchTestGrabAll("a", "grab-all", "Enable ability to grab everything while level testing", false);
        TCLAP::SwitchArg switchTestShowFPS("m", "show-fps", "Show FPS counter on the screen", false);
        TCLAP::SwitchArg switchTestMaxFPS("x", "max-fps", "Run FPS as fast as possible", false);

        cmd.add(&switchFrameSkip);
        cmd.add(&switchNoSound);
        cmd.add(&switchNoPause);
        cmd.add(&switchBattleMode);

        cmd.add(&switchTestGodMode);
        cmd.add(&switchTestGrabAll);
        cmd.add(&switchTestShowFPS);
        cmd.add(&switchTestMaxFPS);

        cmd.parse(argc, argv);

        setup.frameSkip = switchFrameSkip.getValue();
        setup.noSound   = switchNoSound.getValue();
        setup.neverPause = switchNoPause.getValue();

        std::string rt = renderType.getValue();
        if(rt == "sw")
            setup.renderType = CmdLineSetup_t::RENDER_SW;
        else if(rt == "vsync")
            setup.renderType = CmdLineSetup_t::RENDER_VSYNC;
        else if(rt == "hw")
            setup.renderType = CmdLineSetup_t::RENDER_HW;

        setup.testLevel = testLevel.getValue();
        setup.testLevelMode = !setup.testLevel.empty();
        setup.testNumPlayers = int(numPlayers.getValue());
        if(setup.testNumPlayers > 2)
            setup.testNumPlayers = 2;
        setup.testBattleMode = switchBattleMode.getValue();
        if(setup.testLevelMode)
        {
            strToPlayerSetup(1, playerCharacter1.getValue());
            strToPlayerSetup(2, playerCharacter2.getValue());
        }

        setup.testGodMode = switchTestGodMode.getValue();
        setup.testGrabAll = switchTestGrabAll.getValue();
        setup.testShowFPS = switchTestShowFPS.getValue();
        setup.testMaxFPS = switchTestMaxFPS.getValue();
    }
    catch(TCLAP::ArgException &e)   // catch any exceptions
    {
        std::cerr << "Error: " << e.error() << " for arg " << e.argId() << std::endl;
        std::cerr.flush();
        return 2;
    }

    // set this flag before SDL initialization to allow game be quit when closing a window before a loading process will be completed
    GameIsActive = true;

    if(frmMain.initSDL(setup))
    {
        frmMain.freeSDL();
        return 1;
    }

    int ret = GameMain(setup);
    frmMain.freeSDL();

    return ret;
}
