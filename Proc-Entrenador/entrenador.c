/*
 * entrenador.c
 *
 *  Created on: 29/8/2016
 *      Author: utnso
 */

#include <commons/collections/list.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <commons/config.h>
#include <commons/log.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include <pkmn/factory.h>

typedef struct{
	int minutos;
	int segundos;
}tiempo;

tiempo tiempoTardado(tiempo inicio, tiempo fin){
	int totalInicio, totalFin, tardadoEnSegundos;
	tiempo tiempoTardado;
	totalInicio = ((inicio.minutos)*60 + (inicio.segundos));
	totalFin = ((fin.minutos)*60 + (fin.segundos));
	tardadoEnSegundos = totalFin - totalInicio;
	tiempoTardado.minutos = (tardadoEnSegundos / 60);
	tiempoTardado.segundos = (tardadoEnSegundos % 60);
	return tiempoTardado;
}

tiempo organizarTiempo(tiempo unTiempo){
	int total;
	tiempo tiempoOrganizado;
	total = ((unTiempo.minutos)*60 + (unTiempo.segundos));
	tiempoOrganizado.minutos = (total / 60);
	tiempoOrganizado.segundos = (total % 60);
	return tiempoOrganizado;
}

#include "headers/struct.h"

Entrenador entrenador;
bool flag_SIGNALMUERTE = false;
bool flag_BLOQUEADO=false;

#include "headers/socket.h"
#include "headers/configEntrenador.h"
#include "headers/send.h"
#include "headers/serializeEntrenador.h"
#include "headers/pokenest.h"

/*
void leerObjetivos(char* objetivos, t_config* config, int cantViajes,char** hojaDeViaje)
{
	char* ciudad;
	char* leer;
	char** a;

	int i;
	for(i=0;i<cantViajes;i++)
	{
		ciudad = hojaDeViaje[i];
		leer = concatObjetivo(ciudad);
		objetivos[i] = config_get_array_value(config,leer);
		mostrarObjetivos(a);
		free(leer);
	}
}
*/

void copiarPokemon(char *archivoPokemon, ParametrosConsola parametros, char* nombreMapa){
	char *origen, *destino, *nombre, *comandoCopiar;
	int tamanio;
	tamanio = strlen(archivoPokemon);
	nombre = malloc(tamanio - 6);
	memcpy(nombre,archivoPokemon,tamanio-7);

	char* barraCero = "\0";
	memcpy(nombre+tamanio-7,barraCero,sizeof(char));
	origen = malloc(sizeof(char)*256);
	destino = malloc(sizeof(char)*256);

	strcpy(origen, parametros.dirPokedex);
	//strcat(origen, "/Mapas/");	//Para otros
	strcat(origen, "Mapas/");	//Para montaje de fs
	strcat(origen, nombreMapa);
	strcat(origen, "/Pokenest/");
	strcat(origen, nombre);
	strcat(origen, "/");
	strcat(origen, archivoPokemon);

	strcpy(destino, parametros.dirPokedex);
	strcat(destino, "Entrenadores/");
	strcat(destino, parametros.nombreEntrenador);
	strcat(destino, "/Dir\\ de\\ Bill/");
	strcat(destino, barraCero);

	comandoCopiar = malloc(strlen(origen)+strlen(destino)+5);
	sprintf(comandoCopiar, "cp %s %s", origen, destino);

	system(comandoCopiar);

	/*Agregue esto y empezo a copiar pero solo a partir de la segunda pokenest que llegaba, de la primera no copia
	origen = NULL;
	destino = NULL;
	nombre = NULL;
	comandoCopiar = NULL;
*/
	free(nombre);
	free(origen);
	free(destino);
	free(comandoCopiar);
}

void borrarPokemones(ParametrosConsola parametros){
	char *directorio, *comandoBorrar;
	directorio = malloc(sizeof(char)*256);

	strcpy(directorio, parametros.dirPokedex);
	//strcat(directorio, "/Entrenadores/");
	strcat(directorio, "Entrenadores/");	//para montaje de fs
	strcat(directorio, parametros.nombreEntrenador);
	strcat(directorio, "/Dir\\ de\\ Bill/");

	comandoBorrar = malloc(strlen(directorio)+9);
	sprintf(comandoBorrar, "rm -rf %s*", directorio);

	system(comandoBorrar);

	free(directorio);
	free(comandoBorrar);
}

