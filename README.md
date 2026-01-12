# Tank Clash 🚀

A 2D tank battle game developed using C++ and iGraphics library. Engage in intense tank warfare with smooth gameplay mechanics and immersive audio-visual effects.

## 🎮 Game Features

- **2D Tank Combat**: Control your tank and battle against enemies
- **Smooth Graphics**: Built with iGraphics for fluid visual experience
- **Audio Integration**: Immersive sound effects and background music
- **Real-time Gameplay**: Fast-paced action with responsive controls
- **Cross-platform**: Runs on Windows systems with proper setup

## 🛠️ Technologies Used

- **Programming Language**: C++
- **Graphics Library**: iGraphics
- **Audio System**: Integrated audio support
- **Development Environment**: Compatible with Visual Studio/Code::Blocks

## 📋 Prerequisites

Before running the game, ensure you have:

- C++ compiler (GCC/MSVC)
- iGraphics library installed
- OpenGL support
- Audio libraries (for sound functionality)
- Windows OS (recommended)

## 🚀 Installation & Setup

1. **Clone the Repository**
   ```bash
   git clone https://github.com/Aragon-07/Tank_Clash.git
   cd Tank_Clash
   ```

2. **Install iGraphics**
   - Download and install the iGraphics library
   - Configure your IDE to include iGraphics headers and libraries

3. **Compile the Game**
   ```bash
   g++ -o TankClash *.cpp -liGraphics -lGL -lGLU -lglut
   ```

4. **Run the Game**
   ```bash
   ./TankClash
   ```

5.## Download the Game**

You can download the Tank Clash game from the following link:

[Download Tank Clash](https://drive.google.com/drive/folders/19aP4vLF8vIlSMo1VOForPkWuj4E7w7zj?usp=sharing)


## 🎯 How to Play

- **Movement**: Use arrow keys or WASD to move your tank
- **Shooting**: Use spacebar or mouse click to fire
- **Objective**: Destroy enemy tanks while avoiding their attacks
- **Survival**: Manage your tank's health and ammunition

## 📁 Project Structure

```
Tank_Clash/
├── Audios/Audios/          # Game sound effects and music
├── image/                  # Game sprites and textures
├── Audio.hpp               # Audio system header
├── Constants.hpp           # Game constants and configurations
├── GameLogic.hpp          # Core game logic and mechanics
├── Globals.hpp            # Global variables and definitions
├── Structures.hpp         # Data structures and classes
├── iMain.cpp              # Main game file and entry point
└── .gitattributes         # Git configuration
```

## 🎨 Assets

- **Graphics**: Custom tank sprites, backgrounds, and UI elements
- **Audio**: Sound effects for shooting, explosions, and background music
- **Animations**: Smooth tank movements and battle effects

## 🔧 Configuration

Game settings can be modified in `Constants.hpp`:
- Screen resolution
- Game speed
- Audio settings
- Control mappings

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 📝 Development Notes

- Built as a learning project to explore C++ game development
- Utilizes object-oriented programming principles
- Modular code structure for easy maintenance and expansion
- Optimized for smooth gameplay experience

## 🐛 Known Issues

- Performance may vary based on system specifications
- Audio might require additional codec support on some systems
- Graphics rendering depends on OpenGL compatibility

## 🔮 Future Enhancements

- [ ] Multiplayer support
- [ ] Additional tank types and weapons
- [ ] Level progression system
- [ ] Enhanced AI for enemy tanks
- [ ] Mobile platform support

## 📄 License

This project is open source and available under the [MIT License](LICENSE).

## 👨‍💻 Author

- **Mrinmoy Shib**        
- **Shirsha Chowdhury**          
- **Naimul Islam Jehan** 
  

## Project Contributors

1. Shirsha Chowdhury
2. Mrinmoy Shib
3. Naimul Islam Jehan


## 🙏 Acknowledgments

- iGraphics library developers
- C++ community for resources and support
- Game development tutorials and documentation

---

**Enjoy the Tank Clash experience! 🎮**

*For bug reports or feature requests, please open an issue on GitHub.*
