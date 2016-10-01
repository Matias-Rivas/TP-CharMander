/*
 * deadlock.c
 *
 *  Created on: 19/9/2016
 *      Author: utnso
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <curses.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <tad_items.h>

typedef struct{
	char simbolo;
}Pokemon;

typedef struct{
	char simbolo;
	t_list* recursos;
	char** objetivos_restantes;
	int cantidad_restante;
}EntrenadorDL;

typedef struct{
	char simbolo;
	int cant;
}Pokenest;



int** inicializar_matriz(int cant_filas, int cant_columnas)
{
	int i=0,j=0;
	int** matriz;
	matriz=malloc(cant_filas*sizeof(int));
	for(j=0;j<cant_columnas;j++)
	{
		matriz[j]=malloc(cant_filas*sizeof(int));
	}
	for(i=0;i<cant_filas;i++)
	{
		for(j=0;j<cant_columnas;j++)
		{
			matriz[i][j]=0;
		}
	}
	return matriz;
}

void mostrar_matriz(int** matriz,int filas, int columnas)
{
	int i,j;
	for(i=0;i<filas;i++)
	{
		for(j=0;j<columnas;j++)
		{
			printf("%d",matriz[i][j]);
		}
		printf("\n");
	}
}


int cantidad_obtenidos_de_un_tipo(EntrenadorDL entrenador,char simbolo)//CANT DE POKEMONES ASIGNADOS A ESE ENTRENADOR DE UNA POKENEST
{
	int i=0,cantidad_total,cantidad=0;
	Pokemon* pokemon_aux;
	cantidad_total = list_size(entrenador.recursos);
	while(i<cantidad_total)
	{
		pokemon_aux=list_get(entrenador.recursos,i);
		if(pokemon_aux->simbolo==simbolo) //SI ESE POKEMON ES DE ESA POKENEST
		{
			cantidad++;
		}
		i++;
	}
	return cantidad;
}

int cantidad_a_obtener_de_un_tipo(EntrenadorDL entrenador,char simbolo)//CANT DE OBJETIVOS A OBTENER DE UNA POKENEST
{
	int i=0,cantidad_total,cantidad=0;
	cantidad_total = entrenador.cantidad_restante;
	while(i<cantidad_total)
	{
		if(entrenador.objetivos_restantes[i][0]==simbolo) //SI ESE POKEMON ES DE ESA POKENEST
		{
			cantidad++;
		}
		i++;
	}
	return cantidad;
}

int** generar_matriz_peticiones_maximas(t_list* entrenadores, t_list* pokenests)
{
	EntrenadorDL* entrenador_aux;
	Pokenest* pokenest_aux;
	int i=0,j=0,cantpokenests,cantentrenadores,cantidad_recursos;

	int** matriz= inicializar_matriz(list_size(entrenadores), list_size(pokenests));//DEVUELVE MATRIZ CON TODOS 0

	cantpokenests=list_size(pokenests);
	cantentrenadores=list_size(entrenadores);

	while(i<cantpokenests)
	{
		pokenest_aux = (Pokenest*)list_get(pokenests,i);
		while(j<cantentrenadores)
		{
			entrenador_aux = (EntrenadorDL*)list_get(entrenadores,j);
			cantidad_recursos = cantidad_a_obtener_de_un_tipo(*entrenador_aux, pokenest_aux->simbolo); //CANT DE POKEMONES QUE EL ENTRENADOR NECESITA DE ESA POKENEST
			matriz[j][i]=cantidad_recursos;
			j++;
		}
		i++;
	}
	return matriz;
}

int** generar_matriz_asignados(t_list* entrenadores, t_list* pokenests)
{
	EntrenadorDL* entrenador_aux;
	Pokenest* pokenest_aux;
	int i=0,j=0,cantpokenests,cantentrenadores,cantidad_recursos;

	int** matriz= inicializar_matriz(list_size(entrenadores), list_size(pokenests));//DEVUELVE MATRIZ CON TODOS 0

	cantpokenests=list_size(pokenests);
	cantentrenadores=list_size(entrenadores);

	while(i<cantpokenests)
	{
		pokenest_aux = (Pokenest*)list_get(pokenests,i);
		while(j<cantentrenadores)
		{
			entrenador_aux = (EntrenadorDL*)list_get(entrenadores,j);
			cantidad_recursos = cantidad_obtenidos_de_un_tipo(*entrenador_aux, pokenest_aux->simbolo); //CANT DE POKEMONES ASIGNADOS A ESE ENTRENADOR DE ESA POKENEST
			matriz[j][i]=cantidad_recursos;
			j++;
		}
		i++;
	}
	return matriz;
}

int* generar_vector_recursos_disponibles(t_list* pokenests)
{
	int i=0,tamanio;
	int* vector=malloc(sizeof(int)*(list_size(pokenests)));
	tamanio=list_size(pokenests);
	while(i<tamanio)
	{
		vector[i]=((Pokenest*)list_get(pokenests,i))->cant;
		i++;
	}
	return vector;
}

void detectar_deadlock(int** matriz_peticiones_maximas, int** matriz_recursos_asignados,int* recursos_disponibles, int entrenadores, int pokenests)
{
	bool hay_deadlock=false, tiene_algun_recurso;
	int en_deadlock[entrenadores], no_estan_en_deadlock[entrenadores];
	int recursos_diferentes_disponibles; //PARA CONTROLAR QUÉ RECURSOS PUEDE CONSEGUIR UN ENTRENADOR DE LAS POKENESTS
	int i,j;
	int** matriz_necesidad = inicializar_matriz(entrenadores,pokenests);

    for(i=0;i<entrenadores;i++)
    {
    	no_estan_en_deadlock[i]=0; //HASTA QUE NO VERIFIQUEMOS LOS RECURSOS QUE NECESITA CADA ENTRENADOR, ASUMIMOS QUE ESTA EN DL
    }

    /////SE CARGA MATRIZ DE NECESIDAD

	for(i=0;i<entrenadores;i++)
	{
		for(j=0;j<pokenests;j++)
		{
			matriz_necesidad[i][j] = matriz_peticiones_maximas[i][j]-matriz_recursos_asignados[i][j];
		}
	}

	for(i=0;i<entrenadores;i++)  //RECORRO CADA ENTRENADOR
	{	tiene_algun_recurso = false;
		recursos_diferentes_disponibles = 0; //POR AHORA NO NECESITA
		for(j=0;j<pokenests;j++)  //RECORRO CADA POKENEST (RECURSO)
		{
			if(matriz_recursos_asignados[i][j] > 0)
			{
				tiene_algun_recurso = true;
			}
		    if( (!no_estan_en_deadlock[i])  &&  (matriz_necesidad[i][j] <= recursos_disponibles[j]) )  //VEMOS SI LA POKENEST (RECURSO) TIENE LAS INSTANCIAS QUE PIDE EL ENTRENADOR
		    {
		    	recursos_diferentes_disponibles++; //ESTE ENTRENADOR PUEDE OBTENER LAS INSTANCIAS DE ESA POKENEST

		    }
		}
    	if(recursos_diferentes_disponibles == pokenests) //PUEDE OBTENER TODOS LOS POKEMONES DE LAS POKENESTS
    	{
    		no_estan_en_deadlock[i] = 1; //ESE ENTRENADOR NO ESTA EN DL
    	}
		if((!tiene_algun_recurso) && (no_estan_en_deadlock[i] == 0))
		{
			no_estan_en_deadlock[i] = 1;    //NO TIENE RECURSO ASIGNADO, NO CUMPLE "ESPERA"
		}
	}

	j=0;

	for(i=0;i<entrenadores;i++)
	{
		if(!no_estan_en_deadlock[i])
		{
			hay_deadlock = true;
			en_deadlock[j] = i; //GUARDO UN VECTOR CON EL NRO DE FILA DE LOS ENTRENADORES EN DEADLOCK
			j++;
		}
	}

}

int espera_circular(int recurso_esperado, int** matriz_asignados,int entrenadores)
{

}

bool tiene_lo_que_pide(EntrenadorDL* entrenador, EntrenadorDL* otro_entrenador)
{
	int i,j;
	Pokemon* pokeaux;
	for(i=0;i<entrenador->cantidad_restante;i++)
	{
		for(j=0;j<list_size(otro_entrenador->recursos);j++)
		{
			if(entrenador->objetivos_restantes[i][0]==*(char*)list_get(otro_entrenador->recursos,j))
			{
				return true;
			}
		}
	}
	return false;
}

t_list* separar_deadlock(t_list* entrenadores)    //ME DEVUELVE UN DEADLOCK
{
	t_list* deadlock = list_create();
	EntrenadorDL* aux,*primero;
	int cantidad,i=0;
	primero = ((EntrenadorDL*)list_get(entrenadores,0));//COMO TODOS ESTAN EN DEADLOCK, AGARRO UNO Y VEO CON CUALES ESTA EN DEADLOCK

	for(i=1;i<cantidad;i++)
	{
		aux = (EntrenadorDL*)list_get(entrenadores,i);
		if(tiene_lo_que_pide(primero,aux))
		{
			list_add(deadlock,aux);
		}
	}

	return deadlock;

}

int main(void)
{
    t_list* pokenests = list_create();
    t_list* entrenadores = list_create();




}