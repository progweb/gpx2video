# GPX2Video tool - Telemetry overlay

GPX GoPro Quick not working ? No tool working with Linux distribution. That's why I have written 
this small tool to extract telemetry data to enable stickers or gauges on your videos.

GPX2Video can convert the data recorded by your GoPro (GPS, accelerometer, camera settings and more)
or reads an input GPX file. Visualize data or maps on your videos.

Finally, gpx2video should work with any camera and you are able to customize your gauges and much more.

gpx2video is **only a test tool and isn't yet a final release!**

![overview](./assets/overview.png)

*Please test and report issues. Tested with GoPro 5, 6, 7, 8 and 9 (with last firmware).*


## Features

gpx2video can read and extract:
  - time, 
  - position, 
  - elevation, 
  - speed, 
  - average speed, 
  - max speed,
  - heartrate, 
  - cadence
from your gpx input file.

gpx2video can draw a map widget with your track. It supports multi map source: 
  - OpenStreetMap I © OpenStreetMap contributors
  - Maps-For-Free Maps © Maps-For-Free
  - OpenCycleMap Maps © thunderforest.com, Data © osm.org/copyright
  - OpenTopoMap © OpenTopoMap (CC-BY-SA)
  - Public Transport Maps © ÖPNVKarte, Data © OpenStreetMap contributors
  - Google Maps Map provided by Google
  - Google Satellite Map provided by Google 
  - Google Hybrid Map provided by Google
  - Virtual Earth Map provided by Microsoft
  - Virtual Earth Satellite Map provided by Microsoft
  - Virtual Earth Hybrid Map provided by Microsoft

gpx2video can synchronize your video with your gpx input file.


## How it works ?

gpx2video is able to extract and parse metadata and sensor data recorded by your GoPro.

```
$ ffprobe GH010337.MP4
ffprobe version 3.2.2 Copyright (c) 2007-2016 the FFmpeg developers
  built with gcc 6.2.1 (Debian 6.2.1-5) 20161124
...
Input #0, mov,mp4,m4a,3gp,3g2,mj2, from 'GH010337.MP4':
  Metadata:
    major_brand     : mp41
    minor_version   : 538120216
    compatible_brands: mp41
    creation_time   : 2021-12-08T09:56:26.000000Z
...
  Duration: 00:00:52.38, start: 0.000000, bitrate: 100345 kb/s
    Stream #0:0(eng): Video: h264 (High) (avc1 / 0x31637661), yuvj420p(pc, bt709), 2704x1520 [SAR 1:1 DAR 169:95], 100078 kb/s, 50 fps, 50 tbr, 90k tbn, 100 tbc (default)
    Metadata:
      creation_time   : 2021-12-08T09:56:26.000000Z
      handler_name    : GoPro AVC  
      encoder         : GoPro AVC encoder
      timecode        : 09:56:26:43
    Stream #0:1(eng): Audio: aac (LC) (mp4a / 0x6134706D), 48000 Hz, stereo, fltp, 189 kb/s (default)
    Metadata:
      creation_time   : 2021-12-08T09:56:26.000000Z
      handler_name    : GoPro AAC  
      timecode        : 09:56:26:43
    Stream #0:2(eng): Data: none (tmcd / 0x64636D74) (default)
    Metadata:
      creation_time   : 2021-12-08T09:56:26.000000Z
      handler_name    : GoPro TCD  
      timecode        : 09:56:26:43
    Stream #0:3(eng): Data: none (gpmd / 0x646D7067), 48 kb/s (default)
    Metadata:
      creation_time   : 2021-12-08T09:56:26.000000Z
      handler_name    : GoPro MET  
    Stream #0:4(eng): Data: none (fdsc / 0x63736466), 12 kb/s (default)
    Metadata:
      creation_time   : 2021-12-08T09:56:26.000000Z
      handler_name    : GoPro SOS  
```

gpx2video uses the `creation_time` field to synchronize your video with your GPX file. 
But this date isn't synchronized with the GPS source.

If gpx2video finds the 'GoPro MET' stream, it determines the offset time to use.


## Build
```
$ git clone https://github.com/progweb/gpx2video.git
$ mkdir gpx2video/build
$ cd gpx2video/build
$ cmake ..
$ make
$ ln -s ../assets assets
$ ./gpx2video -h
```

*Please execute gpx2video tool from the build path so as it finds assets data.*


## Usage

gpx2video is a command line tool.

