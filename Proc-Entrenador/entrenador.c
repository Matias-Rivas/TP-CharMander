/*
 * entrenador.c
 *
 *  Created on: 29/8/2016
 *      Author: utnso
 */
#include <stdio.h>
#include <string.h>
#include <commons/config.h>
#include "headers/struct.h"
#include "headers/socket.h"
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>


ParametrosConsola leerParametrosConsola(char** parametros)
{
	ParametrosConsola p;

	p.nombreEntrenador = malloc(strlen(parametros[1])+1);
	p.dirPokedex = malloc(strlen(parametros[2])+1);
	strcat(p.nombreEntrenador,parametros[1]);
	strcat(p.dirPokedex,parametros[2]);

	return p;
}

void destruct_ParametrosConsola(ParametrosConsola *p)
{
	free(p->nombreEntrenador);
	free(p->dirPokedex);
}


void verificarParametros(int argc)
{
	if(argc!=3)
	{
		printf("\n\n Error - Faltan parametros\n\n");
		exit(1);
	}
}



//El calculo es 4 de "obj[" + largociudad + "]" + /n
char* concatObjetivo(char* ciudad, char* obj)
{
	obj = malloc(4*sizeof(char)+strlen(ciudad)*sizeof(char)+2);

	strcat(obj,"obj[");
	strcat(obj,ciudad);
	strcat(obj,"]");

	return obj;

}

void mostrarObjetivos (char **a)
{
	int cantViajes=0;

	printf("Objetivos Mapa: ");
	while(a[cantViajes] != '\0')
	{
			printf("%s",a[cantViajes]);
			cantViajes++;

	}
	printf("\n");

}


int cantidadDeViajes(char** hojaDeViaje)
{
	int cantViajes=0;

	while(hojaDeViaje[cantViajes] != '\0')
	{
			cantViajes++;
	}

	return cantViajes;
}
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


metadata leerMetadata()
{
	metadata mdata;
	t_config* config; //Estructura
	int cantViajes;
	char* auxiliar;

//	config = config_create("/home/utnso/SistOp/tp-2016-2c-Breaking-Bug/Proc-Entrenador/config/metadata.config");
	//Se lee el archivo con config_create y se almacena lo leido en config
	config = config_create("../config/metadata.config");

	if (config==NULL) exit(20);

	auxiliar = config_get_string_value(config,"nombre");
	mdata.nombre = malloc(strlen(auxiliar)+1);
	strcpy(mdata.nombre, auxiliar);

	//auxiliar = config_get_string_value(config,"simbolo");
	mdata.simbolo = *config_get_string_value(config,"simbolo");
	//strcpy(mdata.simbolo, auxiliar);

	mdata.hojaDeViaje = config_get_array_value(config, "hojaDeViaje");


	//---------------ACA SE LEEN LOS OBJETIVOS -------------
	cantViajes = cantidadDeViajes(mdata.hojaDeViaje);
	mdata.objetivos = malloc(cantViajes*sizeof(char*));

	//Esto deberia ir en una funcion aparte pero no se como pasar por parametro mdata.objetivos porque es un char**
	//Hay una funcion leerObjetivos que habría que arreglar
	////leerObjetivos(mdata.objetivos, config,cantViajes,mdata.hojaDeViaje);

	char* ciudad;
	char* leer = NULL;

	int i;
	for(i=0;i<cantViajes;i++)
	{
		//4*sizeof(char)+strlen(ciudad)*sizeof(char)+2
		ciudad = mdata.hojaDeViaje[i];
		leer = malloc(4*sizeof(char)+strlen(ciudad)*sizeof(char)+2);
		strcpy(leer,"obj[");
		strcat(leer,ciudad);
		strcat(leer,"]");
		mdata.objetivos[i] = config_get_array_value(config,leer);
		mostrarObjetivos(mdata.objetivos[i]);
		free(leer);
	}

	//---------------------------------------------------------

	mdata.vidas = config_get_int_value(config,"vidas");
	mdata.reintentos = config_get_int_value(config,"reintentos");

	config_destroy(config);
	return mdata;
}

ConexionEntrenador leerConexionMapa()
{
	ConexionEntrenador connect;
	t_config* config; //Estructura
	char* auxiliar;

//	config = config_create("/home/utnso/SistOp/tp-2016-2c-Breaking-Bug/Proc-Mapa/config/mapa.config");
	config = config_create("../../Proc-Mapa/config/mapa.config");
	auxiliar = config_get_string_value(config,"Puerto");
	connect.puerto = malloc(strlen(auxiliar)+1);
	strcpy(connect.puerto, auxiliar);

	auxiliar = config_get_string_value(config,"IP");
	connect.ip = malloc(strlen(auxiliar)+1);
	strcpy(connect.ip, auxiliar);

	config_destroy(config);
	return connect;

}

int main(int argc, char** argv)
{
	/*
	//Recibimos el nombre del entrenador y la direccion de la pokedex por Consola
	 *
	ParametrosConsola parametros;
	verificarParametros(argc); //Verificamos que la cantidad de Parametros sea correcta
	parametros = leerParametrosConsola(argv); //Leemos los parametros necesarios
	printf("%s --- %s", parametros.dirPokedex, parametros.nombreEntrenador);
	destruct_ParametrosConsola(&parametros);//Liberamos los parametros

	//--------------------------------
	//Ahora se deberia leer la Hoja de Viaje, la direccion de la Pokedex esta en parametros.dirPokedex
	*/

	metadata mdata;
	mdata = leerMetadata();

	//Leida la hoja de viaje, se debe buscar el socket y puerto del primer mapa

	ConexionEntrenador connect;
	connect = leerConexionMapa();

	printf("\nIP %s \nPuerto: %s \n\n", connect.ip,connect.puerto);

	//Ahora debemos conectarnos al mapa

	char* numero_IP = "127.0.0.1";
	char* numero_Puerto="9034";
	int fd_server = get_fdServer(numero_IP,numero_Puerto);

	//--------------------------------------------
	//----------------------------------------------

	//A partir de aca, comienza el juego, es decir hacer acciones en el mapa
	/*
	int a;
	while(1)
	{
		a=1;
	}
*/

	return 0;
}

