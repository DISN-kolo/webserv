# webserv
42's webserv w/ Molasz

### TODOs

* limit client body size
* directory listing
* redirect particularities...
* CGI
* CGI check for while true (add a timeout to cgi itself)
* binary data ?
* check out the poll's limits on \_connsAmt. it gives an error if the total polling size is too big... ulimit -n?
* big big BIG data should be stored in tmp or something instead. or cut off the connection. or start parsing at the same time to throw an error immediately instead of reading spam/bullshit.
* test sigpipe ignorance further

### current bugs

* parsing of error pages is weird, it needs one more space between the code and the file path
* ctrl+C in telnet/nc leads to a weird timeout situation? I guess. *Idk if that's an improper termination problem or the server's problem.*

### CONFIGURATION FILE

#### SERVER
* port: mandatory
* host: default 127.0.0.1
* server\_name:
* error\_page:
* client\_max\_body\_size: default 10MB
* index: default index.html
* root: mandatory

#### ROUTE
* allow\_methods: default GET
* return:
* name: mandatory
* root: mandatory
* autoindex: default false
* index: default index.html
* cgi\_ext: default .php
* cgi\_path: default /bin/php