void copiarMedalla(ParametrosConsola parametros, char* nombreMapa){
	char *origen, *destino, *archivoMedalla, *comandoCopiar;

		char* barraCero = "\0";

		archivoMedalla = malloc(strlen(nombreMapa)+13);
		origen = malloc(sizeof(char)*256);
		destino = malloc(sizeof(char)*256);

		strcpy(archivoMedalla, "medalla-");
		strcat(archivoMedalla, nombreMapa);
		strcat(archivoMedalla, ".jpg");

		strcpy(origen, parametros.dirPokedex);
		//strcat(origen, "/Mapas/");
		strcat(origen, "Mapas/");	///Para montaje de fs
		strcat(origen, nombreMapa);
		strcat(origen, "/");
		strcat(origen, archivoMedalla);

		strcpy(destino, parametros.dirPokedex);
		strcat(destino, "Entrenadores/");
		strcat(destino, parametros.nombreEntrenador);
		strcat(destino, "/medallas");

		comandoCopiar = malloc(strlen(origen)+strlen(destino)+5);
		sprintf(comandoCopiar, "cp %s %s", origen, destino);

		system(comandoCopiar);

		free(archivoMedalla);
		free(origen);
		free(destino);
		free(comandoCopiar);
}

int get_pokemon_mas_fuerte()
{
	int* mas_fuerte = (int*)list_get(entrenador.pokemonesCapturados,0);
	int* aux;

	int i;
	int indicePok = 0;

	for(i=1;i<list_size(entrenador.pokemonesCapturados);i++)
	{
		aux = (int*)list_get(entrenador.pokemonesCapturados,i);

		if(*aux > *mas_fuerte)
		{
			mas_fuerte = (int*)list_get(entrenador.pokemonesCapturados,i);
			indicePok= i;
		}
	}

	return indicePok;
}

int recv_turnoConcedido(int fd_server)
{
	int tam_buffer = sizeof(int);
	void* mensaje[tam_buffer];
	int codigo;

	int valor_recv = recv(fd_server, (void*)mensaje, tam_buffer, 0);

	if(valor_recv == -1)
	{
		perror("Error recv\n\n");
		exit(1);
	}
	//Deserializamos
	memcpy(&codigo,mensaje,sizeof(int));

	if(codigo == TURNO)
	{
		return 1;
	}
	return 0;
}

void recv_MoverOK(int fdServer)
{
	Paquete paquete;
	paquete.tam_buffer = size_MOVER_OK;
	paquete.buffer = malloc(paquete.tam_buffer);

	recv(fdServer,paquete.buffer,paquete.tam_buffer,0);

	int codigoOperacion = dsrlz_MoverOK(paquete.buffer);

	if(codigoOperacion!=MOVER_OK)
	{
		printf("Exit(1) - Func %s - Linea: %d - Codigo MOVER_OK invalido",__func__,__LINE__);
		exit(1);
	}

}

Paquete recv_BatallaInforme(int fdServer)
{
	Paquete paquete;
	paquete.tam_buffer=size_BATALLA_INFORME;
	paquete.buffer = malloc(paquete.tam_buffer);
	recv(fdServer,paquete.buffer,paquete.tam_buffer,0);
	return paquete;
}

void reiniciarEntrenador(Entrenador *entrenador)
{
	entrenador->posx = 1;
	entrenador->posy = 1;
	entrenador->movAnterior = 'y';
    entrenador->flagx = 0;
    entrenador->flagy = 0;
}

void avanzarNivel(Nivel* nivel,Entrenador* entrenador)
{
	nivel->finNivel = 1;
	nivel->nivelActual++;
	nivel->numPokenest = 0;

	reiniciarEntrenador(entrenador);
}

void informar_perdidaVida()
{
	printf("Conectando nuevamente al mapa para reiniciar objetivos ... \n");
}

