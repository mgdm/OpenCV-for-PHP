<?php
use OpenCV\Image as Image;
use OpenCV\Histogram as Histogram;

echo "Loading sample\n";
$im = Image::load("elephpant_sample.jpg", Image::LOAD_IMAGE_COLOR);
$im2 = Image::load("dragonbe_elephpants.jpg", Image::LOAD_IMAGE_COLOR);

for ($i = 0; $i < 6; $i++) {
	$result = $im2->matchTemplate($im, $i);
	$result->save("mt_output_$i.jpg");
}
