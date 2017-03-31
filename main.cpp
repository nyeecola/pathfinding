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
#include "constants.hpp"

Map *global_map;
Fl_Double_Window *global_window;
map<int, bool> global_keys_pressed;
static int global_offset_x = 0;
static int global_offset_y = 0;

/*
 * Updates the game and redraws the screen after a periodic delay of `UPDATE_TICK_DELAY` seconds.
 */ 
void update(void *)
{
    if (global_keys_pressed[FL_Up] && global_offset_y <= -CAMERA_SPEED)
    {
        global_offset_y += CAMERA_SPEED;
    }
    if (global_keys_pressed[FL_Down] && global_offset_y >= global_window->h() - global_map->img->h() + CAMERA_SPEED)
    {
        global_offset_y -= CAMERA_SPEED;
    }
    if (global_keys_pressed[FL_Left] && global_offset_x <= -CAMERA_SPEED)
    {
        global_offset_x += CAMERA_SPEED;
    }
    if (global_keys_pressed[FL_Right] && global_offset_x >= global_window->w() - global_map->img->w() + CAMERA_SPEED)
    {
        global_offset_x -= CAMERA_SPEED;
    }

    Fl::redraw();
    Fl::repeat_timeout(UPDATE_TICK_DELAY, update);
}

/*
 * Draws the map and handles user input via clicks.
 */
class MapArea : public Fl_Widget
{
    int *path = NULL;
    int path_size = 0;
    int path_start = -1;
    int path_end = -1;

    void debug_print_path()
    {
        int unit_size = global_map->hitbox->unit_size;
        for (int i = this->path_size - 1; i >= 0; i--)
        {
            int index = this->path[i];
            int y = index / global_map->hitbox->width;
            int x = index - y * global_map->hitbox->width;
            fl_rectf(global_offset_x + unit_size * x, global_offset_y + unit_size * y, unit_size, unit_size, FL_RED);
        }
    }

#if DEBUG
    void debug_print_squares()
    {
        int unit_size = global_map->hitbox->unit_size;
        for (int i = 0; i < global_map->hitbox->width; i++)
        {
            for (int j = 0; j < global_map->hitbox->height; j++)
            {
                if (global_map->hitbox->data[i + j * global_map->hitbox->width] == TERRAIN)
                {
                    fl_rectf(global_offset_x + unit_size * i, global_offset_y + unit_size * j, unit_size, unit_size, FL_BLUE);
                }
            }
        }
    }
#endif

    void draw()
    {
        global_map->img->draw(global_offset_x, global_offset_y);
        fl_push_clip(x(),y(),w(),h());
        fl_push_matrix();
#if DEBUG
        this->debug_print_squares();
#endif
        // TODO: print line_path instead of sprite_path on normal mode
        this->debug_print_path();
        fl_pop_matrix();
        fl_pop_clip();
    }

    /*
     * Handles events that every widget has ignored.
     * Should return 0 if the event is ignore, and 1 if not.
     */
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
                        if (global_map->hitbox->data[index] == TERRAIN) return 1;
                        this->path_start = index;
                    }
                    else if (Fl::event_button() == FL_RIGHT_MOUSE)
                    {
                        if (global_map->hitbox->data[index] == TERRAIN) return 1;
                        this->path_end = index;
                    }

                    if (this->path_start != -1 && this->path_end != -1)
                    {
                        int err = find_path_astar(global_map, this->path_start, this->path_end, &this->path, &this->path_size);
                        if (!err) Fl::redraw();
                    }

                    return 1;
                }
        }
        return 0;
    }

public:
    MapArea() : Fl_Widget(0, 0, global_map->img->w(), global_map->img->h()) {}
};

/*
 * Entry point.
 */
int main(int argc, char** argv) {

    // try to open input file and load image
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
    MapArea *drawing_area = new MapArea();
    global_window->end();

    // show window
    global_window->show(argc, argv);

    // schedule a periodic update after a delay of `UPDATE_TICK_DELAY` seconds
    Fl::add_timeout(UPDATE_TICK_DELAY, update);

    return Fl::run();
}
