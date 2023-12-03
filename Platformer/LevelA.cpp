#include "LevelA.h"
#include "Utility.h"

#define LEVEL_WIDTH 19
#define LEVEL_HEIGHT 11

unsigned int LEVELA_DATA[] =
{
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 12,  0,
     0, 26,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 44,  0,
     0, 42,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  4,  5,  5,  5,  6,
     0, 73,  0,  0,  0,  0,  0,  0,  1,  2,  2,  3,  0,  0,  7,  0, 53,  0,  0,
     0, 74,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  9, 39,  0, 53,  0,  0,
     0, 17, 18, 19,  0,  0,  0,  0,  0,  0,  0,  0, 17, 19,  0,  0, 69,  0,  0,
     0, 33, 34, 34, 18, 18, 19,  0,  0, 76, 17, 18, 34, 35,  0, 84, 85, 86,  0,
    17, 18, 18, 18, 19, 34, 35,  0,  0, 79, 33, 34, 34, 34, 18, 18, 18, 18, 19,
    33, 34, 34, 34, 35, 34, 35, 13, 13, 95, 33, 34, 34, 34, 34, 34, 34, 34, 35,
    33, 34, 34, 34, 35, 34, 35, 45, 45, 45, 33, 34, 34, 34, 34, 34, 34, 34, 35,
    33, 34, 34, 34, 35, 34, 35, 45, 45, 45, 33, 34, 34, 34, 34, 34, 34, 34, 35
};

LevelA::~LevelA()
{
    delete [] m_state.enemies;
    delete    m_state.player;
    delete    m_state.map;
    Mix_FreeMusic(m_state.bgm);
    Mix_FreeChunk(m_state.jump_sfx);
    Mix_FreeChunk(m_state.revive_sfx);
}

void LevelA::initialise(int lives){
    
    m_state.next_scene_id = -1;
    
    
    GLuint map_texture_id = Utility::load_texture("assets/tilemap_packed.png");
    m_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELA_DATA, map_texture_id, 1.0f, 16, 7);

    // ––––– PLAYER ––––– //
    m_state.player = new Entity();
    m_state.player->set_entity_type(PLAYER);
    m_state.player->m_num_lives = lives;
    m_state.player->set_position(glm::vec3(2.0f, -4.0f, 0.0f));
    m_state.player->set_movement(glm::vec3(0.0f));
    m_state.player->set_speed(2.75f);
    m_state.player->set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    m_state.player->m_texture_id = Utility::load_texture("assets/male.png");

    // Walking
    m_state.player->m_walking[m_state.player->LEFT]   = new int[3] { 3, 4, 5 };
    m_state.player->m_walking[m_state.player->RIGHT]  = new int[3] { 6, 7, 8 };

    m_state.player->m_animation_indices = m_state.player->m_walking[m_state.player->RIGHT];
    m_state.player->m_animation_frames  = 3;
    m_state.player->m_animation_index   = 0;
    m_state.player->m_animation_time    = 0.0f;
    m_state.player->m_animation_cols    = 3;
    m_state.player->m_animation_rows    = 4;
    m_state.player->set_height(1.0f);
    m_state.player->set_width(0.6f);

    // Jumping
    m_state.player->set_jumping_power(4.75f);

    // ––––– ENEMIES ––––– //
    GLuint guard_texture_id = Utility::load_texture("assets/guard.png");
    GLuint walker_texture_id = Utility::load_texture("assets/walker.png");

    m_state.enemies = new Entity[ENEMY_COUNT];
    
    for (int i = 0; i < 2; i++){
        m_state.enemies[i].set_entity_type(ENEMY);
        m_state.enemies[i].set_movement(glm::vec3(0.0f));
        m_state.enemies[i].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
        
        m_state.enemies[i].m_walking[m_state.enemies[i].LEFT]   = new int[3] { 3, 4, 5 };
        m_state.enemies[i].m_walking[m_state.enemies[i].RIGHT]  = new int[3] { 6, 7, 8 };
        m_state.enemies[i].m_animation_indices = m_state.enemies[i].m_walking[m_state.enemies[i].RIGHT];
        m_state.enemies[i].m_animation_frames  = 3;
        m_state.enemies[i].m_animation_index   = 0;
        m_state.enemies[i].m_animation_time    = 0.0f;
        m_state.enemies[i].m_animation_cols    = 3;
        m_state.enemies[i].m_animation_rows    = 4;
        m_state.enemies[i].set_height(0.9f);
        m_state.enemies[i].set_width(0.9f);
    }
    
    
    m_state.enemies[0].set_ai_type(GUARD);
    m_state.enemies[0].set_ai_state(IDLE);
    m_state.enemies[0].m_texture_id = guard_texture_id;
    m_state.enemies[0].set_position(glm::vec3(6.0f, -5.0f, 0.0f));
    m_state.enemies[0].set_speed(0.5f);

    
    m_state.enemies[1].set_ai_type(WALKER);
    m_state.enemies[1].set_ai_state(ROAM_RIGHT);
    m_state.enemies[1].m_texture_id = walker_texture_id;
    m_state.enemies[1].set_position(glm::vec3(9.0f, -2.0f, 0.0f));
    m_state.enemies[1].set_speed(2.0f);
    m_state.enemies[1].set_left_edge(8.0f);
    m_state.enemies[1].set_right_edge(11.0f);
   

    // ––––– AUDIO ––––– //
    m_state.jump_sfx = Mix_LoadWAV("assets/jump.wav");
    m_state.revive_sfx = Mix_LoadWAV("assets/revive.wav");
    
}

void LevelA::update(float delta_time){
    m_state.player->update(delta_time, m_state.player, m_state.enemies, ENEMY_COUNT, m_state.map);
    
    for (int i = 0; i < ENEMY_COUNT; i++){
        m_state.enemies[i].update(delta_time, m_state.player, 0, 0, m_state.map);
        if (m_state.enemies[i].get_position().y < -6.75f) m_state.enemies[i].deactivate();
    }
    
    if (m_state.player->get_position().y < -6.75f) {
        m_state.player->m_is_dead = true;
        m_state.player->deactivate();
    }

    if (m_state.player->get_position().x > 16.15f) m_state.next_scene_id = 2;
    
    
    
    if (m_state.player->m_is_dead && m_state.player->m_num_lives > 0){
        m_state.player->m_num_lives--;
        if (m_state.player->m_num_lives > 0){
            Mix_PlayChannel(-1, m_state.revive_sfx, 0);
            m_state.player->m_is_dead = false;
            m_state.player->activate();
            m_state.player->set_position(glm::vec3(2.0f, -4.0f, 0.0f));
            m_state.player->set_movement(glm::vec3(0.0f));
            
            m_state.enemies[0].set_ai_state(IDLE);
            m_state.enemies[0].set_movement(glm::vec3(0.0f));
            m_state.enemies[0].set_position(glm::vec3(6.0f, -5.0f, 0.0f));
            
            m_state.enemies[1].set_ai_state(ROAM_RIGHT);
            m_state.enemies[1].set_movement(glm::vec3(0.0f));
            m_state.enemies[1].set_position(glm::vec3(9.0f, -2.0f, 0.0f));
            
        }
    }
}


void LevelA::render(ShaderProgram *program)
{
    m_state.map->render(program);
    m_state.player->render(program);
    for (int i = 0; i < ENEMY_COUNT; i++){
        m_state.enemies[i].render(program);
    }
}
