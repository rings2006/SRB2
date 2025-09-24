// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 1999-2025 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  i_accessibility.h
/// \brief Accessibility support for screen readers and other assistive technologies

#ifndef __I_ACCESSIBILITY__
#define __I_ACCESSIBILITY__

#include "doomtype.h"

// Initialize accessibility support
boolean I_InitAccessibility(void);

// Shutdown accessibility support  
void I_QuitAccessibility(void);

// Check if accessibility/screen reader support is available
boolean I_AccessibilityEnabled(void);

// Speak text using screen reader (if available)
// text: Text to speak (null-terminated string)
// interrupt: If true, interrupts any currently speaking text
void I_SpeakText(const char *text, boolean interrupt);

// Stop any currently speaking text
void I_StopSpeech(void);

// Check if screen reader is currently speaking
boolean I_IsSpeaking(void);

#endif // __I_ACCESSIBILITY__