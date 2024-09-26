

// GPX is the common gps file format for storing waypoints, routes, and tracks, etc.
// This CPP file is for functions related to navigating and tracking active waypoints and routes
// (Though Leaf may/will support other file types in the future, we'll still use this gpx.cpp file even when dealing with other datatypes)

#include "gpx.h"
#include "gps.h"
#include "speaker.h"

Waypoint emptyPoint = {"empty_point", 0, 0, 0};
Waypoint waypoints[maxWaypoints];
Route routes[maxRoutes];
GPXnav gpxNav = {emptyPoint, emptyPoint, emptyPoint, 0,0,0,   0,0,0,0,0,   false, false};
GPXdata gpxData = {};


//TODO: at least here for testing so we can be navigating right from boot up
void gpx_initNav() {
	gpx_loadWaypoints();
	gpx_loadRoutes();
	//gpx_activatePoint(19);
	gpx_activateRoute(3);
}


// update nav data every second
void updateGPXnav() {	

	// only update if we're tracking to an active point
	if (gpxNav.activePointIndex) {

		// update distance and time remaining 
		gpxNav.pointDistanceRemaining = gps.distanceBetween(gps.location.lat(), gps.location.lng(), gpxNav.activePoint.lat, gpxNav.activePoint.lon);
		if (gps.speed.mps() < 0.5) { 
			gpxNav.pointTimeRemaining = 0;
		}	else {
			gpxNav.pointTimeRemaining = ( gpxNav.pointDistanceRemaining - waypointRadius ) / gps.speed.mps();
		}

		// sequence point if needed (this will also update distance to the new point)
		if (gpxNav.pointDistanceRemaining < waypointRadius && !gpxNav.reachedGoal) gpx_sequenceWaypoint();

		// get degress to active point
		gpxNav.courseToActive = gps.courseTo(
			gps.location.lat(), 
			gps.location.lng(), 
			gpxNav.activePoint.lat,
			gpxNav.activePoint.lon
		);
		gpxNav.turnToActive = gpxNav.courseToActive - gps.course.deg();
		if (gpxNav.turnToActive > 180) gpxNav.turnToActive -= 360;
		else if (gpxNav.turnToActive < -180) gpxNav.turnToActive += 360;

		// if there's a next point, get course to that as well
		if (gpxNav.nextPointIndex) {
			gpxNav.courseToNext = gps.courseTo(
				gps.location.lat(), 
				gps.location.lng(), 
				gpxNav.nextPoint.lat,
				gpxNav.nextPoint.lon
			);
			gpxNav.turnToNext = gpxNav.courseToNext - gps.course.deg();
			if (gpxNav.turnToNext > 180) gpxNav.turnToNext -= 360;
			else if (gpxNav.turnToNext < -180) gpxNav.turnToNext += 360;
		}
	}
}



