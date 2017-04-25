This is a preliminary, draft, prototype of a packet analyzer. The reasons for this project are:

# I wanted a way to summarize traffic on an OpenWrt router but nothing I found quite did what I wanted.
# After attending a C++ user's group I wanted to play around with modern C++ and needed a project.

The design, currently, is a simple binary that monitors flows in and out of the local network and serves the data up via a REST endpoint that provides the summarized flow data to clients.
A javascript frontend queries the endpoint, gathers any additional information that might be useful and provides an interactive view of the data. I want to put as much of the work as possible
in the client so as to allow the collector to run on low powered hardware.

I am not a front end guy and I have not spent any time trying to make the front end code look nice or professional. It lets me test the backend and shows what's possible. Maybe I'll learn Angular next and make it all pretty.

