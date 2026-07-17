#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include <stdio.h>
#include <SDL3/SDL.h>
#include <string>
#include <vector>

const int CELL_WIDTH = 49; 
const int CELL_HEIGHT= 49;
const int NUM_ROWS = 8; 
const int NUM_COLS = 8; 
struct Tile 
{
    std::string path{};
    float x{}; 
    float y{}; 
}; 
SDL_FRect selectedRectangle{-1, -1, -1, -1}; 

using TileMap = std::vector<Tile>;

struct State 
{
    Tile currentTile{};
    TileMap currentTileMap{};
}; 


void beginImguiFrame() {
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
}


void SDLCALL accessFile(void* userdata, const char * const *filelist, int filter) {
    if (!filelist) {
        SDL_Log("An error occured: %s", SDL_GetError());
        return;
    } else if (!*filelist) {
        SDL_Log("The user did not select any file.");
        SDL_Log("Most likely, the dialog was canceled.");
        return;
    }
    while (*filelist) {
        State* state = reinterpret_cast<State*>(userdata); 
        state->currentTile.path = *filelist; 
        state->currentTile.x = selectedRectangle.x; 
        state->currentTile.y = selectedRectangle.y; 
        state->currentTileMap.push_back(state->currentTile); 
        filelist++;
    }
} 

void loadTile(SDL_Window* window, void* userdata) {
    SDL_ShowOpenFileDialog(accessFile, userdata, window, nullptr, 0, "./", false); 
}

void renderTile(SDL_Renderer* renderer, State* state) {
    SDL_Surface* surface = SDL_LoadSurface(state->currentTile.path.c_str()); 
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface); 
    SDL_FRect dstRect{state->currentTile.x, state->currentTile.y, float(texture->w), float(texture->h)}; 
    SDL_RenderTexture(renderer, texture, nullptr, &dstRect); 
    SDL_DestroySurface(surface); 
}


void renderSelectionGrid(SDL_Renderer* renderer) {
    for(int i = 0; i < NUM_ROWS; i++) {
        for(int j = 0; j < NUM_COLS; j++) {
            SDL_FRect rect{float(j) * CELL_WIDTH, float(i) * CELL_HEIGHT, CELL_WIDTH, CELL_HEIGHT}; 
	    SDL_SetRenderDrawColor(renderer, 255.0f, 255.0f, 255.0f, 255.0f); 
            SDL_RenderRect(renderer, &rect); 
        }
    }
}

void renderExportGrid(SDL_Renderer* renderer) {
    for(int i = 0; i < NUM_ROWS; i++) {
        for(int j = 10; j < NUM_COLS + 10; j++) {
            SDL_FRect rect{float(j) * CELL_WIDTH, float(i) * CELL_HEIGHT, CELL_WIDTH, CELL_HEIGHT}; 
	    SDL_SetRenderDrawColor(renderer, 255.0f, 255.0f, 255.0f, 255.0f); 
            SDL_RenderRect(renderer, &rect); 
        }
    }
}


void convertScreenToGrid(float screenX, float screenY, int& row, int& column) {
        column = int(screenX / CELL_WIDTH);
        row = int(screenY / CELL_HEIGHT);
}

void convertGridToScreen(int row, int column, float& x, float& y) {
        x = column * CELL_WIDTH; 
        y = row * CELL_HEIGHT; 
}

void renderSelectionRect(SDL_Window* window, SDL_Renderer* renderer, ImGuiIO* io) {
    if (ImGui::IsMouseHoveringRect(ImVec2{0, 0}, ImVec2{392, 392}, false)) 
    {

        int row{}; 
        int column{};
        convertScreenToGrid(io->MousePos.x, io->MousePos.y, row, column); 
        float x{};
        float y{};
        convertGridToScreen(row, column, x, y); 
        SDL_FRect rect{x, y, CELL_WIDTH, CELL_HEIGHT}; 
        SDL_SetRenderDrawColor(renderer, 0.0f, 0.0f, 255.0f, 0.0f); 
        SDL_RenderRect(renderer, &rect); 

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) 
        {
            int row{}; 
            int column{};
            convertScreenToGrid(io->MousePos.x, io->MousePos.y, row, column); 
            float x{};
            float y{};
            convertGridToScreen(row, column, x, y); 
            selectedRectangle = SDL_FRect{x, y, CELL_WIDTH, CELL_HEIGHT}; 
        }
    }
    if (selectedRectangle.x != -1) 
    {
        SDL_SetRenderDrawColor(renderer, 0.0f, 0.0f, 255.0f, 0.0f); 
        SDL_RenderFillRect(renderer, &selectedRectangle); 
    }
}

int main(int, char**)
{
    SDL_Renderer* renderer{}; 
    SDL_Window* window{}; 
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
                                                              //
    SDL_CreateWindowAndRenderer("Tilemap Editor", 900, 500, SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_HIGH_PIXEL_DENSITY, &window, &renderer); 
    SDL_SetRenderVSync(renderer, 1);
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    bool done = false;
    State state{};
    while (!done) 
    {

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                done = true;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        beginImguiFrame(); 

        ImGui::Begin("Hello, world!");
        ImGui::Text("This is some useful text.");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

        if(ImGui::Button("Add Tile")) {
            loadTile(window, &state); 
        }

        ImGui::End(); 

       
        ImGui::Render();
        SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0); 
        SDL_RenderClear(renderer);
        renderSelectionGrid(renderer); 
        renderSelectionRect(window, renderer, &io); 
        renderExportGrid(renderer); 
        if (!state.currentTile.path.empty()) {
            renderTile(renderer, &state); 
        }

        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }
}
