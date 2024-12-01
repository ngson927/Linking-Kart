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
#include "bn_timer.h"
#include "bn_timers.h"
#include "bn_fixed.h"
#include "bn_color.h"
#include "bn_span.h"
#include "bn_backdrop.h"
#include <numbers>
#include "bn_regular_bg_items_lap_out_loud.h"
#include "bn_sprite_items_paddle.h"
#include "bn_sprite_text_generator.h"
#include "bn_sprite_animate_actions.h"
#include "bn_sprite_palette_ptr.h"
#include "common_variable_8x8_sprite_font.h"
#include "bn_regular_bg_tiles_item.h"
#include "bn_regular_bg_tiles_ptr.h"
#include "bn_bg_tiles.h"
#include "bn_tile.h"
#include "bn_regular_bg_map_ptr.h"
#include "bn_regular_bg_map_cell_info.h"
#include "bn_cameras.h"
#include "bn_camera_ptr.h"
#include "bn_camera_actions.h"
#include "bn_blending.h"
#include "bn_rect_window.h"
#include "bn_rect.h"
#include "bn_seed_random.h"
#include "bn_fixed_point_fwd.h"
#include "bn_sprite_shape_size.h"
#include <vector>

int main() {
    bn::core::init();

    // Create a random number generator object
    bn::random random;

    // Create a camera to follow the car
    bn::camera_ptr camera = bn::camera_ptr::create(0, 0);

    bn::sprite_ptr paddle = bn::sprite_items::paddle.create_sprite(-100, 40);

    float top_boundary = 11;
    float bottom_boundary = 70;

    //Countdown timer setup
    bn::sprite_text_generator text_generator(common::variable_8x8_sprite_font);
    bn::vector<bn::sprite_ptr, 16> text_sprites;
    int countdown_seconds = 0.5 * 60;
    bool game_over = false;  //Flag to check if the game is over

    bn::timer timer;
    uint64_t ticks = 0;
    int frame_counter = 0; // Counter to track frames for slowing down the timer

    //Declaring the items for the background
    bn::regular_bg_ptr backdrop_image = bn::regular_bg_items::backdrop_image.create_bg(0, 0);
    bn::regular_bg_ptr lap_out_loud = bn::regular_bg_items::lap_out_loud.create_bg(0, 1);

    // Variable for a single obstacle
    bn::sprite_ptr obstacle = bn::sprite_items::paddle.create_sprite(0, 0);

    // Initial obstacle setup
    int obstacle_x = random.get_int(-100, 100);  // Starting position on the right side of the screen
    int obstacle_y = random.get_int(top_boundary, bottom_boundary);
    obstacle = bn::sprite_items::paddle.create_sprite(obstacle_x, obstacle_y);  // You can use different obstacle sprite here

    // Waiting for 'a' to start the game
    while (!bn::keypad::a_pressed()) {
        bn::core::update();  // Keep updating the core to handle input, etc.
        // You can display a message or some kind of visual cue that says "Press A to Start"
        bn::string<16> start_text = "Press A to Start";
        text_sprites.clear();
        text_generator.generate(-30, 0, start_text, text_sprites);  // Display at a position
    }

    while (true) {
        bn::core::update();

        // Update ticks based on elapsed time
        ticks += timer.elapsed_ticks_with_restart();

        // Slow down the countdown timer by updating it every 60 frames
        frame_counter++;
        if (frame_counter >= 60) { // 60 frames = roughly 1 second
            frame_counter = 0;
            if (!game_over && countdown_seconds > 0) {
                countdown_seconds--;
            } else if (!game_over) {
                game_over = true;  // Trigger game over when the timer hits 0
            }
        }

        // Convert countdown_seconds to minutes and seconds
        int minutes = countdown_seconds / 60;
        int seconds_left = countdown_seconds % 60;

        // Update the countdown timer text display
        bn::string<6> timer_text = bn::to_string<6>(minutes) + ":" + bn::to_string<6>(seconds_left);
        text_sprites.clear();
        text_generator.generate(-6 * 16, -68, timer_text, text_sprites);  // Position at the top-left corner

        // If the game is over, display "Game Over" and stop the car's movement
        if (game_over) {
            text_sprites.clear(); // Clear the vector, removing the references to old sprites
            text_generator.generate(-30, 0, "Game Over", text_sprites);  // Display it below the timer
            continue;  // Skip the rest of the loop
        }

        // For the paddle and obstacle, use the position and shape size to create a rect.
        bn::rect paddle_rect(int(paddle.x()), int(paddle.y()), paddle.shape_size().width(), paddle.shape_size().height());
        bn::rect obstacle_rect(int(obstacle.x()), int(obstacle.y()), obstacle.shape_size().width(), obstacle.shape_size().height());

        // If the paddle is touching the obstacle, end the game
        if (paddle_rect.touches(obstacle_rect)) {
            game_over = true;  // Set the game over flag
        }

        // Move the obstacle to the left
        obstacle.set_x(obstacle.x() - 1);  // Move the obstacle left by 1 pixel

        // If the obstacle goes off-screen to the left, generate a new one on the right side
        if (obstacle.x() < -110) {  // If the obstacle goes completely off the left side
            obstacle.set_x(100);  // Reset it to the right side of the screen
            obstacle.set_y(random.get_int(top_boundary, bottom_boundary));  // Set a new random y position
        }

        backdrop_image.set_camera(camera);
        lap_out_loud.set_camera(camera);
        if (bn::keypad::a_held()) {
            camera.set_x(camera.x() + 5);
            if (paddle.x() > 80) {
                paddle.set_x(80);
            } else {
                paddle.set_x(paddle.x() + 0.1);
            }
            if (bn::keypad::up_held()) {
                if (paddle.y() > top_boundary) {
                    paddle.set_y(paddle.y() - 1);
                } else {
                    paddle.set_y(11);
                }
            }
            if (bn::keypad::down_held()) {
                if (paddle.y() < bottom_boundary) {
                    paddle.set_y(paddle.y() + 1);
                } else {
                    paddle.set_y(70);
                }
            }
        }
    }
    return 0;
}
