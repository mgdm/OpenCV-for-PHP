<?php
$capture = OpenCV\Capture::createFileCapture('movie.avi');
$image = $capture->queryFrame();
$result = $image->haarDetectObjects("data/haarcascades/haarcascade_frontalface_default.xml");
foreach ($result as $r) {
	$image->rectangle($r['x'], $r['y'], $r['width'], $r['height']);
}
$image->save('/tmp/video.jpg');
