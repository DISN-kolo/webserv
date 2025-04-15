# webserv
42's webserv w/ Molasz

### TODOs

* cleanup TODOs, XXXs, FIXMEs...

### current bugs

* using 0.0.0.0 as "host" will break auto-generated links in some places, because the link is generated via "http://" + host + ":" + port + ....; to avoid it, you could specify your actual IP address. Fixing it requires finding out the URL that's in the client's URL field.

### CONFIGURATION FILE

*note: you should use config/myconf.conf*

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
* autoindex: default off
* index: default index.html
