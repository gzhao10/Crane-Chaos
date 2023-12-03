#include "LevelB.h"
#include "Utility.h"

#define LEVEL_WIDTH 22
#define LEVEL_HEIGHT 14

unsigned int LEVELB_DATA[] =
{
     0,  0, 12,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0, 44,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     4,  5,  5,  5,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 42,  0,  0,  0,  0,  0,
     0, 53,  0,  0,  0,  9,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 20, 21,  5,  5,  5,  6,
    87, 88, 88, 88, 88, 88, 90,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  8, 53,  0, 83,
     0, 53, 81,  0,  0,  7,  0,  0,  1,  2,  2,  3,  0,  0,  0,  0,  0,  0, 24, 53,  0,  0,
     0, 53,  0,  0,  0, 39,  0,  0,  0,  0,  0,  0,  0,  0, 62,  0,  0,  0, 56, 53,  0,  0,
     0, 53,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 87, 88, 88, 90,  0,
     0, 53,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 59,  0,  0, 69,  0,  0,
     0, 69,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 16,  0, 76, 17, 18, 19, 84, 85,101, 86,
    17, 18, 18, 18, 19,  0,  0, 17, 18, 18, 19,  0, 32,  0, 78, 33, 34, 17, 18, 18, 18, 19,
    33, 34, 34, 34, 35, 13, 13, 33, 34, 34, 35, 13, 32, 13, 95, 33, 34, 33, 34, 34, 34, 35,
    33, 34, 34, 34, 35, 45, 45, 33, 34, 34, 35, 45, 32, 45, 45, 33, 34, 33, 34, 34, 34, 35,
    33, 34, 34, 34, 35, 45, 45, 33, 34, 34, 35, 45, 32, 45, 45, 33, 34, 33, 34, 34, 34, 35
};

LevelB::~LevelB()
{
    delete [] m_state.enemies;
    delete    m_state.player;
    delete    m_state.map;
    Mix_FreeMusic(m_state.bgm);
    Mix_FreeChunk(m_state.jump_sfx);
    Mix_FreeChunk(m_state.revive_sfx);
}

void LevelB::initialise(int lives){
    
    m_state.next_scene_id = -1;
    
    GLuint map_texture_id = Utility::load_texture("assets/tilemap_packed.png");
    m_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELB_DATA, map_texture_id, 1.0f, 16, 7);

    // ––––– PLAYER ––––– //
    m_state.player = new Entity();
    m_state.player->set_entity_type(PLAYER);
    m_state.player->m_num_lives = lives;
    m_state.player->set_position(glm::vec3(2.0f, -9.0f, 0.0f));
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
    GLuint jumper_texture_id = Utility::load_texture("assets/jumper.png");

    m_state.enemies = new Entity[ENEMY_COUNT];
    
    for (int i = 0; i < 2; i++){
        m_state.enemies[i].set_entity_type(ENEMY);
        m_state.enemies[i].set_ai_type(JUMPER);
        m_state.enemies[i].set_movement(glm::vec3(0.0f));
        m_state.enemies[i].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
        m_state.enemies[i].set_ai_state(IDLE);
        m_state.enemies[i].m_texture_id = jumper_texture_id;
        m_state.enemies[i].set_jumping_power(2.5f);
        m_state.enemies[i].set_height(0.8f);
        m_state.enemies[i].set_width(0.8f);
        m_state.enemies[i].set_speed(2.5f);
    }
    
    m_state.enemies[0].set_position(glm::vec3(10.0f, -9.0f, 0.0f));
    m_state.enemies[1].set_position(glm::vec3(8.0f, -4.0f, 0.0f));
    
    
    // ––––– AUDIO ––––– //
    m_state.jump_sfx = Mix_LoadWAV("assets/jump.wav");
    m_state.revive_sfx = Mix_LoadWAV("assets/revive.wav");
    
}

void LevelB::update(float delta_time){
    m_state.player->update(delta_time, m_state.player, m_state.enemies, ENEMY_COUNT, m_state.map);
    
    for (int i = 0; i < ENEMY_COUNT; i++){
        m_state.enemies[i].update(delta_time, m_state.player, 0, 0, m_state.map);
        if (m_state.enemies[i].get_position().y < -9.75f) m_state.enemies[i].deactivate();
    }
    
    if (m_state.player->get_position().y < -9.75f) {
        m_state.player->m_is_dead = true;
        m_state.player->deactivate();
    }
    
    if (m_state.player->get_position().x < 2.85f && m_state.player->get_position().y > -2.0f ) m_state.next_scene_id = 3;
    
    if (m_state.player->m_is_dead && m_state.player->m_num_lives > 0){
        m_state.player->m_num_lives--;
        if (m_state.player->m_num_lives > 0){
            Mix_PlayChannel(-1, m_state.revive_sfx, 0);
            m_state.player->m_is_dead = false;
            m_state.player->activate();
            m_state.player->set_position(glm::vec3(2.0f, -9.0f, 0.0f));
            m_state.player->set_movement(glm::vec3(0.0f));
            
            m_state.enemies[0].set_ai_state(IDLE);
            m_state.enemies[0].set_movement(glm::vec3(0.0f));
            m_state.enemies[0].set_position(glm::vec3(10.0f, -9.0f, 0.0f));
            
            m_state.enemies[1].set_ai_state(IDLE);
            m_state.enemies[1].set_movement(glm::vec3(0.0f));
            m_state.enemies[1].set_position(glm::vec3(8.0f, -4.0f, 0.0f));
            
        }
    }
}


void LevelB::render(ShaderProgram *program){
    m_state.map->render(program);
    m_state.player->render(program);
    for (int i = 0; i < ENEMY_COUNT; i++){
        m_state.enemies[i].render(program);
    }
}
