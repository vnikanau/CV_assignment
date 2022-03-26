#include "process.h"
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

// Standard C++ I/O library.
#include <iostream>
#include <vector>
#include <random>
#include <queue>
#include <chrono>
#include <thread>
#include <mutex>

uniform_int_distribution<rng_type::result_type> udist(500, 1500);

processorBase::processorBase()
{
    thread procThread( &processorBase::processStream, this );
    procThread.detach();
}

void processorBase::processStream()
{
    while(1)
    {
        if( !processImage() )
        {
            this_thread::sleep_for(chrono::milliseconds(5));
        }
    }
}
    
Mat& processorBase::activeImage()
{
    const std::scoped_lock<std::mutex> lock(this->m_out);
    return currentImageOut;
}

void processorW1::show()
{
    if( !showCurrent && currProcessed )
    {
        Mat im4show;
        const std::scoped_lock<std::mutex> lock(this->m_out);
        cvtColor( currentImageOut, im4show, COLOR_BGR2GRAY );
        resize( im4show, im4show, cv::Size(), 0.5, 0.5 );
        imshow( "Window 1", im4show );
        showCurrent = true;
    }
}
    
void processorW1::addImage( Mat image )
{
    if( !skipSecond )
    {
        const std::scoped_lock<std::mutex> lock(this->m_in);
        frameBuffer.push( std::move( image ) );
        if( frameBuffer.size() > maxBufferSize )
        {
            frameBuffer.pop();
        }
    }
    skipSecond = !skipSecond;
}
    
bool processorW1::processImage()
{
    if( !frameBuffer.empty() )
    {
        const std::scoped_lock<std::mutex> lock_out(this->m_out);
        currProcessed = false;
        
        rotationCount++;
        {
            const std::scoped_lock<std::mutex> lock_in(this->m_in);
            currentImageOut = std::move( frameBuffer.front() );
            frameBuffer.pop();
        }
        
        if( rotationCount > 5 )
        {
            rotationCount = 0;
        
            if( rotationAngle == ROTATE_90_COUNTERCLOCKWISE )
            {
                rotationAngle = -1;
            }
            else
            {
                rotationAngle++;
            }
        }
    
        if( rotationAngle >= ROTATE_90_CLOCKWISE )
        {
            cv::rotate( currentImageOut, currentImageOut, rotationAngle );
        }
        
        currProcessed = true;
        showCurrent = false;
        return true;
    }
    return false;
}

void processorW2::addImage( Mat image )
{
    std::unique_lock<std::mutex> mlock( this->m_in, std::try_to_lock );
    if( mlock )
    {
        currentImageIn = image.clone();
        currProcessed = false;
        //cout << "w2: added" << endl;
    }
    else
    {
        //cout << "skip" << endl;
    }
}
    
bool processorW2::processImage()
{
    const std::scoped_lock<std::mutex> lock( this->m_in );
    
    if( currentImageIn.empty() || currProcessed )
    {
        return false;
    }
    
    //cout << "begin process w2" << endl;
    
    auto t1 = chrono::high_resolution_clock::now();
    
    auto h = currentImageIn.rows;
    auto w = currentImageIn.cols * .5f;
    Mat left = currentImageIn( Range( 0, w ), Range( 0, h ) );
    
    Mat fullImageHSV;
    cvtColor( left, fullImageHSV, COLOR_BGR2HSV );
    
    Mat hsv[4];
    split( fullImageHSV, hsv );
    auto intens = hsv[2];
    auto type = intens.type();
    
    auto val = mean( intens )[0];
    
    // call long time processing function
    LogitechFrameProcessingMagic( currentImageIn );
    
    {
        const std::scoped_lock<std::mutex> lock( this->m_out );
        if( val > 128 )
        {
            // cout << "flip - val = " << val << endl;
            flip( currentImageIn, currentImageOut, 1 );
        }
        else
        {
            currentImageOut = move( currentImageIn );
            // cout << "no flip - val = " << val << endl;
        }
    }
    
    auto t2 = chrono::high_resolution_clock::now();

    auto duration = chrono::duration_cast<chrono::microseconds>( t2 - t1 ).count();
    auto delta = 1000000 - duration;
    
    if( delta > 0 )
    {
        this_thread::sleep_for(chrono::microseconds(delta));
    }
    
    auto t3 = chrono::high_resolution_clock::now();

    cout << "w2 proc. time: " << chrono::duration_cast<chrono::milliseconds>( t3 - t1 ).count() << endl;
    
    currProcessed = true;
    showCurrent = false;
    
    //cout << "end process w2" << endl;
    
    return true;
}
    
void processorW2::show()
{
    if( !showCurrent && !currentImageOut.empty() )
    {
        Mat im4show;
        const std::scoped_lock<std::mutex> lock(this->m_out);
        resize( currentImageOut, im4show, cv::Size(), 0.5, 0.5 );
        imshow( "Window 2", im4show );
        showCurrent = true;
    }
}
    
void processorW2::LogitechFrameProcessingMagic( cv::Mat image )
{
    random_device rd;
    rng_type::result_type const seedval = rd();
    rng.seed(seedval);
    rng_type::result_type random_number = udist(rng);
      
    this_thread::sleep_for(chrono::milliseconds(random_number));

    //cout << "random_number: " << random_number << endl;
}
