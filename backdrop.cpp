// Libraries
#include "bn_core.h"
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
#include "bn_rect.h"
#include "bn_music.h"
#include "bn_backdrop.h"
#include "bn_optional.h"
#include "bn_cameras.h"
#include "bn_camera_ptr.h"
#include "bn_sprite_ptr.h"
#include "bn_seed_random.h"
#include "bn_music_items.h"
#include "bn_sound_items.h"
#include "bn_music_actions.h"
#include "bn_camera_actions.h"
#include "bn_sprite_shape_size.h"
#include "bn_sprite_text_generator.h"
#include "bn_sprite_animate_actions.h"
#include "common_variable_8x8_sprite_font.h"

// Backgrounds
#include "bn_regular_bg_ptr.h"
#include "bn_regular_bg_item.h"
#include "bn_regular_bg_items_blue_bg.h"
#include "bn_regular_bg_items_backdrop_image.h"
#include "bn_regular_bg_items_lap_out_loud.h"
#include "bn_regular_bg_items_instructions_bg.h"

// Sprites
#include "bn_sprite_items_gas.h"
#include "bn_sprite_items_empty.h"
#include "bn_sprite_items_barrel.h"
#include "bn_sprite_items_cursor.h"
#include "bn_sprite_items_explosion.h"
#include "bn_sprite_items_oil_spill.h"
#include "bn_sprite_items_linkin_kart.h"
#include "bn_sprite_items_start_button.h"
#include "bn_sprite_items_how_to_play_button.h"
#include "bn_sprite_items_back_button.h"


enum class ScreenState
{
    START,
    INSTRUCTIONS,
    GAMESCREEN
};

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

