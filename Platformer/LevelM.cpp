#include "LevelM.h"
#include "Utility.h"

#define LEVEL_WIDTH 19
#define LEVEL_HEIGHT 9

LevelM::~LevelM()
{
    delete [] m_state.enemies;
    delete    m_state.player;
    delete    m_state.map;
    Mix_FreeMusic(m_state.bgm);
}

void LevelM::initialise(int lives){
    m_state.next_scene_id = -1;
    
    m_state.player = new Entity();
    m_state.player->set_entity_type(PLAYER);
    m_state.player->m_num_lives = lives;
}

void LevelM::update(float delta_time){}


void LevelM::render(ShaderProgram *program){
    Utility::draw_text(program, Utility::load_texture("assets/font1.png"), std::string("CRANE CHAOS"), 0.6f, 0.0f, glm::vec3(-3.0f, 1.0f, 0.0f));
    Utility::draw_text(program, Utility::load_texture("assets/font1.png"), std::string("Press enter to start"), 0.45f, 0.0f, glm::vec3(-4.25f, -0.5f, 0.0f));
}