bool informar_signalMuerteEntrenador()
{
	char opc;

	printf("\n\n†††††††† -- You are dead -- †††††††† \n");

	printf("No tienes mas vidas - cantidad de reintentos: %d\n", entrenador.reintentos);
	printf("1) Reiniciar \n");
	printf("2) Salir \n");

	while(1)
	{
		fflush(stdin);
		opc = getchar();

		switch(opc)
		{
			case '1':
				printf("1) Reiniciando... \n");
				return true;
				break;
			case '2':
				printf("2) Saliendo... \n");
				return false;
				break;
			default:
				printf("Opcion Invalida - Ingrese nuevamente \n");
				break;
			}
		}
}

int recv_codigoOperacion(int fd_server)
{
	Paquete paquete;
	paquete.buffer = malloc(sizeof(int));
	int retval = 0;
	int codop = -1;


	retval = recv(fd_server,paquete.buffer,sizeof(int),0);

	if(retval > 0)
	{
		memcpy(&codop,paquete.buffer,sizeof(int));
		free(paquete.buffer);
	}

	return codop;

}

void liberarPokemonesCapturados(t_list* pokemones)
{
	int tamList = list_size(pokemones);
	int i;

	for(i=0;i<tamList;i++)
	{
		free(list_remove(pokemones,0));
	}

	list_clean(pokemones);
}

ParametrosConsola parametros;

void manejar_signals(int operacion){

	if(flag_SIGNALMUERTE == false)
	{
		fflush(stdout);
		printf("\nSEÑAL RECIBIDA\n");
		switch(operacion){

		case SIGUSR1: //Le sumamos una vida al entrenador
			entrenador.vidas ++;
			printf("SumarVida - Quedan %d vidas\n\n", entrenador.vidas);
			break;
		case SIGTERM://Le restamos una vida al entrenador
			entrenador.vidas --;

			printf("RestaVida - Quedan %d vidas\n\n", entrenador.vidas);
			if(entrenador.vidas <= 0)
			{
				flag_SIGNALMUERTE = true;
				borrarPokemones(parametros);
				if(flag_BLOQUEADO) close(fd_server);
			}
			break;
		}
	}
}

void sigHandler_endProcess(int signal)
{
	switch(signal)
	{
	case SIGINT:
		close(fd_server);
		borrarPokemones(parametros);
		printf("\n\nCerrando Mapa %i \n\n", signal);
		exit(1);
		break;
	case SIGHUP:
		close(fd_server);
		borrarPokemones(parametros);
		printf("\n\nCerrando Mapa %i \n\n", signal);
		exit(1);
		break;
	}
}


