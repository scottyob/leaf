#define WEB_DEBUG true
#pragma once
// The web debug module creates a webserver to allow for remote debugging of the vario.  
// You'll want to define WEB_DEBUG to enable it

#ifdef WEB_DEBUG

void webdebug_setup();
void webdebug_update();


#endif