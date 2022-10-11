/** encode_polyline
  * takes a array of doubles (lat1, long1, lat2, long2, etc.) and a number
  * of point pairs and encodes the polyline to a char buffer out.
 **/
void encode_polyline(double* a, int points, char* out);
void encode_polyline_threaded(double* a, int points, int threads, char* out);
