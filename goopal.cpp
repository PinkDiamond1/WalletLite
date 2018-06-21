#include "goopal.h"

Goopal* Goopal::goo = 0;

Goopal::Goopal()
{
}

Goopal::~Goopal()
{
}

Goopal* Goopal::getInstance()
{
    if( goo == NULL)
    {
        goo = new Goopal;
    }
    return goo;
}
