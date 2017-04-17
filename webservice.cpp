#include <sstream>
#include <tins/tins.h>

#include "webservice.h"

using namespace std;
using namespace Tins;

int WebService::on_request(void * cls,
		    struct MHD_Connection * connection,
		    const char * url,
		    const char * method,
                    const char * version,
		    const char * upload_data,
		    size_t * upload_data_size,
                    void ** ptr) {
  static int dummy;
  struct MHD_Response * response;
  int ret;

  if (string("GET") != method)
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

  WebService* webservice = static_cast<WebService*>(cls);
    
  const string& data = webservice->functor(url);
    
  *ptr = NULL; /* clear context pointer */
  response = MHD_create_response_from_buffer (data.size(),
                                              const_cast<char*>(data.c_str()),
  					      MHD_RESPMEM_PERSISTENT);
  ret = MHD_queue_response(connection,
			   MHD_HTTP_OK,
			   response);
  MHD_destroy_response(response);
  return ret;
}
