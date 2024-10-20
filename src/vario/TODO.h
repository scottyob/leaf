/*



Infrastructure Stuff
  Power Saving / Sleep functions during operation (and can we do PWM speaker during sleep?)
  Wifi & Bluetooth
  OTA updates, or at least non-IDE updates
  More precise partition and memory plan (put fonts and stuff elsewhere?  Partition for OTA updates?  How many waypoints/routes can we store in NVM?  etc)
  
GPS Logging
  Additional file formats for logs (gpx, IGC, KML-with-timestamps, etc)

UI
  Additional menu screens (choose GPX file to load; list of waypoints and routes; possibly create/save/edit waypoint and create/save/edit route)
  Final touches on Thermal Page & Nav Page (need to show "NO gps" when don't have fix for various fields)
  User editable fields on various pages
  Debug screen?  / Advanced options?


Features and Performance
  Better altitude filter to reduce noise but speed up reaction time
  Include accelerometer in vario signal
  Keep a running/filtered average of speed, glide, and climb, to use in various calculations (especially time to intercept etc)
  Wind estimate based on GPS speed/bearing vectors
  Full logbook support (min/max/total values from each flight.  Where to save?)
  Fine tune ambient temp sensor (it's about 5C high because of self heating of the board, but test over various ranges and with varied airflow)
  Zen mode (no beeping until flying)
  Various tone-modes (change pitch of beeping; making beeping follow major and minor scales for climb/sink, etc)
  Possibly update parts of the display more often than every 0.5sec (like the graphical bars and needles etc).  But keep text to 0.5sec or it's too hard to read as it changes quickly
  Support mag and maybe gyro angles; need to calibrate and test how it works at various installation angles



  


Bugs and obvservations
* GPS time shows valid and 4:59pm on boot up, and even keeps time (incrementing minutes), but is incorrect until later (better signal?)
* Can't always reprogram even though ECO_MODE is turned off (no sleep)



*/