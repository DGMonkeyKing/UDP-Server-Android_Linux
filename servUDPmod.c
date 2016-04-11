#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#define BUFFER 80

int main(int argc, char** argv){
	if(argc < 3){
		printf("Uso: %s <Ip4> <port>\n", argv[0]);
		return -1;
	}
	
	struct addrinfo hints, *res;
	
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	
	int rc = getaddrinfo(argv[1], argv[2], &hints, &res);
	if(rc != 0){
		printf("Error getaddrinfo(): %s\n", gai_strerror(rc));
		return -1;
	}
	
	int s = socket(res->ai_family, res->ai_socktype, 0);
	
	bind(s, (struct sockaddr *) res->ai_addr, res->ai_addrlen);
	
	char host[NI_MAXHOST];
	char serv[NI_MAXSERV];
	struct sockaddr cliente;
	socklen_t clientelen;
	
	while(1){
		char buffer[BUFFER];
		int bytes = recvfrom(s, buffer, BUFFER-1, 0, (struct sockaddr *)&cliente, &clientelen);
		buffer[bytes] = '\0';
		
		if(bytes == 0) break;
		
		getnameinfo((struct sockaddr *)&cliente, clientelen, host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST|NI_NUMERICSERV);
		
		printf("Conexion desde Host: %s Puerto: %s\n", host, serv);
		
		int data;
		char name[30];
		char buffer_instr[BUFFER];
		
		if(sscanf(buffer, "add %i", &data) != 0){
			int fd = open("/proc/multilist/default", O_WRONLY);
			sprintf(buffer_instr, "add %i", data);
			write(fd, buffer_instr, strlen(buffer_instr));
		}else if(strcmp(buffer, "cat\n\0") == 0){
			int fd = open("/proc/multilist/default", O_RDONLY);
			read(fd, buffer_instr, BUFFER);
			printf("%s", buffer_instr);
		}else if(sscanf(buffer, "remove %i", &data) != 0){
			int fd = open("/proc/multilist/default", O_WRONLY);
			sprintf(buffer_instr, "remove %i", data);
			write(fd, buffer_instr, strlen(buffer_instr));
		}else if(strcmp(buffer, "cleanup\n\0") == 0){
			int fd = open("/proc/multilist/default", O_WRONLY);
			sprintf(buffer_instr, "cleanup\n");
			write(fd, buffer_instr, strlen(buffer_instr));
		}else if(sscanf(buffer, "create %s", name) != 0){
			int fd = open("/proc/multilist/control", O_WRONLY);
			sprintf(buffer_instr, "create %s", name);
			write(fd, buffer_instr, strlen(buffer_instr));
		}else if(sscanf(buffer, "delete %s", name) != 0){
			int fd = open("/proc/multilist/control", O_WRONLY);
			sprintf(buffer_instr, "delete %s", name);
			write(fd, buffer_instr, strlen(buffer_instr));
		}else{
			sendto(s, "Comando no soportado\n", 21, 0, (struct sockaddr *)&cliente, clientelen);
		}
		
//		printf("\tMensaje: %s", msg);
	}
	
	close(s);
	
	return 0;
}
