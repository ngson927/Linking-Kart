// Butano libraries
#include "bn_core.h"
#include "bn_keypad.h"
#include "bn_regular_bg_ptr.h"
#include "bn_sprite_ptr.h"

int main()
{
    //initialization of our program, we want to make sure that all of our items and feats initialized after this point
    bn::core::init();

    //Loading in track background
    bn:: regular_bg_ptr track = bn::regular_bg_items::track_bg.create_bg(0,0);

    //loading the car sprite in
    bn::sprite_ptr car = bn::sprite_items::car_sprite.create_sprite(0,64);

    //PLACEHOLDER: loading in the obstacles
    //bn::sprite_ptr  = bn::sprite_items:: .create_sprite();

    //bn::sprite_ptr  = bn::sprite_items:: .create_sprite();

    //bn::sprite_ptr  = bn::sprite_items:: .create_sprite();

    int speed = 0;

    //toying with the boundaries for off roading later
    int road_left_boundary = -60;
    int road_right_boundary = 60;

    while(true)
    {

        bool is_off_road = (car.x() < road_left_boundary || car.x() > road_right_boundary);

        if(bn::keypad::a_held())
        {
            //randomly setting the max speed to be 5, can probably be changed later
            if(speed < 5)
            {
                //just as long as A is held, you increase speed with 5=max
                speed++;
            }

            //car.set_y(track.y()+ 1); //scroll the track to emulate motion

            //if left directional button is held, the car will move left but not offscreen
            if(bn::keypad::left_held() && car.x() > -120)
            {
                //if the car is off road, then we also want to slow down the turning because of traction
                car.set_x(car.x() - (is_off_road ? 0.5: 1)); 
            }
            //if right directional button is held, move car over to the right but not offscreen
            if(bn::keypad::right_held() && car.x() < 120)
            {
                //if the car is off road, then we also want to slow down the turning because of traction
                car.set_x(car.x() + (is_off_road ? 0.5: 1));
            }
        }
        else
        {
            //if the A button is released and speed is greater than 0, decrease speed
            if(speed > 0)
            {
                speed--;
            }
        }

        if(is_off_road)
        {
            //since we dont want the car to come to a complete stop, we just gradually decrease speed off_road
            if(speed > 0)
            {
                speed -= 0.5;
            }
        }
        //Just to be sure our speed doesnt somehow end up negative
        if(speed < 0)
        {
            speed = 0;
        }

        //since speed is determined by whether or not the A button is pressed
        //and speed is slowed when A is let go of, we must emulate that scrolling with out background
        track.set_y(track.y() + speed);
        //needed to update the screen, basically display and give sound to the program
        bn::core::update();
    }
}
