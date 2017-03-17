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
#include <FL/Fl_Box.H>

#define FRAME_TIME 0.1

using namespace std;

typedef struct MapHitbox_
{
    int width, height;
    int unit_size;
    unsigned char *data;
} MapHitbox;

Fl_PNG_Image *img;
MapHitbox *global_img_hitbox;

/*
   Draws a grid on the screen using the global variable `gofl` and the #define `MAX_SIZE`.

   @cell_size :: the size of a single cell to be drawn on the screen 
   @offset_x  :: the margin to the left of the grid
   @offset_y  :: the margin to the top of the grid
 */
/*void draw_grid(int cell_size, int offset_x, int offset_y)
{
    int pos_y = 0;
    int pos_x = 0;
    int cell_color = FL_RED;
    for (int i = 0; i < MAX_SIZE ; i++)
    {
        pos_y = offset_y + i*cell_size;
        for (int j = 0; j < MAX_SIZE; j++)
        {
            pos_x = offset_x + j*cell_size;
            //if (gofl.gofl_table[i][j]) cell_color = FL_WHITE;
            else cell_color = FL_RED;
            fl_rectf(pos_x, pos_y, cell_size, cell_size, cell_color);
            fl_color(FL_WHITE);
            fl_line(pos_x, offset_y, pos_x, offset_y + MAX_SIZE*cell_size);
        }
        fl_line(offset_x, pos_y, offset_x + MAX_SIZE*cell_size, pos_y);
    }
    pos_x += cell_size;
    pos_y += cell_size;
    fl_line(pos_x, offset_y, pos_x, offset_y + MAX_SIZE*cell_size);
    fl_line(offset_x, pos_y, offset_x + MAX_SIZE*cell_size, pos_y);
}
*/

// NOTE: currently assuming a pixel size of 3 bytes (RGB)
MapHitbox *create_image_hitbox(const unsigned char *data, int width, int height, int hitbox_unit_size)
{
    MapHitbox *image_hitbox = (MapHitbox *) malloc(sizeof(MapHitbox));
    image_hitbox->unit_size = hitbox_unit_size;
    image_hitbox->width = ceil((float) width / image_hitbox->unit_size);
    image_hitbox->height = ceil((float) height / image_hitbox->unit_size);
    image_hitbox->data = (unsigned char *) malloc(3 * image_hitbox->width * image_hitbox->height);

    cout << image_hitbox->width << " " << image_hitbox->height << endl;
    for (int i = 0; i < image_hitbox->width; i++)
    {
        for (int j = 0; j < image_hitbox->height; j++)
        {
            int counter = 0;
            for (int k = image_hitbox->unit_size * i * 3; k < (i + 1) * image_hitbox->unit_size * 3 && k < width * 3; k+=3)
            {
                for (int w = image_hitbox->unit_size * j; w < (j + 1) * image_hitbox->unit_size && w < height; w++)
                {
                    if (!data[w * width * 3 + k] && !data[w * width * 3 + k + 1] && !data[w * width * 3 + k + 2]) counter++;
                    //if (i > 2 && i < 7 && j > 0 && j < 4) cout << "TEST" << i << " " << j << endl; 
                }
            }
            //cout << "counter for square " << i << " " << j << ": " << counter << endl;
            if ((float) counter / pow(image_hitbox->unit_size, 2) > 0.2) image_hitbox->data[i + j * image_hitbox->width] = 1;
            else image_hitbox->data[i + j * image_hitbox->width] = 0;
        }
    }

    return image_hitbox;
}

void debug_create_squares()
{
    for (int i = 0; i < global_img_hitbox->width; i++)
    {
        for (int j = 0; j < global_img_hitbox->height; j++)
        {
            for (int k = global_img_hitbox->unit_size * i; k < (i + 1) * global_img_hitbox->unit_size; k+=3)
            {
                for (int w = global_img_hitbox->unit_size * j; w < (j + 1) * global_img_hitbox->unit_size; w++)
                {
                    if (global_img_hitbox->data[i + j * global_img_hitbox->width])
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

/*
   Updates the game and redraws the screen after a periodic delay of `FRAME_TIME` seconds.
 */ 
void update(void *)
{
    Fl::redraw();
    Fl::repeat_timeout(FRAME_TIME, update);
}

/*
   Drawing area, the draw() procedure is called once every frame.
 */
class Drawing : public Fl_Widget {
    void draw() {
        img->draw(0, 0);
        fl_push_clip(x(),y(),w(),h());
        fl_push_matrix();
        //draw_grid(11, 11, 12);
        //draw_image_hitbox();
        debug_create_squares();
        fl_pop_matrix();
        fl_pop_clip();
    }
    public:
    Drawing(int X,int Y,int W,int H) : Fl_Widget(X,Y,W,H) {}
};

/*
   Entry point.
 */
int main(int argc, char** argv) {

    // try to open input file
    img = new Fl_PNG_Image("./test2.png");
    if (img->fail())
    {
        fl_alert("./test.png: %s", strerror(errno));
        exit(1);
    }

    // TODO IN PROGRESS:
    global_img_hitbox = create_image_hitbox(img->array, img->w(), img->h(), 5);

    // create window and drawing area
    Fl_Double_Window window(img->w(), img->h());
    Drawing drawing(0, 0, img->w(), img->h());
    //Fl_Box *box = new Fl_Box(0, 0, img->w(), img->h());
    window.end();
    drawing.image(img);
    fl_alert("W:H = %d:%d; Pixel size = %d bytes;", img->w(), img->h(), img->d()); 

    // show window
    window.show(argc, argv);

    // schedule a periodic update after a delay of `FRAME_TIME` seconds
    Fl::add_timeout(FRAME_TIME, update);

    return Fl::run();
}
