# SRB2 Accessibility Features

This document describes the accessibility features added to Sonic Robo Blast 2 to improve the gaming experience for players with visual impairments.

## Features Overview

### 1. Tolk Integration (Screen Reader Support)
- Integrates with Tolk library for screen reader compatibility
- Supports JAWS, NVDA, and other Windows screen readers
- Provides text-to-speech output for menus and game information

### 2. Audio Beacon System
- Spatial audio cues for important game objects:
  - **Rings**: High-pitched beep sounds to locate collectible rings
  - **Enemies**: Warning sounds to alert players of nearby threats
  - **Monitors**: Special item sounds for power-up monitors
  - **Goal**: Objective indicator sounds
  - **Hazards**: Alert sounds for dangerous obstacles

### 3. Menu Narration
- Automatically announces menu names when entering new menus
- Speaks menu item names when navigating with arrow keys
- Provides audio feedback for menu selection changes

### 4. Autopilot System
- Basic automated movement assistance
- Simple pathfinding to help navigate levels
- Can be toggled on/off as needed

## Console Variables

### Core Settings
- `accessibility <on/off>` - Master switch for all accessibility features
- `menunarration <on/off>` - Enable/disable menu narration
- `audiobeacons <on/off>` - Enable/disable audio beacon system
- `beaconvolume <0-31>` - Volume level for beacon sounds
- `autopilot <on/off>` - Enable/disable autopilot system

## Console Commands

### Testing Commands
- `toggleautopilot` - Toggle autopilot on/off
- `testbeacon` - Play a test beacon sound
- `testnarration` - Test text-to-speech output
- `scanarea` - Get count of nearby rings, enemies, and monitors

## Usage Instructions

### Getting Started
1. Enable accessibility features: `accessibility on`
2. Enable desired features:
   - For menu help: `menunarration on`
   - For object location: `audiobeacons on`
   - For movement assistance: `autopilot on`

### Menu Navigation
- Use arrow keys to navigate menus
- Listen for menu name announcements when entering new menus
- Menu items are announced when selected

### Area Scanning
- Use `scanarea` command to get information about nearby objects
- Reports count of rings, enemies, and monitors within range
- Helpful for understanding the immediate game environment

### Audio Beacons
- Beacons play automatically when objects are nearby
- Different sounds indicate different object types:
  - Rings: High beep
  - Enemies: Warning tone  
  - Monitors: Item sound
  - Goals: Objective sound
  - Hazards: Alert sound
- Adjust volume with `beaconvolume` command

### Autopilot
- Toggle with `toggleautopilot` command
- **Intelligent pathfinding** that seeks level goals (end signs, checkpoints)
- **Obstacle avoidance** with multiple movement strategies
- **Intermediate goal seeking** - navigates to rings and monitors when no main goal is visible
- **Stuck detection** - automatically tries alternate routes when blocked
- **Smart turning** - faster turns for large angle changes, precise turns for fine adjustments
- **Multiple movement speeds** - tries different speeds to get around obstacles
- **Jump assistance** - attempts jumping when ground movement is blocked
- Provides audio feedback about movement, obstacles, and progress
- Can be combined with manual controls

## Technical Details

### Implementation
- Located in `src/a11y.c` and `src/a11y.h`
- Integrated with existing sound system (`s_sound.c`)
- Hooks into menu system (`m_menu.c`) and game loop (`g_game.c`)
- Minimal impact on existing codebase

### Tolk Integration
- Uses wrapper functions for screen reader compatibility
- Falls back to console output when screen readers unavailable
- Designed for future integration with actual Tolk library

## Future Enhancements

- True Tolk library integration for production builds
- **Advanced pathfinding algorithms** with A* or similar algorithms
- **Level analysis** for automatic route discovery
- **Player movement integration** for proper jump timing and spin mechanics
- Customizable beacon sounds and frequencies  
- Voice announcements for game events
- Configurable key bindings for accessibility features
- Audio radar system for spatial awareness
- **Ring collection optimization** for speedrun assistance
- **Enemy avoidance patterns** based on enemy types

## Development Notes

This implementation provides a foundation for accessibility features in SRB2. The code is designed to be:
- **Minimal**: Small changes to existing codebase
- **Modular**: Self-contained accessibility system
- **Extensible**: Easy to add new features
- **Safe**: No impact on gameplay when disabled

All accessibility features can be disabled completely via console variables for users who don't need them.