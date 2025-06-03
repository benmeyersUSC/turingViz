#pragma once

#include "helper.h"
#include "graphics/graphics.h"

class Drawable{
    private:
    public:
        virtual void draw(graphics::Window&);
};