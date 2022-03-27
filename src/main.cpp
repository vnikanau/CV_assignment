#include "process.h"

int main(int argc, char** argv)
{
    cv::Mat image;
    
    cv::VideoCapture cap(0);

    if (!cap.isOpened())
    {
        std::cout << "cannot open camera";
        return 0;
    }
    
    processorW1 w1;
    processorW2 w2;
    
    while (true)
    {
        cap >> image;

        w1.addImage( image );
        w1.show();
        
        w2.addImage( image );
        w2.show();

        cv::waitKey(1);
    }

    cv::waitKey();
  return 0;
}

