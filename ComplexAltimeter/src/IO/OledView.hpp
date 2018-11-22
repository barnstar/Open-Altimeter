#ifndef OLEDVIEW_H
#define OLEDVIEW_H

#include <Arduino.h>

class OledView
{
  public:

    void setText(String text, int line, boolean update);
    void clear();


  private:
    void String[6] lines;
    void update();


};




#endif