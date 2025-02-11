# webserv
42's webserv w/ Molasz

### TODOs

* BEFORE WE DO THE FOLLOWING TWO: relativization of the path
* writing files
* deleting files
* big big BIG data should be stored in tmp or something instead. or cut off the connection. or start parsing at the same time to throw an error immediately instead of reading spam/bullshit.

### current bugs

* ctrl+C in telnet/nc leads to a weird timeout situation? I guess. *Idk if that's an improper termination problem or the server's problem.*
* sbuf/rbuf above the size of sending produces an infinite do-nothing loop on an always-hitting poll
