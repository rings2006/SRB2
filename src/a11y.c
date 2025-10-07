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
#include "r_main.h" // for R_PointToAngle2

// Console variables
consvar_t cv_accessibility = CVAR_INIT("accessibility", "Off", CV_SAVE, CV_OnOff, NULL);
consvar_t cv_menu_narration = CVAR_INIT("menunarration", "Off", CV_SAVE, CV_OnOff, NULL);
consvar_t cv_audio_beacons = CVAR_INIT("audiobeacons", "Off", CV_SAVE, CV_OnOff, NULL);
consvar_t cv_beacon_volume = CVAR_INIT("beaconvolume", "8", CV_SAVE, CV_Unsigned, NULL);
consvar_t cv_autopilot = CVAR_INIT("autopilot", "Off", CV_SAVE, CV_OnOff, NULL);

static boolean a11y_initialized = false;
static boolean autopilot_active = false;
static INT32 beacon_timer = 0;

#ifdef USE_TOLK
#include <Tolk.h>
#endif

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

static void Command_ScanArea_f(void)
{
    mobj_t *mobj;
    thinker_t *th;
    int ringCount = 0, enemyCount = 0, monitorCount = 0;
    char statusMessage[256];
    
    if (!players[consoleplayer].mo)
    {
        A11Y_SpeakText("Player not found");
        return;
    }

    // Count nearby objects
    for (th = thlist[THINK_MOBJ].next; th != &thlist[THINK_MOBJ]; th = th->next)
    {
        mobj = (mobj_t *)th;
        
        if (!mobj || P_MobjWasRemoved(mobj))
            continue;

        // Check distance to player (within ~1024 units)
        fixed_t dist = P_AproxDistance(P_AproxDistance(
            mobj->x - players[consoleplayer].mo->x,
            mobj->y - players[consoleplayer].mo->y),
            mobj->z - players[consoleplayer].mo->z);

        if (dist > 1024*FRACUNIT)
            continue;

        switch (mobj->type)
        {
            case MT_RING:
            case MT_COIN:
                ringCount++;
                break;
                
            case MT_GOOMBA:
            case MT_BLUECRAWLA:
            case MT_REDCRAWLA:
            case MT_DETON:
            case MT_TURRET:
                enemyCount++;
                break;
                
            case MT_RING_BOX:
            case MT_1UP_BOX:
            case MT_INVULN_BOX:
            case MT_SNEAKERS_BOX:
            case MT_WHIRLWIND_BOX:
                monitorCount++;
                break;
                
            default:
                // Skip other object types
                break;
        }
    }
    
    snprintf(statusMessage, sizeof(statusMessage), 
             "Area scan: %d rings, %d enemies, %d monitors nearby", 
             ringCount, enemyCount, monitorCount);
    A11Y_SpeakText(statusMessage);
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
    COM_AddCommand("scanarea", Command_ScanArea_f, 0);

#ifdef USE_TOLK
    Tolk_Load();
    tolk_available = Tolk_DetectScreenReader() != NULL;
#else
    // In a real implementation, this would initialize Tolk library
    // For now, we'll just set up our internal state
    tolk_available = false; // Would check for actual screen reader
#endif

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

#ifdef USE_TOLK
    Tolk_Unload();
#endif
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

#ifdef USE_TOLK
    if (tolk_available && text && *text)
    {
        // Convert UTF-8 to wide string for Tolk
        int len = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
        if (len > 0)
        {
            wchar_t *wtext = (wchar_t *)malloc(len * sizeof(wchar_t));
            if (wtext)
            {
                MultiByteToWideChar(CP_UTF8, 0, text, -1, wtext, len);
                Tolk_Output(wtext, false);
                free(wtext);
                return true;
            }
        }
    }
#else
    // Tolk stub - in real implementation would call Tolk_Output
    if (tolk_available && text && *text)
    {
        // This would be: Tolk_Output(text, false);
        CONS_Printf("Screen reader: %s\n", text);
        return true;
    }
#endif
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
            case MT_DETON:
            case MT_TURRET:
                if (M_RandomChance(FRACUNIT/16))
                    A11Y_PlayBeacon(mobj, BEACON_ENEMY);
                break;
                
            case MT_RING_BOX:
            case MT_1UP_BOX:
            case MT_INVULN_BOX:
            case MT_SNEAKERS_BOX:
            case MT_WHIRLWIND_BOX:
                if (M_RandomChance(FRACUNIT/8))
                    A11Y_PlayBeacon(mobj, BEACON_MONITOR);
                break;
                
            case MT_SIGN: // End level sign
                if (M_RandomChance(FRACUNIT/4))
                    A11Y_PlayBeacon(mobj, BEACON_GOAL);
                break;
                
            case MT_SPIKE:
                if (M_RandomChance(FRACUNIT/16))
                    A11Y_PlayBeacon(mobj, BEACON_HAZARD);
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
    mobj_t *goal = NULL;
    thinker_t *th;
    fixed_t goalDistance = INT32_MAX;
    angle_t goalAngle = 0;
    
    if (!cv_autopilot.value || !autopilot_active)
        return;

    player = &players[consoleplayer];
    if (!player->mo || player->mo->health <= 0)
        return;

    // Look for goal objects (end level signs, etc.)
    for (th = thlist[THINK_MOBJ].next; th != &thlist[THINK_MOBJ]; th = th->next)
    {
        mobj_t *mobj = (mobj_t *)th;
        fixed_t dist;
        
        if (!mobj || P_MobjWasRemoved(mobj))
            continue;

        // Check for goal-type objects
        switch (mobj->type)
        {
            case MT_SIGN: // End level sign
                dist = P_AproxDistance(P_AproxDistance(
                    mobj->x - player->mo->x,
                    mobj->y - player->mo->y),
                    mobj->z - player->mo->z);
                    
                if (dist < goalDistance)
                {
                    goal = mobj;
                    goalDistance = dist;
                }
                break;
                
            default:
                break;
        }
    }

    // If we found a goal, move toward it
    if (goal)
    {
        goalAngle = R_PointToAngle2(player->mo->x, player->mo->y, goal->x, goal->y);
        
        // Gradually turn toward the goal
        angle_t angleDiff = goalAngle - player->mo->angle;
        if (angleDiff > (angle_t)ANGLE_180)
            angleDiff -= ANGLE_MAX;
        else if (angleDiff < (angle_t)(-ANGLE_180))
            angleDiff += ANGLE_MAX;
            
        // Turn toward goal (but not too quickly)
        if (angleDiff > (angle_t)ANGLE_22h)
            player->mo->angle += ANGLE_11hh;
        else if (angleDiff < (angle_t)(-ANGLE_22h))
            player->mo->angle -= ANGLE_11hh;
        else
            player->mo->angle = goalAngle;
    }

    // Simple forward movement with basic obstacle avoidance
    fixed_t forwardDist = FRACUNIT * 8; // Try to move 8 units forward
    fixed_t newX = player->mo->x + FixedMul(forwardDist, finecosine[player->mo->angle >> ANGLETOFINESHIFT]);
    fixed_t newY = player->mo->y + FixedMul(forwardDist, finesine[player->mo->angle >> ANGLETOFINESHIFT]);
    
    if (P_TryMove(player->mo, newX, newY, true))
    {
        // Movement successful
        if (goal && M_RandomChance(FRACUNIT/128)) // Announce goal occasionally
        {
            char message[64];
            snprintf(message, sizeof(message), "Moving toward goal, distance %d", 
                    FixedDiv(goalDistance, FRACUNIT));
            A11Y_SpeakText(message);
        }
        else if (M_RandomChance(FRACUNIT/256))
        {
            A11Y_SpeakText("Autopilot moving forward");
        }
    }
    else
    {
        // Try turning if blocked
        player->mo->angle += ANGLE_45;
        if (M_RandomChance(FRACUNIT/64))
            A11Y_SpeakText("Obstacle detected, turning");
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