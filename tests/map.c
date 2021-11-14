#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#define TILESIZE 256
#define MAX_ZOOM 20
#define MIN_ZOOM 0

#define MAX_TILE_ZOOM_OFFSET 10
#define MIN_TILE_ZOOM_OFFSET 0

#define OSM_REPO_URI        "https://tile.openstreetmap.org/#Z/#X/#Y.png"
#define OSM_MIN_ZOOM        1
#define OSM_MAX_ZOOM        20
#define OSM_IMAGE_FORMAT    "png"

#define URI_MARKER_X    "#X"
#define URI_MARKER_Y    "#Y"
#define URI_MARKER_Z    "#Z"
#define URI_MARKER_S    "#S"
#define URI_MARKER_Q    "#Q"
#define URI_MARKER_Q0   "#W"
#define URI_MARKER_YS   "#U"
#define URI_MARKER_R    "#R"

#define URI_HAS_X   (1 << 0)
#define URI_HAS_Y   (1 << 1)
#define URI_HAS_Z   (1 << 2)
#define URI_HAS_S   (1 << 3)
#define URI_HAS_Q   (1 << 4)
#define URI_HAS_Q0  (1 << 5)
#define URI_HAS_YS  (1 << 6)
#define URI_HAS_R   (1 << 7)
//....
#define URI_FLAG_END (1 << 8)

/* equatorial radius in meters */
#define OSM_EQ_RADIUS   (6378137.0)





typedef enum {
    OSM_GPS_MAP_SOURCE_NULL,
    OSM_GPS_MAP_SOURCE_OPENSTREETMAP,
    OSM_GPS_MAP_SOURCE_OPENSTREETMAP_RENDERER,
    OSM_GPS_MAP_SOURCE_OPENAERIALMAP,
    OSM_GPS_MAP_SOURCE_MAPS_FOR_FREE,
    OSM_GPS_MAP_SOURCE_OPENCYCLEMAP,
    OSM_GPS_MAP_SOURCE_OPENTOPOMAP,
    OSM_GPS_MAP_SOURCE_OSM_PUBLIC_TRANSPORT,
    OSM_GPS_MAP_SOURCE_GOOGLE_STREET,
    OSM_GPS_MAP_SOURCE_GOOGLE_SATELLITE,
    OSM_GPS_MAP_SOURCE_GOOGLE_HYBRID,
    OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_STREET,
    OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_SATELLITE,
    OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_HYBRID,
    OSM_GPS_MAP_SOURCE_OSMC_TRAILS,

    OSM_GPS_MAP_SOURCE_LAST
} OsmGpsMapSource_t;


const char * osm_gps_map_source_get_friendly_name(OsmGpsMapSource_t source) {
	switch (source) {
	case OSM_GPS_MAP_SOURCE_NULL:
		return "None";
	case OSM_GPS_MAP_SOURCE_OPENSTREETMAP:
		return "OpenStreetMap I";
	case OSM_GPS_MAP_SOURCE_OPENSTREETMAP_RENDERER:
		return "OpenStreetMap II";
	case OSM_GPS_MAP_SOURCE_OPENAERIALMAP:
		return "OpenAerialMap";
	case OSM_GPS_MAP_SOURCE_OPENCYCLEMAP:
		return "OpenCycleMap";
	case OSM_GPS_MAP_SOURCE_OPENTOPOMAP: 
		return "OpenTopoMap";
	case OSM_GPS_MAP_SOURCE_OSM_PUBLIC_TRANSPORT:
		return "Public Transport";
	case OSM_GPS_MAP_SOURCE_OSMC_TRAILS:
		return "OSMC Trails";
	case OSM_GPS_MAP_SOURCE_MAPS_FOR_FREE:
		return "Maps-For-Free";
	case OSM_GPS_MAP_SOURCE_GOOGLE_STREET:
		return "Google Maps";
	case OSM_GPS_MAP_SOURCE_GOOGLE_SATELLITE:
		return "Google Satellite";
	case OSM_GPS_MAP_SOURCE_GOOGLE_HYBRID:
		return "Google Hybrid";
	case OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_STREET:
		return "Virtual Earth";
	case OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_SATELLITE:
		return "Virtual Earth Satellite";
	case OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_HYBRID:
		return "Virtual Earth Hybrid";
	case OSM_GPS_MAP_SOURCE_LAST:
	default:
		return NULL;
	}

	return NULL;
}


