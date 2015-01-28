<?php
use OpenCV\Capture as Capture;
/* Test the face detectoin feature, using a capture from the camera */
$capture = Capture::createCameraCapture(0);
$image = $capture->queryFrame();
$result = $image->haarDetectObjects("/usr/share/opencv/haarcascades/haarcascade_frontalface_default.xml");
foreach ($result as $r) {
	$image->rectangle($r['x'], $r['y'], $r['width'], $r['height']);
}
//$image = $image->convertColor(7);
$image->save('/tmp/camera.jpg');
