
## Quandoom

It is a well-known fact that all useful computational devices ever created are [capable of running DOOM](https://www.reddit.com/r/itrunsdoom/). Despite decades of active research, there is yet to be developed a single practical use for quantum computers. This changes today, with the release of Quandoom, a port of the first level of DOOM designed for a quantum computer, given as a single QASM file, using a mere 70,000 qubits and 80 million gates. Although such a quantum computer doesn't exist right now, Quandoom is efficiently simulatable on a classical computer, capable of running at 10-20 fps on my laptop using the accompanying lightweight (150 lines of C++) QASM simulator.

The game loop is as follows:
1) the user pressing a key sets the value of one of the input qubits
2) the QASM file containing all of the quantum gates is applied to the entire state
3) the last 64,000 qubits are measured and displayed as a 320 x 200 screen of binary pixels
4) the screen and input qubits are reset and the process repeats

![Gif of Quandoom](example.gif)

### How To Play It

*Please note - it will use about 5-6 GB of ram and take a while to load since it's a very large circuit file.*

First, download the latest release for your system from ![here](https://github.com/Lumorti/Quandoom/releases/latest) and unzip it.

Windows: run the quantum circuit (quandoom.qasm) with the simulator by dragging it onto simulator.exe

Linux: add run permissions to the simulator using `chmod +x ./simulator.AppImage` and then run the circuit by running `./simulator.AppImage quandoom.qasm`

### Changes Versus The Original

Alright, so after like a year of working on this I got bored, so it doesn't have all the features from the original DOOM. More specifically:

- it's only the first level, more could be added, it's just a matter of mapping them
- everything is x-ray due to reversibility shenanigans
- no color because it'd make rendering a lot harder and the x-ray would look weirder
- no music or sound
- no level secrets, although again it's just a matter of mapping
- enemies can't travel between rooms
- no automap, because eh
- imp fireball is hitscan rather than a projectile
- minor tweaks regarding armor/health/damage to make it more reversible-friendly

### Compiling The Simulator

If the distributed executable doesn't work for you (or you don't trust my binaries), you can compile it yourself on Linux using:
```bash
sudo apt-get install g++ make libsdl2-dev libomp-dev -y
git clone https://github.com/Lumorti/Quandoom
cd Quandoom
make
```

Or on macOS using:
```bash
brew install libomp sdl2
git clone https://github.com/Lumorti/Quandoom
cd Quandoom
make macos
```

### Technical Details

The circuit needs 72,376 total qubits, 8,376 qubits not counting the screen, of which 6,986 are ancilla qubits. The circuit file has 83,651,224 lines, so at least that many gates (will actually be more, since many lines are subroutines).

Please note that the simulator is tailor-made for the this QASM file, it will not run a general QASM file. The Quandoom QASM file is also not *completely* QASM compliant, as described in a comment at the top of the QASM, but only because some qubit lists are abbreviated as "ALLQUBITS" because if I didn't do that then it'd be a >30GB file. The rest of the instructions are all valid n-controlled Toffolis and Hadamards, but of course mostly Toffolis since it's replicating a classical algorithm. There is no quantum advantage, it's just a classical algorithm written in a format compatible with a quantum computer.

For now I'm still tidying up the engine code, but basically I have about 8,000 lines of c++ functions allowing a number of reversible binary and arithmetic operations on quantum registers, for example "flipIfLessThanOrEqualTo" which flips all qubits in a register if the value of another register is less than some given value. Everything is done with integers. Using such functions I then wrote a small 3D engine as well as all the game logic. Also present is an ancilla system, a garbage system, as well as a quantum subroutine system and many other handy tools. Mapping and spritework was done by hand. Sprite scaling and ray casting are baked-in (i.e. pre-calculated). Parallelisation is done at the rendering stage, such that the list of render objects is split between cores and comments are left in the QASM to tell the simulator where to jump based on OpenMP thread ID. Maybe soon I'll make a little document and put it on the arXiv if there's interest. Or, if you want the code to expand or improve it, let me know and I'll rush the tidying up.

### Legal Disclaimer

This is a recreation from scratch of the first level of the original DOOM game, which was released as shareware by Id Software in 1997. This project is released as a non-commercial open-source project made in good faith in relation to the "anything can run DOOM" internet meme, for which a vast number of fan-made ports to various devices already exist. This project does not serve as a replacement for the original DOOM, acting purely as a joke/parody, a fact which should be obvious to anyone using it. DOOM, its characters, assets and level design are the intellectual property of Id Software. The author of this work advises anyone who actually wants to play the game to download the official shareware version. 
