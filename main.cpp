#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include <stdio.h>
#include <SDL3/SDL.h>
#include <string>

void beginImguiFrame() {
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
}
struct TileMap {
        std::string path{};
}; 

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
        //SDL_Log("Full path to selected file: '%s'", *filelist);
        TileMap* tile = reinterpret_cast<TileMap*>(userdata); 
        tile->path = *filelist; 
        filelist++;
    }
}; 

void loadTileMap(SDL_Window* window, void* userdata) {
    SDL_ShowOpenFileDialog(accessFile, userdata, window, nullptr, 0, "./", false); 
}


void renderTileMap(SDL_Renderer* renderer, TileMap* tileMap) {
    SDL_Surface* surface = SDL_LoadSurface(tileMap->path.c_str()); 
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface); 
    SDL_FRect dstRect{100.0f, 100.0f, float(texture->w), float(texture->h)}; 
    SDL_RenderTexture(renderer, texture, nullptr, &dstRect); 
    SDL_DestroySurface(surface); 
}




void drawTileGrid(SDL_Renderer* renderer) {
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            SDL_FRect rect{float(j) * 49, float(i) * 49, 49.0f, 49.0f}; 
	    SDL_SetRenderDrawColor(renderer, 255.0f, 255.0f, 255.0f, 255.0f); 
            SDL_RenderRect(renderer, &rect); 
        }
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

    SDL_CreateWindowAndRenderer("Tilemap Editor", 900, 500, SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_HIGH_PIXEL_DENSITY, &window, &renderer); 

    SDL_SetRenderVSync(renderer, 1);
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    bool done = false;
    TileMap tile{};
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

        if(ImGui::Button("Load TileMap")) {
            loadTileMap(window, &tile); 
        }

        ImGui::End(); 

       
        ImGui::Render();
        SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0); 
        SDL_RenderClear(renderer);
        drawTileGrid(renderer); 
        if (!tile.path.empty()) {
            renderTileMap(renderer, &tile); 
        }

        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }
}