int main(int argc, char** argv)
{
	pid_t pid = getpid();
	printf("PID ENTRENADOR:%i \n\n",pid);

	/*Recibimos el nombre del entrenador y la direccion de la pokedex por Consola*/

	verificarParametros(argc); //Verificamos que la cantidad de Parametros sea correcta
	parametros = leerParametrosConsola(argv); //Leemos los parametros necesarios

	//parametros.dirPokedex = "/mnt/juegoFacil2/pokedex/";
	//parametros.dirPokedex = "/home/utnso/tp-2016-2c-Breaking-Bug/Archivos de prueba/pokedex";

	//parametros.nombreEntrenador = "Ash";
	//Ahora se deberia leer la Hoja de Viaje, la direccion de la Pokedex esta en parametros.dirPokedex

	metadata mdata;
	mdata = leerMetadataEntrenador(parametros);

	struct tm *local, *local2, *local3, *local4;
	time_t t, t2, t3, t4;
	tiempo tardado,inicio,fin, bloqueadoInicio, bloqueadoFin, totalBloqueado, bloqueado, totalBloqueadoOrganizado;

	totalBloqueado.minutos = 0;
	totalBloqueado.segundos = 0;

	t = time(NULL);
	local = localtime(&t);
	inicio.minutos = local->tm_min;
	inicio.segundos = local->tm_sec;

	//A partir de aca, comienza el juego, es decir hacer acciones en el mapa

	Nivel nivel = new_nivel();
	nivel.cantNiveles = cantidadDeViajes(mdata.hojaDeViaje);
	DatosMapa mapa;

	int opcion = -1;
	entrenador = new_Entrenador(mdata);

	Paquete paquete;



	//Agregamos las funciones que manejaran las señales enmascaras como SIGTERM Y SIGUSR1.

	signal(SIGUSR1, manejar_signals);
	signal(SIGTERM, manejar_signals);
	signal(SIGINT,sigHandler_endProcess);
	signal(SIGHUP,sigHandler_endProcess);

	//A partir de aca, comienza el juego, es decir hacer acciones en el mapa

	Pokenest pokenest;
	char* pokemonDat;
	int opc;
	bool flag_seguirJugando = true;

	Entrenador entrenador_estadoInicial = entrenador;
	int auxcantNiveles;
	int auxreintentos=0;

	int cantDeadlock=0;
	int cantDeadlocksPerdidos=0;

	int codOp;

	while(nivel.cantNiveles > nivel.nivelActual && flag_seguirJugando == true)
	{
		mapa.nombre = obtenerNombreMapa(mdata.hojaDeViaje,nivel.nivelActual); //Obtenemos el nombre del mapa numero X

		ConexionEntrenador connect;
		connect = leerConexionMapa(parametros.dirPokedex,mapa.nombre); //Leemos la información del Mapa nombre "LoQueSea"

		//Ahora debemos conectarnos al mapa
		fd_server = get_fdServer(connect.ip,connect.puerto);
		send_simboloEntrenador(mdata.simbolo, fd_server);

		nivel.finNivel = 0;
		nivel.cantObjetivos = getCantObjetivos(mdata.objetivos[nivel.nivelActual]);

		pokenest = new_pokenest(mdata.objetivos[nivel.nivelActual],nivel.numPokenest);

		while(nivel.finNivel == 0) //Mientras que no hayamos ganado el nivel
		{
			opcion = evaluar_opciones(entrenador,pokenest);

			switch(opcion)
			{
				case POKENEST://Caso 1: QUEREMOS UNA POKENEST!
					paquete = srlz_solicitarPokenest(pokenest);
					send_solicitarPokenest(&paquete,fd_server);
					recv_solicitarPokenest(&pokenest, fd_server);
					calcular_coordenadas(&entrenador,pokenest.posx,pokenest.posy);
					//send_coordenadasDestino(&entrenador);
					break;

				case MOVER://Caso 2: Queremos movernos!
					mover_entrenador(&entrenador);
					paquete = srlz_movEntrenador(entrenador);
					send_movEntrenador(&paquete,fd_server);
					recv_MoverOK(fd_server);
					break;

				case CAPTURAR: //Caso 3: Ya llegamos a una Pokenest. QUEREMOS CAPTURAR
					paquete = srlz_capturarPokemon(pokenest.simbolo);
					send_capturarPokemon(&paquete,fd_server);
					codOp = recv_codigoOperacion(fd_server);

					if(codOp == CAPTURA_OK)
					{
						paquete = recv_capturarPokemon(fd_server);
						pokemonDat = dsrlz_capturarPokemon(&paquete,&entrenador);
						copiarPokemon(pokemonDat, parametros, mapa.nombre);
						printf("%s - Objetivo Numero: %i \n",pokemonDat,nivel.numPokenest);
					}

					else if(codOp == CAPTURA_BLOQUEADO)
					{
						flag_BLOQUEADO = true;
						printf("Entrenador Bloqueado! \n");
						t3 = time(NULL);
						local3 = localtime(&t3);
						bloqueadoInicio.minutos = local3->tm_min;
						bloqueadoInicio.segundos = local3->tm_sec;
						codOp = -1;

						while(codOp != BATALLA_GANADOR && codOp !=BATALLA_MUERTE && flag_SIGNALMUERTE == false)
						{
							codOp = recv_codigoOperacion(fd_server);
							if(codOp == BATALLA_PELEA)
							{

								//CODIGO DE PELEA
								int pokemonMasFuerte = get_pokemon_mas_fuerte();
								paquete = srlz_pokemonMasFuerte(pokemonMasFuerte);
								send_pokemonMasFuerte(&paquete,fd_server);

								paquete=recv_BatallaInforme(fd_server);
								dsrlz_BatallaInforme(&paquete, fd_server);

								cantDeadlock++;
							}

						}

						if(codOp == BATALLA_GANADOR)
						{
							flag_BLOQUEADO = false;
							t4 = time(NULL);
							local4 = localtime(&t4);
							bloqueadoFin.minutos = local4->tm_min;
							bloqueadoFin.segundos = local4->tm_sec;
							bloqueado = tiempoTardado(bloqueadoInicio,bloqueadoFin);

							totalBloqueado.minutos += bloqueado.minutos;
							totalBloqueado.segundos += bloqueado.segundos;

							paquete = recv_capturarPokemon(fd_server);
							pokemonDat = dsrlz_capturarPokemon(&paquete,&entrenador);
							printf("%s - Objetivo Numero: %i \n",pokemonDat,nivel.numPokenest);
						}
						else if(codOp == BATALLA_MUERTE)
						{
							printf("\nHa sido elegido como víctima durante una batalla pokemon\n");
							entrenador.vidas--;
							t4 = time(NULL);
							local4 = localtime(&t4);
							bloqueadoFin.minutos = local4->tm_min;
							bloqueadoFin.segundos = local4->tm_sec;
							bloqueado = tiempoTardado(bloqueadoInicio,bloqueadoFin);

							totalBloqueado.minutos += bloqueado.minutos;
							totalBloqueado.segundos += bloqueado.segundos;

							borrarPokemones(parametros);
							cantDeadlocksPerdidos++;
							flag_SIGNALMUERTE = true;
						}
					}

					//Aca termina el if de captura

					if(nivel.cantObjetivos <= nivel.numPokenest && flag_SIGNALMUERTE == false) //Si gano el nivel
					{
						printf("Fin Nivel\n");
						borrarPokemones(parametros);

						close(fd_server);

						if(flag_SIGNALMUERTE == false)
						{
							liberarPokemonesCapturados(entrenador.pokemonesCapturados);
							avanzarNivel(&nivel,&entrenador);
							copiarMedalla(parametros, mapa.nombre);
						}
					}
					else if (flag_SIGNALMUERTE == false)
					{
						nivel.numPokenest++;
						pokenest = new_pokenest(mdata.objetivos[nivel.nivelActual],nivel.numPokenest);
					}
					break;
				}

			if(flag_SIGNALMUERTE)
			{
				close(fd_server);

				flag_BLOQUEADO = false;

				auxcantNiveles = nivel.cantNiveles;
				int nivelActual = nivel.nivelActual;
				nivel = new_nivel();
				nivel.cantNiveles = auxcantNiveles;
				entrenador.reintentos = auxreintentos++;
				nivel.finNivel = 1;
				liberarPokemonesCapturados(entrenador.pokemonesCapturados);

				if(entrenador.vidas > 0)
				{
					printf("\nMuerte - Reiniciando Mapa ... - Vidas restantes: %d\n", entrenador.vidas);
					nivel.nivelActual = nivelActual;
					reiniciarEntrenador(&entrenador);
				}

				else
				{
					entrenador = entrenador_estadoInicial;
					reiniciarEntrenador(&entrenador);
					flag_seguirJugando = informar_signalMuerteEntrenador();
				}
				flag_SIGNALMUERTE = false;
			}
	}
		flag_SIGNALMUERTE = false;
	}

	//ACA HAY QUE VER SI GANA

	if(nivel.cantNiveles == nivel.nivelActual)
	{

	t2 = time(NULL);
	local2 = localtime(&t2);
	fin.minutos = local2->tm_min;
	fin.segundos = local2->tm_sec;

	tardado = tiempoTardado(inicio, fin);
	totalBloqueadoOrganizado = organizarTiempo(totalBloqueado);

	printf("\nGanaste el Juego. Tu tiempo: %d minutos y %d segundos\n\n", tardado.minutos, tardado.segundos);
	printf("Pasó %d minutos y %d segundos bloqueado en Pokenests\n", totalBloqueadoOrganizado.minutos, totalBloqueadoOrganizado.segundos);
	printf("Estuvo involucrado en: %d deadlocks, y fue victima en: %d\n\n",cantDeadlock,cantDeadlocksPerdidos);
	//free(paquete.buffer);
	}

	return 0;
}
