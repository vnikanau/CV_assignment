#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

// Standard C++ I/O library.
#include <iostream>
#include <random>
#include <queue>
#include <thread>
#include <mutex>

using namespace cv;
using namespace std;

typedef mt19937 rng_type;

class processorBase
{
protected:
    Mat currentImageOut;
    bool currProcessed{false};
    bool showCurrent{false};
    std::mutex m_in;
    std::mutex m_out;

protected:
    virtual bool processImage() = 0;
    void processStream();
    
public:
    processorBase();
    virtual void show() = 0;
    virtual void addImage( Mat image ) = 0;
    Mat& activeImage();
};

class processorW1 : public processorBase
{
private:
    queue <Mat> frameBuffer;
    bool skipSecond {false};
    unsigned int maxBufferSize{10};
    int rotationAngle{-1};
    unsigned int rotationCount{0};
    
public:
    void show() override;
    void addImage( Mat image ) override;
    bool processImage() override;
};

class processorW2 : public processorBase
{
private:
    rng_type rng;
    Mat currentImageIn;
    
private:
    void LogitechFrameProcessingMagic( Mat image );
    
public:
    void show() override;
    void addImage( Mat image ) override;
    bool processImage() override;
};
