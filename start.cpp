// Libraries
#include "bn_core.h"
#include "bn_keypad.h"
#include "bn_sprite_ptr.h"
#include "bn_regular_bg_ptr.h"
#include "bn_optional.h"
#include "bn_music_actions.h"
#include "bn_music_items.h"

// Backgrounds
#include "bn_regular_bg_items_blue_bg.h"
#include "bn_regular_bg_items_instructions_bg.h"

// Sprites
#include "bn_sprite_items_start_button.h"
#include "bn_sprite_items_how_to_play_button.h"
#include "bn_sprite_items_back_button.h"
#include "bn_sprite_items_cursor.h"

// Enum for different screen states
enum class ScreenState
{
    START,
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

    if (!bn::music::playing()) 
    {
        bn::music_items::how_to_play.play();
    }

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

    // Handle 'A' button press for navigation
    if (bn::keypad::a_pressed())
    {
        start_button.set_visible(false);
        how_to_play_button.set_visible(false);
        cursor.set_visible(false);

        if (cursor.y() == 52) // Start button selected
        {
            // You could replace this with the transition to your game's main screen if applicable.
            current_state = ScreenState::START; 
        }
        else if (cursor.y() == 65) // How to Play button selected
        {
            current_state = ScreenState::INSTRUCTIONS;
        }
    }
}

// Function to update the Instructions screen
void update_instructions_screen(ScreenState& current_state)
{
    load_background(bn::regular_bg_items::instructions_bg, ScreenState::INSTRUCTIONS);

    if (!bn::music::playing()) 
    {
        bn::music_items::how_to_play.play();
    }
    
    // Sprites for the Instructions screen
    static bn::sprite_ptr back_button = bn::sprite_items::back_button.create_sprite(0, 65);
    static bn::sprite_ptr cursor = bn::sprite_items::cursor.create_sprite(-40, 65);

    back_button.set_visible(true);
    cursor.set_visible(true);

    // Handle 'B' button press for navigation
    if (bn::keypad::b_pressed() && cursor.y() == 65) // Back button selected
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
