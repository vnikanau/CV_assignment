#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

// Standard C++ I/O library.
#include <iostream>
#include <random>
#include <queue>
#include <thread>
#include <mutex>

typedef std::mt19937 rng_type;
typedef decltype(std::chrono::high_resolution_clock::now()) time_type;

class processorBase
{
protected:
    cv::Mat     _currentImageOut;
    bool        _currProcessed{false};
    bool        _showCurrent{false};
    std::mutex  _m_in;
    std::mutex  _m_out;
    float       _viewScale{1.f};
    std::shared_ptr<std::thread> _procThread;
    bool        _killProcessor{false};

protected:
    virtual bool processImage( time_type ) = 0;
    void processStream();
    
public:
    processorBase();
    virtual void show() = 0;
    virtual void addImage( cv::Mat image ) = 0;
    cv::Mat& activeImage();
    void stop();
};

class processorW1 : public processorBase
{
private:
    std::queue <cv::Mat> _frameBuffer;
    bool                 _skipSecond {false};
    unsigned int         _maxBufferSize{10};
    int                  _rotationAngle{-1};
    unsigned int         _rotationCount{0};
    const unsigned int   _rotateEvery{5};
    
public:
    processorW1();
    void show() override;
    void addImage( cv::Mat image ) override;
    bool processImage( time_type ) override;
};

class processorW2 : public processorBase
{
private:
    rng_type   _rng;
    cv::Mat    _currentImageIn;
    const int  _minProcessingTime {1000000}; // microseconds
    
private:
    void LogitechFrameProcessingMagic( cv::Mat image );
    
public:
    processorW2();
    void show() override;
    void addImage( cv::Mat image ) override;
    bool processImage( time_type ) override;
};