```
$ ./gpx2video -m GH020340.MP4 -g ACTIVITY.gpx -o output.mp4 --map-source=1 --map-zoom=11 -f 2.0
gpx2video v0.0.0
creation_time = 2021-12-08T10:34:50.000000Z
...
[read the input media metadata]
...
Track info:
  Name        : Road biking
  Comment     : 
  Description : 
  Source      : 
  Type        : road_biking
  Number      : 
  Segments:   : 1
Output #0, mp4, to 'output-overview.mp4':
  Stream #0:0: Video: h264, yuvj420p(pc), 2704x1520 [SAR 1:1 DAR 169:95], q=2-31, 32000 kb/s, 50 tbn
  Stream #0:1: Audio: aac (LC), 48000 Hz, stereo, fltp, 128 kb/s
Time synchronization...
PACKET: 0 - PTS: 0 - TIMESTAMP: 0 ms - TIME: 2021-12-08 09:34:50 - GPS TIME: 2021-12-08 09:38:35.860 - OFFSET: 225
PACKET: 1 - PTS: 1000 - TIMESTAMP: 1000 ms - TIME: 2021-12-08 09:34:51 - GPS TIME: 2021-12-08 09:38:36.850 - OFFSET: 225
[read the GoPro MET stream]
...
Download map from OpenStreetMap I...
  Download tile 6 / 6 [##################################################] DONE
...
[Download, build map then draw your track]
...
Build map...
FRAME: 0 - PTS: 0 - TIMESTAMP: 0 ms - TIME: 2021-12-08 10:38:35
  Time: 2021-12-08 10:38:38. Distance: 35.841 km in 6330.000 seconds, current speed is 25.817 (valid: true)
FRAME: 1 - PTS: 1800 - TIMESTAMP: 20 ms - TIME: 2021-12-08 10:38:35
  Time: 2021-12-08 10:38:38. Distance: 35.841 km in 6330.000 seconds, current speed is 25.817 (valid: true)
[Process each frame]
...
```


### How change gauges ?

For the moment, all can't be set from the command line or better a config file.

You can edit `src/render.cpp` file to enable/disable gauge or edit label and position:
```
void Renderer::draw(FramePtr frame, const GPXData &data) {
    char s[128];

    int pos = 400;

    OIIO::ImageBuf frame_buffer = frame->toImageBuf();

    // Draw gauges
    sprintf(s, "%.0f%%", data.grade());
    this->add(&frame_buffer, 50, pos, "./assets/picto/DataOverlay_icn_grade.png", "PENTE", s, 2.5 * 64.0 / 150.0);

    sprintf(s, "%.0f m", data.elevation());
    this->add(&frame_buffer, 50, pos + 200, "./assets/picto/DataOverlay_icn_elevation.png", "ALTITUDE", s, 2.5);

    sprintf(s, "%.0f km/h", data.speed());
    this->add(&frame_buffer, 50, pos + 400, "./assets/picto/DataOverlay_icn_speed.png", "VITESSE", s, 2.5);

    sprintf(s, "%d tr/min", data.cadence());
    this->add(&frame_buffer, 50, pos + 600, "./assets/picto/DataOverlay_icn_cadence.png", "CADENCE", s, 2.5);

//  sprintf(s, "%d bpm", data.heartrate());
//  this->add(&frame_buffer, 50, 1100, "./assets/picto/DataOverlay_icn_heartrate.png", "FREQ. CARDIAQUE", s, 2.5);

    // Draw map
    if (map_ != NULL)
        map_->render(&frame_buffer, data); // x:1700, y:900, w:800, h:500

    frame->fromImageBuf(frame_buffer);
}
```


## Maps

You can specify map source from a list. Warning, all maps aren't free.

gpx2video downloads each tile with the zoom level in your `~/.gpx2video/cach` path. 
Then build the map.

*map is saved in the current path in the 'map.png' file.*

Finally, gpx2video draws a mapbox in applying the zoom factor.


## ToDo

  - Render more gauges (color, size, position...)
  - Render maps (alpha, size, position, zoom...)
  - Render track (color, remaining, speed gradient...)
  - Photo import
  - GPS fix mode (doesn't render gauge if no signal)
  - User interface integration
  - GPS interpolate data between two waypoints.
  - Progress rendering status


## Credits

gpx2video is based on several open source libraries:
  - libevent - (https://libevent.org)
  - libcurl - (https://curl.se/libcurl)
  - ffmpeg - (https://ffmpeg.org)
  - gpxlib - (http://irdvo.nl/gpxlib)
  - osm-gps-map - (http://nzjrs.github.io/osm-gps-map)
  - olive - (https://www.olivevideoeditor.org)

