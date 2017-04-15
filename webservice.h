#include <microhttpd.h>
#include <stdexcept>
#include <string>

class WebService {
 public:
  WebService() {
    daemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,
			 8080,
			 NULL,
			 NULL,
			 &handle_request,
			 NULL,
			 MHD_OPTION_END);
    if (daemon == NULL)
      throw std::runtime_error("MHD_start_daemon");
  }

  ~WebService() {
    MHD_stop_daemon(daemon);
  }
  static int handle_request(void * cls, struct MHD_Connection * connection, const char * url, const char * method, const char * version,
			    const char * upload_data, size_t * upload_data_size,
			    void ** ptr);
 private:
  MHD_Daemon* daemon;
};
