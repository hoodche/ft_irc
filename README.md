Server class in progress. So far it is accepting connections. Compile with make. Execute 'ircserv 1234 pass'. Open another terminal window, connect to the machine and port being listened on by executing 'nc 127.0.0.1 1234' and a new connection is added to Server::fds vector (a message is printed showing ip and port from the connected "client" as well as the fd assigned to this new connection). If you open another terminal window and repeat the operation a new connection ("client") will be added (nc command allows to connect choosing any interface ip in our host by 'nc -s \<host interface inet field\> 127.0.0.1 1234') (a host´s network interfaces can be listed with command ifconfig -see screenshot-). In 42 Madrid computers, besides localhost (127.0.0.1), server will be listening also on port 1234 of ip´s 172.17.0.1, 10.12.7.6 and 192.168.122.1 - since listen socket is bound to all available interfaces on the host machine by means of this->serverAddress.sin_addr.s_addr = INADDR_ANY; in the Server::initSocket() function -.

A simple client for testing purposes can be compiled with 'make client' and executed with './client'. It will send to the server (ip 127.0.0.1 and port 1234) whatever is typed. Client is finished with CTRL + D

### WEECHAT USAGE
IRC Client being used at the time: `weechat`.

To Install: `sudo apt-get install weechat`
To configure localhost server: 
	1. Open weechat with the `weechat` command on terminal

	2. Run the following command: `/server add localhost 127.0.0.1/1234`

	3. TO TEST PASSWORD: /set irc.server.localhost.password "pass" -> Allegedly, my client connects with no password for some reason

DONE: Client class started. Handler class started. NICK, USER and CAP LS 302 commands initially managed.

TO DO: Polish code and check corner cases. Implement PASS, PING, JOIN commands.
