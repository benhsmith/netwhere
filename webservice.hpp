#ifndef __WEBSERVICE_H__
#define __WEBSERVICE_H__

#include <microhttpd.h>
#include <stdexcept>
#include <string>
#include <iostream>

template <typename Functor>
class WebService {
 public:
  WebService(Functor &functor)
    : functor(functor) {}

  void start() {
    daemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,
			 8080,
			 NULL,
			 NULL,
			 &on_request,
			 this,
			 MHD_OPTION_END);
    if (daemon == NULL)
      throw std::runtime_error("MHD_start_daemon");
  }

  ~WebService() {
    MHD_stop_daemon(daemon);
  }

 private:
  MHD_Daemon* daemon;
  Functor functor;
 
  static int on_request(void * cls, struct MHD_Connection * connection, const char * url, const char * method, const char * version,
			    const char * upload_data, size_t * upload_data_size,
			    void ** ptr);
};

template <typename Functor>
int WebService<Functor>::on_request(void * cls,
		    struct MHD_Connection * connection,
		    const char * url,
		    const char * method,
                    const char * version,
		    const char * upload_data,
		    size_t * upload_data_size,
                    void ** ptr) {
  static int dummy;
  int ret;

  if (std::string("GET") != method)
    return MHD_NO; /* unexpected method */
  if (&dummy != *ptr)
    {
      /* The first time only the headers are valid,
         do not respond in the first round... */
      *ptr = &dummy;
      return MHD_YES;
    }
  if (0 != *upload_data_size)
    return MHD_NO; /* upload data in a GET!? */

  WebService<Functor>* webservice = static_cast<WebService<Functor>*>(cls);

  struct MHD_Response * response;
  *ptr = NULL; /* clear context pointer */

  try {
	const std::string& data = webservice->functor(url);

	response = MHD_create_response_from_buffer (data.size(),
												const_cast<char*>(data.c_str()),
												MHD_RESPMEM_MUST_COPY);
	MHD_add_response_header(response, "Content-Type", "application/json");
	MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");

	ret = MHD_queue_response(connection,
							 MHD_HTTP_OK,
							 response);
  } catch (const std::exception& e) {
	const std::string& data = e.what();
	response = MHD_create_response_from_buffer (data.size(),
												const_cast<char*>(data.c_str()),
												MHD_RESPMEM_MUST_COPY);
	MHD_add_response_header(response, "Content-Type", "text");

	ret = MHD_queue_response(connection,
							 MHD_HTTP_BAD_REQUEST,
							 response);
  }

  MHD_destroy_response(response);
  return ret;
}

#endif
