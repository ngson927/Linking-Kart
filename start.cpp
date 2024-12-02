// Libraries
#include "bn_core.h"
#include "bn_keypad.h"
#include "bn_sprite_ptr.h"
#include "bn_regular_bg_ptr.h"
#include "bn_optional.h"

// Backgrounds
#include "bn_regular_bg_items_blue_bg.h"
#include "bn_regular_bg_items_lvlslct_bg.h"
#include "bn_regular_bg_items_instructions_bg.h"

// Sprites
#include "bn_sprite_items_start_button.h"
#include "bn_sprite_items_how_to_play_button.h"
#include "bn_sprite_items_level_1_button.h"
#include "bn_sprite_items_back_button.h"
#include "bn_sprite_items_cursor.h"

// Enum for different screen states
enum class ScreenState
{
    START,
    LEVEL_SELECT,
    INSTRUCTIONS
};

// Global variables
static bn::optional<bn::regular_bg_ptr> current_background;
static ScreenState last_screen = ScreenState::START;

// Function to load a new background
void load_background(const bn::regular_bg_item& bg_item, ScreenState new_screen)
{
    if (!current_background || last_screen != new_screen)
    {
        if (current_background)
        {
            current_background.reset(); // Remove the previous background
        }
        current_background = bg_item.create_bg(0, 0);
        last_screen = new_screen; // Update the last screen to the new one
    }
}

// Function to update the Start screen
void update_start_screen(ScreenState& current_state)
{
    load_background(bn::regular_bg_items::blue_bg, ScreenState::START);

    // Sprites for the Start screen
    static bn::sprite_ptr start_button = bn::sprite_items::start_button.create_sprite(0, 52);
    static bn::sprite_ptr how_to_play_button = bn::sprite_items::how_to_play_button.create_sprite(0, 65);
    static bn::sprite_ptr cursor = bn::sprite_items::cursor.create_sprite(-40, 52);

    start_button.set_visible(true);
    how_to_play_button.set_visible(true);
    cursor.set_visible(true);

    // Move the cursor between buttons
    if (bn::keypad::down_pressed() && cursor.y() == 52)
    {
        cursor.set_position(-40, 65);
    }
    else if (bn::keypad::up_pressed() && cursor.y() == 65)
    {
        cursor.set_position(-40, 52);
    }

    // Handle 'A' button press
    if (bn::keypad::a_pressed())
    {
        start_button.set_visible(false);
        how_to_play_button.set_visible(false);
        cursor.set_visible(false);

        if (cursor.y() == 52) // Start button selected
        {
            current_state = ScreenState::LEVEL_SELECT;
        }
        else if (cursor.y() == 65) // How to Play button selected
        {
            current_state = ScreenState::INSTRUCTIONS;
        }
    }
}

// Function to update the Level Select screen
void update_level_select_screen(ScreenState& current_state)
{
    load_background(bn::regular_bg_items::lvlslct_bg, ScreenState::LEVEL_SELECT);

    // Sprites for the Level Select screen
    static bn::sprite_ptr level_1_button = bn::sprite_items::level_1_button.create_sprite(0, 0);
    static bn::sprite_ptr back_button = bn::sprite_items::back_button.create_sprite(0, 40);
    static bn::sprite_ptr cursor = bn::sprite_items::cursor.create_sprite(-40, 0);

    level_1_button.set_visible(true);
    back_button.set_visible(true);
    cursor.set_visible(true);

    // Move the cursor between buttons
    if (bn::keypad::down_pressed() && cursor.y() == 0)
    {
        cursor.set_position(-40, 40);
    }
    else if (bn::keypad::up_pressed() && cursor.y() == 40)
    {
        cursor.set_position(-40, 0);
    }

    // Handle 'A' button press
    if (bn::keypad::a_pressed())
    {
        level_1_button.set_visible(false);
        back_button.set_visible(false);
        cursor.set_visible(false);

        if (cursor.y() == 40) // Back button selected
        {
            current_state = ScreenState::START;
        }
    }
}

// Function to update the Instructions screen
void update_instructions_screen(ScreenState& current_state)
{
    load_background(bn::regular_bg_items::instructions_bg, ScreenState::INSTRUCTIONS);

    // Sprites for the Instructions screen
    static bn::sprite_ptr back_button = bn::sprite_items::back_button.create_sprite(0, 40);
    static bn::sprite_ptr cursor = bn::sprite_items::cursor.create_sprite(-40, 40);

    back_button.set_visible(true);
    cursor.set_visible(true);

    // Handle 'A' button press
    if (bn::keypad::a_pressed() && cursor.y() == 40) // Back button selected
    {
        back_button.set_visible(false);
        cursor.set_visible(false);

        current_state = ScreenState::START;
    }
}

// Main function
int main()
{
    bn::core::init();

    ScreenState current_state = ScreenState::START;

    while (true)
    {
        switch (current_state)
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
            // Handle unexpected state (optional, could log an error or reset to START)
            current_state = ScreenState::START;
            break;
        }

        bn::core::update();
    }
}