const char * osm_gps_map_source_get_copyright(OsmGpsMapSource_t source) {
	switch (source) {
	case OSM_GPS_MAP_SOURCE_OPENSTREETMAP:
		// https://www.openstreetmap.org/copyright
		return "© OpenStreetMap contributors";
	case OSM_GPS_MAP_SOURCE_OPENCYCLEMAP:
		// http://www.thunderforest.com/terms/
		return "Maps © thunderforest.com, Data © osm.org/copyright";
	case OSM_GPS_MAP_SOURCE_OSM_PUBLIC_TRANSPORT:
		return "Maps © ÖPNVKarte, Data © OpenStreetMap contributors";
	case OSM_GPS_MAP_SOURCE_MAPS_FOR_FREE:
		return "Maps © Maps-For-Free";
	case OSM_GPS_MAP_SOURCE_OPENTOPOMAP:
		return "© OpenTopoMap (CC-BY-SA)";
	case OSM_GPS_MAP_SOURCE_GOOGLE_STREET:
		return "Map provided by Google";
	case OSM_GPS_MAP_SOURCE_GOOGLE_SATELLITE:
		return "Map provided by Google ";
	case OSM_GPS_MAP_SOURCE_GOOGLE_HYBRID:
		return "Map provided by Google";
	case OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_STREET:
		return "Map provided by Microsoft";
	case OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_SATELLITE:
		return "Map provided by Microsoft";
	case OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_HYBRID:
		return "Map provided by Microsoft";
	default:
		return NULL;
	}

	return NULL;
}


//http://www.internettablettalk.com/forums/showthread.php?t=5209
//https://garage.maemo.org/plugins/scmsvn/viewcvs.php/trunk/src/maps.c?root=maemo-mapper&view=markup
//http://www.ponies.me.uk/maps/GoogleTileUtils.java
//http://www.mgmaps.com/cache/MapTileCacher.perl
const char * osm_gps_map_source_get_repo_uri(OsmGpsMapSource_t source) {
	switch (source) {
	case OSM_GPS_MAP_SOURCE_NULL:
		return "none://";
	case OSM_GPS_MAP_SOURCE_OPENSTREETMAP:
		return OSM_REPO_URI;
	case OSM_GPS_MAP_SOURCE_OPENAERIALMAP:
		// OpenAerialMap is down, offline till furthur notice
		// http://openaerialmap.org/pipermail/talk_openaerialmap.org/2008-December/000055.html
		return NULL;
	case OSM_GPS_MAP_SOURCE_OPENSTREETMAP_RENDERER:
		// The Tile@Home serverhas been shut down.
		// return "http://tah.openstreetmap.org/Tiles/tile/#Z/#X/#Y.png";
		return NULL;
	case OSM_GPS_MAP_SOURCE_OPENCYCLEMAP:
		// return "http://c.andy.sandbox.cloudmade.com/tiles/cycle/#Z/#X/#Y.png";
		return "http://b.tile.opencyclemap.org/cycle/#Z/#X/#Y.png";
	case OSM_GPS_MAP_SOURCE_OSM_PUBLIC_TRANSPORT:
		return "http://tile.xn--pnvkarte-m4a.de/tilegen/#Z/#X/#Y.png";
	case OSM_GPS_MAP_SOURCE_OSMC_TRAILS:
		// Appears to be shut down
		return NULL;
	case OSM_GPS_MAP_SOURCE_MAPS_FOR_FREE:
		return "https://maps-for-free.com/layer/relief/z#Z/row#Y/#Z_#X-#Y.jpg";
	case OSM_GPS_MAP_SOURCE_OPENTOPOMAP:
		return "https://a.tile.opentopomap.org/#Z/#X/#Y.png";
	case OSM_GPS_MAP_SOURCE_GOOGLE_STREET:
		return "http://mt#R.google.com/vt/lyrs=m&hl=en&x=#X&s=&y=#Y&z=#Z";
	case OSM_GPS_MAP_SOURCE_GOOGLE_HYBRID:
		return "http://mt#R.google.com/vt/lyrs=y&hl=en&x=#X&s=&y=#Y&z=#Z";
	case OSM_GPS_MAP_SOURCE_GOOGLE_SATELLITE:
		return "http://mt#R.google.com/vt/lyrs=s&hl=en&x=#X&s=&y=#Y&z=#Z";
	case OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_STREET:
		return "http://a#R.ortho.tiles.virtualearth.net/tiles/r#W.jpeg?g=50";
	case OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_SATELLITE:
		return "http://a#R.ortho.tiles.virtualearth.net/tiles/a#W.jpeg?g=50";
	case OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_HYBRID:
		return "http://a#R.ortho.tiles.virtualearth.net/tiles/h#W.jpeg?g=50";
	case OSM_GPS_MAP_SOURCE_LAST:
	default:
		return NULL;
	}

	return NULL;
}

