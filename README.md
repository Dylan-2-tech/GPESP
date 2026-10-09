# GPESP
GPESP is a project of a GPS for bikes using a ESP32 (GPS + ESP = GPESP... genius huh?)

## Requirements
In order to compile the program in the ESP32, there are a few libraries that you need to install via the library manage in Arduino IDE:
-Adafruit SSD1306 (by Adafruit)
-wifiManager (By tzapu)
-ArduinoJson (By Benoit Blanchon)

## Address search

The HTTPS map page supports searching for departure and arrival addresses in
France. Suggestions appear after typing at least three characters. Each
search is sent to the ESP32 `/search` endpoint, which forwards it to
OpenRouteService geocoding and returns up to five matches. Select a match to
The first selected address becomes the arrival and the second becomes the departure. Once a marker exists, its Change button can be used to replace that address. Pressing Enter selects the first suggestion for the active slot. Each search is sent to the ESP32 `/search` endpoint, which forwards previews to ORS autocomplete and committed searches to ORS geocoding. Use **Let's go** after both markers are set.

Use a complete address where possible: street number, street name, postal
code, city, and `France`, for example `5 Avenue Anatole France, 75007 Paris,
France`. The search is restricted to France with the ORS `FRA` country
boundary.

The ORS key in `navigation/ors_api_key.h` must have access to both directions
and geocoding. Address search uses the ORS Pelias forward-geocoding service;
Leaflet remains responsible only for displaying the map and markers.
