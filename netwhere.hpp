#ifndef __NETWHERE_H__
#define __NETWHERE_H__

#include <tins/tins.h>

#include "flow.hpp"
#include "host.hpp"
#include "reader_writer_exclusion.hpp"
#include "webservice.hpp"

class NetWhere {
public:
  NetWhere() {}
  void start();

  bool handle_packet(const Tins::PDU &pdu);
  std::string handle_request(const std::string& url);

private:
  FlowMap flows;
  HostMap hosts;
  ReaderWriterExclusion rw_exclusion;

  std::string full_stats();
  std::string stats_since_counter(int counter);
};

#endif
