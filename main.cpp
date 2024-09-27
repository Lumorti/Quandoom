#include <fstream>
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include "SDL.h" // sudo apt-get install libsdl2-dev
#include <omp.h> // sudo apt-get install libomp-dev

// Entry point
int main(int argc, char** argv) {

    // Load the circuit file
    std::vector<std::vector<std::vector<int>>> circuits(1);
    std::ifstream file(argv[1]);
    std::string line = "";
    std::vector<int> info = {};
    int currentFunction = 0;
    while (std::getline(file, line)) {
        if (line[0] == '}') {currentFunction = 0;}
        else if (line[0] == 'x' || line[0] == 'h' || line[0] == 'g' || line[0] == 'G' || line[0] == 'c' || line[0] == '#') {
            std::vector<int> qubits = {0};
            std::string currentNum = "";
            bool isFuncCall = false;
            for (size_t i = 0; i < line.size(); i++) {
                if (line[i] >= '0' && line[i] <= '9') {currentNum += line[i];}
                else if (line[0] != 'g' && line[i] == 'G') {isFuncCall = true;}
                else if (line[i] == 'c') {qubits[0]++;}
                else if ((line[i] == ']' || line[i] == ' ' || line[i] == ';') && currentNum != "") {
                    qubits.push_back(std::stoi(currentNum));
                    currentNum = "";
                }
            }
            if (isFuncCall) {
                std::vector<int> newQubits = {qubits[0]};
                for (int i = 2; i < 2+qubits[0]; i++) {
                    newQubits.push_back(qubits[i]);
                }
                newQubits.push_back(-qubits[1]);
                qubits = newQubits;
            }
            if (line[0] == '#' && line[1] == 'w') {qubits = {-1};}
            else if (line[0] == 'h') {qubits.insert(qubits.begin()+1, info[0]);}
            else if (line[0] == '#' && line[1] == 'j') {qubits[0] = -qubits[1]; qubits.erase(qubits.begin()+1);}
            if (line[0] == '#' && line[1] == 'q') {info = qubits; info.erase(info.begin());}
            else if (line[0] == 'g') {currentFunction = qubits[1]; circuits.resize(std::max(circuits.size(), size_t(qubits[1]+1)));}
            else {circuits[currentFunction].push_back(qubits);}
        }
    }
    file.close();

    // Create a window and start the main game loop
    omp_set_num_threads(info[5]);
    SDL_Window *screen = SDL_CreateWindow("Quandoom", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_RESIZABLE);
    SDL_Renderer *renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_SOFTWARE);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, 0);
    SDL_Event e;
    const Uint8* keystates = SDL_GetKeyboardState(NULL);
    bool quit = false;
    std::vector<bool> stateInit(info[0], 0);
    #pragma omp parallel default(shared)
    {
        int thread = omp_get_thread_num() + 1;
        while (!quit) {
            auto t1 = std::chrono::high_resolution_clock::now();

            // Run the circuit
            for (int i = info[1]; i < info[0]; i++) {stateInit[i] = false;}
            int currentCircuit = 0;
            std::vector<std::pair<int, int>> prevPos;
            std::vector<bool> state(stateInit);
            for (int i = 0; i < int(circuits[currentCircuit].size()); i++) {
                if (circuits[currentCircuit][i].size() == 1 && circuits[currentCircuit][i][0] == -1) {
                    #pragma omp barrier
                } else if (circuits[currentCircuit][i].size() >= 2 && circuits[currentCircuit][i][0] < 0) {
                    if (thread == -circuits[currentCircuit][i][0]) {
                        i += circuits[currentCircuit][i][1];
                    }
                } else {
                    bool allOn = true;
                    for (int j = 1; j < 1+circuits[currentCircuit][i][0]; j++) {
                        if (!state[circuits[currentCircuit][i][j]]) {allOn = false; break;}
                    }
                    if (allOn) {
                        if (circuits[currentCircuit][i][1+circuits[currentCircuit][i][0]] == info[0]) {
                            for (size_t j = 1; j < circuits[currentCircuit][i].size(); j++) {
                                state[circuits[currentCircuit][i][j]] = rand() % 2;
                            }
                        } else {
                            for (size_t j = 1+circuits[currentCircuit][i][0]; j < circuits[currentCircuit][i].size(); j++) {
                                if (circuits[currentCircuit][i][j] < 0) {
                                    prevPos.push_back({currentCircuit, i});
                                    currentCircuit = -circuits[currentCircuit][i][j];
                                    i = -1; break;
                                } else {state[circuits[currentCircuit][i][j]] = !state[circuits[currentCircuit][i][j]];}
                            }
                        }
                    }
                }
                if (currentCircuit > 0 && i >= int(circuits[currentCircuit].size())-1) {
                    i = prevPos[prevPos.size()-1].second;
                    currentCircuit = prevPos[prevPos.size()-1].first;
                    prevPos.pop_back();
                }
            }

            // Combine all of the local states
            if (thread == 1) {
                for (int i = 0; i < info[1]; i++) {stateInit[i] = state[i];}
            }
            #pragma omp critical
            for (int i = info[1]; i < info[0]; i++) {stateInit[i] = stateInit[i] != state[i];}
            #pragma omp barrier

            // Output the state to the screen
            if (thread == 1) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
                SDL_RenderClear(renderer);
                SDL_RenderSetLogicalSize(renderer, info[3], info[4]);
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                for (int j = 0; j < info[4]; j++) {
                    for (int i = 0; i < info[3]; i++) {
                        if (stateInit[info[2] + info[3]*j + i]) {
                            SDL_RenderDrawPoint(renderer, i, j);
                        }
                    }
                }
                SDL_RenderPresent(renderer);
            }

            // Framerate limiter: 20 fps = 1 frame / 50 milliseconds
            auto t3 = std::chrono::high_resolution_clock::now();
            int millis = std::chrono::duration_cast<std::chrono::milliseconds>(t3-t1).count();
            if (millis < 50) {SDL_Delay(50-millis);}

            // Adjust the state based on the keyboard inputs
            if (thread == 1) {
                while (SDL_PollEvent(&e)) {if (e.type == SDL_QUIT) {quit = true;}}
                std::vector<int> stateList = {SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT, SDL_SCANCODE_UP, 
                                  SDL_SCANCODE_DOWN, SDL_SCANCODE_SPACE, SDL_SCANCODE_LALT, 
                                  SDL_SCANCODE_LCTRL, SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3};
                for (size_t i = 0; i < stateList.size(); i++) {
                    if (keystates[stateList[i]]) {stateInit[i] = true;} 
                    else {stateInit[i] = false;}
                }
            }
            #pragma omp barrier

        }

    }

    // Quit the game
    SDL_DestroyWindow(screen);
    SDL_Quit();
    return 0;

}
