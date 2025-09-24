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
// Update autopilot system with intelligent pathfinding
//
void A11Y_AutopilotUpdate(void)
{
    player_t *player;
    mobj_t *goal = NULL;
    thinker_t *th;
    fixed_t goalDistance = INT32_MAX;
    angle_t goalAngle = 0;
    fixed_t dist;
    static INT32 stuck_counter = 0;
    static fixed_t lastX = 0, lastY = 0;
    static angle_t exploration_angle = 0;
    
    if (!cv_autopilot.value || !autopilot_active)
        return;

    player = &players[consoleplayer];
    if (!player->mo || player->mo->health <= 0)
        return;

    // Check if we're stuck (haven't moved much)
    fixed_t movement = P_AproxDistance(player->mo->x - lastX, player->mo->y - lastY);
    if (movement < FRACUNIT * 2) // Less than 2 units of movement
    {
        stuck_counter++;
        if (stuck_counter > 35) // About 1 second at 35 FPS
        {
            // We're stuck, try a different approach
            exploration_angle += ANGLE_45; // Turn 45 degrees
            if (M_RandomChance(FRACUNIT/32))
                A11Y_SpeakText("Autopilot exploring new path");
            stuck_counter = 0;
        }
    }
    else
    {
        stuck_counter = 0;
        lastX = player->mo->x;
        lastY = player->mo->y;
    }

    // Look for goal objects (end level signs, exits, etc.)
    // Also look for intermediate goals like rings if no main goal is visible
    mobj_t *intermediate_goal = NULL;
    fixed_t intermediateDistance = INT32_MAX;
    
    for (th = thlist[THINK_MOBJ].next; th != &thlist[THINK_MOBJ]; th = th->next)
    {
        mobj_t *mobj = (mobj_t *)th;
        
        if (!mobj || P_MobjWasRemoved(mobj))
            continue;

        dist = P_AproxDistance(P_AproxDistance(
            mobj->x - player->mo->x,
            mobj->y - player->mo->y),
            mobj->z - player->mo->z);

        // Check for primary goal objects
        switch (mobj->type)
        {
            case MT_SIGN: // End level sign
            case MT_STARPOST: // Checkpoint
                if (dist < goalDistance)
                {
                    goal = mobj;
                    goalDistance = dist;
                }
                break;
                
            // Look for intermediate goals when no main goal is nearby
            case MT_RING:
            case MT_COIN:
                if (!goal && dist < 512*FRACUNIT && dist < intermediateDistance)
                {
                    intermediate_goal = mobj;
                    intermediateDistance = dist;
                }
                break;
                
            case MT_RING_BOX:
            case MT_1UP_BOX:
            case MT_INVULN_BOX:
                if (!goal && dist < 256*FRACUNIT && dist < intermediateDistance)
                {
                    intermediate_goal = mobj;
                    intermediateDistance = dist;
                }
                break;
                
            default:
                break;
        }
    }
    
    // Use intermediate goal if no primary goal found
    if (!goal && intermediate_goal)
    {
        goal = intermediate_goal;
        goalDistance = intermediateDistance;
    }

    // Intelligent pathfinding behavior
    if (goal && stuck_counter < 20) // If we have a goal and aren't stuck
    {
        goalAngle = R_PointToAngle2(player->mo->x, player->mo->y, goal->x, goal->y);
        
        // Use sophisticated angle adjustment like the enemy AI
        angle_t angleDiff = goalAngle - player->mo->angle;
        
        // Normalize angle difference properly for unsigned type
        while (angleDiff > ANGLE_180)
            angleDiff -= ANGLE_MAX;
        while (angleDiff > ANGLE_MAX - ANGLE_180)  
            angleDiff += ANGLE_MAX;
            
        // Smart turning - faster when far from target, slower when close
        angle_t turnSpeed = ANGLE_11hh; // Base turn speed
        boolean turnLeft = (angleDiff <= ANGLE_180);
        angle_t absDiff = turnLeft ? angleDiff : (ANGLE_MAX - angleDiff);
        
        if (absDiff > ANGLE_90)
            turnSpeed = ANGLE_22h; // Turn faster if we need to turn a lot
        else if (absDiff < ANGLE_11hh)
            turnSpeed = absDiff; // Fine adjustment when close
            
        if (absDiff > 0)
        {
            if (turnLeft)
                player->mo->angle += turnSpeed;
            else
                player->mo->angle -= turnSpeed;
        }
    }
    else if (stuck_counter >= 20)
    {
        // Use exploration angle when stuck
        player->mo->angle = exploration_angle;
    }

    // Intelligent movement with multiple attempt distances
    fixed_t moveSpeeds[] = {FRACUNIT * 12, FRACUNIT * 8, FRACUNIT * 4, FRACUNIT * 2};
    boolean moveSuccessful = false;
    
    for (INT32 i = 0; i < 4 && !moveSuccessful; i++)
    {
        fixed_t moveSpeed = moveSpeeds[i];
        fixed_t newX = player->mo->x + FixedMul(moveSpeed, finecosine[player->mo->angle >> ANGLETOFINESHIFT]);
        fixed_t newY = player->mo->y + FixedMul(moveSpeed, finesine[player->mo->angle >> ANGLETOFINESHIFT]);
        
        if (P_TryMove(player->mo, newX, newY, true))
        {
            moveSuccessful = true;
            
            // Announce progress occasionally
            if (goal && M_RandomChance(FRACUNIT/256))
            {
                char message[64];
                INT32 distanceUnits = FixedDiv(goalDistance, FRACUNIT);
                if (distanceUnits > 1000)
                    snprintf(message, sizeof(message), "Goal is far away");
                else if (distanceUnits > 500)
                    snprintf(message, sizeof(message), "Getting closer to goal");
                else if (distanceUnits > 100) 
                    snprintf(message, sizeof(message), "Goal nearby, distance %d", distanceUnits);
                else
                    snprintf(message, sizeof(message), "Very close to goal!");
                A11Y_SpeakText(message);
            }
            break;
        }
    }
    
    if (!moveSuccessful)
    {
        // Try different angles if straight ahead doesn't work
        angle_t testAngles[] = {ANGLE_22h, -ANGLE_22h, ANGLE_45, -ANGLE_45, ANGLE_90, -ANGLE_90};
        
        for (INT32 i = 0; i < 6 && !moveSuccessful; i++)
        {
            angle_t testAngle = player->mo->angle + testAngles[i];
            fixed_t newX = player->mo->x + FixedMul(FRACUNIT * 6, finecosine[testAngle >> ANGLETOFINESHIFT]);
            fixed_t newY = player->mo->y + FixedMul(FRACUNIT * 6, finesine[testAngle >> ANGLETOFINESHIFT]);
            
            if (P_TryMove(player->mo, newX, newY, true))
            {
                player->mo->angle = testAngle;
                moveSuccessful = true;
                if (M_RandomChance(FRACUNIT/64))
                    A11Y_SpeakText("Autopilot navigating around obstacle");
                break;
            }
        }
    }
    
    if (!moveSuccessful)
    {
        // Last resort: turn around and try to find a new path
        player->mo->angle += ANGLE_90;
        stuck_counter += 5; // Increase stuck counter faster
        if (M_RandomChance(FRACUNIT/32))
            A11Y_SpeakText("Autopilot finding alternate route");
    }
    
    // Try jumping if we're stuck and there might be a platform above
    if (stuck_counter > 10 && P_IsObjectOnGround(player->mo))
    {
        // Simple jump attempt - this would need proper jump mechanics integration
        if (M_RandomChance(FRACUNIT/16))
        {
            player->mo->momz = 8 * FRACUNIT; // Basic jump
            if (M_RandomChance(FRACUNIT/8))
                A11Y_SpeakText("Autopilot jumping");
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