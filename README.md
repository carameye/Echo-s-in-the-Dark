# ECHOs in the Dark

## Playable Game Report:

For this milestone, we have completed the following parts of our plan:
- Added lighting physics and shadows
- Added menu screens with the ability to save, load, reset the level, pause the game, and exit
- Fixed some graphical lag by making only a single mesh (and other render data) for each object of which there are many instances
- Added a couple more levels and refined older levels
- Most of the momentum/physics parts were completed in previous milestones
- Made the movement of the robot more consistent
- Made the robot's head collide with the ceiling and walls
- Added coloured bricks and ghosts that only show up when the correct colour light is shone on them
- Added a screen effect that notifies the player that there is something invisible nearby
- Made pushable blocks
- Followed advice from crossplay and made the headlamp turn all the way around with the mouse
- Added a bunch of sprites



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
