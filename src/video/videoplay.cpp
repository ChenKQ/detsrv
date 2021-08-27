#include "rtspcapture.h"
#include <opencv2/opencv.hpp>
#include <string>
#include <memory>


int main(int argc, char* argv[])
{
    std::string rtspuri = "rtsp://172.20.10.9:8554/live/test2";

    detsvr::RtspCapture cap(8);
    if(!cap.open(rtspuri))
    {
        return -1;
    }

    // cap.run();
    cap.play();

    cv::namedWindow("CSI Camera", cv::WINDOW_AUTOSIZE);
    cv::Mat img;

    std::cout << "Hit ESC to exit" << "\n" ;
    int count = 0;

    while(true)
    {
    	if (!cap.read(img)) 
        {
            // std::cout<<"Capture read error"<<std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
            // break;
	    }

        cv::imshow("CSI Camera",img);
        int keycode = cv::waitKey(1) & 0xff ; 
        if (keycode == 27) break ;
    }

    return 0;
}