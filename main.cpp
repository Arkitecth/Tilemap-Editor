#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include <array>
#include <iostream>
#include <stdio.h>
#include <SDL3/SDL.h>
#include <string>
const int CELL_WIDTH = 49; 
const int CELL_HEIGHT= 49;
const int NUM_ROWS = 8; 
const int NUM_COLS = 8; 
struct Tile 
{
    SDL_Texture* texture{};
    float x{}; 
    float y{}; 
}; 

Tile selectedTile{};

SDL_FRect selectedRectangle{-1, -1, -1, -1}; 

using TileMap = std::array<std::array<Tile, NUM_COLS>, NUM_ROWS>; 

struct State 
{
    SDL_Renderer* renderer{}; 
    SDL_Window* window{}; 
    std::string currentFilePath{};
    Tile currentTile{};
    TileMap selectionTileMap{};
    TileMap canvasTileMap{};
}; 


void beginImguiFrame() {
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
}

void convertScreenToGrid(float screenX, float screenY, int& row, int& column) {
        column = int(screenX / CELL_WIDTH);
        row = int(screenY / CELL_HEIGHT);
}

void convertGridToScreen(int row, int column, float& x, float& y) {
        x = column * CELL_WIDTH; 
        y = row * CELL_HEIGHT; 
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
        //Initalize Tile
        State* state = reinterpret_cast<State*>(userdata); 
        state->currentFilePath = *filelist; 
        state->currentTile.x = selectedRectangle.x; 
        state->currentTile.y = selectedRectangle.y; 
        SDL_Surface* surface = SDL_LoadSurface(state->currentFilePath.c_str()); 
        SDL_Texture* texture = SDL_CreateTextureFromSurface(state->renderer, surface); 
        state->currentTile.texture = texture; 
        SDL_DestroySurface(surface); 
        //Place Tile
        int row{};
        int column{}; 
        convertScreenToGrid(state->currentTile.x, state->currentTile.y, row, column); 
        state->selectionTileMap[row][column] = state->currentTile;
        filelist++;
    }
} 

void loadTile(void* state) {
    State* s = reinterpret_cast<State*>(state); 
    SDL_ShowOpenFileDialog(accessFile, state, s->window, nullptr, 0, "./", false); 
}

void renderSelectorTileMap(State* state) {
    for(int i = 0; i < NUM_ROWS; i++) 
    {
        for(int j = 0; j < NUM_COLS; j++) 
        {
            if (state->selectionTileMap[i][j].texture) 
            {
                SDL_FRect dstRect
                {
                    state->selectionTileMap[i][j].x, 
                    state->selectionTileMap[i][j].y, 
                    float(state->selectionTileMap[i][j].texture->w), 
                    float(state->selectionTileMap[i][j].texture->h)
                }; 

                SDL_RenderTexture(state->renderer, state->selectionTileMap[i][j].texture, nullptr, &dstRect); 
            
            }
        }
    }
}

void renderCanvasTileMap(State* state) {
    for(int i = 0; i < NUM_ROWS; i++) 
    {
        for(int j = 0; j < NUM_COLS; j++) 
        {
            if (state->canvasTileMap[i][j].texture) 
            {
                SDL_FRect dstRect
                {
                    state->canvasTileMap[i][j].x, 
                    state->canvasTileMap[i][j].y, 
                    float(state->canvasTileMap[i][j].texture->w), 
                    float(state->canvasTileMap[i][j].texture->h)
                }; 
                SDL_RenderTexture(state->renderer, state->canvasTileMap[i][j].texture, nullptr, &dstRect); 
            }
        }
    }
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



void renderSelectionRect(ImGuiIO* io, State* state) {
    if (ImGui::IsMouseHoveringRect(ImVec2{0, 0}, ImVec2{392, 392}, false)) 
    {
        int row{}; 
        int column{};
        convertScreenToGrid(io->MousePos.x, io->MousePos.y, row, column); 
        float x{};
        float y{};
        convertGridToScreen(row, column, x, y); 
        SDL_FRect rect{x, y, CELL_WIDTH, CELL_HEIGHT}; 
        SDL_SetRenderDrawColor(state->renderer, 0.0f, 0.0f, 255.0f, 0.0f); 
        SDL_RenderRect(state->renderer, &rect); 
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) 
        {
            int row{}; 
            int column{};
            convertScreenToGrid(io->MousePos.x, io->MousePos.y, row, column); 
            float x{};
            float y{};
            convertGridToScreen(row, column, x, y); 
            state->currentTile = state->selectionTileMap[row][column];
            state->currentTile.x = x; 
            state->currentTile.y = y; 
            selectedRectangle = SDL_FRect{x, y, CELL_WIDTH, CELL_HEIGHT}; 
        }

    } else if(ImGui::IsMouseHoveringRect(ImVec2{500, 0}, ImVec2{900, 500}, false)) {
            int row{}; 
            int column{};
            convertScreenToGrid(io->MousePos.x, io->MousePos.y, row, column); 
            float x{};
            float y{};
            convertGridToScreen(row, column, x, y); 
            SDL_FRect rect{x, y, CELL_WIDTH, CELL_HEIGHT}; 
            SDL_SetRenderDrawColor(state->renderer, 0.0f, 0.0f, 255.0f, 0.0f); 
            SDL_RenderRect(state->renderer, &rect); 
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) 
            {
                int row{}; 
                int column{};
                convertScreenToGrid(io->MousePos.x, io->MousePos.y, row, column); 
                float x{};
                float y{};
                convertGridToScreen(row, column, x, y); 
                state->currentTile.x = x; 
                state->currentTile.y = y; 
                state->canvasTileMap[row % NUM_ROWS][column % NUM_COLS] = state->currentTile;
            }
     }

    if (selectedRectangle.x != -1) 
    {
        SDL_SetRenderDrawColor(state->renderer, 0.0f, 0.0f, 255.0f, 0.0f); 
        SDL_RenderFillRect(state->renderer, &selectedRectangle); 
    }
}

int main(int, char**)
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    bool done = false;
    State state{};
                                                              //
    SDL_CreateWindowAndRenderer("Tilemap Editor", 900, 500, SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_HIGH_PIXEL_DENSITY, &state.window, &state.renderer); 
    SDL_SetRenderVSync(state.renderer, 1);
    ImGui_ImplSDL3_InitForSDLRenderer(state.window, state.renderer);
    ImGui_ImplSDLRenderer3_Init(state.renderer);

    while (!done) 
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                done = true;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(state.window))
                done = true;
        }
        beginImguiFrame(); 

        ImGui::Begin("Select Tile Position");
        ImGui::Text("This is some useful text.");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

        if(ImGui::Button("Add Tile")) {
            loadTile(&state); 
        }

        ImGui::End(); 
       
        ImGui::Render();
        SDL_SetRenderScale(state.renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 0); 
        SDL_RenderClear(state.renderer);
        renderSelectionGrid(state.renderer); 
        renderExportGrid(state.renderer); 
        renderSelectionRect(&io, &state); 
        renderCanvasTileMap(&state); 
        renderSelectorTileMap(&state); 
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), state.renderer);
        SDL_RenderPresent(state.renderer);
    }
}
