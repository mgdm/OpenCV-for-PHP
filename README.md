# OpenCV for PHP

This is a PHP extension wrapping the OpenCV library for image processing. It
lets you use the OpenCV library for image recognition and modification tasks.

It requires PHP 5.3, and OpenCV 2.0 or above.

###Installing

#####Ubuntu and Debian based systems using synaptic package manager

OpenCV uses [cmake](https://cmake.org/ "Cmake.org") build system. We also need to install dev dependencies, **gtk > 2.0** and some others listed in the command below.

`sudo apt-get update`

`sudo apt-get install build-essential unzip`

`sudo apt-get install cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev`

`sudo apt-get install python-dev python-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev libjasper-dev libdc1394-22-dev`

####Install OpenCV from source
*You can also pull the latest version from the master branch*

`wget https://github.com/Itseez/opencv/archive/2.4.13.zip`

`unzip opencv-2.4.13.zip`

`cd ./opencv-2.4.13/`

####Run cmake
To keep our source directory clean, we'll run cmake in build directory instead

`mkdir build`

`cd ./build`

`cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/usr/local ..`

`sudo make -j7`

 *install all opencv opencv2 headers and shared libraries*
 
`sudo make install`

####Enable PHP extension
`sudo echo "extension=opencv.so" >> /etc/php5/mods-available/opencv.ini`

`sudo ln -s /etc/php5/mods-available/opencv.ini /etc/php5/cli/conf.d/20-opencv.ini`

`sudo ln -s /etc/php5/mods-available/opencv.ini /etc/php5/apache2/conf.d/20-opencv.ini`

Reload apache if necessary
