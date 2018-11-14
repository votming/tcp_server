
#include <iostream>
#include <string>
#include <vector>
#include "libpq-fe.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3490"  // the port users will be connecting to

#define BACKLOG 10     // how many pending connections queue will hold

static PGconn* connection = NULL;
static PGresult* result = NULL;

void sigchld_handler(int s)
{ 
	int saved_errno = errno; 
	while (waitpid(-1, NULL, WNOHANG) > 0); 
	errno = saved_errno;
} 
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
	//code below(strings 44-105) is the trivial realisation of tcp server

	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes = 1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; 

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	 
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
			p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
			sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); 

	if (p == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}


	int lib_ver = PQlibVersion(); //check postgressql lib version(also check if postgresql even work)
	printf("Lib version: %d\n", lib_ver);

	connection = PQconnectdb("user=admin password=123 host=localhost dbname=calc_base"); 
	if (PQstatus(connection) != CONNECTION_OK)
		printf("PostgreSQL not working. Error: %s\n", PQerrorMessage(connection));
	 
	printf("server: waiting for connections...\n");
	 
	int numbytes, balance;
	char buf[100], login[100], password[100];
	char query[100], output[20]; 
	char *comand_check;
	login[0] = '\0';

	while (1) {  // main accept() loop  
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
		printf("server: got connection from %s\n", s);

		if (!fork()) { // this is the child process
			close(sockfd); 
			while (1) { 

				if ((numbytes = recv(new_fd, buf, 100 - 1, 0)) == -1) { //getting user input string
					perror("recv"); 
				}
				else {
					printf("client: received '%s', %d\n", buf, numbytes);

					if (numbytes == 1) { continue; }
					buf[numbytes] = '\0'; 
				}

				comand_check = strtok(buf, " "); //split incoming string with spaces and take a first part  

				if (strcmp(comand_check, "login") == 0) //if the first part is "login", then we need to check user login in database
				{
					comand_check = strtok(NULL, " "); //take second part of incoming string which contains login itself
					 
					sprintf(query, "SELECT * from users where login='%s';",  comand_check); 
					result = PQexec(connection, query);
					int nrows = PQntuples(result); 
					if (nrows == 0)//if we have no results - no such login in database
						strcpy(output, "Code90: No such login in database! Try again.");
					else
					{
						strcpy(login, comand_check); 
						strcpy(output, "Code1: Correct login!");
					}

					if (send(new_fd, output, 100, 0) == -1)
						perror("error");

				}
				else if (strcmp(comand_check, "password") == 0)//if the first part is "password", then we need to check user password in database
				{
					if (strlen(login) != 0) // if user logged in
					{
						comand_check = strtok(NULL, " ");
						sprintf(query, "SELECT * from users where login='%s';", login);
						result = PQexec(connection, query);

						if (strcmp(comand_check, PQgetvalue(result, 0, 1)) == 0)//check password by compare it with password from out database
						{
							strcpy(password, PQgetvalue(result, 0, 1));
							balance = atoi(PQgetvalue(result, 0, 2));
							sprintf(output, "code2 Correct password! Your balance is %d.", balance); 
						}
						else
							strcpy(output, "Code91: Incorrect password! Try again.");
						if (send(new_fd, output, 100, 0) == -1)
							perror("error");
					}
					else
					{
						if (send(new_fd, "Code92: You need select login first\n", 100, 0) == -1)
							perror("error");
					}
				}
				else if (strcmp(comand_check, "logout") == 0)//if the first part is "logout", then we need to logout user
				{
					memset(login, 0, sizeof(login));
					memset(password, 0, sizeof(login));
					balance = 0;
					close(new_fd);
					exit(0);
				}
				else if (strcmp(comand_check, "calc") == 0)//if the first part is "calc", then we need to calculate the expression
				{ 
					if (strlen(login) == 0 || strlen(password) == 0) //check for user log in
					{
						send(new_fd, "Code93: You have to log in first.", 100, 0);
						continue;
					}
					if (balance <= 0)//check user balance
					{
						send(new_fd, "Code94: Your account balance is negative", 100, 0);
						continue;
					}
					std::vector<std::string> all_numbers, all_actions;//two vectors. One is to store all numbers, second is to store all ariphmetical actions
					comand_check = strtok(NULL, " ");
					bool append = false;//as long we read string symbol by symbol we need to know do we need to add this new char to new vector item or we need to append this char to previous item

					for (int i = 0; i < strlen(comand_check); i++)//read string char by char
					{  
						if (comand_check[i] == '+' || comand_check[i] == '-' || comand_check[i] == '*' || comand_check[i] == '/') //if next char is ariphmetical action
						{  
							all_actions.push_back("");  //add this action to all_actions vector
							all_actions.at(all_actions.size() - 1) += comand_check[i];
							append = false; 
						}
						else
							if (append == false)//if we don't need to append - we create new vector instance
							{ 
								all_numbers.push_back("");
								all_numbers.at(all_numbers.size() - 1) += comand_check[i];
								append = true;
							}
							else //or append char to the last instance of all_numbers vector
								all_numbers.at(all_numbers.size() - 1) += comand_check[i];

					}

					//then we need to run through all_actions vector instance and calculate all actions that have priority(* and /)
					
					for (int i = 0; i < all_actions.size(); i++)
					{
						if (all_actions.at(i) == "*" || all_actions.at(i) == "/")
						{
							int x1 = i, x2 = i + 1;
							while (true)
							{
								if (all_numbers.at(x1) == "0")
									x1--;
								else break;
							}

							if (all_actions.at(i) == "*") 
								all_numbers.at(x1) = std::to_string(std::stof(all_numbers.at(x1))*std::stof(all_numbers.at(x2))); 
							else if (all_actions.at(i) == "/") 
								all_numbers.at(x1) = std::to_string(std::stof(all_numbers.at(x1)) / std::stof(all_numbers.at(x2))); 
							 
							all_actions.at(i) = "0";
							all_numbers.at(x2) = "0";
						}

					}
					//after it we need to run through this vector once again and now calculate secondary actions(+ and -)
					for (int i = 0; i < all_actions.size(); i++)
					{
						if (all_actions.at(i) == "+" || all_actions.at(i) == "-")
						{
							int x1 = i, x2 = i + 1;
							while (true)
							{
								if (all_numbers.at(x1) == "0")
									x1--;
								else break;
							}

							if (all_actions.at(i) == "+") 
								all_numbers.at(x1) = std::to_string(std::stof(all_numbers.at(x1)) + std::stof(all_numbers.at(x2))); 
							else if (all_actions.at(i) == "-") 
								all_numbers.at(x1) = std::to_string(std::stof(all_numbers.at(x1)) - std::stof(all_numbers.at(x2)));

							all_numbers.at(x2) = "0";
							all_actions.at(i) = "0";
						}

					}

					//all we have to do is update the user's balance
					sprintf(query, "update users set balance=%d  where login='%s';", --balance, login); 
					result = PQexec(connection, query);

					//and register this calculation in the database
					float expression_result = std::stof(all_numbers.at(0));
					sprintf(query, "insert into activities (login,activiti_time,user_request,result) values ('%s',now(),'%s','%f');", login, comand_check, expression_result);
					result = PQexec(connection, query);

					sprintf(buf, "Code3: Answer is: %f. Your balance: %d", expression_result, balance);
					send(new_fd, buf, 100, 0);
				}
				else {
				//in other input strings we just send user the same string(as response)
					printf("Code4: Reply send to client: %s\n", buf);
					if (send(new_fd, buf, 100, 0) == -1)
						perror("error"); 
				} 
			}
		}
		close(new_fd);   
	}

	return 0;
}