const char * osm_gps_map_source_get_image_format(OsmGpsMapSource_t source) {
	switch (source) {
	case OSM_GPS_MAP_SOURCE_NULL:
	case OSM_GPS_MAP_SOURCE_OPENSTREETMAP:
	case OSM_GPS_MAP_SOURCE_OPENSTREETMAP_RENDERER:
	case OSM_GPS_MAP_SOURCE_OPENCYCLEMAP:
	case OSM_GPS_MAP_SOURCE_OSM_PUBLIC_TRANSPORT:
	case OSM_GPS_MAP_SOURCE_OSMC_TRAILS:
	case OSM_GPS_MAP_SOURCE_OPENTOPOMAP:
		return "png";
	case OSM_GPS_MAP_SOURCE_OPENAERIALMAP:
	case OSM_GPS_MAP_SOURCE_GOOGLE_STREET:
	case OSM_GPS_MAP_SOURCE_GOOGLE_HYBRID:
	case OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_STREET:
	case OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_SATELLITE:
	case OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_HYBRID:
	case OSM_GPS_MAP_SOURCE_MAPS_FOR_FREE:
	case OSM_GPS_MAP_SOURCE_GOOGLE_SATELLITE:
		return "jpg";
	case OSM_GPS_MAP_SOURCE_LAST:
	default:
		return "bin";
	}

	return "bin";
}


int osm_gps_map_source_get_min_zoom(OsmGpsMapSource_t source) {
	(void) source;

	return 1;
}

int osm_gps_map_source_get_max_zoom(OsmGpsMapSource_t source) {
	switch (source) {
	case OSM_GPS_MAP_SOURCE_NULL:
		return 18;
	case OSM_GPS_MAP_SOURCE_OPENSTREETMAP:
		return 19;
	case OSM_GPS_MAP_SOURCE_OPENCYCLEMAP:
		return 18;
	case OSM_GPS_MAP_SOURCE_OSM_PUBLIC_TRANSPORT:
		return OSM_MAX_ZOOM;
	case OSM_GPS_MAP_SOURCE_OPENSTREETMAP_RENDERER:
	case OSM_GPS_MAP_SOURCE_OPENAERIALMAP:
	case OSM_GPS_MAP_SOURCE_OPENTOPOMAP:
		return 17;
	case OSM_GPS_MAP_SOURCE_GOOGLE_STREET:
	case OSM_GPS_MAP_SOURCE_GOOGLE_SATELLITE:
	case OSM_GPS_MAP_SOURCE_GOOGLE_HYBRID:
		return OSM_MAX_ZOOM;
	case OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_STREET:
	case OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_SATELLITE:
	case OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_HYBRID:
		return 19;
	case OSM_GPS_MAP_SOURCE_OSMC_TRAILS:
		return 15;
	case OSM_GPS_MAP_SOURCE_MAPS_FOR_FREE:
		return 11;
	case OSM_GPS_MAP_SOURCE_LAST:
	default:
		return 17;
	}
	return 17;
}

int osm_gps_map_source_is_valid(OsmGpsMapSource_t source)
{
	return osm_gps_map_source_get_repo_uri(source) != NULL;
}


static void
source_dump(void)
{
	int i;

	printf("Valid map sources:\n");

	for(i=OSM_GPS_MAP_SOURCE_NULL; i <= OSM_GPS_MAP_SOURCE_LAST; i++) {
		const char *name = osm_gps_map_source_get_friendly_name(i);
		const char *uri = osm_gps_map_source_get_repo_uri(i);
		if (uri != NULL)
			printf("\t%d:\t%s\n",i,name);
	}
}


typedef struct OsmGpsMap {
	int min_zoom;
	int max_zoom;

	int uri_format;
	const char *repo_uri;
	const char *image_format;

	int is_google;
} OsmGpsMap;





/*
 * Description:
 *   Find and replace text within a string.
 *
 * Parameters:
 *   src  (in) - pointer to source string
 *   from (in) - pointer to search text
 *   to   (in) - pointer to replacement text
 *
 * Returns:
 *   Returns a pointer to dynamically-allocated memory containing string
 *   with occurences of the text pointed to by 'from' replaced by with the
 *   text pointed to by 'to'.
 */
