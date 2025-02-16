# webserv
42's webserv w/ Molasz

### TODOs

* transform the ips from the config into the proper ips for the listeners. actually, do context for the listeners.
* transfer some arrays into the "global" space of the server object.
* CGI
* binary data ?
* check out the poll's limits on \_connsAmt. it gives an error if the total polling size is too big... ulimit -n?
* deleting files. both with DELETE and with having a wrong file written
* big big BIG data should be stored in tmp or something instead. or cut off the connection. or start parsing at the same time to throw an error immediately instead of reading spam/bullshit.
* test sigpipe ignorance further

### current bugs

* ctrl+C in telnet/nc leads to a weird timeout situation? I guess. *Idk if that's an improper termination problem or the server's problem.*
* sbuf/rbuf above the size of sending produces an infinite do-nothing loop on an always-hitting poll
