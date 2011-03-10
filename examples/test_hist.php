<?php
use OpenCV\Image as Image;
use OpenCV\Histogram as Histogram;

/* Load the sample image */
$i = Image::load("sample.jpg", Image::LOAD_IMAGE_COLOR);
$hsv = $i->convertColor(Image::RGB2HSV);
$planes = $hsv->split();
$hist = new Histogram(1, 32, CV_HIST_ARRAY);
$hist->calc($planes[0]);

/* Load the target image */
$i2 = Image::load("target.jpg", Image::LOAD_IMAGE_COLOR);
$hsv2 = $i2->convertColor(Image::RGB2HSV);
$planes2 = $hsv2->split();
$result = $planes2[0]->backProject($hist);

/* Dilate the image to make the objects more obvious */
$result = $result->dilate(2);
$result->save("back_project_output.jpg");
