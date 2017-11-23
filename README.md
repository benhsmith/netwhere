# Netwhere

*Netwhere* is a packet monitor that serves packet monitoring data as a JSON document over a REST endpoint. The capture process is intended to be lightweight enough that it can run comfortably on a home router. An example site that displays monitoring data is provided in the 'html' directory. I do not claim to be a JavaScript programmer, the project is provided to show what *Netwhere* can do and for testing. I am hoping *Netwhere* is useful for building tools to examine networks.
I started thinking about network monitoring because I wanted to see who is on my network and where they are talking to. If I install a new IoT device on my home network, it would be nice to have an easy way to see what it's doing.

**netwhere** is the collector process that collects packets and summarizes flows in and out of the local network. The summarized data is availble via REST endpoints.
A JavasSript frontend queries the endpoint, gathers any additional information that might be useful and provides an interactive view of the data. I want to offload as much of the work as possible onto the client so as to minimize the collector's hardware requirements.

*Netwhere* is available as an OpenWrt package.

## Usage

To start the collector:

    netwhere interface ip mask

The IP and mask tell the netwhere collector how to identify traffic that it should track as a "host". The _/_ endpoint returns a JSON array of hosts where each host constains a list of connections and the number of bytes that went to and from that connection. **netwhere** includes all addresses in this ip/mask range as "hosts".



For example:

    netwhere eth0 192.168.0.0 255.255.255.0

Load html/index.html in a browser with a URL parameter of collector=host:ip, e.g.:

    http://localhost/netwhere/?collector=192.168.0.33:8080#

or, from your local filesystem:

    file:///local/path/netwhere-0.6/html/index.html?collector=localhost:8080#

The collector currently always listens on 8080. That should probably be a command line parameter.

## Build

    mkdir build
    cd build
    cmake ..
    make

*for debug build: cmake -DCMAKE_BUILD_TYPE=Debug ..*

## REST endpoints
A webserver provides host and flow information. Currently the webserver always listens on port 8080.

There are two endoints:
- / : returns a list of hosts
- /MAC-IP : returns flows for a host

### / endpoint
Returns a JSON array. The array will have one entry per Netwhere host. Each host entry has the structure:

    [ MAC, IP, HOSTNAME]

where
- MAC: MAC addresss of host
- IP: IP addresss of host
- HOSTNAME: Host name of host, if Netwhere was able to determine it. Otherwise it will be the IP address.

**example**

    [
      ["192.168.0.123","aa:bb:cc:dd:ee:ff","hostname1"],
      ["192.168.0.124","aa:bb:cc:dd:ff:ee","hostname2"]
    ]

### /?MAC-IP endpoint
The *path* portion of the URL should be a host identifier of the form: MAC-IP.
Returns a JSON array. Each entry in the array represents a flow. Each flow entry has the structure:

    [
      [
        [ SRC_MAC, SRC_IP ],
    	[ DST_MAC, DST_IP ],
    	DST_IP_PORT,
    	IP_PROTOCOL
      ],
      BYTES_FROM_SRC,
      BYTES_FROM_DST
    ]

- SRC_MAC: MAC addresss of source
- SRC_IP: IP addresss of source
- DST_MAC: MAC addresss of destination
- DST_IP: IP addresss of destination
- DST_PORT: Port on destination
- IP_PROTOCOL: 6 for TCP or 17 for UDP.
- BYTES_FROM_SRC: number of bytes transferred from source to destination.
- BYTES_FROM_DST: number of bytes transferred from destination to source.

## Dependencies

https://libtins.github.io

https://www.gnu.org/software/libmicrohttpd

http://www.boost.org

https://code.jquery.com/

http://minicss.org/
