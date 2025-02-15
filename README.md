# webserv
42's webserv w/ Molasz

### TODOs

* check out the poll's limits on \_connsAmt. it gives an error if the total polling size is too big... ulimit -n?
* writing files
* for writing: always have to write as you read from the req, at the same time. if it blocks/file unavailable as per revents, must skip til the next read of the req. in the meantime, must have a loop, independent of the main polld loop, that runs thru all the connections, checks if any of them have any files to write, and writes (again) if poll says the file's available. or, better yet, just always rely on this loop instead of read-post-write-file in the same code block.
* deleting files
* big big BIG data should be stored in tmp or something instead. or cut off the connection. or start parsing at the same time to throw an error immediately instead of reading spam/bullshit.

### current bugs

* ctrl+C in telnet/nc leads to a weird timeout situation? I guess. *Idk if that's an improper termination problem or the server's problem.*
* sbuf/rbuf above the size of sending produces an infinite do-nothing loop on an always-hitting poll
* keepalive not functioning in bus size 2. seems to be due to polling a file that has a -1 fd for some reason.
