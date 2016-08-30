/*
 * mapa.c
 *
 *  Created on: 30/8/2016
 *      Author: utnso
 */

#include <stdio.h>
#include <commons/config.h>
#include "headers/struct.h"

ParametrosMapa leerParametrosConsola(char** argv)
{
	ParametrosMapa parametros;
	parametros.nombreMapa = argv[1];
	parametros.dirPokedex = argv[2];

	return parametros;
}


void verificarParametros(int argc)
{
	if(argc!=3)
	{
		printf("Error - Faltan parametros \n");
		exit(1);
	}
}


MetadataMapa leerMetadata()
{
	MetadataMapa mdata;
	t_config* config; //Estructura

	config = config_create("config/mapa.config");

	mdata.tiempoChequeoDeadlock = config_get_int_value(config, "TiempoChequeoDeadlock");
	mdata.modoBatalla = config_get_int_value(config, "Batalla");
	mdata.algoritmo = config_get_string_value(config,"algoritmo");
	mdata.quantum = config_get_int_value(config,"quantum");
	mdata.retardo = config_get_int_value(config,"retardo");
	mdata.ip = config_get_string_value(config,"IP");
	mdata.puerto = config_get_string_value(config, "Puerto");

	return mdata;
}


int main(int argc, char** argv)
{
	/*
	ParametrosMapa parametros;
	verificarParametros(argc); //Verificamos que la cantidad de Parametros sea correcta
	parametros = leerParametrosConsola(argv); //Leemos parametros por Consola

	printf("Nombre Mapa: %s --- Dir Pokedex: %s \n",parametros.nombreMapa, parametros.dirPokedex);

	 */
	//----------------
	MetadataMapa mdata;
	mdata = leerMetadata();

	//**********************************
	//PARA HACER: FALTAN LEER LOS ARCHIVOS DE CONFIGURACION DE POKEMON Y POKENEST, YA ESTAN LAS ESTRUCTURAS DEFINIDAS EN EL HEADER!

	printf("Batalla: %i",mdata.modoBatalla);



}
