// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 1999-2024 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  a11y.c
/// \brief Accessibility features for SRB2 - screen reader support, audio beacons, autopilot

#include "a11y.h"
#include "doomdef.h"
#include "s_sound.h"
#include "p_local.h"
#include "g_game.h"
#include "console.h"
#include "i_system.h"
#include "m_random.h"
#include "command.h"

// Console variables
consvar_t cv_accessibility = CVAR_INIT("accessibility", "Off", CV_SAVE, CV_OnOff, NULL);
consvar_t cv_menu_narration = CVAR_INIT("menunarration", "Off", CV_SAVE, CV_OnOff, NULL);
consvar_t cv_audio_beacons = CVAR_INIT("audiobeacons", "Off", CV_SAVE, CV_OnOff, NULL);
consvar_t cv_beacon_volume = CVAR_INIT("beaconvolume", "8", CV_SAVE, CV_Unsigned, NULL);
consvar_t cv_autopilot = CVAR_INIT("autopilot", "Off", CV_SAVE, CV_OnOff, NULL);

static boolean a11y_initialized = false;
static boolean autopilot_active = false;
static INT32 beacon_timer = 0;

// Tolk stubs - in a real implementation, these would call actual Tolk functions
static boolean tolk_available = false;

//
// Console command functions
//
static void Command_ToggleAutopilot_f(void)
{
    A11Y_AutopilotToggle();
}

static void Command_TestBeacon_f(void)
{
    if (players[consoleplayer].mo)
    {
        A11Y_PlayBeacon(players[consoleplayer].mo, BEACON_RING);
        CONS_Printf("Test beacon played\n");
    }
}

static void Command_TestNarration_f(void)
{
    A11Y_SpeakText("Accessibility test: This is a narration test");
}

//
// A11Y_Init
//
// Initialize accessibility subsystem
//
void A11Y_Init(void)
{
    if (a11y_initialized)
        return;

    // Register console variables
    CV_RegisterVar(&cv_accessibility);
    CV_RegisterVar(&cv_menu_narration);
    CV_RegisterVar(&cv_audio_beacons);
    CV_RegisterVar(&cv_beacon_volume);
    CV_RegisterVar(&cv_autopilot);

    // Register console commands
    COM_AddCommand("toggleautopilot", Command_ToggleAutopilot_f, 0);
    COM_AddCommand("testbeacon", Command_TestBeacon_f, 0);
    COM_AddCommand("testnarration", Command_TestNarration_f, 0);

    // In a real implementation, this would initialize Tolk library
    // For now, we'll just set up our internal state
    tolk_available = false; // Would check for actual screen reader

    CONS_Printf("Accessibility system initialized\n");
    a11y_initialized = true;
}

//
// A11Y_Shutdown
//
// Shutdown accessibility subsystem
//
void A11Y_Shutdown(void)
{
    if (!a11y_initialized)
        return;

    // In a real implementation, this would shutdown Tolk
    tolk_available = false;
    a11y_initialized = false;
}

//
// A11Y_ScreenReaderOutput
//
// Send text to screen reader via Tolk
//
boolean A11Y_ScreenReaderOutput(const char *text)
{
    if (!cv_accessibility.value || !a11y_initialized)
        return false;

    // Tolk stub - in real implementation would call Tolk_Output
    if (tolk_available && text && *text)
    {
        // This would be: Tolk_Output(text, false);
        CONS_Printf("Screen reader: %s\n", text);
        return true;
    }
    return false;
}

//
// A11Y_SpeakText
//
// Speak text immediately via screen reader
//
void A11Y_SpeakText(const char *text)
{
    if (!cv_accessibility.value || !text || !*text)
        return;

    // Try screen reader first
    if (A11Y_ScreenReaderOutput(text))
        return;

    // Fallback: add to console for debug
    CONS_Printf("A11Y: %s\n", text);
}

//
// A11Y_NarrateMenuItem  
//
// Narrate current menu item selection
//
void A11Y_NarrateMenuItem(const char *itemname, const char *value)
{
    if (!cv_menu_narration.value)
        return;

    if (itemname && value)
    {
        char narration[256];
        snprintf(narration, sizeof(narration), "%s: %s", itemname, value);
        A11Y_SpeakText(narration);
    }
    else if (itemname)
    {
        A11Y_SpeakText(itemname);
    }
}

//
// A11Y_NarrateMenuChange
//
// Announce menu change
//
void A11Y_NarrateMenuChange(const char *menuname)
{
    if (!cv_menu_narration.value || !menuname)
        return;

    char narration[128];
    snprintf(narration, sizeof(narration), "%s menu", menuname);
    A11Y_SpeakText(narration);
}

