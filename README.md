servUDPmod.c es un programa de usuario que crea un servidor UDP para 
intercambio de mensajes.

FUNCIONAMIENTO:

Compilar el programa c con gcc y lanzar a ejecución el ejecutable con la 
siguiente sintaxis

	./udp <dirIP host> <puerto>   -   Ej: ./udp 192.168.21.129 3000

Arrancar otra terminal y ejecutar el comando:

	nc -u <dirIP host> <puerto>   -   Ej: nc -u 192.168.21.129 3000

Por falta de tiempo, solo se ha añadido la funcionalidad de creacion y borrado
de /procs y añadir y borrar elementos de la lista de default, solo de default.

POSIBLES MENSAJES
	add <int> = Añade un elemento a la lista de default
	remove <int> = Elimina todas las apariciones de default
	cleanup = Elimina toda la lista de default
	create <string> = Crea un /proc llamado string
	delete <string> = Borra un /proc llamado string

TAMBIEN FUNCIONA SOBRE ANDROID-CONECTARSE CON LA DIR IP DE LA MÁQUINA DE ANDROID