// Start, Sequence, and End Navigation Functions

	bool gpx_activatePoint(int16_t pointIndex) {
		gpxNav.navigating = true;
		gpxNav.reachedGoal = false;

		gpxNav.activeRouteIndex = 0;			// Point navigation is exclusive from Route navigation, so cancel any Route navigation 
		gpxNav.activePointIndex = pointIndex;	
		gpxNav.activePoint = gpxData.waypoints[gpxNav.activePointIndex];

		speaker_playSound(fx_enter);

		double newDistance = gps.distanceBetween(
			gps.location.lat(),
			gps.location.lng(),
			gpxNav.activePoint.lat,
			gpxNav.activePoint.lon);

		gpxNav.segmentDistance = newDistance;
		gpxNav.totalDistanceRemaining = newDistance;
		gpxNav.pointDistanceRemaining = newDistance;

		return gpxNav.navigating;
	}

	bool gpx_activateRoute(uint16_t routeIndex) {

		// first check if any valid points
		uint8_t validPoints = gpxData.routes[routeIndex].totalPoints;
		if (!validPoints) {
			gpxNav.navigating = false;
		} else {
			gpxNav.navigating = true;
			gpxNav.reachedGoal = false;
			gpxNav.activeRouteIndex = routeIndex;

			Serial.print("*** NEW ROUTE: ");
			Serial.println(gpxData.routes[gpxNav.activeRouteIndex].name);

			// set activePointIndex to 0, then call sequenceWaypoint() to increment and populate new activePoint, and nextPoint, if any
			gpxNav.activePointIndex = 0;
			gpx_sequenceWaypoint();

			//calculate TOTAL Route distance
				gpxNav.totalDistanceRemaining = 0;
				// if we have at least 2 points:
				if (gpxData.routes[gpxNav.activeRouteIndex].totalPoints >= 2) {
					for (int i = 1; i < routes[gpxNav.activeRouteIndex].totalPoints; i++) {
						gpxNav.totalDistanceRemaining += gps.distanceBetween(
							gpxData.routes[gpxNav.activeRouteIndex].routepoints[i].lat,
							gpxData.routes[gpxNav.activeRouteIndex].routepoints[i].lon,
							gpxData.routes[gpxNav.activeRouteIndex].routepoints[i+1].lat,
							gpxData.routes[gpxNav.activeRouteIndex].routepoints[i+1].lon		
						);
					}
				// otherwise our Route only has 1 point, so the Route distance is from where we are now to that one point
				} else if (gpxData.routes[gpxNav.activeRouteIndex].totalPoints == 1) {
					gpxNav.totalDistanceRemaining = gps.distanceBetween(
						gps.location.lat(),
						gps.location.lng(),
						gpxData.routes[gpxNav.activeRouteIndex].routepoints[1].lat,
						gpxData.routes[gpxNav.activeRouteIndex].routepoints[1].lon);	
				}
		}
		return gpxNav.navigating;
	}

	bool gpx_sequenceWaypoint() {
		Serial.print("entering sequence..");

		bool successfulSequence = false;

		// sequence to next point if we're on a route && there's another point in the Route
		if (gpxNav.activeRouteIndex && gpxNav.activePointIndex < gpxData.routes[gpxNav.activeRouteIndex].totalPoints) {
			successfulSequence = true;

			//TODO: play going to next point sound, or whatever
			speaker_playSound(fx_enter);

			gpxNav.activePointIndex++;
			Serial.print(" new active index:");
			Serial.print(gpxNav.activePointIndex);
			Serial.print(" route index:");
			Serial.print(gpxNav.activeRouteIndex);
			gpxNav.activePoint = gpxData.routes[gpxNav.activeRouteIndex].routepoints[gpxNav.activePointIndex];

			Serial.print(" new point:");
			Serial.print(gpxNav.activePoint.name);
			Serial.print(" new lat: ");
			Serial.print(gpxNav.activePoint.lat);

			if (gpxNav.activePointIndex + 1 < gpxData.routes[gpxNav.activeRouteIndex].totalPoints) { 				// if there's also a next point in the list, capture that
				gpxNav.nextPointIndex = gpxNav.activePointIndex + 1;
				gpxNav.nextPoint = gpxData.routes[gpxNav.activeRouteIndex].routepoints[gpxNav.nextPointIndex];
			} else {																																	// otherwise signify no next point, so we don't show display functions related to next point
				gpxNav.nextPointIndex = -1;
			}

			// get distance between present (prev) point and new activePoint (used for distance progress bar)
			// if we're sequencing to the very first point, then there's no previous point to use, so use our current location instead
			if (gpxNav.activePointIndex == 1) {
				gpxNav.segmentDistance = gps.distanceBetween(
					gps.location.lat(),
					gps.location.lng(),
					gpxData.routes[gpxNav.activeRouteIndex].routepoints[gpxNav.activePointIndex].lat,
					gpxData.routes[gpxNav.activeRouteIndex].routepoints[gpxNav.activePointIndex].lon);		
			} else {
				gpxNav.segmentDistance = gps.distanceBetween(
					gpxData.routes[gpxNav.activeRouteIndex].routepoints[gpxNav.activePointIndex - 1].lat,
					gpxData.routes[gpxNav.activeRouteIndex].routepoints[gpxNav.activePointIndex - 1].lon,
					gpxData.routes[gpxNav.activeRouteIndex].routepoints[gpxNav.activePointIndex].lat,
					gpxData.routes[gpxNav.activeRouteIndex].routepoints[gpxNav.activePointIndex].lon);				
			} 		

		} else { 										// otherwise, we made it to our destination!
			//TODO: celebrate!  (play reaching goal sound, or whatever)	
			gpxNav.reachedGoal = true;
			speaker_playSound(fx_confirm);
			
			}		
		Serial.print(" succes is: ");
		Serial.println(successfulSequence);
		return successfulSequence;
	}

	void gpx_cancelNav() {
		gpxNav.pointDistanceRemaining = 0;
		gpxNav.pointTimeRemaining = 0;
		gpxNav.activeRouteIndex = 0;
		gpxNav.activePointIndex = 0;
		gpxNav.reachedGoal = false;
		gpxNav.navigating = false;
		gpxNav.turnToActive = 0;
		gpxNav.turnToNext = 0;
		speaker_playSound(fx_cancel);
	}










