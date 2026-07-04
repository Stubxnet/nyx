## Nyx

> [!WARNING]
> Actually this project is under active development, so you can find :
> Dead code ;
> Not-working features ;
> Bugs ;
> Crashes.

This project aims to create a original, Minecraft-like voxel game.

Not to copy Minecraft, but creating an alternate universe where forgotten suggestions from the r/minecraftsuggestions subreddit can finally be implemented...

### Compilation

Actually the project works only for Linux.

I'm trying to create a little script to launch the project automatically but I'm very lazy so...

```bash
rm build/
```
```bash
mkdir build && cd build
```
```bash
cmake ..
```
```
make
```
### Key bindings

Everywhere :

`Ctrl+C` to exit program.

When in-game :

 - mouse to look around,
 - `WASD` to move,
 - `Space` to jump/move up,
 - `Left-Shift` to sneak/move down,
 - Left-click to break a block,
 - Right-click to place a block,
 - `F1` to hide interface,
 - `F2` to take a screenshot,
 - `F3` to show/hide debug menu,
 - `F5` to third-person view (currently not working)
 - `T` to open chat,
 - `Enter` to send message/execute command typed in chat bar
 - `Backspace` to delete the last character typed

Supported commands :

 - `/setblock  x y z id` (alias : `/sb`)
 - `/fill ax ay az bx by bz id`
 - `/tp x y z` (currently not working due to a bug)
 - `/renderdistance  renderdistance` (alias `/rd`, only way to change render distance)
 - `/rotation  x y z` (alias `/rt`, three float, not working due to the same bug as /tp)
 - `/fov fov` (only way to change FOV)
 - `/gamemode [0|1|2|3] [survival|creative|spectator|builder]`
 - `/fly` to activate/deactivate creative fly mode

### Supported features

- [X] Chunks meshes
- [X] Commands
- [X] Collisions player/blocks
- [X] DDA Raycasting
- [X] Textures atlas
- [ ] Region files
- [ ] Client/Server system
- [ ] Entities
- [ ] Entities raycasting
- [ ] Lighting system
- [ ] Pathfinding
- [ ] Procedural generation:
  - [ ] Biome generator
  - [ ] Heightmaps generator
  - [ ] Prefabs
  - [ ] Structures
- [ ] Adjustable keybindings
- [ ] Video settings
- [ ] Audio
- [ ] Music

### Contributors

Thanks to ME to have developped ALL this project !

(And to Energomy for pointing out some of the glaring flaws in my code...)