static char * replace_string(const char *src, const char *from, const char *to) {
	size_t tolen   = strlen(to);
	size_t fromlen = strlen(from);
	size_t size    = strlen(src) + 1;

	// Allocate the first chunk with enough for the original string.
	char *value = malloc(size);

	// We need to return 'value', so let's make a copy to mess around with.
	char *dst = value;

	if (value == NULL)
		return NULL;

	for (;;) {
		// Try to find the search text.
		const char *match = strstr(src, from);

		if (match != NULL) {
			char *temp;
			// Find out how many characters to copy up to the 'match'.
			size_t count = match - src;

			// Calculate the total size the string will be after the
			// replacement is performed.
			size += tolen - fromlen;

			temp = realloc(value, size);
			if (temp == NULL) {
				free(value);
				return NULL;
			}

			// we'll want to return 'value' eventually, so let's point it
			// to the memory that we are now working with.
			// And let's not forget to point to the right location in
			// the destination as well.
			dst = temp + (dst - value);
			value = temp;

			// Copy from the source to the point where we matched. Then
			// move the source pointer ahead by the amount we copied. And
			// move the destination pointer ahead by the same amount.
			memmove(dst, src, count);
			src += count;
			dst += count;

			// Now copy in the replacement text 'to' at the position of the
			// match. Adjust the source pointer by the text we replaced.
			// Adjust the destination pointer by the amount of replacement text.
			memmove(dst, to, tolen);
			src += fromlen;
			dst += tolen;
		}
		else {
			// Copy any remaining part of the string. This includes the null
			// termination character.
			strcpy(dst, src);
			break;
		}
	}

	return value;
}


static void map_convert_coords_to_quadtree_string(OsmGpsMap *map, int x, int y, int zoomlevel,
		char *buffer, const char initial,
		const char *const quadrant) {
	int n;
	char *ptr = buffer;

	(void) map;

	if (initial)
		*ptr++ = initial;

	for (n = zoomlevel-1; n >= 0; n--) {
		int xbit = (x >> n) & 1;
		int ybit = (y >> n) & 1;
		*ptr++ = quadrant[xbit + 2 * ybit];
	}

	*ptr++ = '\0';
}


static void inspect_map_uri(OsmGpsMap *map) {
	map->uri_format = 0;
	map->is_google = 0;

	if (strstr(map->repo_uri, URI_MARKER_X))
		map->uri_format |= URI_HAS_X;

	if (strstr(map->repo_uri, URI_MARKER_Y))
		map->uri_format |= URI_HAS_Y;

	if (strstr(map->repo_uri, URI_MARKER_Z))
		map->uri_format |= URI_HAS_Z;

	if (strstr(map->repo_uri, URI_MARKER_S))
		map->uri_format |= URI_HAS_S;

	if (strstr(map->repo_uri, URI_MARKER_Q))
		map->uri_format |= URI_HAS_Q;

	if (strstr(map->repo_uri, URI_MARKER_Q0))
		map->uri_format |= URI_HAS_Q0;

	if (strstr(map->repo_uri, URI_MARKER_YS))
		map->uri_format |= URI_HAS_YS;

	if (strstr(map->repo_uri, URI_MARKER_R))
		map->uri_format |= URI_HAS_R;

	if (strstr(map->repo_uri, "google.com"))
		map->is_google = 1;

//	g_debug("URI Format: 0x%X (google: %X)", priv->uri_format, priv->is_google);
}


