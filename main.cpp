#include <iostream>
#include <cmath>
#include <cerrno>
#include <cstring>
#include <cstdlib>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/fl_draw.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/fl_ask.H>

#include "binheap.hpp"
#include "map.hpp"
#include "astar.hpp"

#define FRAME_TIME 0.0166f
#define FLOOR_SIZE 9
#define CAMERA_SPEED 40

using namespace std;

// controls debug_information shown, higher means more and 0 means no debug information.
static int global_debug_level = 0;
static int global_offset_x = 0;
static int global_offset_y = 0;
static int global_map_width = 0, global_map_height = 0; // TODO: maybe remove this (?)
static int global_window_width = 0, global_window_height = 0;
Fl_Double_Window *window;

/*
   Updates the game and redraws the screen after a periodic delay of `FRAME_TIME` seconds.
 */ 
void update(void *)
{
    // TODO: maybe remove this (?)
    global_window_width = window->w();
    global_window_height = window->h();

    Fl::redraw();
    Fl::repeat_timeout(FRAME_TIME, update);
}

/*
   DrawingArea area, the draw() procedure is called once every frame.
   It is necessary to specify a map to be drawn when instancing this class.
 */
class DrawingArea : public Fl_Widget
{
    // TODO: create a linked list or a hash table for this
    int clicked_pixels[80][2];
    int clicked_pixels_index;

    void debug_print_squares()
    {
        // show hitboxes
        int unit_size = this->map->hitbox->unit_size;
        for (int i = 0; i < this->map->hitbox->width; i++)
        {
            for (int j = 0; j < this->map->hitbox->height; j++)
            {
                if (this->map->hitbox->data[i + j * this->map->hitbox->width] == 4) fl_color(FL_DARK_YELLOW);
                else if (this->map->hitbox->data[i + j * this->map->hitbox->width] == 1) fl_color(FL_GREEN);
                else if (this->map->hitbox->data[i + j * this->map->hitbox->width] == 2) fl_color(FL_BLUE);
                else if (this->map->hitbox->data[i + j * this->map->hitbox->width] == 3) fl_color(FL_RED);
                else fl_color(FL_DARK3);

                fl_rectf(global_offset_x + unit_size * i, global_offset_y + unit_size * j, unit_size, unit_size);
            }
        }

        // TODO: not working because of the movement system
        if (global_debug_level == 1)
        {
            for (int i = 0; i < this->clicked_pixels_index; i++)
            {
                fl_color(FL_YELLOW);
                fl_point(this->clicked_pixels[i][0], this->clicked_pixels[i][1]);
            }
        }
    }

    void draw()
    {
        this->map->img->draw(global_offset_x, global_offset_y);
        fl_push_clip(x(),y(),w(),h());
        fl_push_matrix();
        if(global_debug_level) this->debug_print_squares();
        fl_pop_matrix();
        fl_pop_clip();
    }

    int clicked_squares[2];
    int *path;
    int path_size;

    int handle(int event)
    {
        switch (event)
        {
            case FL_PUSH:
                if (Fl::event_button() == FL_LEFT_MOUSE)
                {
                    int x = Fl::event_x() - global_offset_x, y = Fl::event_y() - global_offset_y;
                    int index = (int) (floor((float) x/this->map->hitbox->unit_size) + floor((float) y/this->map->hitbox->unit_size) * this->map->hitbox->width);

                    if (this->map->hitbox->data[index] == 1) return 1;

                    // TODO: do this better, currently not working
                    if (this->clicked_pixels_index <= 1) this->clicked_squares[this->clicked_pixels_index] = index;

                    this->map->hitbox->data[index] = 2;
                    this->clicked_pixels[this->clicked_pixels_index][0] = x;
                    this->clicked_pixels[this->clicked_pixels_index][1] = y;
                    this->clicked_pixels_index++;
                    // END OF TODO

                    // TODO: do this better
                    if (this->clicked_pixels_index >= 2)
                    {
                        if(!find_path_astar(
                            this->map,
                            this->clicked_squares[0],
                            this->clicked_squares[1],
                            &this->path,
                            &this->path_size
                        ))
                        {
                            for (int i = this->path_size - 1; i >= 0; i--)
                            {
                                this->map->hitbox->data[this->path[i]] = 3;
                            }
                        }
                    }
                    // END OF TODO

                    return 1;
                }
                return 0;
        }
        return 0;
    }

public:
    Map *map;

    DrawingArea(Map *map) : Fl_Widget(0, 0, map->img->w(), map->img->h())
    {
        this->map = map;
        this->clicked_pixels_index = 0;
    }
};

/*
   Handles events that every widget has ignored.

   @ event   :: type of the event to be handled or ignored

   @ returns :: 0 if the event was ignored, 1 otherwise.
 */
int global_event_handler(int event)
{
    switch (event)
    {
        case FL_SHORTCUT:
            if (!strcmp(Fl::event_text(), "1")) global_debug_level = 0;
            else if (!strcmp(Fl::event_text(), "2")) global_debug_level = 1;
            else if (!strcmp(Fl::event_text(), "3")) global_debug_level = 2;
            else if (Fl::event_key() == FL_Up && global_offset_y <= -CAMERA_SPEED) global_offset_y += CAMERA_SPEED;
            else if (Fl::event_key() == FL_Down && global_offset_y >= global_window_height - global_map_height + CAMERA_SPEED) global_offset_y -= CAMERA_SPEED;
            else if (Fl::event_key() == FL_Left && global_offset_x <= -CAMERA_SPEED) global_offset_x += CAMERA_SPEED;
            else if (Fl::event_key() == FL_Right && global_offset_x >= global_window_width - global_map_width + CAMERA_SPEED) global_offset_x -= CAMERA_SPEED;
            else return 0;
            return 1;
    }

    return 0;
}

/*
   Entry point.
 */
int main(int argc, char** argv) {

    // try to open input file and load image
    Fl_PNG_Image *img = new Fl_PNG_Image("./lol.png");
    if (img->fail())
    {
        // TODO: remove this hardcoded line
        fl_alert("./test.png: %s", strerror(errno));
        exit(1);
    }

    // TODO: maybe remove this (?)
    global_map_width = img->w();
    global_map_height = img->h();

    // create map using the loaded image
    Map *map = new Map(img, FLOOR_SIZE);

    // create window and drawing area
    window = new Fl_Double_Window(map->img->w(), map->img->h());
    DrawingArea drawing_area(map);
    window->end();

    // show window
    window->show(argc, argv);

    // handle some events globally
    Fl::add_handler(global_event_handler);

    // schedule a periodic update after a delay of `FRAME_TIME` seconds
    Fl::add_timeout(FRAME_TIME, update);

    return Fl::run();
}
