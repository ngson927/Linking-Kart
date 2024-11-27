#include "bn_core.h"
#include "bn_regular_bg_ptr.h"
#include "bn_regular_bg_item.h"
#include "bn_regular_bg_items_backdrop_image.h"
#include "bn_keypad.h"
#include "bn_math.h"
#include "bn_sram.h"
#include "bn_string.h"
#include "bn_display.h"
#include "bn_random.h"
#include "bn_time.h"
#include "bn_regular_bg_items_lap_out_loud.h"
#include "bn_regular_bg_items_right_turn.h"
#include "bn_regular_bg_items_left_turn.h"
#include "bn_sprite_items_paddle.h"
#include "bn_sprite_text_generator.h"
#include "bn_sprite_animate_actions.h"
#include "bn_sprite_palette_ptr.h"
#include "common_variable_8x8_sprite_font.h"

int main() {
    bn::core::init();

    bn::regular_bg_ptr backdrop_image = bn::regular_bg_items::backdrop_image.create_bg(0, 0);

    bn::sprite_ptr paddle = bn::sprite_items::paddle.create_sprite(100, 100);

    bn::regular_bg_ptr right_turn = bn::regular_bg_items::right_turn.create_bg(0, 0);

    bn::regular_bg_ptr left_turn = bn::regular_bg_items::left_turn.create_bg(0, 0);

    bn::regular_bg_ptr lap_out_loud = bn::regular_bg_items::lap_out_loud.create_bg(0, 0);

    bn::sprite_text_generator text_generator(common::variable_8x8_sprite_font);

    bn::vector<bn::sprite_ptr, 16> text_sprites;

    // Create a bn::time object for the countdown timer (1 minute 15 seconds)
    bn::time countdown_time(0, 1, 15); // 1 minute 15 seconds

    // Initial position and speed for the paddle's downward movement
    float paddle_speed = 1;  // Speed at which the paddle moves down
    float car_x = 0;  // Starting x-position of the paddle
    float car_y = 80;  // Starting y-position (middle of the screen)

    bool moving_up = false;  // Flag to indicate if the paddle is moving down or up

    // Boundaries for horizontal movement
    const float LEFT_BOUNDARY = -120;  // Left boundary of the screen
    const float RIGHT_BOUNDARY = 120;  // Right boundary of the screen

    //toying with the boundaries for off roading later
    float road_left_boundary = -20;
    float road_right_boundary = 20;

    // Variable to track car's speed (will slow down when off-road)
    float speed = paddle_speed;

    // Countdown timer setup
    bool game_over = false;  // Flag to check if the game is over

    while(true) {
        bn::core::update();

        // If the game is over, do not update the timer
        if (!game_over) {
            // Decrease the time by 1 second
            int current_seconds = countdown_time.second();
            int current_minutes = countdown_time.minute();

            if (current_seconds > 0) {
                // If there are seconds left, just decrease the seconds
                countdown_time.set_second(current_seconds - 1);
            } else if (current_minutes > 0) {
                // If seconds reach 0, reset seconds to 59 and decrease the minute
                countdown_time.set_second(59);
                countdown_time.set_minute(current_minutes - 1);
            } else {
                // If no time is left, set the game over flag
                countdown_time.set_minute(0);
                countdown_time.set_second(0);
                game_over = true;
            }
            // Manually convert the minutes and seconds to characters
            int minutes = countdown_time.minute();
            int seconds = countdown_time.second();

            // Create the timer text manually as "MM:SS"
            char timer_text_buffer[6]; // Buffer to hold "MM:SS" string
            timer_text_buffer[0] = '0' + minutes / 10; // Tens place of minutes
            timer_text_buffer[1] = '0' + minutes % 10; // Ones place of minutes
            timer_text_buffer[2] = ':';                  // Colon separator
            timer_text_buffer[3] = '0' + seconds / 10;   // Tens place of seconds
            timer_text_buffer[4] = '0' + seconds % 10;   // Ones place of seconds
            timer_text_buffer[5] = '\0';                  // Null-terminate the string

            text_sprites.clear(); // Clear the sprite vector

            // Update the timer text display
            text_generator.generate(-6 * 16, -68, timer_text_buffer, text_sprites);  // Position at the top-left corner
        }

        // If the game is over, display "Game Over" and stop the car's movement
        if (game_over) {
            // Clear the previous "Game Over" text sprites
            text_sprites.clear(); // Clear the vector, removing the references to old sprites
            // Set the position of the "Game Over" text
            text_generator.generate(-6 * 16, -50, "Game Over", text_sprites);  // Display it below the timer
            continue;
        }

        bool is_off_road = (car_x < road_left_boundary || car_x > road_right_boundary);

        // If the car is off-road, slow it down
        if (is_off_road) {
            speed = paddle_speed * 0.5;  // Reduce speed to 50% of normal speed
        } else {
            speed = paddle_speed;  // Restore normal speed
        }

        // If 'a' is pressed, move the car up
        if (bn::keypad::a_held() && !game_over) {
            moving_up = true;  // Start moving up
        } else {
            moving_up = false;  // Stop moving up if 'a' is not pressed
        }

        // If 'a' is pressed, move the car upwards
        if (moving_up && !game_over) {
            car_y -= speed;  // Move up
            if (car_y <= 0) {
                car_y = 0;
            }
        } 
        // If 'a' is not pressed, move the car back down to the bottom
        else if (!game_over) {
            car_y += speed;  // Move down
            if (car_y >= 70) {
                car_y = 70;  // Stop at the bottom (80 is the middle position)
            }
        }

        // Handle left and right movements with keypad input
        if (bn::keypad::right_held() && car_x < RIGHT_BOUNDARY && !game_over) {
            car_x += 1;  // Move to the right
        }
        if (bn::keypad::left_held() && car_x > LEFT_BOUNDARY && !game_over) {
            car_x -= 1;  // Move to the left
        }

        // Set the new position of the paddle (car)
        paddle.set_position(car_x, car_y);
    }

    return 0;
}
