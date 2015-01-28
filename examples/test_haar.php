<?php
use OpenCV\Image as Image;
use OpenCV\Histogram as Histogram;

$i = Image::load("sailing.jpg", Image::LOAD_IMAGE_COLOR);
$result = $i->haarDetectObjects("/usr/share/opencv/haarcascades/haarcascade_frontalface_default.xml");

foreach ($result as $r) {
	$i->rectangle($r['x'], $r['y'], $r['width'], $r['height']);
}

$i->save("haar_output.jpg");
