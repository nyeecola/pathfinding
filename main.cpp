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
#define FLOOR_SIZE 5

using namespace std;

// controls debug_information shown, higher means more and 0 means no debug information.
static int global_debug_level = 0;

class BinHeap
{
    int *data;
    int current_size, max_size;

    // TODO: remove this dirty hack
    int *test_matrix;

    void bubble_down(int index)
    {
        int left_child_index = 2 * index + 1;
        int right_child_index = 2 * index + 2;

        if (left_child_index > this->current_size) return;

        int min_index = index;

        if (this->test_matrix[this->data[index]] > this->test_matrix[this->data[left_child_index]]) min_index = left_child_index;
        // if (this->data[index] > this->data[left_child_index]) min_index = left_child_index;
        if ((right_child_index < this->current_size) && (this->test_matrix[this->data[min_index]] > this->test_matrix[this->data[right_child_index]])) min_index = right_child_index;
        // if ((right_child_index < this->current_size) && (this->data[min_index] > this->data[right_child_index])) min_index = right_child_index;

        if (min_index != index)
        {
            int temp = this->data[index];
            this->data[index] = this->data[min_index];
            this->data[min_index] = temp;
            bubble_down(min_index);
        }
    }

    void bubble_up(int index)
    {
        if (!index) return;

        int parent_index = (index - 1) / 2;

        // if (this->data[parent_index] > this->data[index])
        if (this->test_matrix[this->data[parent_index]] > this->test_matrix[this->data[index]])
        {
            int temp = this->data[parent_index];
            this->data[parent_index] = this->data[index];
            this->data[index] = temp;
            bubble_up(parent_index);
        }
    }

public:
    BinHeap(int max_size, int *matrix) // TODO: remove test matrix
    {
        this->test_matrix = matrix;
        this->max_size = max_size;
        this->data = (int *) malloc(this->max_size * sizeof(int));
        this->current_size = 0;
    }

    void insert(int value)
    {
        this->data[this->current_size] = value;
        this->current_size++;

        bubble_up(this->current_size - 1);
    }

    int get_next()
    {
        if (!this->current_size) return -1;

        return this->data[0];
    }
    
    void remove_next()
    {
        if (!this->current_size) return;

        this->data[0] = this->data[this->current_size - 1];
        this->current_size--;

        bubble_down(0);
    }

    int pop_next()
    {
        int next = this->get_next();

        this->remove_next();

        return next;
    }
};

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

    Map(Fl_PNG_Image *img, int hitbox_unit_size)
    {
        this->img = img;
        this->create_image_hitbox(hitbox_unit_size);
    }
};

/*


   @ start

   @ returns    :: 0 if the path was found, -1 if there is no path
 */
