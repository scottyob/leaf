#ifndef kml_h
#define kml_h

const char KMLtrackHeader[] = R"--8<--8<--(<?xml version="1.0" encoding="UTF-8"?>
<kml xmlns="http://www.opengis.net/kml/2.2">
  <Document>
    <name>Paths</name>
    <description>Examples of paths. Note that the tessellate tag is by default
      set to 0. If you want to create tessellated lines, they must be authored
      (or edited) directly in KML.</description>
    <Style id="yellowLineGreenPoly">
      <LineStyle>
        <color>7f00ffff</color>
        <width>4</width>
      </LineStyle>
      <PolyStyle>
        <color>7f00ff00</color>
      </PolyStyle>
    </Style>
    <Placemark>
      <name>Absolute Extruded</name>
      <description>Transparent green wall with yellow outlines</description>
      <styleUrl>#yellowLineGreenPoly</styleUrl>
      <LineString>
        <extrude>1</extrude>
        <tessellate>1</tessellate>
        <altitudeMode>absolute</altitudeMode>
        <coordinates>
)--8<--8<--";

// Coordinates go here in the form of:
// LONG_DEC, LAT_DEC, ALT
// LONG_DEC, LAT_DEC, ALT
// .. etc.  Note no comma after ALT

const char KMLtrackFooter[] = R"--8<--8<--(        </coordinates>
      </LineString>
    </Placemark>
  </Document>
</kml>)--8<--8<--";





#endif