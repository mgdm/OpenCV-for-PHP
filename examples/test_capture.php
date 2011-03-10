<?php
/* Test the face detectoin feature, using a capture from the camera */
$capture = new OpenCV\Capture(0);
$image = $capture->queryFrame();
$result = $image->haarDetectObjects("data/haarcascades/haarcascade_frontalface_default.xml");
foreach ($result as $r) {
	$image->rectangle($r['x'], $r['y'], $r['width'], $r['height']);
}
$image = $image->convertColor(RGB2GRAY);
$image->save('/tmp/camera.jpg');
