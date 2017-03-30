#include <iostream>
#include <map>
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

// TODO: create a constants file
// NOTE: this is a terrible way of getting 60-fps, but this library has no better tool for that
// TODO: maybe find a way to better update the screen anyway
#define UPDATE_TICK_DELAY 0.017
#define CAMERA_SPEED 30

using namespace std;

Map *global_map;
Fl_Double_Window *global_window;
map<int, bool> global_keys_pressed;

// controls debug_information shown, higher means more and 0 means no debug information.
static int global_debug_level = 0;
static int global_offset_x = 0;
static int global_offset_y = 0;

static int global_start = -1;
static int global_end = -1;

/*
   Updates the game and redraws the screen after a periodic delay of `UPDATE_TICK_DELAY` seconds.
 */ 
void update(void *)
{
    if (global_keys_pressed[FL_Up] && global_offset_y <= -CAMERA_SPEED)
    {
        global_offset_y += CAMERA_SPEED;
        Fl::redraw();
    }
    if (global_keys_pressed[FL_Down] && global_offset_y >= global_window->h() - global_map->img->h() + CAMERA_SPEED)
    {
        global_offset_y -= CAMERA_SPEED;
        Fl::redraw();
    }
    if (global_keys_pressed[FL_Left] && global_offset_x <= -CAMERA_SPEED)
    {
        global_offset_x += CAMERA_SPEED;
        Fl::redraw();
    }
    if (global_keys_pressed[FL_Right] && global_offset_x >= global_window->w() - global_map->img->w() + CAMERA_SPEED)
    {
        global_offset_x -= CAMERA_SPEED;
        Fl::redraw();
    }

    Fl::repeat_timeout(UPDATE_TICK_DELAY, update);
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
        int unit_size = global_map->hitbox->unit_size;
        for (int i = 0; i < global_map->hitbox->width; i++)
        {
            for (int j = 0; j < global_map->hitbox->height; j++)
            {
                // TODO: put this back in, it works (but slows down the path shower)
                /*if (global_map->hitbox->data[i + j * global_map->hitbox->width] == 4) fl_color(FL_DARK_YELLOW);
                else*/ if (global_map->hitbox->data[i + j * global_map->hitbox->width] == 1) fl_color(FL_GREEN);
                else if (global_map->hitbox->data[i + j * global_map->hitbox->width] == 2) fl_color(FL_BLUE);
                else if (global_map->hitbox->data[i + j * global_map->hitbox->width] == 3) fl_color(FL_RED);
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
        global_map->img->draw(global_offset_x, global_offset_y);
        fl_push_clip(x(),y(),w(),h());
        fl_push_matrix();
        if (global_debug_level) this->debug_print_squares();
        // TODO: clean up this
        int unit_size = global_map->hitbox->unit_size;
        for (int i = this->path_size - 1; i >= 0; i--)
        {
            int index = this->path[i];
            int y = index / global_map->hitbox->width;
            int x = index - y * global_map->hitbox->width;
            fl_rectf(global_offset_x + unit_size * x, global_offset_y + unit_size * y, unit_size, unit_size, FL_RED);
        }
        fl_pop_matrix();
        fl_pop_clip();
    }

    int clicked_squares[2];
    int *path = NULL;
    int path_size = 0;

    int handle(int event)
    {
        switch (event)
        {
            case FL_FOCUS:
                return 1;

            case FL_UNFOCUS:
                return 1;

            case FL_KEYDOWN:
                {
                    int key = Fl::event_key();
                    if (key != FL_Up && key != FL_Down && key != FL_Right && key != FL_Left) return 0;
                    global_keys_pressed[key] = true;
                    return 1;
                }

            case FL_KEYUP:
                {
                    int key = Fl::event_key();
                    if (key != FL_Up && key != FL_Down && key != FL_Right && key != FL_Left) return 0;
                    global_keys_pressed[key] = false;
                    return 1;
                }

            case FL_PUSH:
                {
                    int raw_x = Fl::event_x() - global_offset_x, raw_y = Fl::event_y() - global_offset_y;
                    int index_y = raw_y / global_map->hitbox->unit_size;
                    int index_x = raw_x / global_map->hitbox->unit_size; 
                    int index = index_x + index_y * global_map->hitbox->width;

                    if (Fl::event_button() == FL_LEFT_MOUSE)
                    {

                        if (global_map->hitbox->data[index] == 1) return 1;

                        // TODO: STILL IN PROGRESS
                        global_start = index;

                        // TODO: do this better, currently not working
                        if (this->clicked_pixels_index <= 1) this->clicked_squares[this->clicked_pixels_index] = index;

                        //global_map->hitbox->data[index] = 2;

                        this->clicked_pixels[this->clicked_pixels_index][0] = raw_x;
                        this->clicked_pixels[this->clicked_pixels_index][1] = raw_y;
                        this->clicked_pixels_index++;
                        // END OF TODO
                    }
                    else if (Fl::event_button() == FL_RIGHT_MOUSE)
                    {
                        // TODO: STILL IN PROGRESS
                        if (global_map->hitbox->data[index] == 1) return 1;

                        global_end = index;
                    }

                    if (global_start != -1 && global_end != -1)
                    {
                        // clean previous path
                        for (int i = this->path_size - 1; i >= 0; i--)
                        {
                            global_map->hitbox->data[this->path[i]] = 0;
                        }

                        if(!find_path_astar(global_map, global_start, global_end, &this->path, &this->path_size))
                        {
                            for (int i = this->path_size - 1; i >= 0; i--)
                            {
                                global_map->hitbox->data[this->path[i]] = 3;
                            }
                        }

                        Fl::redraw();
                    }

                    return 1;
                }
        }
        return 0;
    }

public:
    DrawingArea() : Fl_Widget(0, 0, global_map->img->w(), global_map->img->h())
    {
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
            const char *key_text = Fl::event_text();
            if (!strcmp(key_text, "1")) global_debug_level = 0;
            else if (!strcmp(key_text, "2")) global_debug_level = 1;
            else if (!strcmp(key_text, "3")) global_debug_level = 2;
            else return 0;
            Fl::redraw();
            return 1;
    }

    return 0;
}

/*
   Entry point.
 */
int main(int argc, char** argv) {

    // try to open input file and load image
    const char background_img_path[] = "./map.png";
    Fl_PNG_Image *img = new Fl_PNG_Image(background_img_path);
    if (img->fail())
    {
        fl_alert("%s: %s", background_img_path, strerror(errno));
        exit(1);
    }

    // create map using the loaded image
    global_map = new Map(img);

    // create window and drawing area
    global_window = new Fl_Double_Window(global_map->img->w(), global_map->img->h());
    DrawingArea *drawing_area = new DrawingArea();
    global_window->end();

    // show window
    global_window->show(argc, argv);

    // handle some events globally
    Fl::add_handler(global_event_handler);

    // schedule a periodic update after a delay of `UPDATE_TICK_DELAY` seconds
    Fl::add_timeout(UPDATE_TICK_DELAY, update);

    return Fl::run();
}
