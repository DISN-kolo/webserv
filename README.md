# webserv
42's webserv w/ Molasz

### TODOs

* keep-aliving needs request-parsing, which will generate a req object, which will have a keep-alive flag, which will influence the "close(..)" line
* sending big data in chunks to non-block would be goated.
* timeouts vector for every fd in poll to close after 30s
* reading big data in chunks ends when CRLFx2
* big big BIG data should be stored in tmp or something instead. or cut off the connection. or start parsing at the same time to throw an error immediately instead of reading spam/bullshit.
