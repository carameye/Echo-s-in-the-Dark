# ECHOs in the Dark

## Playable Game Report:



For this milestone, we:
Fixed the following from ideas from crossplay/previous bugs:
- Made the headlamp turn all the way around with the mouse
- Fixed some graphical lag by making only a single mesh (and other render data) for each object of which there are many instances

Have completed the following parts of our plan:
- Added lighting physics and shadows
- Added a couple more levels and refined older levels
- Most of the momentum/physics parts were completed in previous milestones
- Made the robot's head collide with the ceiling and walls
- Made ghosts pathfind only around blocks that are currently visible
- Made pushable blocks
- Added a bunch of sprites
- Added sound effects (e.g. rocket, locked doors, etc.) and better background music
- Extended our use of the JSON library to include save files (previously was just level data)

Have made changes that were not in the plan but were small extra features we thought were good ideas:
- Added menu screens with the ability to save, load, reset the level, pause the game, and exit
- Made the movement of the robot more consistent
- Added coloured bricks and ghosts that only show up when the correct colour light is shone on them
- Increased camera pan size and made camera pan from the level-goal to the player on level load



## For our own reference:

When building levels:
 
 Bricks:
 
  - B = normal brick show up regardless of headlight color
  - C = Brick that shows up only in red light
  - M = Brick that shows up only in green light
  - N = Brick that shows up only in blue light
  
Ghosts:
  
  - G = Normal ghost chases regardless of headlight color
  - Y = Ghost that chases in any light other than red
  - Z = Ghost that chases in any light other than green
  - L = Ghost that chases in any light other than blue
