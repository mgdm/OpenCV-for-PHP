<?php
$i = OpenCV\Image::load("test.jpg", OpenCV\Image::LOAD_IMAGE_COLOR);
$hsv = $i->convertColor(OpenCV\Image::RGB2HSV);
$hsv->save("hsv.jpg");
