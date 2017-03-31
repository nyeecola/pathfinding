#ifndef CAMERA
#define CAMERA

class Camera
{
public:
    int x, y;
    int speed;

    Camera(int x, int y, int speed)
    {
        this->x = x;
        this->y = y;
        this->speed = speed;
    }
};

#endif
