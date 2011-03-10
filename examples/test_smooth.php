<?php
use OpenCV\Image as Image;
use OpenCV\Histogram as Histogram;

$i = Image::load("test.jpg", Image::LOAD_IMAGE_COLOR);
$dst = $i->smooth(Image::GAUSSIAN, 31, 0, 0, 0);
$dst->save("test_smoothed.jpg");
