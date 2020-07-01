#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/joystick.h>


#include "InputDevice.h"


/**
 * Current state of an axis.
 */
struct axis_state {
    short x, y;
};

#define MAX_STICK 5
#define STICK_MAX_VALUE 65535.0

/**
 * \brief Joystick support class. Mostly stollen from Jason White (https://gist.github.com/jasonwhite/)
 */
class Joystick : public InputDevice
{
    public:
        /** Default constructor */
        Joystick();
        /** Default destructor */
        ~Joystick();

        void updateInput();

        bool isButtonPressed(int id) {return button[id];}
        double getAxis(int axis_id) {
            int stick = axis_id / 2;
            if (axis_id % 2 == 0)
                return axes[stick].x/STICK_MAX_VALUE;
            else
                return axes[stick].y/STICK_MAX_VALUE;
        }

    protected:

    private:
        bool initialised;
        const char *device;
        int js;
        struct js_event event;
        struct axis_state axes[MAX_STICK] = {0};
        size_t axis;
        bool button[100];
};

#endif // JOYSTICK_H
