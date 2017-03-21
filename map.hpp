#ifndef MAP
#define MAP

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

#endif