#define _XOPEN_SOURCE
#define _DEFAULT_SOURCE
#include "time.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"


int main() {
	// from GPX (exiftool) - UTC
	//char *t1 = "2021:10:03 17:11:11.410";
	// GOPRO1860
	char *t1 = "2020:12:13 08:55:48.215";
	// from MP4 - local time
	//char *t2 = "2021-10-03T19:12:01.000000Z";
	// GOPRO1860 
	char *t2 = "2020-12-13T09:56:27.000000Z";
	// from GPX (garmin connect) UTC!!!
	// 1: 2020-07-28T07:04:43.000Z
	// 2: GOPRO1860 : 2020-12-13T08:07:30.000Z <= 9h07



	char buf[128];

	time_t v;
	struct tm t;

	time_t now = time(NULL);
	localtime_r(&now, &t);

	int isdst = t.tm_isdst;


	// t1
	memset(&t, 0, sizeof(t));
	strptime(t1, "%Y:%m:%d %H:%M:%S.", &t);

	strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S %Z", &t);

//	t.tm_isdst = isdst;

	printf("T1: %s\n", t1);
	printf("  => : %s\n", buf);

//	v = mktime(&t);
	v = timegm(&t);

	localtime_r(&v, &t);

	strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S %Z", &t);

	printf("  => Local: %s\n", buf);

	gmtime_r(&v, &t);

	strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S %Z", &t);

	printf("  => UTC: %s\n", buf);


	// t2
	memset(&t, 0, sizeof(t));
	strptime(t2, "%Y-%m-%dT%H:%M:%S.", &t);

	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S %Z", &t);

	t.tm_isdst = isdst;

	printf("T2: %s\n", t2);
	printf("  => : %s\n", buf);

	v = mktime(&t);
	v = timelocal(&t);

	localtime_r(&v, &t);

	strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S %Z", &t);

	printf("  => Local: %s\n", buf);

	gmtime_r(&v, &t);

	strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S %Z", &t);

	printf("  => UTC: %s\n", buf);

	exit(EXIT_SUCCESS);
}
