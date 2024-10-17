

// GPX is the common gps file format for storing waypoints, routes, and tracks, etc.
// This CPP file is for functions related to navigating and tracking active waypoints and routes
// (Though Leaf may/will support other file types in the future, we'll still use this gpx.cpp file even when dealing with other datatypes)

#include "gpx.h"
#include "gps.h"
#include "speaker.h"
#include <FS.h>
#include <SD_MMC.h>

Waypoint emptyPoint = {"empty_point", 0, 0, 0};
Waypoint waypoints[maxWaypoints];
Route routes[maxRoutes];
GPXnav gpxNav = {emptyPoint, emptyPoint, emptyPoint, 0,0,0,   0,0,0,0,0,   false, false};
GPXdata gpxData = {};


//TODO: at least here for testing so we can be navigating right from boot up
void gpx_initNav() {
	
	Serial.println("Loading GPX file...");
	delay(100);
	bool result = gpx_readFile(SD_MMC, "/SoCal_GPX.gpx");
	Serial.print("...");
	Serial.println(result);
	
	//gpx_loadWaypoints();
	//gpx_loadRoutes();
	//gpx_activatePoint(19);
	//gpx_activateRoute(3);
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

#define BUFFER_LEN (256)

class FileReader {
  public:
    FileReader(fs::FS &fs, String fileName) {
		// open file from SD card
		_file = fs.open(fileName, FILE_READ);
		if (!_file) {
			_error = "Could not open file";
			_complete = true;
		}
		_buffer_count = 0;
		_buffer_index = 0;
	}

	char nextChar() {
		if (_buffer_index >= _buffer_count) {
			// We need to read another block from the file
			_buffer_count = _file.read((uint8_t*)_buffer, BUFFER_LEN);
			_buffer_index = 0;
		}
		if (_buffer_count == 0) {
			return 0;  // This should not happen, but avoid overrunning the buffer if it does
		}
		return _buffer[_buffer_index++];
	}

	bool contentRemaining() {
		if (_buffer_index < _buffer_count) {
			return true;
		}
		return _file.available();
	}

	inline String error() {
		return _error;
	}

	~FileReader() {
		_file.close();
	}

  private:
	bool _complete;
	String _error;
	File _file;
	char _buffer[BUFFER_LEN];
	uint16_t _buffer_count;
	uint16_t _buffer_index;
};

// The maximum length of a value in a GPX (tag name, latitude, longitude, elevation, etc)
#define MAX_VALUE_LENGTH (32)

inline bool isWhitespace(char c) {
	return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

bool equalsIgnoreCase(char* value, const char* constant) {
	uint16_t i = 0;
	while (i < MAX_VALUE_LENGTH) {
		if (value[i] == 0 && constant[i] == 0) {
			return true;
		}
		if (value[i] != constant[i]) {
			bool lcase_value = 'a' <= value[i] && value[i] <= 'z';
			bool ucase_value = 'A' <= value[i] && value[i] <= 'Z';
			bool lcase_const = 'a' <= constant[i] && constant[i] <= 'A';
			bool ucase_const = 'A' <= constant[i] && constant[i] <= 'Z';
			if (lcase_value && ucase_const) {
				if (value[i] != constant[i] + ('a' - 'A')) {
					return false;
				}
			} else if (ucase_value && lcase_const ) {
				if (value[i] + ('a' - 'A') != constant[i]) {
					return false;
				}
			} else {
				return false;
			}
		}
		i++;
	}
	return true;
}

enum class ReadTagNameResult {
	TagClosed,
	TagOpen,
	Error,
};

/// @brief Parses select GPX content from a provided FileReader
/// @details File may not contain any 0-value byes; these will be considered the end of the file.
/// @details Only literals may appear between <ele> and <name> tags; no additional subtags are allowed.
/// @details No self-closing tags are allowed inside <wpt> or <rtept> tags.
class GPXParser {
  public:
	GPXParser(FileReader* file_reader) {
		_file_reader = file_reader;
		_line = 1;
		_col = 1;
		_error = "";
	}

	bool parse(GPXdata* result) {
		result->totalRoutes = 0;
		result->totalWaypoints = 0;

		char value_buffer[MAX_VALUE_LENGTH + 1];

		while (scrollToTagBoundary('<')) {
			ReadTagNameResult name_result = readTagName(value_buffer);
			if (name_result == ReadTagNameResult::Error) {
				_error += " while parsing GPX";
				return false;
			}
			if (equalsIgnoreCase(value_buffer, "wpt")) {
				if (name_result == ReadTagNameResult::TagClosed) {
					_error = "missing lat and lon attributes for wpt tag";
					return false;
				}
				if (result->totalWaypoints >= maxWaypoints) {
					_error = "maximum number of GPX waypoints exceeded";
					return false;
				}
				Waypoint waypoint;
				if (readWaypoint(&waypoint, "wpt")) {
					result->waypoints[result->totalWaypoints] = waypoint;
					result->totalWaypoints++;
				} else {
					_error += " in wpt tag";
					return false;
				}
			} else if (equalsIgnoreCase(value_buffer, "rte")) {
				if (name_result == ReadTagNameResult::TagOpen) {
					if (!scrollToTagBoundary('>')) {
						_error = "reached end of file when reading rte tag";
						return false;
					}
				}
				if (result->totalRoutes >= maxRoutes) {
					_error = "maximum number of GPX routes exceeded";
					return false;
				}
				Route route;
				if (readRoute(&route)) {
					result->routes[result->totalRoutes] = route;
					result->totalRoutes++;
				} else {
					_error += " in rte tag";
					return false;
				}
			}
		}

		return true;
	}

	inline String error() {
		return _error;
	}

	inline uint16_t line() {
		return _line;
	}

	inline uint16_t col() {
		return _col;
	}

  private:
	/// @brief Get next character from FileReader
	/// @return Next character, or 0 if there is no next character
	char getNextChar() {
		if (!_file_reader->contentRemaining()) {
			return 0;
		}
		char c = _file_reader->nextChar();
		if (c == '\n') {
			_line++;
			_col = 1;
		} else if (c == '\r') {
			// Do not increment line nor column for carriage returns
		} else {
			_col++;
		}
		return c;
	}

	/// @brief Read characters until the specified boundary of a tag is found
	/// @return True if the specified boundary was found, false if boundary was not found
	bool scrollToTagBoundary(char boundary) {
		char c;
		do {
			c = getNextChar();
			if (c == boundary) {
				return true;
			}
		} while (c != 0);
		return false;
	}

	/// @brief Read just the name of a tag into the provided value buffer, ensuring null termination
	ReadTagNameResult readTagName(char* value) {
		ReadTagNameResult result;
		char c;
		uint16_t i = 0;
		bool name_started = false;
		while (true) {
			c = getNextChar();
			if (c == '>') {
				result = ReadTagNameResult::TagClosed;
				break;
			} else if (isWhitespace(c)) {
				if (name_started) {
					result = ReadTagNameResult::TagOpen;
					break;
				}
			} else if (c == 0) {
				_error = "reached end of file while reading tag name";
				result = ReadTagNameResult::Error;
				break;
			} else {
				value[i++] = c;
				name_started = true;
				if (i >= MAX_VALUE_LENGTH) {
					_error = "tag name was too long";
					result = ReadTagNameResult::Error;
					break;
				}
			}
		}
		value[i] = 0; // Null-terminate the value
		return result;
	}

	/// @brief Read characters until encountering a non-whitespace character
	/// @return First non-whitespace character encountered
	char skipWhitespace() {
		char c;
		do {
			c = getNextChar();
		} while (isWhitespace(c));
		return c;
	}

	/// @brief Read the data from a wpt or rtept tag into the provided waypoint
	/// @details Must start inside the wpt/rtept tag just after the tag name
	bool readWaypoint(Waypoint* waypoint, const char* tag_name) {
		char key[MAX_VALUE_LENGTH + 1];
		char value[MAX_VALUE_LENGTH + 1];

		// Read attributes of opening tag (until tag is closed)
		bool found_lat = false;
		bool found_lon = false;
		while (true) {
			char c = skipWhitespace();
			if (c == 0) {
				_error = "reached end of file while looking for attributes in waypoint tag";
				return false;
			} else if (c == '>') {
				break;
			}
			key[0] = c;
			bool attribute_success = readAttribute(key + 1, value);
			if (!attribute_success) {
				_error += " while reading attribute in waypoint tag";
				return false;
			}
			if (equalsIgnoreCase(key, "lat")) {
				waypoint->lat = atof(value);
				found_lat = true;
			} else if (equalsIgnoreCase(key, "lon")) {
				waypoint->lon = atof(value);
				found_lon = true;
			}
		}
		if (!found_lat) {
			_error = "couldn't find lat attribute in waypoint tag";
			return false;
		}
		if (!found_lon) {
			_error = "couldn't find lon attribute in waypoint tag";
			return false;
		}

		// Look for content or the closing tag
		while (true) {
			// Read next tag
			if (!readFullTagName(key)) {
				_error += " while looking for the closing waypoint tag";
				return false;
			}

			if (key[0] == '/' && equalsIgnoreCase(key + 1, tag_name)) {
				// This was the closing tag for the waypoint
				break;
			} else if (equalsIgnoreCase(key, "ele")) {
				// This was an opening elevation tag
				if (!readLiteral(value)) {
					_error += " while reading ele value in waypoint";
					return false;
				}
				waypoint->ele = atof(value);
				ReadTagNameResult closing_outcome = readTagName(key);
				if (closing_outcome == ReadTagNameResult::Error) {
					_error += " while reading name of tag that should be a closing ele tag in waypoint";
					return false;
				} else if (closing_outcome == ReadTagNameResult::TagOpen) {
					if (!scrollToTagBoundary('>')) {
						_error = "reached end of file while looking for end of tag after ele in waypoint";
						return false;
					}
				}
				if (!equalsIgnoreCase(key, "/ele")) {
					_error = "tag after ele in waypoint was not a closing ele tag";
					return false;
				}
			} else if  (equalsIgnoreCase(key, "name")) {
				// This was an opening name tag
				if (!readLiteral(value)) {
					_error += " while reading value of name tag in waypoint";
					return false;
				}
				waypoint->name = String(value);
				ReadTagNameResult closing_outcome = readTagName(key);
				if (closing_outcome == ReadTagNameResult::Error) {
					_error += " while reading name of tag that should be a closing name tag in waypoint";
					return false;
				} else if (closing_outcome == ReadTagNameResult::TagOpen) {
					if (!scrollToTagBoundary('>')) {
						_error = "reached end of file while looking for end of tag after name in waypoint";
						return false;
					}
				}
				if (!equalsIgnoreCase(key, "/name")) {
					_error = "tag after name in waypoint was not a closing name tag";
					return false;
				}
			} else {
				// This was an irrelevant tag; look for its closing tag
				Serial.println("readWaypoint: found irrelevant tag");
				while (true) {
					if (!readFullTagName(value)) {
						_error += " while reading name of tag that should be a closing irrelevant tag in waypoint";
						return false;
					}
					if (value[0] == '/' && strncmp(key, value + 1, MAX_VALUE_LENGTH - 1)) {
						// This was the closing tag for the irrelevant tag within the waypoint; proceed with parsing
						break;
					}
				}
			}
		}

		return true;
	}

	/// @brief  Read the key and value of an attribute
	/// @details Routine may start at whitespace before the attribute
	bool readAttribute(char* key, char* value) {
		// Read key
		uint16_t i = 0;
		char c = skipWhitespace();
		while (true) {
			if (c == 0) {
				_error = "reached end of file while looking for attribute in tag";
				return false;
			} else if (c == '=') {
				break;
			} else if (c == '>') {
				_error = "couldn't find attribute value before end of tag";
				return false;
			} else if (isWhitespace(c)) {
				c = skipWhitespace();
				if (c == '=') {
					break;
				} else {
					_error = "couldn't find attribute value before end of tag";
					return false;
				}
			} else {
				key[i++] = c;
				if (i >= MAX_VALUE_LENGTH) {
					_error = "attribute key was too long";
					return false;
				}
				c = getNextChar();
			}
		}
		key[i] = 0;
		
		// Read value
		i = 0;
		c = skipWhitespace();
		if (c != '"') {
			_error = "attribute value did not start with an opening quote mark";
			return false;
		}
		while (true) {
			c = getNextChar();
			if (c == 0) {
				_error = "reached end of file while looking for end of attribute value in tag";
				return false;
			} else if (c == '"') {
				value[i] = 0;
				return true;
			} else {
				value[i++] = c;
				if (i >= MAX_VALUE_LENGTH) {
					_error = "attribute value was too long";
					return false;
				}
			}
		}
	}

	/// @brief Read a literal value located between two tags
	/// @details Leaves cursor at first character of next tag (past opening < character)
	bool readLiteral(char* value) {
		uint16_t i = 0;
		while (true) {
			char c = getNextChar();
			if (c == '<') {
				break;
			} else if (c == 0) {
				_error = "reached end of file while reading literal value";
				return false;
			}
			value[i++] = c;
			if (i >= MAX_VALUE_LENGTH) {
				_error = "literal value was too long";
				return false;
			}
		}
		value[i] = 0;
		return true;
	}

	/// @brief Starting outside a tag, read up to and through an entire tag and set `key` to the name of the tag
	bool readFullTagName(char* key) {
		if (!scrollToTagBoundary('<')) {
			_error = "reached end of file while reading tag name";
			return false;
		}
		ReadTagNameResult name_outcome = readTagName(key);
		if (name_outcome == ReadTagNameResult::TagOpen) {
			if (!scrollToTagBoundary('>')) {
				_error = "reached end of file while looking for end of tag";
				return false;
			}
		} else if (name_outcome == ReadTagNameResult::TagClosed) {
			return true;
		}
		return false;
	}

	/// @brief Read the data from a rte tag into the provided `route`
	/// @details Must start outside the end of the opening rte tag
	bool readRoute(Route* route) {
		route->totalPoints = 0;

		char key[MAX_VALUE_LENGTH + 1];
		char value[MAX_VALUE_LENGTH + 1];

		// Look for content or the closing tag
		while (true) {
			// Find next tag
			if (!scrollToTagBoundary('<')) {
				_error = "reached end of file while reading tags in route";
				return false;
			}

			// Read next tag
			ReadTagNameResult name_outcome = readTagName(key);
			if (name_outcome == ReadTagNameResult::Error) {
				_error += " while reading tag in route";
				return false;
			}

			// For every tag except rtept, scroll to end of tag
			if (!equalsIgnoreCase(key, "rtept") &&
				name_outcome == ReadTagNameResult::TagOpen &&
				!scrollToTagBoundary('>')) {
					_error = "reached end of file while looking for end of tag in route";
					return false;
			}

			if (equalsIgnoreCase(key, "/rte")) {
				// This was the closing tag for the route
				break;
			} else if (equalsIgnoreCase(key, "rtept")) {
				// This is an opening route point tag
				if (route->totalPoints >= maxRoutePoints) {
					_error = "maximum number of route points exceeded";
					return false;
				}
				Waypoint waypoint;
				if (!readWaypoint(&waypoint, "rtept")) {
					_error = " while reading rtept";
					return false;
				}
				route->routepoints[route->totalPoints] = waypoint;
				route->totalPoints++;
			} else if  (equalsIgnoreCase(key, "name")) {
				// This is an opening name tag
				if (!readLiteral(value)) {
					_error += " while reading route name";
					return false;
				}
				route->name = String(value);
				ReadTagNameResult closing_outcome = readTagName(key);
				if (closing_outcome == ReadTagNameResult::Error) {
					_error += " while reading closing name tag in route";
					return false;
				} else if (closing_outcome == ReadTagNameResult::TagOpen) {
					if (!scrollToTagBoundary('>')) {
						_error = "reached end of file while reading name of tag that should be a closing name tag in route";
						return false;
					}
				}
				if (!equalsIgnoreCase(key, "/name")) {
					_error = "missing closing name tag in route";
					return false;
				}
			} else {
				// This was an irrelevant tag; look for its closing tag
				while (true) {
					if (!readFullTagName(value)) {
						_error += " while reading the name of a closing tag for an irrelevant tag in a route";
						return false;
					}
					if (value[0] == '/' && strncmp(key, value + 1, MAX_VALUE_LENGTH - 1)) {
						// This was the closing tag for the irrelevant tag within the route; proceed with parsing
						break;
					}
				}
			}
		}

		return true;
	}

	FileReader* _file_reader;
	uint16_t _line;
	uint16_t _col;
	String _error;
};

GPXdata parse_result;

bool gpx_readFile(fs::FS &fs, String fileName) {
	FileReader file_reader(fs, fileName);
	if (file_reader.error() != "") {
		Serial.print("Found file_reader error: ");
		Serial.println(file_reader.error());
		return false;
	}
	GPXParser parser(&file_reader);
	bool success = parser.parse(&parse_result);
	if (success) {
		Serial.println("gpx_readFile was successful:");
		Serial.print("  ");
		Serial.print(parse_result.totalWaypoints);
		Serial.println(" waypoints");
		for (uint8_t wp = 0; wp < parse_result.totalWaypoints; wp++) {
			Serial.print("    ");
			Serial.print(parse_result.waypoints[wp].name);
			Serial.print(" @ ");
			Serial.print(parse_result.waypoints[wp].lat);
			Serial.print(", ");
			Serial.print(parse_result.waypoints[wp].lon);
			Serial.print(", ");
			Serial.print(parse_result.waypoints[wp].ele);
			Serial.println("m");
		}
		Serial.print("  ");
		Serial.print(parse_result.totalRoutes);
		Serial.println(" routes");
		for (uint8_t r = 0; r < parse_result.totalRoutes; r++) {
			Serial.print("    ");
			Serial.print(parse_result.routes[r].name);
			Serial.print(" (");
			Serial.print(parse_result.routes[r].totalPoints);
			Serial.println(" points)");
			for (uint8_t wp = 0; wp < parse_result.routes[r].totalPoints; wp++) {
				Serial.print("      ");
				Serial.print(parse_result.waypoints[wp].name);
				Serial.print(" @ ");
				Serial.print(parse_result.waypoints[wp].lat);
				Serial.print(", ");
				Serial.print(parse_result.waypoints[wp].lon);
				Serial.print(", ");
				Serial.print(parse_result.waypoints[wp].ele);
				Serial.println("m");
			}
		}
		gpxData = parse_result;
		return true;
	} else {
		// TODO: Display error to user (create appropriate method in GPXParser looking at _error, _line, and _col)
		Serial.print("gpx_readFile error parsing GPX at line ");
		Serial.print(parser.line());
		Serial.print(" col ");
		Serial.print(parser.col());
		Serial.print(": ");
		Serial.println(parser.error());
		return false;
	}
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
