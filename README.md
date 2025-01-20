# webserv
42's webserv w/ Molasz

### TODOs

* timeouts vector for every fd in poll to close after 30s (maybe)
* reading big data in chunks ends when newline x2 (not true for body? YES true for head)
* big big BIG data should be stored in tmp or something instead. or cut off the connection. or start parsing at the same time to throw an error immediately instead of reading spam/bullshit.

* multiline URIs will break the getline by space thing. need to make a "from the start until the first space is the method, from the end until the first-from-backwards space is the protocol, the rest is URI"

#### req-parsing an allat

* requests must have a head (ends on a newline x2)
* if they do, the head gets scanned by a request head parser
* the parsed info gets passed into the Connect class
* if the Connect class needs a body, the funny stage begins
* MAYBE DO THIS: clean the head from the local recv buffer!! erase the sucker.
* MAYBE DON'T DO THIS INSTEAD: the Connect class should have an aux buffer for body
* you start writing \<there/local recv buffer\> from the local recv buffer, but first you pass (up to connection length) bytes from the current local recv buffer into there, AFTER the double newline
* chunked? nah, whatever. IF we have time/will
* the body ends on........ either stopping receiving data, or reaching the counter, whichever comes first
* if we reach the counter or more, just "PUT" what you have until the counter.
* if we DON'T reach the counter, either on the same cycle (recv's bytes < rbuf\_size) or on the next cycle (revents == 0, connection still there like those "is this a joke" variants), we maybe throw an error?
* how to deal with insanely huge bodies? approaching the maximum size of the string, we could start writing to a new string... have a vector of strings for that exact reason, for example. in the body parser obj. and then if the body size is reeeeeeally large, we can start writing to a temp file....................... uh.....
* wait. is writing to a file via POST a poll-must-have operation? seems so :/ we need to add it to the fd list......... I think.............
* we go to the response generator with the... either only parsed head, or both parsed head and parsed body.
* at any point in time a bad request should throw badRequest (surprise), that gets to the try-catch block and gets handled appropriately

### current bugs

* ctrl+C in telnet leads to a weird timeout situation? I guess.
* some signal combos outright crash the webserv? maybe after a considerable amount of data?
* data reading seems to be weird. something wrong with all the ifs, we don't ever  seem to catch the size correctly.... hm.
* just in case -- check if the local recv is cleaned after a correct req. even if body-less type.
