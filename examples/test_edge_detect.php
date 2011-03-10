<?php
use OpenCV\Image as Image;
use OpenCV\Histogram as Histogram;

$i = Image::load("test.jpg", Image::LOAD_IMAGE_COLOR);
$dst = $i->sobel(1, 0, 1);
$dst->save("test_sobel.jpg");

$dst2 = $i->canny(10, 50, 3);
$dst2->save("test_canny.jpg");
