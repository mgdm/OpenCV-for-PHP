<?php
/* Split the image by planes */
$i = OpenCV\Image::load("test.jpg", OpenCV\Image::LOAD_IMAGE_COLOR);
$split = $i->split();
$i = 0;
foreach ($split as $plane) {
	$i++;
	$plane->save("split_$i.jpg");
}
