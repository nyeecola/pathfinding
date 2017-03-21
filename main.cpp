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

#include "binheap.h"
#include "map.h"
#include "astar.h"

#define FRAME_TIME 0.0166f
#define FLOOR_SIZE 15

using namespace std;

// controls debug_information shown, higher means more and 0 means no debug information.
static int global_debug_level = 0;

/*
   Updates the game and redraws the screen after a periodic delay of `FRAME_TIME` seconds.
 */ 
void update(void *)
{
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
        // handle different debug levels
        int k_increment = 1;
        if (global_debug_level == 1) k_increment = 3;
        else if (global_debug_level == 2) k_increment = 1;

        // show hitboxes
        for (int i = 0; i < this->map->hitbox->width; i++)
        {
            for (int j = 0; j < this->map->hitbox->height; j++)
            {
                for (int k = this->map->hitbox->unit_size * i; k < (i + 1) * this->map->hitbox->unit_size; k+=k_increment)
                {
                    for (int w = this->map->hitbox->unit_size * j; w < (j + 1) * this->map->hitbox->unit_size; w++)
                    {
                        if (this->map->hitbox->data[i + j * this->map->hitbox->width] == 1)
                        {
                            fl_color(FL_GREEN);
                            fl_point(k, w);
                        }
                        else if (global_debug_level == 2 && this->map->hitbox->data[i + j * this->map->hitbox->width] == 2)
                        {
                            fl_color(FL_BLUE);
                            fl_point(k, w);
                        }
                        else if (global_debug_level == 2 && this->map->hitbox->data[i + j * this->map->hitbox->width] == 3)
                        {
                            fl_color(FL_RED);
                            fl_point(k, w);
                        }
                        else
                        {
                            fl_color(FL_DARK3);
                            fl_point(k, w);
                        }
                    }
                }
            }
        }

        if (global_debug_level == 2)
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
        map->img->draw(0, 0);
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
                    int x = Fl::event_x(), y = Fl::event_y();
                    int index = (int) (floor((float) x/this->map->hitbox->unit_size) + floor((float) y/this->map->hitbox->unit_size) * this->map->hitbox->width);

                    if (this->map->hitbox->data[index] == 1) return 1;

                    // TODO: do this better
                    if (this->clicked_pixels_index <= 1) this->clicked_squares[this->clicked_pixels_index] = index;
                    // END OF TODO

                    this->map->hitbox->data[index] = 2;
                    this->clicked_pixels[this->clicked_pixels_index][0] = x;
                    this->clicked_pixels[this->clicked_pixels_index][1] = y;
                    this->clicked_pixels_index++;

                    // TODO: do this better
                    if (this->clicked_pixels_index >= 2)
                    {
                        //this->path = (int *) malloc(40 * sizeof(int));
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
            if (!strcmp(Fl::event_text(),"1")) global_debug_level = 0;
            else if (!strcmp(Fl::event_text(),"2")) global_debug_level = 1;
            else if (!strcmp(Fl::event_text(),"3")) global_debug_level = 2;
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
    Fl_PNG_Image *img = new Fl_PNG_Image("./test2.png");
    if (img->fail())
    {
        fl_alert("./test.png: %s", strerror(errno));
        exit(1);
    }

    // create map using the loaded image
    Map *map = new Map(img, FLOOR_SIZE);

    // create window and drawing area
    Fl_Double_Window window(map->img->w(), map->img->h());
    DrawingArea drawing_area(map);
    window.end();

    // show window
    window.show(argc, argv);
 
    // handle some events globally
    Fl::add_handler(global_event_handler);

    // schedule a periodic update after a delay of `FRAME_TIME` seconds
    Fl::add_timeout(FRAME_TIME, update);

    return Fl::run();
}
