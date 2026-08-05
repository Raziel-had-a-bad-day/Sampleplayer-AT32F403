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


Serial port 3 now receives part edit info  
0:low 16b 1:high 16b 2:value 3:checksum , 
ie 113 is sample 1, part 1 , feature 3  , then value 0-127 , then checksum 
features 0=start add 1=length 2=pitch 3=gap (repeat rate when looping)
4=phaser rate 5=delay_level 
pitch is 2 octaves atm  (103-127)
gap is around 24 once per bar (temp) 

pressing a keyboard key toggles playback of selected sample (any octave) 0-3  or 5-8 for part 1 (looping)


