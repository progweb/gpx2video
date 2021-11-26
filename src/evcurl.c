#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/ssl.h>

#include "evcurl.h"


struct evcurl_s {
	CURLM *curlmh;
	struct event_base *evbase;
	struct event *event;

	struct timeval delay;

	int still_running;
	int prev_running;
	int nbr_tasks;
};


struct evcurl_task_s {
	CURL *curl;
	evcurl_t *evcurlh;

	evcurl_ssl_t use_ssl_version;
	char error[CURL_ERROR_SIZE];

	struct curl_slist *slist;

	int is_running;
	char *url;
	FILE *fp;
	evcurl_callback_t cb;
	void *userdata;
};


CURLcode evcurl_ssl_ctx_callback(CURL *curl, void *sslctx, void *parm) {
	SSL_CTX *context = (SSL_CTX *) sslctx;

	evcurl_task_t *evtaskh = (evcurl_task_t *) parm;

	(void) curl;

	switch (evtaskh->use_ssl_version) {
	case EVCURL_SSL_VERSION_2:
//#ifndef OPENSSL_NO_SSL2
//		SSL_CTX_set_ssl_version(context, SSLv2_client_method());
//#endif
		break;

	case EVCURL_SSL_VERSION_3:
#ifndef OPENSSL_NO_SSL3_METHOD
		SSL_CTX_set_ssl_version(context, SSLv3_client_method());
#endif
		break;

	case EVCURL_SSL_VERSION_2_AND_3:
		SSL_CTX_set_ssl_version(context, SSLv23_client_method());
		break;

	case EVCURL_TLS_VERSION_1_0:
		SSL_CTX_set_ssl_version(context, TLSv1_client_method());
		break;

	case EVCURL_TLS_VERSION_1_1:
		SSL_CTX_set_ssl_version(context, TLSv1_1_client_method());
		break;

	case EVCURL_TLS_VERSION_1_2:
		SSL_CTX_set_ssl_version(context, TLSv1_2_client_method());
		break;

	case EVCURL_DEFAULT_SSL_VERSION:
	case EVCURL_UNKNOWN:
	default:
		break;
	}

	return CURLE_OK;
}


