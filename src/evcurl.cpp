#include <iostream>
#include <string>

#include "evcurl.h"


EVCurl::EVCurl(struct event_base *evbase) {
	evcurlh = evcurl_init(evbase);
}


EVCurl::~EVCurl() {
	evcurl_cleanup(evcurlh);
}


EVCurl * EVCurl::init(struct event_base *evbase) {
	return new EVCurl(evbase);
}


EVCurlTask * EVCurl::download(const char *url, EVCurl::callback_t cb, void *userdata) {
	EVCurlTask *evtask = new EVCurlTask(cb, userdata);

	evtask->evtaskh = evcurl_download(evcurlh, url, EVCurlTask::complete, evtask);

	return evtask;
}


EVCurlTask * EVCurl::upload(const char *url, EVCurl::callback_t cb, void *userdata) {
	EVCurlTask *evtask = new EVCurlTask(cb, userdata);
   
	evtask->evtaskh = evcurl_upload(evcurlh, url, EVCurlTask::complete, evtask);

	return evtask;
}


EVCurlTask::EVCurlTask(EVCurl::callback_t cb, void *userdata)
	: cb_(cb)
	, userdata_(userdata) {
}


const char * EVCurlTask::url(void) {
	return evcurl_get_url(evtaskh);
}


void EVCurlTask::setSSLVersion(evcurl_ssl_t version) {
	evcurl_set_ssl_version(evtaskh, version);
}


void EVCurlTask::checkHostAndPeer(const int enable) {
	evcurl_check_host_and_peer(evtaskh, enable);
}


void EVCurlTask::setHeader(const char *header) {
	evcurl_set_header(evtaskh, header);
}


int EVCurlTask::cancel(void) {
	return evcurl_cancel(evtaskh);
}


int EVCurlTask::perform(void) {
	return evcurl_perform(evtaskh);
}


void EVCurlTask::complete(struct evcurl_task_s *evtaskh, CURLcode result, void *userdata) {
	EVCurlTask *task = (EVCurlTask *) userdata;

	(void) evtaskh;

	task->cb_(task, result, task->userdata_);

	delete task;
}

