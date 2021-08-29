#include "io.h"
#include <opencv2/opencv.hpp>
#include <string>
#include <memory>
#include <thread>


int main(int argc, char* argv[])
{
    std::string rtspuri = "rtsp://192.168.1.7:8554/live/test1";

    detsvr::RtspReader cap;
    if(!cap.open((void*)rtspuri.c_str()))
    {
        return -1;
    }

    // cap.run();
    // cap.play();

    // cv::namedWindow("CSI Camera", cv::WINDOW_AUTOSIZE);
    cv::Mat img;

    std::cout << "Hit ESC to exit" << "\n" ;
    int count = 0;
    int failureCount = 0;

    while(true)
    {
    	if (!cap.read(img)) 
        {
            // std::cout<<"Capture read error"<<std::endl;
            std::cout << "failure count: " << ++failureCount << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
            // break;
	    }
        failureCount = 0;
        std::cout << "get one new frame: " << ++count << "\n";

        // cv::imshow("CSI Camera",img);
        // int keycode = cv::waitKey(1) & 0xff ; 
        // if (keycode == 27) break ;
    }

    return 0;
}