#include "cameraconfigure.h"
#include "configure.h"
#include "contourfeature.h"
#include "myserial.h"
#include "matchandgroup.h"
#include "databuff.h"

using namespace cv;
using namespace std;

int threshold_Value,Armor_olor;
int t1, t2, t3, FPS;
float RunTime;     //用于测试帧率
Mat src_img;    //原图
Mat gray_img;   //灰度图
Mat bin_img;    //二值图
Mat dst_img;    //输出图

int main()
{
    cameraconfigure camera;
    /*----------调用相机----------*/
    camera.CameraSet();
    /*----------调用相机----------*/

    /*----------串口部分----------*/
    if(serialisopen == 1)
    {
        serialSet();//串口初始化函数
    }
    /*----------串口部分----------*/

    /*----------参数初始化----------*/
    if(armor_color == 0)
    {
        threshold_Value = 20;
        Armor_olor = 0;
    }
    else
    {
        threshold_Value = 10;
        Armor_olor = 1;
    }
    int SendBuf_COUNT = 0;    //ifSendSuccess
    /*----------参数初始化----------*/
    //----------识别部分----------
    for(;;)
    {
        t1 = getTickCount();
        if(CameraGetImageBuffer(camera.hCamera,&camera.sFrameInfo,&camera.pbyBuffer,1000) == CAMERA_STATUS_SUCCESS)
        {
            //----------读取原图----------//
            CameraImageProcess(camera.hCamera, camera.pbyBuffer, camera.g_pRgbBuffer,&camera.sFrameInfo);
            if (camera.iplImage)
            {
                cvReleaseImageHeader(&camera.iplImage);
            }
            camera.iplImage = cvCreateImageHeader(cvSize(camera.sFrameInfo.iWidth,camera.sFrameInfo.iHeight),IPL_DEPTH_8U,camera.channel);
            cvSetData(camera.iplImage,camera.g_pRgbBuffer,camera.sFrameInfo.iWidth*camera.channel);//此处只是设置指针，无图像块数据拷贝，不需担心转换效率
            src_img = cvarrToMat(camera.iplImage,true);//这里只是进行指针转换，将IplImage转换成Mat类型
            src_img.copyTo(dst_img);

            //--------------色彩分割	-----------------//
            cvtColor(src_img, gray_img, COLOR_BGR2GRAY);
            threshold(gray_img, bin_img, threshold_Value, 255, THRESH_BINARY);
            medianBlur(bin_img, bin_img,5);
            Canny(bin_img,bin_img,120,240);

            vector<vector<Point>> contours;
            vector<Rect> boundRect;
            vector<RotatedRect> rotateRect;
            vector<Vec4i> hierarchy;
            vector<Point2f> midPoint(2);
            vector<vector<Point2f>> midPoint_pair;

            //查找轮廓
            findContours(bin_img, contours, hierarchy, RETR_LIST, CHAIN_APPROX_NONE, Point(0,0));

            //第一遍过滤
            for (int i = 0; i < (int)contours.size(); ++i)
            {
                if (contours.size() <= 1)
                    break;
                Rect B_rect_i = boundingRect(contours[i]);
                RotatedRect R_rect_i = minAreaRect(contours[i]);
                float ratio = (float)B_rect_i.width / (float)B_rect_i.height;
                bool H_W;
                if(B_rect_i.height >= B_rect_i.width)
                {
                    switch (Light_State(R_rect_i))
                    {
                    case 1:
                        if (ratio < 0.7)
                            H_W = true;
                        else
                            H_W = false;
                        break;
                    case 2:
                        if (ratio < 0.8)
                            H_W = true;
                        else
                            H_W = false;
                        break;
                    case 3:
                        if (ratio < 0.9)
                            H_W = true;
                        else
                            H_W = false;
                        break;
                    case 4:
                        if (ratio < 1)
                            H_W = true;
                        else
                            H_W = false;
                        break;
                    default:
                        break;
                    }
                }
                if (H_W)
                {
                    boundRect.push_back(B_rect_i);
                    rotateRect.push_back(R_rect_i);
                }
            }
            float distance_max = 0.f;
            float slope_min = 10.0;
            float ratio_maxW_distance_min = 0.f;
            //第二遍两个循环匹配灯条
            for (int k1 = 0;k1<(int)rotateRect.size();++k1)
            {
                if(rotateRect.size()<=1)
                    break;
                for (int k2 = k1+1;k2<(int)rotateRect.size();++k2)
                {
                    if(Light_filter(rotateRect[k1],rotateRect[k2]))
                    {
                        if(Rect_different(rotateRect[k1],rotateRect[k2]))
                        {
                            float distance_temp = CenterDistance(rotateRect[k1].center,rotateRect[k2].center);
                            float slope_temp = fabs((rotateRect[k1].center.y-rotateRect[k2].center.y)/(rotateRect[k1].center.x-rotateRect[k2].center.x));
                            float ratio_maxW_distance_temp = max(rotateRect[k1].size.width,rotateRect[k2].size.width) / distance_temp;
                            if (Distance_Height(rotateRect[k1],rotateRect[k2]))
                            {
                                //ROI_1
                                Point2f verices_1[4];
                                Point2f verdst_1[4];
                                int roi_w1;
                                int roi_h1;
                                if (rotateRect[k1].size.width > rotateRect[k1].size.height)
                                {
                                    rotateRect[k1].points(verices_1);
                                    roi_w1 = rotateRect[k1].size.width;
                                    roi_h1 = rotateRect[k1].size.height;
                                    verdst_1[0] = Point2f(0,roi_h1);
                                    verdst_1[1] = Point2f(0,0);
                                    verdst_1[2] = Point2f(roi_w1,0);
                                    verdst_1[3] = Point2f(roi_w1,roi_h1);
                                }
                                else
                                {
                                    rotateRect[k1].points(verices_1);
                                    roi_w1 = rotateRect[k1].size.height;
                                    roi_h1 = rotateRect[k1].size.width;
                                    verdst_1[0] = Point2f(roi_w1,roi_h1);
                                    verdst_1[1] = Point2f(0,roi_h1);
                                    verdst_1[2] = Point2f(0,0);
                                    verdst_1[3] = Point2f(roi_w1,0);
                                }
                                //ROI_2
                                Point2f verices_2[4];
                                Point2f verdst_2[4];
                                int roi_w2;
                                int roi_h2;
                                if (rotateRect[k2].size.width > rotateRect[k2].size.height)
                                {
                                    rotateRect[k2].points(verices_2);
                                    roi_w2 = rotateRect[k2].size.width;
                                    roi_h2 = rotateRect[k2].size.height;
                                    verdst_2[0] = Point2f(0,roi_h2);
                                    verdst_2[1] = Point2f(0,0);
                                    verdst_2[2] = Point2f(roi_w2,0);
                                    verdst_2[3] = Point2f(roi_w2,roi_h2);
                                }
                                else
                                {
                                    rotateRect[k2].points(verices_2);
                                    roi_w2 = rotateRect[k2].size.width;
                                    roi_h2 = rotateRect[k2].size.height;
                                    verdst_2[0] = Point2f(roi_w2,roi_h2);
                                    verdst_2[1] = Point2f(0,roi_h2);
                                    verdst_2[2] = Point2f(0,0);
                                    verdst_2[3] = Point2f(roi_w2,0);
                                }
                                Mat roi_1 = Mat(roi_h1,roi_w1,CV_8UC1);
                                Mat warpMatrix1 = getPerspectiveTransform(verices_1,verdst_1);
                                warpPerspective(dst_img,roi_1,warpMatrix1,roi_1.size(),INTER_LINEAR, BORDER_CONSTANT);
                                Mat roi_2 = Mat(roi_h2,roi_w2,CV_8UC1);
                                Mat warpMatrix2 = getPerspectiveTransform(verices_2,verdst_2);
                                warpPerspective(dst_img,roi_2,warpMatrix2,roi_2.size(),INTER_LINEAR, BORDER_CONSTANT);
                                if(Test_Armored_Color(roi_1,Armor_olor)==1)
                                {
                                    if(Test_Armored_Color(roi_2,Armor_olor)==1)
                                    {
                                        if(distance_temp >= distance_max)
                                        {
                                            distance_max = distance_temp;
                                        }
                                        if(slope_temp <=slope_min )
                                        {
                                            slope_min = slope_temp;
                                        }
                                        if(ratio_maxW_distance_temp <= ratio_maxW_distance_min )
                                        {
                                            ratio_maxW_distance_min = ratio_maxW_distance_temp;
                                        }

                                        //imshow("roi",roi_1);
                                        rectangle(dst_img,boundRect[k1].tl(), boundRect[k1].br(), Scalar(0,255,0),2,8,0);
                                        //imshow("roi2",roi_2);
                                        rectangle(dst_img,boundRect[k2].tl(), boundRect[k2].br(), Scalar(0,255,0),2,8,0);
                                        midPoint[0].x = rotateRect[k1].center.x;
                                        midPoint[0].y = rotateRect[k1].center.y;
                                        midPoint[1].x = rotateRect[k2].center.x;
                                        midPoint[1].y = rotateRect[k2].center.y;
                                        midPoint_pair.push_back(midPoint);
                                     }
                                }
                            }
                        }
                    }
                }
            }
            int RecoginitionSuccess_FLAG = 0;    //ifRecoginitionSuccess
            int X_Widht;
            int Y_height;

            //第三遍求最优灯条
            for (int k3 = 0;k3<(int)midPoint_pair.size();++k3)
            {
                float distance = CenterDistance(midPoint_pair[k3][0],midPoint_pair[k3][1]);
                float slope = fabs((midPoint_pair[k3][0].y-midPoint_pair[k3][1].y)/(midPoint_pair[k3][0].x-midPoint_pair[k3][1].x));
                if(distance >= distance_max)//|| slope <= slope_min)
                {
                    if(slope <= 0.26)
                    {
                        line(dst_img,midPoint_pair[k3][0],midPoint_pair[k3][1],Scalar(0,0,255),2,8);
                        int x1 = midPoint_pair[k3][0].x;
                        int y1 = midPoint_pair[k3][0].y;
                        int x2 = midPoint_pair[k3][1].x;
                        int y2 = midPoint_pair[k3][1].y;
                        Point mid_point = Point(int((x1 + x2)/2), int((y1 + y2)/2));
                        //cout<<"x:"<<mid_point.x<<"   y:"<<mid_point.y;
                        X_Widht = mid_point.x;
                        Y_height = mid_point.y;
                        RecoginitionSuccess_FLAG = 2;
                        if(isCentralBUffer(src_img,mid_point))
                        {
                            RecoginitionSuccess_FLAG = 1;
                        }
                        cout<<"X"<<src_img.cols/2<<"  "<<"Y"<<src_img.rows/2<<endl;                        
                        t2 = getTickCount();
                        RunTime = (t2-t1)/getTickFrequency();
                        FPS = 1 / RunTime;
                        //cout<<"time:"<<RunTime<<endl;
                        cout<<"FPS:"<<FPS<<endl;
                        break;
                    }
                }
            }
            if(serialisopen == 1)
            {
                switch (RecoginitionSuccess_FLAG)
                {
                 case 0:
                {
                    if(SendBuf_COUNT < 3)
                    {
                        SendBuf_COUNT += 1;
                        sendData(X_Widht,Y_height,0);
                        cout<<"send buf_temp"<<endl;
                    }
                    else
                    {
                        int leftorright = missingflag(src_img,X_Widht);
                        switch(leftorright)
                        {
                        case 1:
                        {
                            sendData(X_Widht,Y_height,1);
                            cout<<"send None"<<endl;
                        }
                            break;
                        case 2:
                        {
                            sendData(X_Widht,Y_height,2);
                            cout<<"send None"<<endl<<"missing left"<<endl;
                        }
                            break;
                        case 3:
                        {
                            sendData(X_Widht,Y_height,3);
                            cout<<"send None"<<endl<<"missing right"<<endl;
                        }
                            break;
                        default:
                            break;
                        }
                    }
                }
                    break;
                case 1:
                {
                    SendBuf_COUNT = 0;
                    int X = src_img.cols/2;
                    int Y = src_img.rows/2;
                    sendData(X,Y,0);
                    sprintf(buf_temp,"%s%03d%s%03d","S",X_Widht,",",Y_height);
                    cout<<"send center"<<endl;
                }
                    break;
                case 2:
                {
                    SendBuf_COUNT = 0;
                    sendData(X_Widht,Y_height,0);
                    buf_temp = buf;
                    cout<<"send success"<<endl;
                }
                    break;
                default:
                    break;
                }
            }
            imshow("th",bin_img);
            imshow("input" ,src_img);
            imshow("output",dst_img);
            int key = waitKey(1);
            if(char(key) == 27)
            {
                CameraReleaseImageBuffer(camera.hCamera,camera.pbyBuffer);
                break;
            }
            //在成功调用CameraGetImageBuffer后，必须调用CameraReleaseImageBuffer来释放获得的buffer。
            //否则再次调用CameraGetImageBuffer时，程序将被挂起一直阻塞，直到其他线程中调用CameraReleaseImageBuffer来释放了buffer
            CameraReleaseImageBuffer(camera.hCamera,camera.pbyBuffer);
        }
    }
    CameraUnInit(camera.hCamera);
    //注意，现反初始化后再free
    free(camera.g_pRgbBuffer);
    return 0;
}
