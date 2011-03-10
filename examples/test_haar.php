<?php
use OpenCV\Image as Image;
use OpenCV\Histogram as Histogram;

$i = Image::load("test.jpg", Image::LOAD_IMAGE_COLOR);
$result = $i->haarDetectObjects("data/haarcascades/haarcascade_frontalface_default.xml");

foreach ($result as $r) {
	$i->rectangle($r['x'], $r['y'], $r['width'], $r['height']);
}

$i->save("haar_output.jpg");
