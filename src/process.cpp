#include "process.h"

// Standard C++ I/O library.
#include <chrono>

std::uniform_int_distribution<rng_type::result_type> udist(500, 1500);

processorBase::processorBase()
{
    _procThread = std::make_shared<std::thread>( &processorBase::processStream, this );
    _procThread->detach();
}

void processorBase::stop()
{
    _killProcessor = true;
}

void processorBase::processStream()
{
    auto t0 = std::chrono::high_resolution_clock::now();
    while( !_killProcessor )
    {
        if( !processImage( t0 ) )
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        else
        {
            t0 = std::chrono::high_resolution_clock::now();
        }
    }
}
    
cv::Mat& processorBase::activeImage()
{
    const std::scoped_lock<std::mutex> lock(this->_m_out);
    return _currentImageOut;
}

processorW1::processorW1()
{
    _viewScale = .6f;
}

void processorW1::show()
{
    if( !_showCurrent && _currProcessed )
    {
        cv::Mat im4show;
        const std::scoped_lock<std::mutex> lock(this->_m_out);
        cvtColor( _currentImageOut, im4show, cv::COLOR_BGR2GRAY );
        resize( im4show, im4show, cv::Size(), _viewScale, _viewScale );
        imshow( "Window 1", im4show );
        _showCurrent = true;
    }
}
    
void processorW1::addImage( cv::Mat image )
{
    if( !_skipSecond )
    {
        const std::scoped_lock<std::mutex> lock( this->_m_in );
        _frameBuffer.push( std::move( image ) );
        if( _frameBuffer.size() > _maxBufferSize )
        {
            _frameBuffer.pop();
        }
    }
    _skipSecond = !_skipSecond;
}
    
bool processorW1::processImage( time_type t0 )
{
    if( !_frameBuffer.empty() )
    {
        const std::scoped_lock<std::mutex> lock_out(this->_m_out);
        _currProcessed = false;
        
        _rotationCount++;
        {
            const std::scoped_lock<std::mutex> lock_in(this->_m_in);
            _currentImageOut = std::move( _frameBuffer.front() );
            _frameBuffer.pop();
        }
        
        if( _rotationCount > _rotateEvery )
        {
            _rotationCount = 0;
        
            if( _rotationAngle == cv::ROTATE_90_COUNTERCLOCKWISE )
            {
                _rotationAngle = -1;
            }
            else
            {
                _rotationAngle++;
            }
        }
    
        if( _rotationAngle >= cv::ROTATE_90_CLOCKWISE )
        {
            cv::rotate( _currentImageOut, _currentImageOut, _rotationAngle );
        }
        
        _currProcessed = true;
        _showCurrent = false;
        return true;
    }
    return false;
}

processorW2::processorW2()
{
    _viewScale = .8f;
}

void processorW2::addImage( cv::Mat image )
{
    std::unique_lock<std::mutex> mlock( this->_m_in, std::try_to_lock );
    if( mlock )
    {
        _currentImageIn = image.clone();
        _currProcessed = false;
        //cout << "w2: added" << endl;
    }
    else
    {
        //cout << "skip" << endl;
    }
}
    
bool processorW2::processImage( time_type t0 )
{
    const std::scoped_lock<std::mutex> lock( this->_m_in );
    
    if( _currentImageIn.empty() || _currProcessed )
    {
        return false;
    }
    
    //cout << "begin process w2" << endl;
    
    // call long time processing function
    LogitechFrameProcessingMagic( _currentImageIn );
    
    cv::Mat rgb[4];
    cv::split( _currentImageIn, rgb );
    auto intens = ( rgb[0] + rgb[1] + rgb[2] ) / 3;
    double intens_min, intens_max;
    cv::minMaxLoc( intens, &intens_min, &intens_max );
    
    double intens_treshold = 0.5 * ( intens_min + intens_max );
    
    auto h = _currentImageIn.rows;
    auto w = _currentImageIn.cols * .5f;
    cv::Mat left = _currentImageIn( cv::Range( 0, w ), cv::Range( 0, h ) );
    
    cv::Mat rgb_left[4];
    cv::split( left, rgb_left );
    
    auto intens_left = ( rgb_left[0] + rgb_left[1] + rgb_left[2] ) / 3;
    auto intens_left_mean = cv::mean( intens_left )[0];
    
    {
        const std::scoped_lock<std::mutex> lock( this->_m_out );
        if( intens_left_mean > intens_treshold )
        {
            std::cout << "flip - left intens = " << intens_left_mean << std::endl;
            flip( _currentImageIn, _currentImageOut, 1 );
        }
        else
        {
            _currentImageOut = std::move( _currentImageIn );
            // cout << "no flip - val = " << val << endl;
        }
    }
    
    auto t_proc = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t_proc - t0 ).count();
    auto delta = _minProcessingTime - duration;
    
    if( delta > 0 )
    {
        std::this_thread::sleep_for(std::chrono::microseconds(delta));
    }
    
    auto t_wait = std::chrono::high_resolution_clock::now();

    std::cout << "w2 proc. time: " << std::chrono::duration_cast<std::chrono::milliseconds>( t_wait - t0 ).count() << std::endl;
    
    _currProcessed = true;
    _showCurrent = false;
    
    //cout << "end process w2" << endl;
    
    return true;
}
    
void processorW2::show()
{
    if( !_showCurrent && !_currentImageOut.empty() )
    {
        cv::Mat im4show;
        const std::scoped_lock<std::mutex> lock(this->_m_out);
        resize( _currentImageOut, im4show, cv::Size(), _viewScale, _viewScale );
        imshow( "Window 2", im4show );
        _showCurrent = true;
    }
}
    
void processorW2::LogitechFrameProcessingMagic( cv::Mat image )
{
    std::random_device rd;
    rng_type::result_type const seedval = rd();
    _rng.seed(seedval);
    rng_type::result_type random_number = udist(_rng);
      
    std::this_thread::sleep_for(std::chrono::milliseconds(random_number));

    //cout << "random_number: " << random_number << endl;
}
