#ifndef ANIM_H_
#define ANIM_H_
#include <stdio.h>
#include "LedControl.h"
#include <Arduino.h>

Class Anim{

public:
    Anim(LedControl lc);
    void load(String fileName);
    void play(bool loop);
    void save(String fileName);
    void stop();
    void append();
    void handleRender();


private:

}


#endif // ANIM_H_
