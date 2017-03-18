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

#define FRAME_TIME 0.0166f
#define FLOOR_SIZE 15

using namespace std;

// controls debug_information shown, higher means more and 0 means no debug information.
static int global_debug_level = 0;

/*
   Defines a map's collision mask.
 */
typedef struct MapHitbox_
{
    int width, height;
    int unit_size;
    unsigned char *data;
} MapHitbox;

/*
   Defines a map that has a visual representation (img) and a collision mask (hitbox).
 */
class Map
{
public:
    Fl_PNG_Image *img;
    MapHitbox *hitbox;

    void create_image_hitbox(int hitbox_unit_size)
    {
        this->hitbox = (MapHitbox *) malloc(sizeof(MapHitbox));
        this->hitbox->unit_size = hitbox_unit_size;
        this->hitbox->width = ceil((float) this->img->w() / this->hitbox->unit_size);
        this->hitbox->height = ceil((float) this->img->h() / this->hitbox->unit_size);
        this->hitbox->data = (unsigned char *) malloc(3 * this->hitbox->width * this->hitbox->height);

        for (int i = 0; i < this->hitbox->width; i++)
        {
            for (int j = 0; j < this->hitbox->height; j++)
            {
                int counter = 0;
                for (int k = this->hitbox->unit_size * i * 3; k < (i + 1) * this->hitbox->unit_size * 3 && k < this->img->w() * 3; k+=3)
                {
                    for (int w = this->hitbox->unit_size * j; w < (j + 1) * this->hitbox->unit_size && w < this->img->h(); w++)
                    {
                        if (!this->img->array[w * img->w() * 3 + k] && !this->img->array[w * img->w() * 3 + k + 1] && !this->img->array[w * this->img->w() * 3 + k + 2]) counter++;
                    }
                }
                if ((float) counter / pow(this->hitbox->unit_size, 2) > 0.25) this->hitbox->data[i + j * this->hitbox->width] = 1;
                else this->hitbox->data[i + j * this->hitbox->width] = 0;
            }
        }
    }

    Map (Fl_PNG_Image *img, int hitbox_unit_size)
    {
        this->img = img;
        this->create_image_hitbox(hitbox_unit_size);
    }
};

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
                        if (this->map->hitbox->data[i + j * this->map->hitbox->width])
                        {
                            fl_color(FL_GREEN);
                            fl_point(k, w);
                        }
                        else
                        {
                            fl_color(FL_WHITE);
                            fl_point(k, w);
                        }
                    }
                }
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

public:
    Map *map;

    DrawingArea(Map *map) : Fl_Widget(0, 0, map->img->w(), map->img->h())
    {
        this->map = map;
    }
};

/*
   Handles events that every widget has ignored.

   @ event :: type of the event to be handled or ignored
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
