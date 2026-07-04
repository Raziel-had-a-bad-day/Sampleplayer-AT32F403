# Sampleplayer
 Basic sampleplay micro with midi in (uart 3)
 
 Send samples through serial , saves it automatically to ram then copies the lot to flash (uart 4). 
 Copies external flash to psram on start
 cc 90-97 sample pitch
 cc 98-105 sample length
 4 note poly atm
 Channel 10 triggered from note 0-7 on any octave 
 Channel 5 , 2 note poly with basic wavetable playback ,tune with cc=19 , has delay with some feedback , cc=5 for delay time
 Samples get a stereo flanger effect , depth: cc=7  , rate: cc=78
Settings are saved to internal flash at intervals.
Flash and psram are wired parallel with separate CS. 