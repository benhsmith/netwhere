# netwhere

This is a preliminary, draft, untested prototype of a network monitor. The reasons for this project are:

* I wanted a way to summarize traffic on an OpenWrt router but nothing I found quite did what I wanted.
* After attending a C++ user's group I wanted to play around with modern C++ and needed a project.

The design, currently, is to have a basic binary that does the work of monitors flows in and out of the local network while providing the montoring results via a REST endpoint.
A javascript frontend queries the endpoint, gathers any additional information that might be useful and provides an interactive view of the data. I want to offload as much of the work as possible onto the client so as to allow the collector to run on low-powered hardware.

The frontend part of this is some cobbled together javascript to see what I can do with the data available from the backend. It's minimal.

## Dependencies

https://libtins.github.io

https://www.gnu.org/software/libmicrohttpd

http://www.boost.org
