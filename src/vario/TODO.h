/*


ESP32 Main things to figure out before committing to PCB design and pinouts etc

Pin/Function-Specific:
  HSPI - other spi bus for more than 3 devices (also better to use for devices that don't send stuff back, like LCD)
  Interrupt pins available for button lines (or just poll)
  PWM for speaker output
  ADC battery sense



Firmware only
  Power Saving / Sleep functions
  Wifi & Bluetooth
  Partition and Non volatile storage (for fonts, user settings, etc)


gx:track kml intro:
https://www.avenza.com/resources/blog/2016/11/30/difference-between-lines-and-tracks-when-exporting-from-avenza-maps/#:~:text=A%20Linestring%20element%20contains%20a,degrees%20and%20elevations%20in%20meters.&text=Tracks%20use%20the%20KML%20element,Track%E2%80%9D%20which%20contains%20several%20tags.&text=There%20are%20equal%20numbers%20of%20each%20of%20these%20tags.

done: UART
done: VSPI 


Baro sensor 
   done, untested: better temperature adjustment accounting for <20C conditions and <-15C conditions

  


Bugs and obvservations
* GPS time shows valid and 4:59pm on boot up, and even keeps time (incrementing minutes), but is incorrect until later (better signal?)
* Can't always reprogram even though ECO_MODE is turned off (no sleep)



*/