# Hypercube
Game with an adaptive difficulty system based on Unreal Engine 4.

![image](https://github.com/wrongserenity/shabash-studios-project/assets/43683367/1c047c66-b500-4023-96a2-658ce8ed838e)


# Gave description
## Controls
- wasd - walk
- space - jump
- shift - dash
- lmb - attack
- esc - pause

![image](https://github.com/wrongserenity/shabash-studios-project/assets/43683367/d56bf54c-d883-4d80-a5ed-7c217f3f4645)


## Player target
Score the maximum number of points.
The more enemies run after you - the greater the damage and points multiplier.

## Level complite and adaptivity
The race ends in two cases:
- HP less than 1 (defeat)
- Less than 1 enemies (victory)

![image](https://github.com/wrongserenity/shabash-studios-project/assets/43683367/e4695831-dd9e-4595-9941-6f961fc46dc6)
![image](https://github.com/wrongserenity/shabash-studios-project/assets/43683367/62316864-f866-434d-a5f6-5b6a35b84f41)

Each subsequent race will differ in a number of parameters. For example: character speeds, damage, vampirism, number of enemies on the level, modifier

## Details

![image](https://github.com/wrongserenity/shabash-studios-project/assets/43683367/ea54d2b5-d2ef-4a7f-a7f0-9c466359959a)

On the top left of the HUD you can find:
- health points
- multiplier
- dash cooldown
- points scored

At the levels you can find interactive elements:
- reduction of enemy damage
- slowing down the enemy
- player character acceleration

## Released features
Player
- running and turning the camera
- jump and dash
- attack
- animations
- sounds
- visual effects

Enemy
- run, jump and attack
- navmesh and navlink system
- crowd control
- detection of the main character
- animations, sounds and visual effects

UI
- pause menu with local leaderboard
- HUD with HP, Multiplier and Score
- screen damage effects
- indication of enemy aggro and effect status
- enemy health display

Adaptive difficulty
- depends on
  - number of restarts
  - number of opponents at the time of loss
  - last race time
- affects
  - mc speed 
  - mc damage modifier 
  - mc vampirism 
  - speed of enemies
  - enemy damage
  - enemy visibility radius
  - number of enemies in the level
- appears in the pause menu

Logic
- multiplier depending on opponents
- sequential launch of levels

Sound
- combat music with analysis of combat activity
- sounds of systems and various actions
- sound spars system

Levels
- Tutorial
- first level
- second level
- final level
- elements of diversity
  - acceleration
  - slowdown
  - damage reduction
 
