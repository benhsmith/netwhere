#ifndef __NETWHERE_H__
#define __NETWHERE_H__

#include <tins/tins.h>

#include "flow_collector.hpp"
#include "reader_writer_exclusion.hpp"
#include "webservice.hpp"

class NetWhere {
public:
  NetWhere(const std::string interface, Tins::IPv4Range hosts_range)
    : _interface(interface), _collector(hosts_range), _last_stats(0) {}

  void start();

  bool handle_packet(const Tins::PDU &pdu);
  std::string handle_request(const std::string& url);

private:
  const int _print_stats_interval = 60;

  FlowsCollector _collector;
  ReaderWriterExclusion _rw_exclusion;
  const std::string _interface;
  time_t _last_stats;

  std::string hosts();
  void print_stats();
};

#endif
