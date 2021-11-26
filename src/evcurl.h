#ifndef __EVCURL__EVCURL_H__
#define __EVCURL__EVCURL_H__

#include <event2/event.h>
#include <curl/curl.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	EVCURL_DEFAULT_SSL_VERSION,
	EVCURL_SSL_VERSION_2,
	EVCURL_SSL_VERSION_3,
	EVCURL_SSL_VERSION_2_AND_3,
	EVCURL_TLS_VERSION_1_0,
	EVCURL_TLS_VERSION_1_1,
	EVCURL_TLS_VERSION_1_2,
	EVCURL_UNKNOWN
} evcurl_ssl_t;


typedef struct evcurl_s evcurl_t;
typedef struct evcurl_task_s evcurl_task_t;


evcurl_t * evcurl_init(struct event_base *evbase);
int evcurl_cleanup(evcurl_t *evcurlh);

typedef void (*evcurl_callback_t)(evcurl_task_t *evtaskh, CURLcode result, void *userdata);

evcurl_task_t * evcurl_download(evcurl_t *evcurlh, const char *url, evcurl_callback_t cb, void *userdata);
evcurl_task_t * evcurl_upload(evcurl_t *evcurlh, const char *url, evcurl_callback_t cb, void *userdata);

CURLM * evcurl_get_handle(evcurl_t *evcurlh);
CURL * evcurl_get_task_handle(evcurl_task_t *evtaskh);

const char * evcurl_get_url(evcurl_task_t *evtaskh);

void evcurl_set_ssl_version(evcurl_task_t *evtaskh, evcurl_ssl_t version);
void evcurl_check_host_and_peer(evcurl_task_t *evtaskh, const int enable);

void evcurl_set_header(evcurl_task_t *evtaskh, const char *header);

int evcurl_cancel(evcurl_task_t *evtaskh);
int evcurl_perform(evcurl_task_t *evtaskh);

#define evcurl_setmopt(evcurlh, tag, ... ) \
	curl_multi_setopt(evcurl_get_handle(evcurlh), tag, ## __VA_ARGS__)

#define evcurl_setopt(evtaskh, tag, ... ) \
	curl_easy_setopt(evcurl_get_task_handle(evtaskh), tag, ## __VA_ARGS__)

#define evcurl_getinfo(evtaskh, tag, ... ) \
	curl_easy_getinfo(evcurl_get_task_handle(evtaskh), tag, ## __VA_ARGS__)


#ifdef __cplusplus
}
#endif


#ifdef __cplusplus
class EVCurl;
class EVCurlTask;


class EVCurl {
public:
	static EVCurl * init(struct event_base *evbase);
	~EVCurl();

	typedef void (*callback_t)(EVCurlTask *evtaskh, CURLcode result, void *userdata);

	template <typename T> CURLMcode setOption(CURLMoption option, T arg) {
		return evcurl_setmopt(evcurlh, option, arg);
	}

	EVCurlTask * download(const char *url, callback_t cb, void *userdata);
	EVCurlTask * upload(const char *url, callback_t cb, void *userdata);

private:
	EVCurl(struct event_base *evbase);

	struct evcurl_s *evcurlh;
};


class EVCurlTask {
public:
	const char * url(void);

	void setSSLVersion(evcurl_ssl_t version);
	void checkHostAndPeer(const int enable);
	void setHeader(const char *header);
	template <typename T> CURLcode setOption(CURLoption option, T arg) {
		return evcurl_setopt(evtaskh, option, arg);
	}

	int cancel(void);
	int perform(void);
	
protected:
	static void complete(struct evcurl_task_s *evtaskh, CURLcode result, void *userdata);

private:
	EVCurlTask(EVCurl::callback_t cb, void *userdata);

	struct evcurl_task_s *evtaskh;

	friend EVCurlTask * EVCurl::download(const char *url, EVCurl::callback_t cb, void *userdata);
	friend EVCurlTask * EVCurl::upload(const char *url, EVCurl::callback_t cb, void *userdata);

	EVCurl::callback_t cb_;
	void *userdata_;
};

#endif

#endif