int main() {
    bn::core::init();

    ScreenState current_state = ScreenState::START;

    while (true) {
        switch (current_state) {
            case ScreenState::START: {
                bn::core::update();
                load_background(bn::regular_bg_items::blue_bg, ScreenState::START);
                if (!bn::music::playing()) {
                    bn::music_items::how_to_play.play(1);
                    bn::music::set_volume(0.5);
                }
                // Sprites for the Start screen
                static bn::sprite_ptr start_button = bn::sprite_items::start_button.create_sprite(0, 52);
                static bn::sprite_ptr how_to_play_button = bn::sprite_items::how_to_play_button.create_sprite(0, 65);
                static bn::sprite_ptr cursor = bn::sprite_items::cursor.create_sprite(-40, 52);

                start_button.set_visible(true);
                how_to_play_button.set_visible(true);
                cursor.set_visible(true);

                // Move the cursor between buttons
                if (bn::keypad::down_pressed() && cursor.y() == 52) {
                    cursor.set_position(-40, 65);
                } else if (bn::keypad::up_pressed() && cursor.y() == 65) {
                    cursor.set_position(-40, 52);
                }

                // Handle 'A' button press for navigation
                if (bn::keypad::a_pressed()) {
                    start_button.set_visible(false);
                    how_to_play_button.set_visible(false);
                    cursor.set_visible(false);

                    if (cursor.y() == 52) { // Start button selected
                        current_state = ScreenState::GAMESCREEN;
                    } else if (cursor.y() == 65) { // How to Play button selected
                        current_state = ScreenState::INSTRUCTIONS;
                    }
                }
                break;
            }
            case ScreenState::INSTRUCTIONS: {
                bn::core::update();
                load_background(bn::regular_bg_items::instructions_bg, ScreenState::INSTRUCTIONS);
                static bn::sprite_ptr back_button = bn::sprite_items::back_button.create_sprite(0, 65);
                static bn::sprite_ptr cursor = bn::sprite_items::cursor.create_sprite(-40, 65);

                back_button.set_visible(true);
                cursor.set_visible(true);

                // Handle 'B' button press for navigation
                if (bn::keypad::b_pressed() && cursor.y() == 65) // Back button selected
                {
                    current_state = ScreenState::START;
                    back_button.set_visible(false);
                    cursor.set_visible(false);
                }
                break;
            }
            case ScreenState::GAMESCREEN: {
                bn::music_items::save_the_beach.play(0.1);

                enum ObstacleType {
                    GAS_CAN,
                    OIL_SPILL,
                    NORMAL_OBSTACLE
                };

                struct Obstacle {
                    bn::sprite_ptr sprite;
                    ObstacleType type;

                    // Default constructor for array initialization
                    Obstacle() : sprite(bn::sprite_items::empty.create_sprite(-1024, 0)), type(NORMAL_OBSTACLE) {}

                    // Explicit constructor to initialize the obstacle
                    Obstacle(bn::sprite_ptr new_sprite, ObstacleType new_type)
                        : sprite(new_sprite), type(new_type) {}

                    bool operator==(const Obstacle& other) const {
                        return sprite == other.sprite && type == other.type;
                    }
                };

                bn::timer timer;

                const size_t max_obstacles = 3;  // We only have 3 obstacles
                int num_obstacles = 0;

                // Variables for game state and time
                int elapsed_time = 0;
                bool game_over = false;

                // Create a random number generator object
                bn::random random;

                // Create a camera
                bn::camera_ptr camera = bn::camera_ptr::create(0, 0);

                bn::sprite_ptr linkin_kart = bn::sprite_items::linkin_kart.create_sprite(-100, 40);

                float top_boundary = 30;
                float bottom_boundary = 66;

                //Countdown timer setup
                bn::sprite_text_generator text_generator(common::variable_8x8_sprite_font);
                bn::vector<bn::sprite_ptr, 16> text_sprites;

                int display_best_score = 0;
                bn::sram::read(display_best_score);
                int best_time = display_best_score; // Store the best (maximum) time achieved

                uint64_t ticks = 0;
                int frame_counter = 0; // Counter to track frames for slowing down the timer

                //Declaring the items for the background
                bn::regular_bg_ptr backdrop_image = bn::regular_bg_items::backdrop_image.create_bg(0, 0);
                bn::regular_bg_ptr lap_out_loud = bn::regular_bg_items::lap_out_loud.create_bg(-622, 0);

                // Frequency control for each type of obstacle (Barrel, Gas Can, Oil Spill)
                int barrel_spawn_frequency = 100;  // Barrels spawn every 100 frames
                int gas_spawn_frequency = 150;  // Gas cans spawn every 150 frames
                int oil_spawn_frequency = 150;  // Oil spills spawn every 150 frames
                int barrel_counter = 0;
                int gas_counter = 0;
                int oil_counter = 0;

                // Variables for car speed
                bn::timer speed_timer;
                bn::timer slow_timer;
                bool speed_up = false;
                bool slow_down = false;

                Obstacle obstacles[max_obstacles];

                // Waiting for 'a' to start the game
                while (!bn::keypad::a_pressed()) {
                    bn::core::update();  // Keep updating the core to handle input, etc.
                    bn::string<16> start_text = "Press A to Start";
                    text_sprites.clear();
                    text_generator.generate(-58, -50, start_text, text_sprites);
                }
                while (current_state == ScreenState::GAMESCREEN) {
                    bn::core::update();
                    backdrop_image.set_camera(camera);
                    lap_out_loud.set_camera(camera);
                    if (bn::keypad::a_held()) {
                        camera.set_x(camera.x() + 2);
                        if (linkin_kart.x() > 80) {
                            linkin_kart.set_x(30);
                        } else {
                            linkin_kart.set_x(linkin_kart.x() + 0.005);
                        }
                        if (bn::keypad::up_held()) {
                            if (linkin_kart.y() > top_boundary) {
                                linkin_kart.set_y(linkin_kart.y() - 1);
                            } else {
                                linkin_kart.set_y(30);
                            }    
                        }
                        if (bn::keypad::down_held()) {
                            if (linkin_kart.y() < bottom_boundary) {
                                linkin_kart.set_y(linkin_kart.y() + 1);
                            } else {
                                linkin_kart.set_y(66);
                            }
                        }

                        // Update ticks based on elapsed time
                        ticks += timer.elapsed_ticks_with_restart();

                        // Slow down the countdown timer by updating it every 60 frames
                        frame_counter++;
                        if (frame_counter >= 60) { // 60 frames = roughly 1 second
                            frame_counter = 0;
                            if (!game_over) {
                                elapsed_time++;  // Increment elapsed time every second
                            }
                        }

                        // Convert countdown_seconds to minutes and seconds
                        int minutes = elapsed_time / 60;
                        int seconds_left = elapsed_time % 60;

                        // Update the countdown timer text display
                        bn::string<32> timer_text = "Current: " + bn::to_string<32>(minutes) + ":" + bn::to_string<32>(seconds_left) + " Best: " + bn::to_string<32>(best_time / 60) + ":" + bn::to_string<32>(best_time % 60);
                        text_sprites.clear();
                        text_generator.generate(-6 * 16, -68, timer_text, text_sprites);  // Position at the top-left corner

                        // Update car speed based on active powerups
                        if (speed_up && speed_timer.elapsed_ticks() < 5 * 60) {
                            linkin_kart.set_x(linkin_kart.x() + 0.1);  // Speed up for 5 seconds
                        } else {
                            speed_up = false;  // Reset speed up state after 5 seconds
                        }
                        if (slow_down && slow_timer.elapsed_ticks() < 5 * 60) {
                            linkin_kart.set_x(linkin_kart.x() - 0.05);
                        } else {
                            slow_down = false;
                        }

                        // Manage obstacle spawning
                        if (barrel_counter >= barrel_spawn_frequency && num_obstacles < max_obstacles) {
                            barrel_counter = 0;
                            int x_position = 100;  // Starting position on the right side
                            int y_position = random.get_int(top_boundary, bottom_boundary);  // Random y position

                            // Check for overlap with existing obstacles before creating a barrel
                            bool overlap = false;
                            for (int i = 0; i < num_obstacles; ++i) {
                                if (obstacles[i].sprite.x() == x_position && obstacles[i].sprite.y() == y_position) {
                                    overlap = true;
                                    break;
                                }
                            }

                            if (!overlap) {
                                obstacles[num_obstacles] = Obstacle(bn::sprite_items::barrel.create_sprite(x_position, y_position), NORMAL_OBSTACLE);
                                num_obstacles++;
                            }
                        }

                        if (gas_counter >= gas_spawn_frequency && num_obstacles < max_obstacles) {
                            gas_counter = 0;
                            int x_position = 100;  // Starting position on the right side
                            int y_position = random.get_int(top_boundary, bottom_boundary);  // Random y position

                            obstacles[num_obstacles] = Obstacle(bn::sprite_items::gas.create_sprite(x_position, y_position), GAS_CAN);
                            num_obstacles++;
                        }

                        if (oil_counter >= oil_spawn_frequency && num_obstacles < max_obstacles) {
                            oil_counter = 0;
                            int x_position = 100;  // Starting position on the right side
                            int y_position = random.get_int(top_boundary, bottom_boundary);  // Random y position

                            obstacles[num_obstacles] = Obstacle(bn::sprite_items::oil_spill.create_sprite(x_position, y_position), OIL_SPILL);
                            num_obstacles++;
                        }

                        // Increment the counters for the obstacle spawn frequency
                        barrel_counter++;
                        gas_counter++;
                        oil_counter++;

                        // Move and check obstacles
                        for (int i = 0; i < num_obstacles; ++i) {
                            // Move obstacles left, independent of car's position
                            obstacles[i].sprite.set_x(obstacles[i].sprite.x() - 1);  // Move the obstacle left

                            // If the obstacle goes off-screen to the left, reset it to the right
                            if (obstacles[i].sprite.x() < -110) {
                                obstacles[i].sprite.set_x(100);  // Reset x position
                                obstacles[i].sprite.set_y(random.get_int(top_boundary, bottom_boundary));  // Set new y position
                            }

                            // Check for interaction with the car (collision detection)
                            bn::rect car_rect(int(linkin_kart.x()), int(linkin_kart.y()), 16, 16);
                            bn::rect obstacle_rect(int(obstacles[i].sprite.x()), int(obstacles[i].sprite.y()), 8, 4);

                            if (car_rect.intersects(obstacle_rect)) {
                                // Handle collision with the car
                                if (obstacles[i].type == GAS_CAN) {
                                    speed_up = true;
                                    speed_timer.restart();
                                    obstacles[i].sprite.set_visible(false);
                                } else if (obstacles[i].type == OIL_SPILL) {
                                    slow_down = true;  // Slow down the car when hitting oil spill
                                    slow_timer.restart();
                                    obstacles[i].sprite.set_visible(false);
                                } else if (obstacles[i].type == NORMAL_OBSTACLE) {
                                    bn::sound_items::collision_new.play();
                                    game_over = true;  // End the game if the car hits a normal obstacle
                                }
                            }
                        }
                    }
                    if (game_over) {
                        if (elapsed_time > best_time) {
                            best_time = elapsed_time;
                            bn::sram::write(best_time);
                        }
                        bn::sprite_ptr explosion = bn::sprite_items::explosion.create_sprite(int(linkin_kart.x()), int(linkin_kart.y()));
                        elapsed_time = 0;
                        text_sprites.clear(); // Clear the vector, removing the references to old sprites
                        text_generator.generate(-30, -40, "Game Over", text_sprites);
                        text_generator.generate(-58, 5, "Press A to restart", text_sprites);
                        text_generator.generate(-60, 20, "Press B to go to Start", text_sprites);

                        explosion.set_visible(true);
                        bn::core::update();
                        // Handle game over, wait for input to restart or quit
                        while (true) {
                            bn::core::update();
                            if (bn::keypad::a_pressed()) {
                                game_over = false;
                                elapsed_time = 0;
                                num_obstacles = 0;
                                for (int i = 0; i < max_obstacles; ++i) {
                                    obstacles[i].sprite.set_visible(false);
                                }
                                backdrop_image.set_position(0, 0);
                                lap_out_loud.set_position(-622, 0);
                                backdrop_image.set_camera(camera);
                                lap_out_loud.set_camera(camera);
                                linkin_kart.set_position(-100, 40);
                                timer.restart();
                                current_state = ScreenState::GAMESCREEN;
                                break;
                            } else if (bn::keypad::b_pressed()) {
                                game_over = false;
                                bn::music_items::how_to_play.play(1);
                                current_state = ScreenState::START;
                                break;
                            }
                        }
                        break;
                    }
                }
                break;
            }
            break;
        }
        bn::core::update();
    }
    return 0;
}
