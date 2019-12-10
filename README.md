# ECHOs in the Dark

## Final Game Report:

For this milestone, we:

Fixed the following from ideas from crossplay/previous bugs:
- Added the ability to change light colors to the mouse so that player can always have one hand on mouse to direct light, and one on keyboard
- Fixed lag and memory issues by refactoring the way we were storing bricks
- Fixed a memory leaks caused by not releasing all of the smoke sprites and sound resources
- Fixed lag by improving our hitbox calculation/maintenance so that hitboxes were not being calculated hundreds of times per update
- Fixed a bug where the robot would keep moving, if the player brought up the menu while the robot was moving on the ground
- Made it so that collision sounds would not continously play if the player kept trying to move forward after colliding with a block
- Fixed a bug where the rocket sound would keep playing as long as the player held down the spacebar (this would happen even when the robot could no longer fly)
- Fixed some lag that was coming from shadows by making far away pixels be less accurate
- Fixed a menu bug that meant if you didn't move your mouse after going to a new menu and clicked the click would happen in the wrong spot
- Made the camera not pan off to the left or right of the level since it's just darkness anyway
- Fixed a bug where some sound effects would fail to play/play very quietly. Not 100% sure that this is fixed, but it happens less frequently now
- Fixed a bug where sound effects would keep playing, even when the paused menu was opened. Now, sound effects pause when opening the menu and resume upon closing the menu

Have completed the following parts of our plan:
- Added story element that tie together the game play, and give players a concrete goal while playing the game
- Added intro story and information about background
- Added cut scene for successful completion of the game
- Added more sound effects (new menu music, level-editor music, sounds that play when the robot lands)
- Made a new sound system for better sound management
- Added invisible blocks which react the way that normal ones do, but are invisible. (Mainly for menu screens and level select)
- Added new sprites for level selection
- Added new ghost sprites (we were worried that the first draft didn't completely fit with our overall theme)

Have made changes that were not in the plan but were small extra features we thought were good ideas:
- Added a settings menu so that players can adjust the scroll speed (usefull when testing on a laptop)
- Level Editor for continued playability (Not actually very small at all! Go Harlan!)



## Playable Game Report:

For this milestone, we:

Fixed the following from ideas from crossplay/previous bugs:
- Made the headlamp turn all the way around with the mouse
- Added tutorials for how to use the mouse to shine the light
- Fixed some graphical lag by making only a single mesh (and other render data) for each object of which there are many instances

Have completed the following parts of our plan:
- Added lighting physics and shadows
- Added a couple more levels and refined older levels
- Most of the momentum/physics parts were completed in previous milestones
- Made the robot's head collide with the ceiling and walls
- Made ghosts pathfind only around blocks that are currently visible
- Added a bunch of sprites
- Added sound effects (e.g. rocket, locked doors, etc.) and better background music
- Extended our use of the JSON library to include save files (previously was just level data)

Have made changes that were not in the plan but were small extra features we thought were good ideas:
- Added menu screens with the ability to save, load, reset the level, pause the game, and exit
- Made the movement of the robot more consistent
- Added coloured bricks and ghosts that only show up when the correct colour light is shone on them
- Increased camera pan size and made camera pan from the level-goal to the player on level load

Began Implementation to be refined for the next milestone:
- Made pushable blocks
- Clues for player about where there are 'hidden' bricks



## For our own reference:

When building levels:
 
 Bricks:
 
  - B = normal brick show up regardless of headlight color
  - C = Brick that shows up only in red light
  - M = Brick that shows up only in green light
  - N = Brick that shows up only in blue light
  - I = Brick that never appear, but are always interactable
  
Ghosts:
  
  - G = Normal ghost chases regardless of headlight color
  - Y = Ghost that chases in any light other than red
  - Z = Ghost that chases in any light other than green
  - L = Ghost that chases in any light other than blue