// Ingest data from .gpx file on SD Card
// fileName will look something like:  /GPX_files/MarshallCrestline.gpx
// Function will return true if any non-zero number of waypoints and/or routes (with non zero route points) were loaded, false otherwise
// Uncertain what to do with missing fields... for example, if a waypoint doesn't have an elevation, should we populate it with 0' MSL?  Or not load the waypoint?  If it doesn't have a name, should we name it Waypoint_N?
// Probably, Waypoints need names (otherwise how would you select them or find them?), but if elevations is missing, we can put -1000 (so we can know it's a missing value)
// Probably, Routes need names (so you can select/find them), but the individual route points DON'T need names (could be auto-named whatever the route name is, followed by a suffix, like: RouteName_1, RouteName_2)

/*  Example GPX file:

<?xml version="1.0" encoding="UTF-8"?>
<gpx xmlns="http://www.topografix.com/GPX/1/1" xmlns:gpxx="http://www.garmin.com/xmlschemas/GpxExtensions/v3" creator="CALTOPO" version="1.1">

   <metadata>
      <name><![CDATA[SoCal GPX]]></name>
   </metadata>

   <wpt lat="34.19318" lon="-117.32334">
      <ele>521</ele>
      <name>Marshall_LZ</name>
   </wpt>

   <wpt lat="34.21016" lon="-117.30274">
      <ele>1223</ele>
      <name>Marshall_Launch</name>
   </wpt>

   <rte>
      <name>The Circuit</name>
      <rtept lat="34.21016" lon="-117.30274">
         <ele>1223</ele>
         <name>Marshall_Launch</name>
      </rtept>	
      <rtept lat="34.23531" lon="-117.32608">
         <ele>1572</ele>
         <name>Billboard</name>
      </rtept>	
      <rtept lat="34.23762" lon="-117.35115">
         <ele>1553</ele>
         <name>Pine_Mt</name>
      </rtept>	
      <rtept lat="34.21016" lon="-117.30274">
         <ele>1223</ele>
         <name>Marshall_Launch</name>
      </rtept>	
   </rte>


*/

