// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 1999-2025 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  i_accessibility.c
/// \brief Accessibility support for screen readers and other assistive technologies

#include "i_accessibility.h"
#include "doomdef.h"
#include "console.h"

#ifdef HAVE_TOLK
#ifdef _WIN32
#include "tolk.h"

static boolean tolk_initialized = false;
static boolean accessibility_enabled = false;

boolean I_InitAccessibility(void)
{
	if (tolk_initialized)
		return accessibility_enabled;
		
	CONS_Printf("Initializing accessibility support...\n");
	
	if (Tolk_Load())
	{
		tolk_initialized = true;
		accessibility_enabled = Tolk_IsLoaded();
		
		if (accessibility_enabled)
		{
			const wchar_t* screen_reader = Tolk_DetectScreenReader();
			if (screen_reader)
			{
				CONS_Printf("Screen reader detected: %ls\n", screen_reader);
				// Announce SRB2 startup to screen reader
				I_SpeakText("Sonic Robo Blast 2 accessibility enabled", false);
			}
			else
			{
				CONS_Printf("No screen reader detected\n");
				accessibility_enabled = false;
			}
		}
		else
		{
			CONS_Printf("No screen reader available\n");
		}
	}
	else
	{
		CONS_Printf("Failed to initialize Tolk library\n");
	}
	
	return accessibility_enabled;
}

void I_QuitAccessibility(void)
{
	if (tolk_initialized)
	{
		CONS_Printf("Shutting down accessibility support\n");
		Tolk_Unload();
		tolk_initialized = false;
		accessibility_enabled = false;
	}
}

boolean I_AccessibilityEnabled(void)
{
	return accessibility_enabled;
}

void I_SpeakText(const char *text, boolean interrupt)
{
	if (!accessibility_enabled || !text)
		return;
		
	// Use Tolk_Speak for UTF-8 text
	Tolk_Speak(text, interrupt);
}

void I_StopSpeech(void)
{
	if (!accessibility_enabled)
		return;
		
	Tolk_Silence();
}

boolean I_IsSpeaking(void)
{
	if (!accessibility_enabled)
		return false;
		
	return Tolk_IsSpeaking();
}

#else // !_WIN32
// Non-Windows stub implementation
boolean I_InitAccessibility(void)
{
	CONS_Printf("Accessibility support is only available on Windows\n");
	return false;
}

void I_QuitAccessibility(void) {}
boolean I_AccessibilityEnabled(void) { return false; }
void I_SpeakText(const char *text, boolean interrupt) { (void)text; (void)interrupt; }
void I_StopSpeech(void) {}
boolean I_IsSpeaking(void) { return false; }
#endif // _WIN32

#else // !HAVE_TOLK
// Tolk not available - stub implementation
boolean I_InitAccessibility(void)
{
	CONS_Printf("Accessibility support not compiled in\n");
	return false;
}

void I_QuitAccessibility(void) {}
boolean I_AccessibilityEnabled(void) { return false; }
void I_SpeakText(const char *text, boolean interrupt) { (void)text; (void)interrupt; }
void I_StopSpeech(void) {}
boolean I_IsSpeaking(void) { return false; }
#endif // HAVE_TOLK