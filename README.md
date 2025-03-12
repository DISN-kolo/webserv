# webserv
42's webserv w/ Molasz

### TODOs

* cleanup path deobfuscation. for example, POST /cgi/../../../ does weird things. maybe it's 500 cuz posting a dir? shan't it be forbidden?
* cleanup TODOs, XXXs, FIXMEs...
* evaluate

### current bugs

* can't receive an execve exception. nothing is printed on the execve branch.
* pipe fail leads to getting stuck like with minishell. must create its own fail. but first handle exec fail, since it doesn't seem to work.

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
* autoindex: default off
* index: default index.html
