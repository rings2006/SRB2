// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 1999-2024 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  a11y.h
/// \brief Accessibility features for SRB2 - screen reader support, audio beacons, autopilot

#ifndef __A11Y__
#define __A11Y__

#include "doomtype.h"
#include "command.h"

// Forward declaration
typedef struct mobj_s mobj_t;

// Console variables for accessibility features
extern consvar_t cv_accessibility;
extern consvar_t cv_menu_narration;
extern consvar_t cv_audio_beacons;
extern consvar_t cv_beacon_volume;
extern consvar_t cv_autopilot;

// Screen reader / Tolk integration
void A11Y_Init(void);
void A11Y_Shutdown(void);
boolean A11Y_ScreenReaderOutput(const char *text);
void A11Y_SpeakText(const char *text);

// Menu narration
void A11Y_NarrateMenuItem(const char *itemname, const char *value);
void A11Y_NarrateMenuChange(const char *menuname);

// Area scanning
void A11Y_ScanArea(void);

// Audio beacon system
typedef enum {
    BEACON_RING,
    BEACON_ENEMY,
    BEACON_MONITOR,
    BEACON_GOAL,
    BEACON_HAZARD
} beacontype_t;

void A11Y_PlayBeacon(mobj_t *mobj, beacontype_t type);
void A11Y_UpdateBeacons(void);

// Autopilot functionality
void A11Y_AutopilotUpdate(void);
void A11Y_AutopilotToggle(void);
boolean A11Y_AutopilotActive(void);

#endif