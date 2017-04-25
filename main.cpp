#include <iostream>
#include <tins/tins.h>

#include "netwhere.hpp"

using namespace std;
using namespace Tins;

int main(int argc, char **argv) {
  if (argc != 4) {
    cout << "usage: " << argv[0] << " interface ip netmask" << endl;
    return 1;
  }
  
  NetWhere netwhere(argv[1], IPv4Range::from_mask(argv[2], argv[3]));
  netwhere.start();
}
