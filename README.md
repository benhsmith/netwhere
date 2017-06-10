# netwhere

This is a bare bones network monitor. The reasons for this project are:

* I wanted a way to summarize traffic on an OpenWrt router but nothing I found quite did what I wanted.
* After attending a C++ user's group I wanted to play around with modern C++ and needed a project.

The design, currently, is a binary that does the work of collecting and summarizing flows in and out of the local network while providing the montoring results via a REST endpoint.
A javascript frontend queries the endpoint, gathers any additional information that might be useful and provides an interactive view of the data. I want to offload as much of the work as possible onto the client so as to allow the collector to run on low-powered hardware.

The frontend part of this is some cobbled together javascript to see what I can do with the data available from the backend. It's minimal.

## Usage

To start the collector:

    netwhere interface ip mask

The IP and mask tell the netwhere collector how to identify traffic that it should track as a "host". When you query the endpoint you will get a JSON object of hosts where each host constains a list of connections and the number of bytes that went to and from that connection. netwhere includes all addresses in the ip/mask range as "hosts".

For example:

    netwhere eth0 192.168.0.0 255.255.255.0

Load html/index.html in a browser with a URL parameter of collector=host:ip

    http://localhost/netwhere/?collector=192.168.0.33:8080#

The collector currently always listens on 8080. That should probably be a command line parameter.

## Build

    mkdir build
	cd build
	cmake ..
    make

## Dependencies

https://libtins.github.io

https://www.gnu.org/software/libmicrohttpd

http://www.boost.org

http://minicss.org/
