/**
 * Tolk stub implementation for non-Windows platforms
 * This provides dummy implementations that do nothing
 */

#include "tolk.h"

#ifndef _WIN32

// Stub implementations for non-Windows platforms
bool Tolk_Load(void) { return false; }
void Tolk_Unload(void) {}
bool Tolk_IsLoaded(void) { return false; }
const wchar_t* Tolk_DetectScreenReader(void) { return NULL; }
bool Tolk_HasSpeech(void) { return false; }
bool Tolk_HasBraille(void) { return false; }
bool Tolk_Output(const wchar_t* str, bool interrupt) { (void)str; (void)interrupt; return false; }
bool Tolk_Speak(const char* str, bool interrupt) { (void)str; (void)interrupt; return false; }
bool Tolk_Braille(const wchar_t* str) { (void)str; return false; }
bool Tolk_IsSpeaking(void) { return false; }
bool Tolk_Silence(void) { return false; }

#endif /* !_WIN32 */