bool gpx_readFile(String fileName) {

	bool success = false;

	// for reading waypoints
	int8_t read_waypoint_index = 0;
	bool waypoint_lat_success = false;
	bool waypoint_lon_success = false;
	bool waypoint_ele_success = false;
	bool waypoint_name_success = false;

	// for reading routes
	int8_t read_route_index = 0;
	bool route_name_success = false;
	bool route_routepoint_success = false;

	// for reading the points within a route (route points)
	int8_t read_routepoint_index = 0;
	bool routepoint_lat_succcess = false;
	bool routepoint_lon_succcess = false;
	bool routepoint_ele_succcess = false;
	bool routepoint_name_succcess = false;


	/*

	// open file from SD card
	if (!open(fileName)) return success;		// if file doesn't open propely, kick out with 'false' return

	


	// LOOP THROUGH FILE looking for "<wpt" or "<rte>"

	if ("<wpt" && read_waypoint_index < maxWaypoints) {		// only save a new waypoint if we still have room 
		read_waypoint_index++;
		waypoint_lat_success = false;
		waypoint_lon_success = false;
		waypoint_ele_success = false;
		waypoint_name_success = false;

		//populate Lat, Lon
		gpxData.waypoints[read_waypoint_index].lat = <lat>;
		if (lat saved properly) waypoint_lat_success = true;
		gpxData.waypoints[read_waypoint_index].lon = <lon>;
		if (lon saved properly) waypoint_lon_success = true;

		look for "<ele>" or "<name>" tages

		if ("<ele>") {
			gpxData.waypoints[read_waypoint_index].ele = <ele>;
			if (sucessfully saved elevation) waypoint_ele_success = true;
		} else if ("<name>") {
			gpxData.waypoints[read_waypoint_index].name = <name>;
			if (successfully saved name) waypoint_name_success = true;
		}

		if (all the values were populated properly) {
			gpxData.totalWaypoints = read_waypoint_index;			// increment total number of saved waypoints
			success = true; 																	// we had at least one successful waypoint save, so the function can return true
		} else {
			read_waypoint_index--;		// undo this attempt at saving a waypoint and overwrite it with the next waypoint
		}
	}

	if ("<rte>" && read_route_index < maxRoutes) {
		read_route_index++;
		route_name_success = false;
	  route_routepoint_success = false;

		look for "<rtept" or "<name>" tags
		
		if ("<name>") {
			gpxData.routes[read_route_index].name = <name>;
			if (name saved properly) route_name_succcess = true;
		} else if ("<rtept") {
			read_routepoint_index++;
			routepoint_lat_succcess = false;
			routepoint_lon_succcess = false;
			routepoint_ele_succcess = false;
			routepoint_name_succcess = false;
			
			//populate Lat, Lon
			gpxData.routes[read_route_index].routepoints[read_routepoint_index].lat = <lat>;
			if (lat saved properly) routepoint_lat_success = true;
			gpxData.routes[read_route_index].routepoints[read_routepoint_index].lon = <lon>;
			if (lon saved properly) routepoint_lon_success = true;

			look for "<ele>" or "<name>" tags

			if ("<ele>") {
				gpxData.routes[read_route_index].routepoints[read_routepoint_index].ele = <ele>;
				if (sucessfully saved elevation) routepoint_ele_success = true;
			} else if ("<name>") {
				gpxData.routes[read_route_index].routepoints[read_routepoint_index].name = <name>;
				if (successfully saved name) waypoint_name_success = true;
			}

			if (all the routepoint values were populated properly) {
				gpxData.routes[read_route_index].totalPoints = read_routepoint_index;			// increment total number of saved routepoints in this route				
			} else {
				read_routepoint_index--;		// undo this attempt at saving a routepoint and overwrite it with the next routepoint
			}


		}

		if (all the route values were populated properly && there's at least one routepoint) {
			gpxData.totalRoutes = read_route_index;						// increment total number of saved routes
			success = true; 																	// we had at least one successful route save, so the function can return true
		} else {
			read_route_index--;																// undo this attempt at saving a route and overwrite it with the next route
		}

		
	}

	*/

}