static char * replace_map_uri(OsmGpsMap *map, const char *uri, int zoom, int x, int y)
{
    char *url;
    unsigned int i;

    char location[22];

    i = 1;
    url = strdup(uri);

    while (i < URI_FLAG_END) {
        char s[16];
        char *old;

        old = url;

        switch (i & map->uri_format) {
		case URI_HAS_X:
			snprintf(s, sizeof(s), "%d", x);
			url = replace_string(url, URI_MARKER_X, s);
			break;
		case URI_HAS_Y:
			snprintf(s, sizeof(s), "%d", y);
			url = replace_string(url, URI_MARKER_Y, s);
			break;
		case URI_HAS_Z:
			snprintf(s, sizeof(s), "%d", zoom);
			url = replace_string(url, URI_MARKER_Z, s);
			break;
		case URI_HAS_S:
			snprintf(s, sizeof(s), "%d", map->max_zoom-zoom);
			url = replace_string(url, URI_MARKER_S, s);
			break;
		case URI_HAS_Q:
			map_convert_coords_to_quadtree_string(map,x,y,zoom,location,'t',"qrts");
			url = replace_string(url, URI_MARKER_Q, location);
			break;
		case URI_HAS_Q0:
			map_convert_coords_to_quadtree_string(map,x,y,zoom,location,'\0', "0123");
			url = replace_string(url, URI_MARKER_Q0, location);
			//g_debug("FOUND " URI_MARKER_Q0);
			break;
		case URI_HAS_YS:
			//              g_snprintf(s, sizeof(s), "%d", y);
			//              url = replace_string(url, URI_MARKER_YS, s);
//			g_warning("FOUND " URI_MARKER_YS " NOT IMPLEMENTED");
			//            retval = g_strdup_printf(repo->url,
			//                    tilex,
			//                    (1 << (MAX_ZOOM - zoom)) - tiley - 1,
			//                    zoom - (MAX_ZOOM - 17));
			break;
		case URI_HAS_R:
			snprintf(s, sizeof(s), "%d", (int) (random() % 4)); //g_random_int_range(0, 4));
			url = replace_string(url, URI_MARKER_R, s);
			break;
		default:
			break;
        }

        if (old != url)
            free(old);

        i = (i << 1);
    }

    return url;
}



int
lat2pixel(  int zoom,
            float lat)
{
    float lat_m;
    int pixel_y;

    double latrad = lat * M_PI/180.0;

    lat_m = atanh(sin(latrad));

    /* the formula is
     *
     * some more notes
     * http://manialabs.wordpress.com/2013/01/26/converting-latitude-and-longitude-to-map-tile-in-mercator-projection/
     *
     * pixel_y = -(2^zoom * TILESIZE * lat_m) / 2PI + (2^zoom * TILESIZE) / 2
     */
    pixel_y = -(int)( (lat_m * TILESIZE * (1 << zoom) ) / (2*M_PI)) +
        ((1 << zoom) * (TILESIZE/2) );


    return pixel_y;
}


int
lon2pixel(  int zoom,
            float lon)
{
    int pixel_x;

    double lonrad = lon * M_PI/180.0;

    /* the formula is
     *
     * pixel_x = (2^zoom * TILESIZE * lon) / 2PI + (2^zoom * TILESIZE) / 2
     */
    pixel_x = (int)(( lonrad * TILESIZE * (1 << zoom) ) / (2*M_PI)) +
        ( (1 << zoom) * (TILESIZE/2) );
    return pixel_x;
}


int lon2tilex(double lon, int z)
{
	return (int)(floor((lon + 180.0) / 360.0 * (1 << z)));
}

int lat2tiley(double lat, int z)
{
    double latrad = lat * M_PI/180.0;
	return (int)(floor((1.0 - asinh(tan(latrad)) / M_PI) / 2.0 * (1 << z)));
}



int main(int argc, char *argv[], char *envp[]) {
	int zoom;

	char *s;

	const char *uri;

	// <trkpt lat="49.1485011" lon="1.8859783">
	float lat = 49.1485011;
	float lon = 1.8859783;

	OsmGpsMap map;
	OsmGpsMapSource_t map_source; // = OSM_GPS_MAP_SOURCE_OPENSTREETMAP;

	(void) envp;

	if (argc != 3) {
		source_dump();
		exit(EXIT_FAILURE);
	}

	map_source = atoi(argv[1]);
	zoom = atoi(argv[2]);

	uri = osm_gps_map_source_get_repo_uri(map_source);

	map.repo_uri = uri;
	map.image_format = osm_gps_map_source_get_image_format(map_source);
	map.min_zoom = osm_gps_map_source_get_min_zoom(map_source);
	map.max_zoom = osm_gps_map_source_get_max_zoom(map_source);

	inspect_map_uri(&map);

    int x = lon2pixel(zoom, lon);
    int y = lat2pixel(zoom, lat);

//    int x = lon2tilex(lon, zoom); //lon2pixel(zoom, lon);
//    int y = lat2tiley(lat, zoom); //lat2pixel(zoom, lat);

	int tile_x0 =  floorf((float) x / (float) TILESIZE);                                                
	int tile_y0 =  floorf((float) y / (float) TILESIZE);  
//	int tile_x0 = x;
//	int tile_y0 = y;

	s = replace_map_uri(&map, uri, zoom, tile_x0, tile_y0);

	printf("lat: %f / lon: %f\n", lat, lon);
	printf("URI: %s\n", s);
	printf("x: %d / y: %d\n", x % TILESIZE, y % TILESIZE);

	exit(EXIT_SUCCESS);
}

