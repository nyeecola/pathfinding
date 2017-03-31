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
#include "camera.hpp"

Map *global_map;
Fl_Double_Window *global_window;
map<int, bool> global_keys_pressed;
Camera *global_camera;

/*
 * Updates the game and redraws the screen after a periodic delay of `UPDATE_TICK_DELAY` seconds.
 */ 
void update(void *)
{
    if (global_keys_pressed[FL_Up] && global_camera->y <= -global_camera->speed)
    {
        global_camera->y += global_camera->speed;
    }
    if (global_keys_pressed[FL_Down] && global_camera->y >= global_window->h() - global_map->img->h() + global_camera->speed)
    {
        global_camera->y -= global_camera->speed;
    }
    if (global_keys_pressed[FL_Left] && global_camera->x <= -global_camera->speed)
    {
        global_camera->x += global_camera->speed;
    }
    if (global_keys_pressed[FL_Right] && global_camera->x >= global_window->w() - global_map->img->w() + global_camera->speed)
    {
        global_camera->x -= global_camera->speed;
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
    int path_start_pixel[2] = {-1, -1};
    int path_end_pixel[2] = {-1, -1};

    // walks on top of path squares (always passing exactly in the center of each one)
    void print_pixel_path()
    {
        int pixel_start_x = global_camera->x + path_start_pixel[0];
        int pixel_start_y = global_camera->y + path_start_pixel[1];
        int pixel_end_x = global_camera->x + path_end_pixel[0];
        int pixel_end_y = global_camera->y + path_end_pixel[1];

        fl_color(FL_GREEN);
        int last_pixel[2] = {pixel_start_x, pixel_start_y};
        for (int i = this->path_size - 2; i >= 1 ; i--)
        {
            int index = this->path[i];
            int y = index / global_map->hitbox->width;
            int x = index - y * global_map->hitbox->width;
            int pixel_x = (x * global_map->hitbox->unit_size) + global_map->hitbox->unit_size / 2;
            int pixel_y = (y * global_map->hitbox->unit_size) + global_map->hitbox->unit_size / 2;
            int next_pixel[2] = {global_camera->x + pixel_x, global_camera->y + pixel_y};

            fl_line(last_pixel[0], last_pixel[1], next_pixel[0], next_pixel[1]);

            last_pixel[0] = next_pixel[0];
            last_pixel[1] = next_pixel[1];
        }
        fl_line(last_pixel[0], last_pixel[1], pixel_end_x, pixel_end_y);
    }

    void debug_print_sprite_path()
    {
        int unit_size = global_map->hitbox->unit_size;
        for (int i = this->path_size - 1; i >= 0; i--)
        {
            int index = this->path[i];
            int y = index / global_map->hitbox->width;
            int x = index - y * global_map->hitbox->width;
            fl_rectf(global_camera->x + unit_size * x, global_camera->y + unit_size * y, unit_size, unit_size, FL_RED);
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
                    fl_rectf(global_camera->x + unit_size * i, global_camera->y + unit_size * j, unit_size, unit_size, FL_BLUE);
                }
            }
        }
    }
#endif

    void draw()
    {
        global_map->img->draw(global_camera->x, global_camera->y);
        fl_push_clip(x(),y(),w(),h());
        fl_push_matrix();
#if DEBUG
        this->debug_print_squares();
#endif
        // TODO: print line_path instead of sprite_path on normal mode
        if (this->path)
        {
            this->debug_print_sprite_path();
            this->print_pixel_path();
        }
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
                    int raw_x = Fl::event_x() - global_camera->x, raw_y = Fl::event_y() - global_camera->y;
                    int index_y = raw_y / global_map->hitbox->unit_size;
                    int index_x = raw_x / global_map->hitbox->unit_size; 
                    int index = index_x + index_y * global_map->hitbox->width;

                    if (Fl::event_button() == FL_LEFT_MOUSE)
                    {
                        if (global_map->hitbox->data[index] == TERRAIN) return 1;
                        this->path_start = index;
                        this->path_start_pixel[0] = raw_x;
                        this->path_start_pixel[1] = raw_y;
                    }
                    else if (Fl::event_button() == FL_RIGHT_MOUSE)
                    {
                        if (global_map->hitbox->data[index] == TERRAIN) return 1;
                        this->path_end = index;
                        this->path_end_pixel[0] = raw_x;
                        this->path_end_pixel[1] = raw_y;
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
    MapArea() : Fl_Widget(0, 0, global_window->w(), global_window->h()) {}
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

    // create camera
    global_camera = new Camera(0, 0, CAMERA_SPEED);

    // create window and drawing area
    global_window = new Fl_Double_Window(WINDOW_WIDTH, WINDOW_HEIGHT);
    new MapArea();
    global_window->end();

    // show window
    global_window->show(argc, argv);

    // initialize keys pressed
    {
        global_keys_pressed[FL_Up] = false;
        global_keys_pressed[FL_Down] = false;
        global_keys_pressed[FL_Left] = false;
        global_keys_pressed[FL_Right] = false;
    }

    // schedule a periodic update after a delay of `UPDATE_TICK_DELAY` seconds
    Fl::add_timeout(UPDATE_TICK_DELAY, update);

    return Fl::run();
}