//
// A11Y_PlayBeacon
//
// Play audio beacon for game object
//
void A11Y_PlayBeacon(mobj_t *mobj, beacontype_t type)
{
    sfxenum_t sound = sfx_None;
    
    if (!cv_audio_beacons.value || !mobj)
        return;

    // Only play beacons occasionally to avoid spam
    if (beacon_timer > 0)
        return;

    // Choose sound based on beacon type
    switch (type)
    {
        case BEACON_RING:
            sound = sfx_itemup; // High pitched beep for rings
            break;
        case BEACON_ENEMY:
            sound = sfx_altow1; // Warning sound for enemies  
            break;
        case BEACON_MONITOR:
            sound = sfx_ding; // Monitor/item sound
            break;
        case BEACON_GOAL:
            sound = sfx_wdjump; // Goal/objective sound
            break;
        case BEACON_HAZARD:
            sound = sfx_buzz1; // Hazard warning
            break;
        default:
            return;
    }

    // Play sound with volume based on beacon settings
    if (sound != sfx_None)
    {
        S_StartSound(mobj, sound);
        beacon_timer = TICRATE/4; // Quarter second cooldown
    }
}

//
// A11Y_UpdateBeacons
//
// Update beacon system each tic
//
void A11Y_UpdateBeacons(void)
{
    mobj_t *mobj;
    thinker_t *th;
    
    if (!cv_audio_beacons.value || !players[consoleplayer].mo)
        return;

    // Decrement beacon timer
    if (beacon_timer > 0)
        beacon_timer--;

    // Check nearby objects for beacons
    for (th = thlist[THINK_MOBJ].next; th != &thlist[THINK_MOBJ]; th = th->next)
    {
        mobj = (mobj_t *)th;
        
        if (!mobj || P_MobjWasRemoved(mobj))
            continue;

        // Check distance to player
        fixed_t dist = P_AproxDistance(P_AproxDistance(
            mobj->x - players[consoleplayer].mo->x,
            mobj->y - players[consoleplayer].mo->y),
            mobj->z - players[consoleplayer].mo->z);

        // Only beacon for nearby objects (within ~512 units)
        if (dist > 512*FRACUNIT)
            continue;

        // Determine beacon type and play if appropriate
        switch (mobj->type)
        {
            case MT_RING:
            case MT_COIN:
                if (M_RandomChance(FRACUNIT/32)) // Random chance to avoid spam
                    A11Y_PlayBeacon(mobj, BEACON_RING);
                break;
                
            case MT_GOOMBA:
            case MT_BLUECRAWLA:
            case MT_REDCRAWLA:
                if (M_RandomChance(FRACUNIT/16))
                    A11Y_PlayBeacon(mobj, BEACON_ENEMY);
                break;
                
            case MT_RING_BOX:
            case MT_1UP_BOX:
            case MT_INVULN_BOX:
                if (M_RandomChance(FRACUNIT/8))
                    A11Y_PlayBeacon(mobj, BEACON_MONITOR);
                break;
                
            default:
                // Do nothing for other object types
                break;
        }
    }
}

//
// A11Y_AutopilotUpdate
//
// Update autopilot system
//
void A11Y_AutopilotUpdate(void)
{
    player_t *player;
    
    if (!cv_autopilot.value || !autopilot_active)
        return;

    player = &players[consoleplayer];
    if (!player->mo)
        return;

    // Very basic autopilot - just move forward when possible
    // In a real implementation, this would use pathfinding
    if (player->mo->health > 0)
    {
        // Simple forward movement
        if (P_TryMove(player->mo, 
                     player->mo->x + FixedMul(FRACUNIT*4, finecosine[player->mo->angle >> ANGLETOFINESHIFT]),
                     player->mo->y + FixedMul(FRACUNIT*4, finesine[player->mo->angle >> ANGLETOFINESHIFT]),
                     true))
        {
            // Movement successful - announce occasionally
            if (M_RandomChance(FRACUNIT/64))
                A11Y_SpeakText("Moving forward");
        }
    }
}

//
// A11Y_AutopilotToggle
//
// Toggle autopilot on/off
//
void A11Y_AutopilotToggle(void)
{
    autopilot_active = !autopilot_active;
    
    if (autopilot_active)
        A11Y_SpeakText("Autopilot enabled");
    else
        A11Y_SpeakText("Autopilot disabled");
}

//
// A11Y_AutopilotActive
//
// Check if autopilot is currently active
//
boolean A11Y_AutopilotActive(void)
{
    return (cv_autopilot.value && autopilot_active);
}