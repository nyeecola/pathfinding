#ifndef MAP
#define MAP

// TODO: create a constants file
#define SPRITE_SIZE 15

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

    void create_image_hitbox()
    {
        this->hitbox = (MapHitbox *) malloc(sizeof(MapHitbox));
        this->hitbox->unit_size = SPRITE_SIZE;
        this->hitbox->width = ceil((float) this->img->w() / this->hitbox->unit_size);
        this->hitbox->height = ceil((float) this->img->h() / this->hitbox->unit_size);
        this->hitbox->data = (unsigned char *) malloc(3 * this->hitbox->width * this->hitbox->height);

        for (int i = 0; i < this->hitbox->width; i++)
        {
            for (int j = 0; j < this->hitbox->height; j++)
            {
                int counter = 0;
                for (int k = this->hitbox->unit_size * i * 3;
                     k < (i + 1) * this->hitbox->unit_size * 3 && k < this->img->w() * 3;
                     k+=3)
                {
                    for (int w = this->hitbox->unit_size * j;
                         w < (j + 1) * this->hitbox->unit_size && w < this->img->h();
                         w++)
                    {
                        if (this->img->array[w * img->w() * 3 + k] < 20 &&
                            this->img->array[w * img->w() * 3 + k + 1] < 20 &&
                            this->img->array[w * this->img->w() * 3 + k + 2] < 20) counter++;
                    }
                }
                if ((float) counter / pow(this->hitbox->unit_size, 2) > 0.15)
                {
                    this->hitbox->data[i + j * this->hitbox->width] = 1;
                }
                else this->hitbox->data[i + j * this->hitbox->width] = 0;
            }
        }
    }

    Map(Fl_PNG_Image *img)
    {
        this->img = img;
        this->create_image_hitbox();
    }
};

#endif
