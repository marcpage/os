#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>

int main(int argc, const char * const argv[]) {
	struct addrinfo req, *ans;
	int code, s;
	
	req.ai_flags = 0;
	
	req.ai_family = PF_INET6;               /* Same as AF_INET6.            */
	req.ai_socktype = SOCK_STREAM;
	
	/*                                                    */
	/* Use default protocol (in this case tcp)            */
	/*                                                    */
	req.ai_protocol = 0;
	if ((code = getaddrinfo(argv[1], "80", &req, &ans)) != 0) {
		fprintf(stderr, "rlogin: getaddrinfo failed code %d\n",
		code);
		return 1;
	}
	
	/*                                                    */
	/* ans must contain at least one addrinfo, use        */
	/* the first.                                         */
	/*                                                    */
	s = socket(ans->ai_family, ans->ai_socktype, ans->ai_protocol);
	if (s < 0) {
		perror("rlogin: socket");
		return 3;
	}
	/* Connect does the bind for us */
	if (connect (s, ans->ai_addr, ans->ai_addrlen) < 0) {
		perror("rlogin: connect");
		return 5;
	}

	/*                                                    */
	/* Free answers after use                             */
	/*                                                    */
	freeaddrinfo(ans);

	return 0;
}
