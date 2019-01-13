#ifndef DATABUFF_H
#define DATABUFF_H

#include "configure.h"

bool isCentralBUffer(Mat src,Point point)
{
    bool is = false;
    Point center;
    center.x = src.cols/2;
    center.y = src.rows/2;
    Point left_focus = Point(center.x-20,center.y);
    Point right_focus = Point(center.x+20,center.y);
    float PF1 = CenterDistance(point,left_focus);
    float PF2 = CenterDistance(point,right_focus);
    if((PF1 + PF2) < 160)
    {
        is = true;
    }

    return is;
}

int missingflag(Mat src,int X_Widht)
{
    int flag;
    Point center;
    center.x = src.cols/2;
    center.y = src.rows/2;
    int Xc = center.x;
    //int Yc = center.y;
    int Xp = X_Widht;
    //int Yp = Y_height;

    if(Xp < (Xc - 480/2))
    {
        flag = 2;
    }
    else if (Xc - 480/2 <= Xp && Xp <= Xc + 480/2)
    {
        flag = 1;
    }
    else if (Xp > (Xc + 480/2))
    {
        flag = 3;
    }
    return flag;
}

#endif // DATABUFF_H
