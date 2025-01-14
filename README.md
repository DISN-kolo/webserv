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
* the Connect class should have an aux buffer for body
* you start writing there from the local recv buffer, but first you pass (up to connection length) bytes from the current local recv buffer into there, AFTER the double newline
* it's like getnextline, but also limited by conten-length
* if for some reason it's a chunked transfer, make sure to do the same exact procedure, but parse each packet after head in a chunked transfer manner (another separate parser?) and add the sum into the Connect's local body storage
* having collected all that, we parse the body. (that's IF WE HAD ONE)
* we go to the response generator with the... either only parsed head, or both parsed head and parsed body (overriding the response generator constructor).
* at any point in time a bad request should throw badRequest (surprise), that gets to the try-catch block and gets handled appropriately
