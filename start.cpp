#include "bn_core.h"
#include "bn_bg_palettes.h"
#include "bn_keypad.h"
#include "bn_sprite_ptr.h"
#include "bn_sprite_items_start_button.h"
#include "bn_sprite_items_how_to_play_button.h"
#include "bn_sprite_items_cursor.h"
#include "bn_sprite_items_level_1_button.h"
#include "bn_sprite_items_how_to_play_title.h"
#include "bn_sprite_items_back_button.h"

// screen states to track which screen we're currently displaying
enum class ScreenState
{
    START,
    LEVEL_SELECT,
    INSTRUCTIONS
};

// screen updates and transitioning with fade effect
void update_start_screen(ScreenState& current_state);
void update_level_select_screen(ScreenState& current_state);
void update_instructions_screen(ScreenState& current_state);
void fade_to_black();

// initializes core and handles screen transitions
int main()
{
    bn::core::init();

    ScreenState current_state = ScreenState::START; // start screen as initial state

    while(true)
    {
        switch (current_state) //current screen based on the state
        {
            case ScreenState::START:
                update_start_screen(current_state);
                break;

            case ScreenState::LEVEL_SELECT:
                update_level_select_screen(current_state);
                break;

            case ScreenState::INSTRUCTIONS:
                update_instructions_screen(current_state);
                break;

            default:
                current_state = ScreenState::START;
                break;
        }

        bn::core::update();
    }
}

// function to transition to black screen for smooth state changes
void fade_to_black()
{
    for(float fade_level = 0.0f; fade_level <= 1.0f; fade_level += 0.05f)
    {
        bn::bg_palettes::set_fade(bn::color(0, 0, 0), fade_level);
        bn::core::update();
    }
}

// update Start Screen
void update_start_screen(ScreenState& current_state)
{
    // create static sprites for buttons and cursor
    static bn::sprite_ptr start_button = bn::sprite_items::start_button.create_sprite(0, -20);
    static bn::sprite_ptr how_to_play_button = bn::sprite_items::how_to_play_button.create_sprite(0, 20);
    static bn::sprite_ptr cursor = bn::sprite_items::cursor.create_sprite(-40, -20);

    start_button.set_visible(true);
    how_to_play_button.set_visible(true);
    cursor.set_visible(true);

    // move cursor between Start and How to Play based on user input
    if (bn::keypad::down_pressed() && cursor.y() == -20)  // move cursor to 'How to Play'
    {
        cursor.set_position(-40, 20);
    }
    else if (bn::keypad::up_pressed() && cursor.y() == 20)  // move cursor back to 'Start'
    {
        cursor.set_position(-40, -20);
    }

    if (bn::keypad::a_pressed())
    {
        fade_to_black();

        // hide sprites before switching state
        start_button.set_visible(false);
        how_to_play_button.set_visible(false);
        cursor.set_visible(false);

        if (cursor.y() == -20)  // Start button selected
        {
            current_state = ScreenState::LEVEL_SELECT;
        }
        else if (cursor.y() == 20)  // How to Play selected
        {
            current_state = ScreenState::INSTRUCTIONS;
        }
    }
}

// update Level Select Screen
void update_level_select_screen(ScreenState& current_state)
{
    // create static sprites for buttons and cursor
    static bn::sprite_ptr level_1_button = bn::sprite_items::level_1_button.create_sprite(0, 0);
    static bn::sprite_ptr back_button = bn::sprite_items::back_button.create_sprite(0, 40);
    static bn::sprite_ptr cursor = bn::sprite_items::cursor.create_sprite(-40, 0);

    level_1_button.set_visible(true);
    back_button.set_visible(true);
    cursor.set_visible(true);

    // move cursor between Back and Level 1 based on user input
    if (bn::keypad::down_pressed() && cursor.y() == 0)  // move cursor to 'Back'
    {
        cursor.set_position(-40, 40);
    }
    else if (bn::keypad::up_pressed() && cursor.y() == 40)  // move cursor to 'Level 1'
    {
        cursor.set_position(-40, 0);
    }

    if (bn::keypad::a_pressed())
    {
        fade_to_black();

        // hide sprites before switching state
        level_1_button.set_visible(false);
        back_button.set_visible(false);
        cursor.set_visible(false);

        if (cursor.y() == 40)  // Back button selected
        {
            current_state = ScreenState::START;
        }
        else if (cursor.y() == 0)  // Level 1 selected
        {
            // Logic to start Level 1 when added
        }
    }
}

// update Instructions Screen
void update_instructions_screen(ScreenState& current_state)
{
    // create static sprites for buttons and cursor
    static bn::sprite_ptr how_to_play_title = bn::sprite_items::how_to_play_title.create_sprite(0, -40);
    static bn::sprite_ptr back_button = bn::sprite_items::back_button.create_sprite(0, 40);
    static bn::sprite_ptr cursor = bn::sprite_items::cursor.create_sprite(-40, 40);

    how_to_play_title.set_visible(true);
    back_button.set_visible(true);
    cursor.set_visible(true);

    // move cursor between Back and Level 1 based on user input
    if (bn::keypad::a_pressed() && cursor.y() == 40)  // Back button selected
    {
        fade_to_black();

        // hide sprites before switching state
        how_to_play_title.set_visible(false);
        back_button.set_visible(false);
        cursor.set_visible(false);

        current_state = ScreenState::START; // return to start screen
    }
}
