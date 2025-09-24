# SRB2 Accessibility Support

This document describes the accessibility features implemented in Sonic Robo Blast 2 to support users who are blind or have visual impairments.

## Overview

SRB2 now includes basic accessibility support through menu narration functionality. When enabled, the game will automatically announce menu item names and types as the user navigates through menus using screen reader technology.

## Supported Platforms

- **Windows**: Full support via Tolk library integration with NVDA, JAWS, and other compatible screen readers
- **Linux/macOS**: Stub implementation (no audio output, but code compiles cleanly)

## Features

### Menu Narration
- Automatic announcement of menu item names when navigating with arrow keys
- Contextual information about item types (button, slider, text field, etc.)
- Non-interrupting speech (new selections don't cut off previous announcements)

### Supported Menu Elements
- Buttons ("Start Game button")
- Submenus ("Options menu") 
- Sliders ("Music Volume slider")
- Text fields ("Player Name text field")
- General options ("Display Options option")

## Installation

### Windows (with Screen Reader Support)

1. **Using System Tolk Library (Recommended)**:
   - Install Tolk library on your system
   - Build SRB2 with `-DSRB2_CONFIG_HAVE_TOLK=ON`
   - CMake will automatically detect and use the system Tolk library

2. **Using Bundled Headers**:
   - No additional installation required
   - Build SRB2 normally on Windows
   - The game will use runtime dynamic loading of screen reader APIs

### Linux/macOS (Stub Implementation)

- No additional setup required
- Accessibility code compiles but provides no audio output
- Useful for developers working on cross-platform builds

## Building with Accessibility Support

### CMake (Recommended)
```bash
mkdir build && cd build
cmake .. -DSRB2_CONFIG_HAVE_TOLK=ON
make
```

### Traditional Makefile
The accessibility support is automatically included in standard builds.

## Usage

1. **Start SRB2** - If a compatible screen reader is running on Windows, you'll hear "Sonic Robo Blast 2 accessibility enabled"
2. **Navigate menus** - Use arrow keys to move between menu items and hear announcements
3. **No configuration needed** - Accessibility features activate automatically when a screen reader is detected

## Technical Details

### Architecture
- `src/i_accessibility.h/c`: Cross-platform accessibility interface
- `thirdparty/tolk/`: Tolk library integration for Windows screen readers  
- Menu integration in `src/m_menu.c` via `M_UpdateItemOn()` function

### How It Works
1. On startup, `I_InitAccessibility()` attempts to initialize Tolk library
2. If successful and screen reader detected, accessibility features are enabled
3. When menu selection changes, `M_AnnounceMenuItem()` is called
4. Text is passed to screen reader via `I_SpeakText()` function
5. On exit, `I_QuitAccessibility()` cleans up resources

### Screen Reader Support (Windows)
The implementation uses the Tolk library which provides unified access to:
- NVDA (NonVisual Desktop Access)
- JAWS (Job Access With Speech)  
- Window-Eyes
- System Access To Go (SAPI)
- Other compatible screen readers

## Limitations

- Currently only supports menu narration (in-game audio cues not implemented)
- Windows-only for actual screen reader output
- Requires compatible screen reader software to be running
- No customization options (speech rate, voice, etc. controlled by screen reader)

## Future Enhancements

Potential areas for expansion:
- In-game audio cues and spatial audio
- Braille display support
- HUD element announcements
- Control scheme announcements
- Game state narration
- Configuration options for accessibility features

## Troubleshooting  

### "Accessibility support not compiled in"
- Ensure you're building on Windows with Tolk support enabled
- Linux/macOS will always show this message (expected behavior)

### No speech output on Windows
- Verify a compatible screen reader (NVDA, JAWS) is running
- Check screen reader speech settings
- Ensure game has audio permissions
- Try restarting the screen reader before launching SRB2

### Build errors
- Ensure `stdbool.h`, `stddef.h`, and `wchar.h` are available
- Check that CMake can find the Tolk library (if using system installation)
- Verify compiler supports C99 standard

## Contributing

To contribute to accessibility features:
1. Test with actual screen readers when possible
2. Keep changes minimal and focused
3. Ensure cross-platform compatibility (Linux/macOS stub implementations)
4. Document any new features in this README

## Credits

- Accessibility implementation by GitHub Copilot
- Tolk library by Davy Kager
- Inspired by accessibility needs in the SRB2 community