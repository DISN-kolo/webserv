# webserv
42's webserv w/ Molasz

### TODOs

* keep-aliving needs request-parsing, which will generate a req object, which will have a keep-alive flag, which will influence the "close(..)" line
* sending big data in chunks to non-block would be goated.
* timeouts vector for every fd in poll to close after 30s
* reading big data in chunks ends when CRLFx2
* big big BIG data should be stored in tmp or something instead. or cut off the connection. or start parsing at the same time to throw an error immediately instead of reading spam/bullshit.

* when we have a message divisible by the buffer, if, say, EVERY message is like this, then we'll have 0 poll events after reading everything. this will ensure we'll never get to checking previous poll events. thus, we NEED to parse right here, right now, or, rather, we NEED to SEARCH for a DOUBLE LINE BREAK in whatever format because yeah.

### TODones

* header's end check in request. if there's a ([CR]LF)x2 in the req, this means a header has ended.
