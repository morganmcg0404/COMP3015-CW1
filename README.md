# COMP3015-CW1: Island Explorer - Coin Collection Game

A 3D first-person game built with OpenGL 4.6 featuring procedural terrain generation, dynamic day-night cycles, advanced graphics effects, and interactive coin collection mechanics.

## Development Environment

- **Operating System:** Windows 10/11 (x64)
- **Visual Studio Version:** Visual Studio 2022 (v143 toolset)
- **C++ Standard:** C++17
- **Graphics API:** OpenGL 4.6 Core Profile
- **Build Configuration:** Debug and Release (x64 platform)

## How It Works

### Game Objective
Explore a procedurally generated island, collect 10 coins scattered across the terrain, and reach the game completion screen. The game features natural physics, water interactions, and a complete day-night cycle.

### Core Mechanics

**Player Movement & Control:**
- WASD keys to move across the island
- Mouse to look around (first-person view)
- Space to jump with realistic physics
- Shift to sprint (1.5x speed multiplier)
- Z + Mouse Scroll to zoom camera (1x-5x magnification)

**Environment Interaction:**
- Walk on procedurally generated terrain with mountain and plains biomes
- Enter the lake and float on the water surface
- Bounce off water when jumping into it
- Smooth camera transitions when climbing slopes

**Coin Collection:**
- 10 coins spawn randomly on the island
- Coins face toward the player and rotate slowly
- Proximity-based collection (3-unit pickup radius)
- Collection cooldown prevents accidental multi-collection
- Completion screen displays when all 10 coins collected

**Visual Features:**
- **Blinn-Phong Shading:** Pure implementation using half-vector approach
- **Normal Mapping:** Tangent-space normal mapping with TBN matrix transformation
- **Day-Night Cycle:** Full 30-second rotation with dynamic lighting and color transitions
- **HDR Rendering:** HDR framebuffer with bloom effect
- **Bloom Effect:** 5-stage post-processing with bright-pass filter and separable Gaussian blur
- **Gaussian Blur:** Separable 5-tap Gaussian blur for bloom and image processing
- **Water System:** Animated waves with Fresnel reflection and specular highlights
- **Vignette:** Screen edge darkening for cinematic feel
- **Gamma Correction:** 2.2 gamma for proper color rendering
- **Procedural Terrain:** Seeded noise-based island generation with biome blending

## Code Structure & Navigation

### Project Layout

```
COMP3015-CW1/
├── scenebasic_uniform.h/.cpp    [Main game logic and scene management]
├── arena.h/.cpp                 [Procedural terrain generation with seeded noise]
├── helper/
│   ├── glslprogram.h/.cpp      [Shader compilation and management]
│   ├── glutils.h/.cpp          [OpenGL utility functions]
│   ├── drawable.h              [Base drawable class interface]
│   ├── trianglemesh.h/.cpp     [Triangle mesh object loader]
│   ├── scene.h                 [Base scene class interface]
│   ├── scenerunner.h           [Window and input management]
│   ├── text_renderer.h/.cpp    [FreeType text rendering system]
│   └── stb/                    [Image loading library]
├── shader/
│   ├── basic_uniform.vert/.frag    [Main scene rendering]
│   ├── shadow.vert/.frag           [Shadow map generation]
│   ├── bloom_*.vert/.frag          [Bloom post-processing (3 stages)]
│   ├── tonemap.vert/.frag          [HDR tone mapping]
│   ├── vignette.vert/.frag         [Vignette effect]
│   ├── water.vert/.frag            [Water surface rendering]
│   └── text.vert/.frag             [Text rendering]
├── models/                      [3D models (coin.gltf)]
├── textures/                    [Terrain and object textures]
└── dependencies/                [External libraries with pre-built binaries]
    ├── GLFW/                    [Window management]
    ├── GLAD/                    [OpenGL loader]
    ├── GLM/                     [Math library]
    └── FreeType/                [Font rendering]
```

### Key Classes & Systems

**SceneBasic_Uniform (Main Scene)**
- `initScene()` - Initializes all game systems, loads models and textures
- `update(float t)` - Game loop: physics, coin collection, input handling
- `render()` - Renders all objects with complete graphics pipeline
- `handleInput()` - Processes keyboard/mouse input
- `updateCamera()` - Smooth camera interpolation for natural movement