void gpx_loadRoutes() {

	gpxData.routes[1].name = "R: TheCircuit";
	gpxData.routes[1].totalPoints = 5;
	gpxData.routes[1].routepoints[1] = gpxData.waypoints[1];
	gpxData.routes[1].routepoints[2] = gpxData.waypoints[7];
	gpxData.routes[1].routepoints[3] = gpxData.waypoints[8];
	gpxData.routes[1].routepoints[4] = gpxData.waypoints[1];
	gpxData.routes[1].routepoints[5] = gpxData.waypoints[2];

	gpxData.routes[2].name = "R: Scenic";
	gpxData.routes[2].totalPoints = 5;
	gpxData.routes[2].routepoints[1] = gpxData.waypoints[1];
	gpxData.routes[2].routepoints[2] = gpxData.waypoints[3];
	gpxData.routes[2].routepoints[3] = gpxData.waypoints[4];
	gpxData.routes[2].routepoints[4] = gpxData.waypoints[5];
	gpxData.routes[2].routepoints[5] = gpxData.waypoints[2];

	gpxData.routes[3].name = "R: Downhill";
	gpxData.routes[3].totalPoints = 4;
	gpxData.routes[3].routepoints[1] = gpxData.waypoints[1];
	gpxData.routes[3].routepoints[2] = gpxData.waypoints[5];
	gpxData.routes[3].routepoints[3] = gpxData.waypoints[2];

	gpxData.routes[4].name = "R: MiniTri";
	gpxData.routes[4].totalPoints = 4;
	gpxData.routes[4].routepoints[1] = gpxData.waypoints[1];
	gpxData.routes[4].routepoints[2] = gpxData.waypoints[4];
	gpxData.routes[4].routepoints[3] = gpxData.waypoints[5];
	gpxData.routes[4].routepoints[4] = gpxData.waypoints[1];


	gpxData.totalRoutes = 4;
}

void gpx_loadWaypoints() {
	gpxData.waypoints[0].lat = 0;
	gpxData.waypoints[0].lon = 0;
	gpxData.waypoints[0].ele = 0;
	gpxData.waypoints[0].name = "EMPTY_POINT";

	gpxData.waypoints[1].lat = 34.21016;
	gpxData.waypoints[1].lon = -117.30274;
	gpxData.waypoints[1].ele = 1223;
	gpxData.waypoints[1].name = "Marshall";

	gpxData.waypoints[2].lat = 34.19318;
	gpxData.waypoints[2].lon = -117.32334;
	gpxData.waypoints[2].ele = 521;
	gpxData.waypoints[2].name = "Marshall_LZ";

	gpxData.waypoints[3].lat = 34.21065;
	gpxData.waypoints[3].lon = -117.31298;
	gpxData.waypoints[3].ele = 1169;
	gpxData.waypoints[3].name = "Cloud";

	gpxData.waypoints[4].lat = 34.20958;
	gpxData.waypoints[4].lon = -117.31982;
	gpxData.waypoints[4].ele = 1033;
	gpxData.waypoints[4].name = "Regionals";

	gpxData.waypoints[5].lat = 34.19991;
	gpxData.waypoints[5].lon = -117.31688;
	gpxData.waypoints[5].ele = 767;
	gpxData.waypoints[5].name = "750";

	gpxData.waypoints[6].lat = 34.23604;
	gpxData.waypoints[6].lon = -117.31321;
	gpxData.waypoints[6].ele = 1611;
	gpxData.waypoints[6].name = "Crestline";

	gpxData.waypoints[7].lat = 34.23531;
	gpxData.waypoints[7].lon = -117.32608;
	gpxData.waypoints[7].ele = 1572;
	gpxData.waypoints[7].name = "Billboard";

	gpxData.waypoints[8].lat = 34.23762;
	gpxData.waypoints[8].lon = -117.35115;
	gpxData.waypoints[8].ele = 1553;
	gpxData.waypoints[8].name = "Pine_Mt";

	gpxData.totalWaypoints = 8;
}