static evcurl_task_t * evcurl_task_new(evcurl_t *evcurlh, const char *url, evcurl_callback_t cb, void *userdata) {
	evcurl_task_t *evtaskh = NULL;

	if (url == NULL)
		goto url_error;

	// Create the download task
	evtaskh = (evcurl_task_t *) malloc(sizeof(evcurl_task_t));

	if (evtaskh == NULL)
		goto url_error;

	memset(evtaskh, 0, sizeof(evcurl_task_t));

	evtaskh->curl = curl_easy_init();

	if (evtaskh->curl == NULL)
		goto url_error;

	evtaskh->url = strdup(url);

	if (evtaskh->url == NULL)
		goto url_error;

	evtaskh->evcurlh = evcurlh;
	evtaskh->cb = cb;
	evtaskh->userdata = userdata;
	evtaskh->use_ssl_version = EVCURL_DEFAULT_SSL_VERSION;

	curl_easy_setopt(evtaskh->curl, CURLOPT_URL, url);
	curl_easy_setopt(evtaskh->curl, CURLOPT_HTTPAUTH , CURLAUTH_ANY);
	curl_easy_setopt(evtaskh->curl, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(evtaskh->curl, CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt(evtaskh->curl, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(evtaskh->curl, CURLOPT_PRIVATE, evtaskh);
	curl_easy_setopt(evtaskh->curl, CURLOPT_FAILONERROR, 1);
	curl_easy_setopt(evtaskh->curl, CURLOPT_ERRORBUFFER, evtaskh->error);

	curl_easy_setopt(evtaskh->curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_DEFAULT);
	curl_easy_setopt(evtaskh->curl, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(evtaskh->curl, CURLOPT_SSL_VERIFYHOST, 0);
	curl_easy_setopt(evtaskh->curl, CURLOPT_SSL_CTX_FUNCTION, evcurl_ssl_ctx_callback);
	curl_easy_setopt(evtaskh->curl, CURLOPT_SSL_CTX_DATA, evtaskh);

	return evtaskh;

url_error:
	if (evtaskh && (evtaskh->url != NULL))
		free(evtaskh->url);

	if (evtaskh && (evtaskh->curl != NULL))
		curl_easy_cleanup(evtaskh->curl);

	if (evtaskh)
		free(evtaskh);

	return NULL;
}


static void evcurl_task_delete(evcurl_task_t *evtaskh) {
	if (evtaskh == NULL)
		goto delete_error;

	if (evtaskh->is_running)
		evtaskh->evcurlh->nbr_tasks--;

	if (evtaskh->url != NULL)
		free(evtaskh->url);

	if (evtaskh->slist)
		curl_slist_free_all(evtaskh->slist);

	if (evtaskh->curl != NULL) {
		curl_easy_reset(evtaskh->curl);
		curl_easy_cleanup(evtaskh->curl);
	}

	free(evtaskh);

delete_error:
	return;
}


static void evcurl_check_task_running(evcurl_t *evcurlh, int force) {
	if (force || (evcurlh->prev_running > evcurlh->still_running)) {
		CURLMsg *msg;
		int msgs_left;

		CURL *easy;
		CURLcode result;

		CURLMcode rc;

		evcurl_task_t *evtaskh;
 
		do {
			easy = NULL;

			// Seek task completed
			while ((msg = curl_multi_info_read(evcurlh->curlmh, &msgs_left))) {
				if (msg->msg == CURLMSG_DONE) {
					easy = msg->easy_handle;
					result = msg->data.result;
					break;
				}
			}

			if (easy) {
				curl_easy_getinfo(easy, CURLINFO_PRIVATE, (void *) &evtaskh);

				rc = curl_multi_remove_handle(evcurlh->curlmh, easy);

				if (rc != CURLM_OK)
					fprintf(stderr, "curl_multi_remove_handle faild with code = %d\n", rc);

				if (evtaskh->cb)
					(evtaskh->cb)(evtaskh, result, evtaskh->userdata);

				evcurl_task_delete(evtaskh);
			}
		} while (easy);
	}

	evcurlh->prev_running = evcurlh->still_running;
}


static void evcurl_timeout_event_cb(evutil_socket_t sock, short which, void *arg) {
	evcurl_t *evcurlh = (evcurl_t *) arg;

	(void) sock;
	(void) which;

	curl_multi_socket_action(evcurlh->curlmh, CURL_SOCKET_TIMEOUT, 0, &(evcurlh->still_running));

	evcurl_check_task_running(evcurlh, 1);
}


static int evcurl_multi_timer_callback(CURLM *multi, long timeout_ms, void *userp) {
	evcurl_t *evcurlh = (evcurl_t *) userp;

	(void) multi;

	if (evcurlh->event != NULL) {
		event_del(evcurlh->event);
		event_free(evcurlh->event);

		evcurlh->event = NULL;
	}

	if (timeout_ms > -1) {
		memset(&evcurlh->delay, 0, sizeof(struct timeval));

		evcurlh->delay.tv_sec = timeout_ms / 1000;
		evcurlh->delay.tv_usec = (timeout_ms % 1000) * 1000;

		evcurlh->event = evtimer_new(evcurlh->evbase, evcurl_timeout_event_cb, evcurlh);
		evtimer_add(evcurlh->event, &evcurlh->delay);
	}

	return 0;
}


typedef struct evcurl_socket_s {
	evcurl_t *evcurlh;

	struct event_base *evbase;
	struct event *event;
} evcurl_socket_t;


static void evcurl_socket_event_cb(int sfd, short kind, void *userdata) {
	int action;

	evcurl_t *evcurlh;
	evcurl_socket_t *evs = (evcurl_socket_t *) userdata;

	evcurlh = evs->evcurlh;

	action = (kind & EV_READ ? CURL_POLL_IN : 0) 
		| (kind & EV_WRITE ? CURL_POLL_OUT : 0);

	curl_multi_socket_action(evcurlh->curlmh, sfd, action, &(evcurlh->still_running));

	evcurl_check_task_running(evcurlh, 0);

	// Last download, remove timer
	if (evcurlh->nbr_tasks == 0) {
		if (evcurlh->event != NULL) {
			event_del(evcurlh->event);
			event_free(evcurlh->event);
		}

		evcurlh->event = NULL;
	}
}


static void evcurl_socket_set(evcurl_socket_t *evs, CURL *e, curl_socket_t s, int action) {
	int event;

	(void) e;

	if (evs->event != NULL) {
		event_del(evs->event);
		event_free(evs->event);

		evs->event = NULL;
	}

	event = ((action & CURL_POLL_IN) ? EV_READ : 0)
		| ((action & CURL_POLL_OUT) ? EV_WRITE : 0)
		| EV_PERSIST;

	// Keep an eye on my socket file descriptor
	evs->event = event_new(evs->evbase, s, event, evcurl_socket_event_cb, evs);
	event_add(evs->event, NULL);
}

static void evcurl_socket_add(evcurl_t *evcurlh, CURL *e, curl_socket_t s, int action) {
	evcurl_socket_t *evs = NULL;

	evs = (evcurl_socket_t *) malloc(sizeof(evcurl_socket_t));

	if (evs == NULL)
		return;

	memset(evs, 0, sizeof(evcurl_socket_t));

	evs->evcurlh = evcurlh;
	evs->evbase = evcurlh->evbase;

	evcurl_socket_set(evs, e, s, action);

	curl_multi_assign(evcurlh->curlmh, s, evs);
}


static void evcurl_socket_remove(evcurl_socket_t *evs) {
	if (evs) {
		if (evs->event) {
			event_del(evs->event);
			event_free(evs->event);
		}

		free(evs);
	}
}

static int evcurl_socket_callback(CURL *e, curl_socket_t s, int action, void *userp, void *socketp) {
	evcurl_t *evcurlh = (evcurl_t *) userp;
	evcurl_socket_t *evs = (evcurl_socket_t *) socketp;

	switch (action) {
	case CURL_POLL_REMOVE:
		evcurl_socket_remove(evs);
		break;

	case CURL_POLL_INOUT:
	case CURL_POLL_OUT:
	case CURL_POLL_IN:
	case CURL_POLL_NONE:
	default:
		if (!evs)
			evcurl_socket_add(evcurlh, e, s, action);
		else
			evcurl_socket_set(evs, e, s, action);
		break;
	}

	return 0;
}


evcurl_t * evcurl_init(struct event_base *evbase) {
	evcurl_t *evcurlh = NULL;

	evcurlh = (evcurl_t *) malloc(sizeof(evcurl_t));

	if (evcurlh == NULL)
		goto init_error;

	memset(evcurlh, 0, sizeof(evcurl_t));

	evcurlh->evbase = evbase;
	evcurlh->curlmh = curl_multi_init();

	if (evcurlh->curlmh == NULL)
		goto init_error;

	curl_multi_setopt(evcurlh->curlmh, CURLMOPT_SOCKETFUNCTION, evcurl_socket_callback);
	curl_multi_setopt(evcurlh->curlmh, CURLMOPT_SOCKETDATA, evcurlh);
	curl_multi_setopt(evcurlh->curlmh, CURLMOPT_TIMERFUNCTION, evcurl_multi_timer_callback);
	curl_multi_setopt(evcurlh->curlmh, CURLMOPT_TIMERDATA, evcurlh);

	return evcurlh;

init_error:
	if (evcurlh && (evcurlh->curlmh != NULL))
		curl_multi_cleanup(evcurlh->curlmh);

	if (evcurlh != NULL)
		free(evcurlh);

	return NULL;
}


CURLM * evcurl_get_handle(evcurl_t *evcurlh) {
	return evcurlh->curlmh;
}


int evcurl_cleanup(evcurl_t *evcurlh) {
	if (evcurlh == NULL)
		goto cleanup_error;

	if (evcurlh->event != NULL) {
		event_del(evcurlh->event);
		event_free(evcurlh->event);
	}

	if (evcurlh->curlmh != NULL)
		curl_multi_cleanup(evcurlh->curlmh);

	free(evcurlh);

	return 0;

cleanup_error:
	return -1;
}


evcurl_task_t * evcurl_download(evcurl_t *evcurlh, const char *url, evcurl_callback_t cb, void *userdata) {
	evcurl_task_t *evtaskh;

	evtaskh = evcurl_task_new(evcurlh, url, cb, userdata);

	if (evtaskh == NULL)
		goto download_error;

	curl_easy_setopt(evtaskh->curl, CURLOPT_UPLOAD, 0L);

//	curl_easy_setopt(evtaskh->curl, CURLOPT_CONNECTTIMEOUT, 120);
//	curl_easy_setopt(evtaskh->curl, CURLOPT_TIMEOUT, 10000);
//	curl_easy_setopt(evtaskh->curl, CURLOPT_LOW_SPEED_LIMIT, 1);
//	curl_easy_setopt(evtaskh->curl, CURLOPT_LOW_SPEED_TIME, 1);

download_error:	
	return evtaskh;
}


evcurl_task_t * evcurl_upload(evcurl_t *evcurlh, const char *url, evcurl_callback_t cb, void *userdata) {
	evcurl_task_t *evtaskh;

	evtaskh = evcurl_task_new(evcurlh, url, cb, userdata);

	if (evtaskh == NULL)
		goto upload_error;

	curl_easy_setopt(evtaskh->curl, CURLOPT_UPLOAD, 1L);

//	curl_easy_setopt(evtaskh->curl, CURLOPT_CONNECTTIMEOUT, 120);
//	curl_easy_setopt(evtaskh->curl, CURLOPT_TIMEOUT, 10000);
//	curl_easy_setopt(evtaskh->curl, CURLOPT_LOW_SPEED_LIMIT, 1);
//	curl_easy_setopt(evtaskh->curl, CURLOPT_LOW_SPEED_TIME, 1);

upload_error:	
	return evtaskh;
}


CURL * evcurl_get_task_handle(evcurl_task_t *evtaskh) {
	return evtaskh->curl;
}


const char * evcurl_get_url(evcurl_task_t *evtaskh) {
	return evtaskh->url;
}


void evcurl_set_ssl_version(evcurl_task_t *evtaskh, evcurl_ssl_t version) {
	evtaskh->use_ssl_version = version;
}


void evcurl_check_host_and_peer(evcurl_task_t *evtaskh, const int enable) {
	curl_easy_setopt(evtaskh->curl, CURLOPT_SSL_VERIFYPEER, enable ? 1 : 0);
	curl_easy_setopt(evtaskh->curl, CURLOPT_SSL_VERIFYHOST, enable ? 1 : 0);
}


void evcurl_set_header(evcurl_task_t *evtaskh, const char *header) {
	evtaskh->slist = curl_slist_append(evtaskh->slist, (const char *) header);
}


int evcurl_cancel(evcurl_task_t *evtaskh) {
	evcurl_task_delete(evtaskh);

	return 0;
}


int evcurl_perform(evcurl_task_t *evtaskh) {
	CURLMcode rc;

	if (evtaskh->slist != NULL)
		curl_easy_setopt(evtaskh->curl, CURLOPT_HTTPHEADER, evtaskh->slist);

	rc = curl_multi_add_handle(evtaskh->evcurlh->curlmh, evtaskh->curl);

	if (rc != CURLM_OK)
		return -1;

	evtaskh->is_running = 1;
	evtaskh->evcurlh->nbr_tasks++;

	return 0;
}


