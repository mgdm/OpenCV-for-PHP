<?php

$i = OpenCV\Image::load("test.jpg", OpenCV\Image::LOAD_IMAGE_COLOR);
//var_dump($i);
//$i->setImageROI(200, 200, 200, 200);
//var_dump($i->getImageROI());
//$i->resetImageROI();
//$i2 = $i->smooth(OpenCV\Image::GAUSSIAN, 31, 0, 0, 0);
//$i2 = $i->laplace(3);
//$i2 = $i->sobel(1, 0, 1);
//$i2 = $i->erode(3);
//$i2 = $i->dilate(3);
//$i2 = $i->open(3);
//$i2 = $i->close(3);
//$i2 = $i->gradient(3);
/*
	$i2 = $i->blackHat(2);
var_dump($i2);
try {
	$i2->save("test2.jpg");
} catch (Exception $e) {
	var_dump($e);
}
 */

/*
$dst = new OpenCV\Image(250, 250, OpenCV\Image::DEPTH_8U, 3);
var_dump($dst);
$i->resize($dst);
try {
	$dst->save("test3.jpg");
} catch (Exception $e) {
	var_dump($e);
}
 */

/*
	$dst = $i->pyrUp(OpenCV\Image::GAUSSIAN_5x5);
$dst = $dst->pyrUp(OpenCV\Image::GAUSSIAN_5x5);
$dst->save("pyr.jpg");
 */
$dst = $i->canny(10, 50, 3);
$dst->save('canny.jpg');


//var_dump($i instanceof OpenCV\Mat);
//var_dump($i instanceof OpenCV\Arr);

//$m = new OpenCV\Mat(100, 100, 0);
//var_dump($m);
