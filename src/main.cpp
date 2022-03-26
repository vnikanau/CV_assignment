#include "process.h"

int main(int argc, char** argv)
{
    Mat image;
    
    VideoCapture cap(0);

    if (!cap.isOpened())
    {
        cout << "cannot open camera";
        return;
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

        waitKey(1);
    }

  waitKey();
  return 0;
}