**Terrain System**
- `createTerrainChunk()` - Generates individual 20x20 unit terrain patches
- `updateTerrainChunks()` - Loads/unloads chunks based on proximity to island
- `getTerrainHeightAt()` - Samples terrain height using seeded Perlin-like noise
- Biome blending: 75% plains, 25% mountains with smooth transitions
- Lake depression: 20-unit radius with smooth falloff edges

**Coin System**
- `loadCoinModel()` - Loads coin model using TriangleMesh loader
- `spawnCoins()` - Initial coin placement
- `spawnSingleCoin()` - Random spawn after collection
- Coins rendered with rotation and face-toward-player logic

**Graphics Pipeline Flow:**
1. **Scene Pass** - Render terrain with Blinn-Phong shading and normal mapping to HDR framebuffer
2. **Coin Rendering** - Render coins with pure Blinn-Phong shading (gold material)
3. **Water Rendering** - Render water surface with Fresnel reflection and animated waves
4. **Bright-Pass Filter** - Extract bright pixels above 0.85 luminance threshold
5. **Gaussian Blur** - Apply separable 5-tap horizontal and vertical blur to bloom texture
6. **Bloom Blending** - Combine bloomed bright areas with scene using additive blending
7. **Vignette Pass** - Apply darkening at screen edges for cinematic effect
8. **Gamma Correction** - Apply 2.2 gamma correction and tone mapping
9. **UI Rendering** - Render coin counter and completion text overlay

### Important Constants

- **Island Radius:** 150 units (with 100-unit falloff)
- **Chunk Size:** 20x20 units
- **Water Level:** 0.5 units
- **Player Height:** 1.7 units
- **Coin Pickup Radius:** 3.0 units
- **Day-Night Cycle Period:** 30 seconds
- **HDR Bloom Threshold:** 0.85
- **Camera Interpolation Speed:** 8.0 units/sec

### Navigation Tips for Programmers

1. **Starting Point:** `main.cpp` calls `SceneRunner` which instantiates `SceneBasic_Uniform`
2. **Game Loop:** `update()` → `handleInput()` → `updateCamera()` → `render()`
3. **Rendering Order:** Scene → Bloom (Bright-Pass → Blur) → Bloom Blending → Vignette → Gamma/Tone Mapping → UI
4. **Shader Uniforms:** All shaders receive `MVP` matrix, lighting info, and effect-specific parameters
5. **Terrain Procedural Generation:** Uses seeded noise with fixed seed per frame for consistency
6. **Memory Management:** Terrain chunks delete GPU resources when unloaded; coins use dynamic spawning

## Technical Details

### Physics Implementation
- Gravity: 9.81 units/sec²
- Jump Power: 8.0 units (impulse-based)
- Water Damping: 0.6x velocity bounce factor
- Smooth camera height interpolation at 8.0 units/sec

### Post-Processing Pipeline
- **Bloom:** 3-stage Gaussian blur with 0.85 brightness threshold
- **Bloom Blending:** Additive blending of bloom texture with gamma correction (2.2)
- **Gamma:** 2.2 correction applied in final pass
- All effects stackable and individually toggleable

### Asset Loading
- Models: TriangleMesh loader
- Textures: STB Image (PNG, JPG support)
- Fonts: FreeType (TTF support)
- All loads include fallback to default white texture/color if missing

## Assets

### Textures
- **Dirt Texture:** [Earth Texture - Freepik](https://www.freepik.com/free-vector/earth-texture_997013.htm#fromView=search&page=1&position=0&uuid=235b724d-4985-419f-a108-d80a2ac18e47&query=Texture+cartoon+ground)
- **Grass Texture:** [Abstract Light Green Background - Freepik](https://www.freepik.com/free-vector/abstract-light-green-background-simple-style_89175903.htm#fromView=search&page=3&position=43&uuid=235b724d-4985-419f-a108-d80a2ac18e47&query=Texture+cartoon+ground)
- **Stone Texture:** [Stacked Stones - Freepik](https://www.freepik.com/free-photo/stacked-stones_1034248.htm#fromView=search&page=1&position=43&uuid=235b724d-4985-419f-a108-d80a2ac18e47&query=Texture+cartoon+ground)


## Demonstration Video

**YouTube Video:** [Insert unlisted YouTube link here]

**Last Updated:** February 26, 2026  
**Status:** Complete for COMP3015 CW1 submission
