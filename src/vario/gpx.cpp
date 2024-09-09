

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

		// update distance remaining 
		gpxNav.pointDistanceRemaining = gps.distanceBetween(gps.location.lat(), gps.location.lng(), gpxNav.activePoint.lat, gpxNav.activePoint.lon);

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
		gpxNav.activePoint = waypoints[gpxNav.activePointIndex];

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
		uint8_t validPoints = routes[routeIndex].totalPoints;
		if (!validPoints) {
			gpxNav.navigating = false;
		} else {
			gpxNav.navigating = true;
			gpxNav.activeRouteIndex = routeIndex;

			Serial.print("*** NEW ROUTE: ");
			Serial.println(routes[gpxNav.activeRouteIndex].name);

			// set activePointIndex to 0, then call sequenceWaypoint() to increment and populate new activePoint, and nextPoint, if any
			gpxNav.activePointIndex = 0;
			gpx_sequenceWaypoint();

			//calculate TOTAL Route distance
				gpxNav.totalDistanceRemaining = 0;
				// if we have at least 2 points:
				if (routes[gpxNav.activeRouteIndex].totalPoints >= 2) {
					for (int i = 1; i < routes[gpxNav.activeRouteIndex].totalPoints; i++) {
						gpxNav.totalDistanceRemaining += gps.distanceBetween(
							routes[gpxNav.activeRouteIndex].routepoints[i].lat,
							routes[gpxNav.activeRouteIndex].routepoints[i].lon,
							routes[gpxNav.activeRouteIndex].routepoints[i+1].lat,
							routes[gpxNav.activeRouteIndex].routepoints[i+1].lon		
						);
					}
				// otherwise our Route only has 1 point, so the Route distance is from where we are now to that one point
				} else if (routes[gpxNav.activeRouteIndex].totalPoints == 1) {
					gpxNav.totalDistanceRemaining = gps.distanceBetween(
						gps.location.lat(),
						gps.location.lng(),
						routes[gpxNav.activeRouteIndex].routepoints[1].lat,
						routes[gpxNav.activeRouteIndex].routepoints[1].lon);	
				}
		}
		return gpxNav.navigating;
	}

	bool gpx_sequenceWaypoint() {
		Serial.print("entering sequence..");

		bool successfulSequence = false;

		// sequence to next point if we're on a route && there's another point in the Route
		if (gpxNav.activeRouteIndex && gpxNav.activePointIndex < routes[gpxNav.activeRouteIndex].totalPoints) {
			successfulSequence = true;

			//TODO: play going to next point sound, or whatever
			speaker_playSound(fx_enter);

			gpxNav.activePointIndex++;
			Serial.print(" new active index:");
			Serial.print(gpxNav.activePointIndex);
			Serial.print(" route index:");
			Serial.print(gpxNav.activeRouteIndex);
			gpxNav.activePoint = routes[gpxNav.activeRouteIndex].routepoints[gpxNav.activePointIndex];

			Serial.print(" new point:");
			Serial.print(gpxNav.activePoint.name);
			Serial.print(" new lat: ");
			Serial.print(gpxNav.activePoint.lat);

			if (gpxNav.activePointIndex + 1 < routes[gpxNav.activeRouteIndex].totalPoints) { 				// if there's also a next point in the list, capture that
				gpxNav.nextPointIndex = gpxNav.activePointIndex + 1;
				gpxNav.nextPoint = routes[gpxNav.activeRouteIndex].routepoints[gpxNav.nextPointIndex];
			} else {																																	// otherwise signify no next point, so we don't show display functions related to next point
				gpxNav.nextPointIndex = -1;
			}

			// get distance between present (prev) point and new activePoint (used for distance progress bar)
			// if we're sequencing to the very first point, then there's no previous point to use, so use our current location instead
			if (gpxNav.activePointIndex == 1) {
				gpxNav.segmentDistance = gps.distanceBetween(
					gps.location.lat(),
					gps.location.lng(),
					routes[gpxNav.activeRouteIndex].routepoints[gpxNav.activePointIndex].lat,
					routes[gpxNav.activeRouteIndex].routepoints[gpxNav.activePointIndex].lon);		
			} else {
				gpxNav.segmentDistance = gps.distanceBetween(
					routes[gpxNav.activeRouteIndex].routepoints[gpxNav.activePointIndex - 1].lat,
					routes[gpxNav.activeRouteIndex].routepoints[gpxNav.activePointIndex - 1].lon,
					routes[gpxNav.activeRouteIndex].routepoints[gpxNav.activePointIndex].lat,
					routes[gpxNav.activeRouteIndex].routepoints[gpxNav.activePointIndex].lon);				
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
		gpxNav.activeRouteIndex = 0;
		gpxNav.activePointIndex = 0;
		gpxNav.reachedGoal = false;
		gpxNav.navigating = false;
		gpxNav.turnToActive = 0;
		gpxNav.turnToNext = 0;
		speaker_playSound(fx_cancel);
	}




// Ingest data from .gpx file on SD Card

void gpx_loadRoutes() {
	routes[1].name = "The Triangle";
	routes[1].totalPoints = 5;
	routes[1].routepoints[1] = waypoints[1];
	routes[1].routepoints[2] = waypoints[7];
	routes[1].routepoints[3] = waypoints[8];
	routes[1].routepoints[4] = waypoints[1];
	routes[1].routepoints[5] = waypoints[2];

	routes[2].name = "Downhill";
	routes[2].totalPoints = 5;
	routes[2].routepoints[1] = waypoints[1];
	routes[2].routepoints[2] = waypoints[3];
	routes[2].routepoints[3] = waypoints[4];
	routes[2].routepoints[4] = waypoints[5];
	routes[2].routepoints[5] = waypoints[2];

	routes[3].name = "Casa_Cielo";
	routes[3].totalPoints = 4;
	routes[3].routepoints[1] = waypoints[20];
	routes[3].routepoints[2] = waypoints[21];
	routes[3].routepoints[3] = waypoints[19];
	routes[3].routepoints[4] = waypoints[20];
}

void gpx_loadWaypoints() {
	waypoints[0].lat = 0;
	waypoints[0].lon = 0;
	waypoints[0].ele = 0;
	waypoints[0].name = "EMPTY_POINT";

	waypoints[1].lat = 34.21016;
	waypoints[1].lon = -117.30274;
	waypoints[1].ele = 1223;
	waypoints[1].name = "Marshall";

	waypoints[2].lat = 34.19318;
	waypoints[2].lon = -117.32334;
	waypoints[2].ele = 521;
	waypoints[2].name = "Marshall_LZ";

	waypoints[3].lat = 34.21065;
	waypoints[3].lon = -117.31298;
	waypoints[3].ele = 1169;
	waypoints[3].name = "Cloud";

	waypoints[4].lat = 34.20958;
	waypoints[4].lon = -117.31982;
	waypoints[4].ele = 1033;
	waypoints[4].name = "Regionals";

	waypoints[5].lat = 34.19991;
	waypoints[5].lon = -117.31688;
	waypoints[5].ele = 767;
	waypoints[5].name = "750";

	waypoints[6].lat = 34.23604;
	waypoints[6].lon = -117.31321;
	waypoints[6].ele = 1611;
	waypoints[6].name = "Crestline";

	waypoints[7].lat = 34.23531;
	waypoints[7].lon = -117.32608;
	waypoints[7].ele = 1572;
	waypoints[7].name = "Billboard";

	waypoints[8].lat = 34.23762;
	waypoints[8].lon = -117.35115;
	waypoints[8].ele = 1553;
	waypoints[8].name = "Pine_Mt";

	waypoints[9].lat = 34.21016;
	waypoints[9].lon = -117.30274;
	waypoints[9].ele = 1223;
	waypoints[9].name = "Marshall";

	waypoints[10].lat = 34.21016;
	waypoints[10].lon = -117.30274;
	waypoints[10].ele = 1223;
	waypoints[10].name = "Marshall";

	waypoints[11].lat = 34.21016;
	waypoints[11].lon = -117.30274;
	waypoints[11].ele = 1223;
	waypoints[11].name = "Marshall";


	waypoints[12].lat = 34.19318;
	waypoints[12].lon = -117.32334;
	waypoints[12].ele = 521;
	waypoints[12].name = "Marshall_LZ";

	waypoints[13].lat = 34.21065;
	waypoints[13].lon = -117.31298;
	waypoints[13].ele = 1169;
	waypoints[13].name = "Cloud";

	waypoints[14].lat = 34.20958;
	waypoints[14].lon = -117.31982;
	waypoints[14].ele = 1033;
	waypoints[14].name = "Regionals";

	waypoints[15].lat = 34.19991;
	waypoints[15].lon = -117.31688;
	waypoints[15].ele = 767;
	waypoints[15].name = "750";

	waypoints[16].lat = 34.23604;
	waypoints[16].lon = -117.31321;
	waypoints[16].ele = 1611;
	waypoints[16].name = "Crestline";

	waypoints[17].lat = 34.23531;
	waypoints[17].lon = -117.32608;
	waypoints[17].ele = 1572;
	waypoints[17].name = "Billboard";

	waypoints[18].lat = 34.23762;
	waypoints[18].lon = -117.35115;
	waypoints[18].ele = 1553;
	waypoints[18].name = "Pine_Mt";

	waypoints[19].lat = 33.847789;
	waypoints[19].lon = -111.974868;
	waypoints[19].ele = 1553;
	waypoints[19].name = "Driveway";

	waypoints[20].lat = 33.847409;
	waypoints[20].lon = -111.974666;
	waypoints[20].ele = 1553;
	waypoints[20].name = "Pool";

	waypoints[21].lat = 33.847486;
	waypoints[21].lon = -111.974927;
	waypoints[21].ele = 1553;
	waypoints[21].name = "Bunnies";
}