int find_path_astar(Map *map, int start, int end, int *path, int path_max_size, int *found_path_size)
{
    int map_size = map->hitbox->width * map->hitbox->height;
    int *list = (int *) calloc(map_size, sizeof(int));
    int *parents = (int *) calloc(map_size, sizeof(int));
    int *f_costs = (int *) calloc(map_size, sizeof(int));
    int *g_costs = (int *) calloc(map_size, sizeof(int));
    BinHeap heap(map_size, f_costs);

    enum {OPEN = 1, CLOSED = 2};

    bool path_found = false;

    // add the starting node to the open list and the heap
    list[start] = OPEN;
    heap.insert(start);
    
    //
    for (;;)
    {
        // find lowest cost square on the open list
        int current_square = heap.pop_next();

        if (current_square == end)
        {
            path_found = true;
            break;
        }

        if (current_square == -1) break;

        // move it to the closed list
        list[current_square] = CLOSED;

        for (int j = current_square - map->hitbox->width - 1; j <= current_square - map->hitbox->width + 1; j++)
        {
            if (j < 0) break;
            if (j < ((current_square / map->hitbox->width) - 1) * (map->hitbox->width) || j >= (current_square / map->hitbox->width) * (map->hitbox->width)) continue;

            if (map->hitbox->data[j] != 1 && list[j] != CLOSED)
            {
                if (list[j] != OPEN)
                {
                    list[j] = OPEN;
                    parents[j] = current_square;

                    int g_cost;
                    if (j == current_square - map->hitbox->width) g_cost = g_costs[current_square] + 10;
                    else g_cost = g_costs[current_square] + 14;

                    int y_offset = abs((j / map->hitbox->width) - (end / map->hitbox->width)) * 10;
                    int x_offset = abs((j - (j / map->hitbox->width) * map->hitbox->width) - (end - (end / map->hitbox->width) * map->hitbox->width)) * 10;
                    int h_cost = x_offset + y_offset;

                    g_costs[j] = g_cost;
                    f_costs[j] = g_cost + h_cost;

                    heap.insert(j);
                }
                else
                {
                    int g_cost;
                    if (j == current_square - map->hitbox->width) g_cost = g_costs[current_square] + 10;
                    else g_cost = g_costs[current_square] + 14;

                    if (g_cost < g_costs[j])
                    {
                        parents[j] = current_square;

                        int y_offset = abs((j / map->hitbox->width) - (end / map->hitbox->width)) * 10;
                        int x_offset = abs((j - (j / map->hitbox->width) * map->hitbox->width) - (end - (end / map->hitbox->width) * map->hitbox->width)) * 10;
                        int h_cost = x_offset + y_offset;

                        g_costs[j] = g_cost;
                        f_costs[j] = g_cost + h_cost;
                    }
                }
            }
        }

        int j = current_square - 1;
        if (j >= (current_square / map->hitbox->width) * map->hitbox->width)
        {
            if (map->hitbox->data[j] != 1 && list[j] != CLOSED)
            {
                if (list[j] != OPEN)
                {
                    list[j] = OPEN;
                    parents[j] = current_square;

                    int g_cost;
                    if (j == current_square - map->hitbox->width) g_cost = g_costs[current_square] + 10;
                    else g_cost = g_costs[current_square] + 14;

                    int y_offset = abs((j / map->hitbox->width) - (end / map->hitbox->width));
                    int x_offset = abs((j - (j / map->hitbox->width) * map->hitbox->width) - (end - (end / map->hitbox->width) * map->hitbox->width));
                    int h_cost = (x_offset + y_offset) * 10;

                    g_costs[j] = g_cost;
                    f_costs[j] = g_cost + h_cost;

                    heap.insert(j);
                }
                else
                {
                    int g_cost;
                    if (j == current_square - map->hitbox->width) g_cost = g_costs[current_square] + 10;
                    else g_cost = g_costs[current_square] + 14;

                    if (g_cost < g_costs[j])
                    {
                        parents[j] = current_square;

                        int y_offset = abs((j / map->hitbox->width) - (end / map->hitbox->width));
                        int x_offset = abs((j - (j / map->hitbox->width) * map->hitbox->width) - (end - (end / map->hitbox->width) * map->hitbox->width));
                        int h_cost = (x_offset + y_offset) * 10;

                        g_costs[j] = g_cost;
                        f_costs[j] = g_cost + h_cost;
                    }
                }
            }
        }

        j = current_square + 1;
        if (j < ((current_square / map->hitbox->width) + 1) * (map->hitbox->width))
        {
            if (map->hitbox->data[j] != 1 && list[j] != CLOSED)
            {
                if (list[j] != OPEN)
                {
                    list[j] = OPEN;
                    parents[j] = current_square;

                    int g_cost;
                    if (j == current_square - map->hitbox->width) g_cost = g_costs[current_square] + 10;
                    else g_cost = g_costs[current_square] + 14;

                    int y_offset = abs((j / map->hitbox->width) - (end / map->hitbox->width)) * 10;
                    int x_offset = abs((j - (j / map->hitbox->width) * map->hitbox->width) - (end - (end / map->hitbox->width) * map->hitbox->width)) * 10;
                    int h_cost = x_offset + y_offset;

                    g_costs[j] = g_cost;
                    f_costs[j] = g_cost + h_cost;

                    heap.insert(j);
                }
                else
                {
                    int g_cost;
                    if (j == current_square - map->hitbox->width) g_cost = g_costs[current_square] + 10;
                    else g_cost = g_costs[current_square] + 14;

                    if (g_cost < g_costs[j])
                    {
                        parents[j] = current_square;

                        int y_offset = abs((j / map->hitbox->width) - (end / map->hitbox->width)) * 10;
                        int x_offset = abs((j - (j / map->hitbox->width) * map->hitbox->width) - (end - (end / map->hitbox->width) * map->hitbox->width)) * 10;
                        int h_cost = x_offset + y_offset;

                        g_costs[j] = g_cost;
                        f_costs[j] = g_cost + h_cost;
                    }
                }
            }
        }

        for (int j = current_square + map->hitbox->width - 1; j <= current_square + map->hitbox->width + 1; j++)
        {
            if (j >= map->hitbox->width * map->hitbox->height) break;
            if (j < ((current_square / map->hitbox->width) + 1) * (map->hitbox->width) || j >= ((current_square / map->hitbox->width) + 2) * (map->hitbox->width)) continue;

            if (map->hitbox->data[j] != 1 && list[j] != CLOSED)
            {
                if (list[j] != OPEN)
                {
                    list[j] = OPEN;
                    parents[j] = current_square;

                    int g_cost;
                    if (j == current_square + map->hitbox->width) g_cost = g_costs[current_square] + 10;
                    else g_cost = g_costs[current_square] + 14;

                    int y_offset = abs((j / map->hitbox->width) - (end / map->hitbox->width)) * 10;
                    int x_offset = abs((j - (j / map->hitbox->width) * map->hitbox->width) - (end - (end / map->hitbox->width) * map->hitbox->width)) * 10;
                    int h_cost = x_offset + y_offset;

                    g_costs[j] = g_cost;
                    f_costs[j] = g_cost + h_cost;

                    heap.insert(j);
                }
                else
                {
                    int g_cost;
                    if (j == current_square + map->hitbox->width) g_cost = g_costs[current_square] + 10;
                    else g_cost = g_costs[current_square] + 14;

                    if (g_cost < g_costs[j])
                    {
                        parents[j] = current_square;

                        int y_offset = abs((j / map->hitbox->width) - (end / map->hitbox->width)) * 10;
                        int x_offset = abs((j - (j / map->hitbox->width) * map->hitbox->width) - (end - (end / map->hitbox->width) * map->hitbox->width)) * 10;
                        int h_cost = x_offset + y_offset;

                        g_costs[j] = g_cost;
                        f_costs[j] = g_cost + h_cost;
                    }
                }
            }
        }
    }

    // path not found
    if (!path_found) return -1;

    // path found
    int current_square = end;
    path[0] = current_square;
    *found_path_size = 1;
    for (int i = 1; current_square != start; i++)
    {
        current_square = parents[current_square];
        path[i] = current_square;
        *found_path_size += 1;
    }

    // success
    return 0;
}

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

                    if (this->map->hitbox->data[index]) return 1;

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
                        this->path = (int *) calloc(2000000, sizeof(int));
                        if(!find_path_astar(
                            this->map,
                            this->clicked_squares[0],
                            this->clicked_squares[1],
                            this->path,
                            0, // TODO: start using this
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
