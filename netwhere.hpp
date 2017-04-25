#ifndef __NETWHERE_H__
#define __NETWHERE_H__

#include <tins/tins.h>

#include "host.hpp"
#include "reader_writer_exclusion.hpp"
#include "webservice.hpp"

class NetWhere {
public:
  NetWhere(const std::string interface, Tins::IPv4Range internal_range)
    : interface(interface), internal_range(internal_range) {}
  void start();

  bool handle_packet(const Tins::PDU &pdu);
  std::string handle_request(const std::string& url);

private:
  HostMap hosts;
  ReaderWriterExclusion rw_exclusion;
  const std::string interface;
  Tins::IPv4Range internal_range;

  std::string full_stats();
  std::string stats_since_counter(int counter);
};

#endif
