#ifndef ORS_API_CALL_H
#define ORS_API_CALL_H

// Fetches the current bike route from the ORS API and prints it to the
// Serial monitor. Requires WiFi to already be connected.
void getBikeRoute();

// Call every loop(). Internally tracks timing and calls getBikeRoute()
// once every requestInterval milliseconds.
void updateBikeRoute();

#endif
