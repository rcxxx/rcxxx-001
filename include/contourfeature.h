
#ifndef CONTOURFEATURE_H
#define CONTOURFEATURE_H

#include "configure.h"

bool Test_Armored_Color(Mat color_roi,const int Armor_Color)
{
    Mat hsv_roi;
    cvtColor(color_roi,hsv_roi,COLOR_BGR2HSV);
    bool is_color = 0;
    double H=0.0,S=0.0,V=0.0;
    int x,y;
    int flag = 0;
<<<<<<< HEAD
    for(x = color_roi.cols/4;x < color_roi.cols/4+color_roi.cols/2; ++x)
=======
    for(x = 0;x < color_roi.cols; ++x)
>>>>>>> c482c2fd3aba5f5e7b8923883a89e597911d1523
    {
        for(y = color_roi.rows/4;y < color_roi.rows/4+color_roi.rows/2; ++y)
        {
            H = hsv_roi.at<Vec3b>(y,x)[0];
            S = hsv_roi.at<Vec3b>(y,x)[1];
            V = hsv_roi.at<Vec3b>(y,x)[2];
            //red
            if(Armor_Color == 0)
            {
<<<<<<< HEAD
                if((H>=145 && H<180)||(H>=0 && H<=13))
                {   if(S >= 135 && S <= 255)
                    {   if(V > 148 && V <= 255)
=======
                if((H>=145 && H<180)||(H>=0 && H<=30))
                {   if(S >= 150 && S <= 255)
                    {   if(V > 80 && V <= 255)
>>>>>>> c482c2fd3aba5f5e7b8923883a89e597911d1523
                        {
                            flag += 1;
                        }
                    }
                }
            }
            else
            {   //blue
<<<<<<< HEAD
                if(H>=100 && H<=155)
                {   if(S >= 215 && S <= 255)
                    {   if(V >= 205 && V <= 255)
=======
                if(H>=100 && H<124)
                {   if(S >= 200-20 && S <= 255)
                    {   if(V > 200-50 && V <= 255)
>>>>>>> c482c2fd3aba5f5e7b8923883a89e597911d1523
                        {
                            flag += 1;
                        }
                    }
                }
            }
            if((flag / color_roi.cols*color_roi.rows) > 0.5)
            {
                is_color = 1;
                continue;
            }
        }
    }
    return is_color;
}

float CenterDistance(Point p1,Point p2)
{
    float D = sqrt(pow(p1.x-p2.x,2)+pow(p1.y-p2.y,2));
    return D;
}

int Light_State(const RotatedRect &rect)
{
    int w;
    int h;
    if (rect.size.height > rect.size.width)
    {
        h = rect.size.height;
        w = rect.size.width;
    }
    else
    {
        w = rect.size.height;
        h = rect.size.width;
    }

    float ratio = (float)w/(float)h;
    if(ratio <= 0.2)
    {
        return 1;
    }
    else if (ratio > 0.2 && ratio <= 0.25)
    {
        return 2;
    }
    else if (ratio > 0.25 && ratio <= 0.3)
    {
        return 3;
    }
    else if (ratio > 0.3 && ratio <= 0.4)
    {
        return 4;
    }
    else
    {
        return 5;
    }
}

void getROI(Mat src,const RotatedRect &rect, Mat roi)
{
    Point2f verices[4];
    Point2f verdst[4];
    int roi_w;
    int roi_h;
    rect.points(verices);
    if(rect.size.width > rect.size.height)
    {
        roi_w = rect.size.height;
        roi_h = rect.size.width;
        verdst[0] = Point2f(0,roi_h);
        verdst[1] = Point2f(0,0);
        verdst[2] = Point2f(roi_w,0);
        verdst[3] = Point2f(roi_w,roi_h);
    }
    else
    {
        roi_w = rect.size.width;
        roi_h = rect.size.height;
        verdst[0] = Point2f(roi_w,roi_h);
        verdst[1] = Point2f(0,roi_h);
        verdst[2] = Point2f(0,0);
        verdst[3] = Point2f(roi_w,0);
    }

    roi = Mat(roi_h,roi_w,CV_8UC1);
    Mat warpMatrix = getPerspectiveTransform(verices,verdst);
    warpPerspective(src,roi,warpMatrix,roi.size(),INTER_LINEAR, BORDER_CONSTANT);
}
#endif // CONTOURFEATURE_H
