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
    
    while( true )
    {
        cap >> image;

        w1.addImage( image );
        w1.show();
        
        w2.addImage( image );
        w2.show();

        int k = cv::waitKey(1); // Wait for a keystroke in the window
        if( k == 27 ) // esc to exit
        {
            w1.stop();
            w2.stop();
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            return 0;
        }
    }
    return 0;
}

