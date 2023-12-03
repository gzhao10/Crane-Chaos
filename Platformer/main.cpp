/**
* Author: Gavin Zhao
* Assignment: Platformer
* Date due: 2023-12-02, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define LOG(argument) std::cout << argument << '\n'
#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_mixer.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"
#include "Map.h"
#include "Scene.h"
#include "Utility.h"
#include "Effects.h"
#include "LevelM.h"
#include "LevelA.h"
#include "LevelB.h"
#include "LevelC.h"

// ––––– CONSTANTS ––––– //
const int   WINDOW_WIDTH = 640,
            WINDOW_HEIGHT = 480;

const float BG_RED = 1.0f,
            BG_BLUE = 0.7137f,
            BG_GREEN = 0.7137f,
            BG_OPACITY = 1.0f;

const int   VIEWPORT_X = 0,
            VIEWPORT_Y = 0,
            VIEWPORT_WIDTH = WINDOW_WIDTH,
            VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char  V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
            F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND  = 1000.0;


// ––––– GLOBAL VARIABLES ––––– //
Scene*  g_current_scene;
LevelM* g_levelM;
LevelA* g_levelA;
LevelB* g_levelB;
LevelC* g_levelC;
Scene*  g_levels[4];
Entity* g_health;

Effects *g_effects;

SDL_Window* g_display_window;
bool g_game_is_running = true;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks  = 0.0f;
float g_accumulator     = 0.0f;

int g_lives = 3;

void switch_to_scene(Scene* scene, int lives){
    g_current_scene = scene;
    g_current_scene->initialise(lives);
}



void initialise()
{
    // Initialising both the video AND audio subsystems
    // We did something similar when we talked about video game controllers
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow("Hello, Platformer!",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif
    

    // ––––– VIDEO SETUP ––––– //
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    
    // ————— LEVEL SETUP ————— //
    g_levelM = new LevelM();
    g_levelA = new LevelA();
    g_levelB = new LevelB();
    g_levelC = new LevelC();
    
    g_levels[0] = g_levelM;
    g_levels[1] = g_levelA;
    g_levels[2] = g_levelB;
    g_levels[3] = g_levelC;
    
    // Start at level M
    switch_to_scene(g_levels[0], g_lives);
    
    g_effects = new Effects(g_projection_matrix, g_view_matrix);
    
    g_health = new Entity[g_lives];
    for (int i = 0; i < g_lives; i++){
        g_health[i].set_entity_type(HEART);
        g_health[i].m_texture_id = Utility::load_texture("assets/heart.png");
    }
    
    // ––––– AUDIO SETUP ––––– //
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    Mix_PlayMusic(Mix_LoadMUS("assets/music.mp3"), -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 2);
    
    Mix_Volume(-1, MIX_MAX_VOLUME / 2);
    

    // ––––– GENERAL ––––– //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input(){
    if (g_current_scene != g_levelM)
    g_current_scene->m_state.player->set_movement(glm::vec3(0.0f));

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            // End game
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_game_is_running = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_q:
                // Quit the game with a keystroke
                g_game_is_running = false;
                break;
            

            case SDLK_SPACE:
                // Jump
                    if (g_current_scene != g_levelM && g_current_scene->m_state.player->m_collided_bottom){
                        g_current_scene->m_state.player->m_is_jumping = true;
                        Mix_PlayChannel(-1, g_current_scene->m_state.jump_sfx, 0);
                    }

                break;

            case SDLK_m:
                // Mute volume
                Mix_HaltMusic();
                break;
                    
                case SDLK_RETURN:
                    if (g_current_scene == g_levelM) {
                        switch_to_scene(g_levelA, g_lives);
                        g_effects->start(FADEIN, 0.3f);
                    }
                    

            default:
                break;
            }

        default:
            break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);
    if (g_current_scene != g_levelM){
        if (key_state[SDL_SCANCODE_LEFT]){
            g_current_scene->m_state.player->move_left();
            g_current_scene->m_state.player->m_animation_indices = g_current_scene->m_state.player->m_walking[g_current_scene->m_state.player->LEFT];
        }
        else if (key_state[SDL_SCANCODE_RIGHT]){
            g_current_scene->m_state.player->move_right();
            g_current_scene->m_state.player->m_animation_indices = g_current_scene->m_state.player->m_walking[g_current_scene->m_state.player->RIGHT];
        }
        
        // This makes sure that the player can't move faster diagonally
        if (glm::length(g_current_scene->m_state.player->get_movement()) > 1.0f){
            g_current_scene->m_state.player->set_movement(glm::normalize(g_current_scene->m_state.player->get_movement()));
        }
    }
}

void update(){
    // ————— DELTA TIME / FIXED TIME STEP CALCULATION ————— //
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    delta_time += g_accumulator;

    if (delta_time < FIXED_TIMESTEP){
        g_accumulator = delta_time;
        return;
    }

    while (delta_time >= FIXED_TIMESTEP) {
        // ————— UPDATING THE SCENE (i.e. map, character, enemies...) ————— //
        g_current_scene->update(FIXED_TIMESTEP);
        g_effects->update(FIXED_TIMESTEP);

        delta_time -= FIXED_TIMESTEP;
    }

    g_accumulator = delta_time;


    // ————— PLAYER CAMERA ————— //
    g_view_matrix = glm::mat4(1.0f);
    for (int i = 0; i < g_lives; i++){
        g_health[i].set_model_matrix(glm::mat4(1.0f));
    }

       
    if (g_current_scene != g_levelM) {
        g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-g_current_scene->m_state.player->get_position().x, -g_current_scene->m_state.player->get_position().y, 0));
    }

    for (int i = 0; i < g_lives; i++){
        float x_coord = g_current_scene->m_state.player->get_position().x - 4.0f + float(i);
        float y_coord = g_current_scene->m_state.player->get_position().y + 3.0f;
        g_health[i].set_model_matrix(glm::translate(g_health[i].get_model_matrix(), glm::vec3 (x_coord, y_coord, 0)));
    }
    
    if (g_current_scene->m_state.player->m_num_lives < g_lives){
        g_lives = g_current_scene->m_state.player->m_num_lives;
        g_health[g_lives].deactivate();
    }
    
    
}

void render(){
    g_shader_program.set_view_matrix(g_view_matrix);

    glClear(GL_COLOR_BUFFER_BIT);

    // ————— RENDERING THE SCENE (i.e. map, character, enemies...) ————— //
    g_current_scene->render(&g_shader_program);
    g_effects->render();
    
    for (int i = 0; i < g_lives; i++){
        if (g_current_scene != g_levelM) g_health[i].render(&g_shader_program);
    }

    if (g_current_scene->m_state.next_scene_id == 4){
        Utility::draw_text(&g_shader_program, Utility::load_texture("assets/font1.png"), std::string("YOU WIN"), 0.75f, 0.0f, glm::vec3(g_current_scene->m_state.player->get_position().x - 2.25f, g_current_scene->m_state.player->get_position().y + 1.0f, 0.0f));
    }
    
    if (g_lives == 0){
        Utility::draw_text(&g_shader_program, Utility::load_texture("assets/font1.png"), std::string("YOU LOSE"), 0.75f, 0.0f, glm::vec3(g_current_scene->m_state.player->get_position().x - 2.5f, g_current_scene->m_state.player->get_position().y + 1.0f, 0.0f));
    }

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_Quit();

    // ————— DELETING LEVEL A DATA (i.e. map, character, enemies...) ————— //
    delete g_levelM;
    delete g_levelA;
    delete g_levelB;
    delete g_levelC;
    delete g_effects;
}

// ––––– GAME LOOP ––––– //
int main(int argc, char* argv[])
{
    initialise();

    while (g_game_is_running){
        process_input();
        update();
        
        if (g_current_scene->m_state.next_scene_id >= 0 && g_current_scene->m_state.next_scene_id <= 3){
            LOG(g_lives);
            switch_to_scene(g_levels[g_current_scene->m_state.next_scene_id], g_lives);
            g_effects->start(FADEIN, 0.3f);
    
        }
        
        render();
        
        
    }

    shutdown();
    return 0;
}
