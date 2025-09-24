/**
 * Tolk -- A library for communicating with Windows screen readers
 * Minimal interface header for SRB2 integration
 * 
 * This is a simplified version of the Tolk interface for compilation.
 * The actual Tolk library handles dynamic loading of screen reader APIs.
 */

#ifndef TOLK_H
#define TOLK_H

#include <stdbool.h>
#include <stddef.h>
#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
	#ifdef TOLK_STATIC
		#define TOLK_API
	#else
		#ifdef TOLK_EXPORTS
			#define TOLK_API __declspec(dllexport)
		#else
			#define TOLK_API __declspec(dllimport)
		#endif
	#endif
#else
	#define TOLK_API
#endif

/**
 * Initialize Tolk library
 * @return true if successful, false otherwise
 */
TOLK_API bool Tolk_Load(void);

/**
 * Uninitialize Tolk library
 */
TOLK_API void Tolk_Unload(void);

/**
 * Check if a screen reader is available
 * @return true if screen reader is available, false otherwise
 */
TOLK_API bool Tolk_IsLoaded(void);

/**
 * Get name of currently active screen reader
 * @return name of screen reader or NULL if none active
 */
TOLK_API const wchar_t* Tolk_DetectScreenReader(void);

/**
 * Check if screen reader has speech capability
 * @return true if speech is supported, false otherwise
 */
TOLK_API bool Tolk_HasSpeech(void);

/**
 * Check if screen reader has braille capability
 * @return true if braille is supported, false otherwise
 */
TOLK_API bool Tolk_HasBraille(void);

/**
 * Speak text using screen reader
 * @param str Text to speak (wide character string)
 * @param interrupt Whether to interrupt current speech
 * @return true if successful, false otherwise
 */
TOLK_API bool Tolk_Output(const wchar_t* str, bool interrupt);

/**
 * Speak text using screen reader (UTF-8 version)
 * @param str Text to speak (UTF-8 string)
 * @param interrupt Whether to interrupt current speech
 * @return true if successful, false otherwise
 */
TOLK_API bool Tolk_Speak(const char* str, bool interrupt);

/**
 * Send text to braille display
 * @param str Text to display (wide character string)
 * @return true if successful, false otherwise
 */
TOLK_API bool Tolk_Braille(const wchar_t* str);

/**
 * Check if screen reader is speaking
 * @return true if currently speaking, false otherwise
 */
TOLK_API bool Tolk_IsSpeaking(void);

/**
 * Stop current speech
 * @return true if successful, false otherwise
 */
TOLK_API bool Tolk_Silence(void);

#ifdef __cplusplus
}
#endif

#endif /* TOLK_H */