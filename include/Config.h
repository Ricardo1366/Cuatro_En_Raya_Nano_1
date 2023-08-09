#pragma once
#include <Arduino.h>
#include <NeoPixelBus.h>
#include <MatrizLed.h>
#include <MakAlc_Alarma.h>
#include <PCF8575.h>
#include <EEPROM.h>

#define DEBUG
#define FILAS 8
#define COLUMNAS 8
void ActivaPaso();
void AsignaLetra();
void AsignaNumero();
bool Comprobar(byte);
bool ComprobarGanador();
void ComprobarLectura();
RgbColor DameColor(byte, byte);
byte DameHueco(byte);
byte DameIntensidadFondo(byte);
void DibujaTurno(byte);
bool IniciaExpansor();
void IniciaProceso();
void IniciaTablero();
void leePulsador();
void LetraEnConfig();
void MuestraConfiguracion();
void OkEnJuego();
void PintaTablero();
void SalvaConfiguracion();
void SinExpansor();
void ValoresxDefecto();

enum class Modo
{
	Horizontal,
	Vertical,
	Diagonal_derecha,
	Diagonal_izquierda
};

// Guarda los datos de la jugada ganadora.
struct Ganador
{
	byte jugador;
	byte fila;
	byte columna;
	Modo modo;
};

// Configuración del juego.
struct Config4eR
{
	byte configurado;
	byte color1;
	byte color2;
	bool Turno;
	byte NivelIntensidad;
};

// Modo configuración
enum ModoConfiguracion
{
	Ninguna,
	Turno,
	Color1,
	Color2,
	Intensidad
};

// Pulsadores
const byte P_OK = 8;
const byte P_START = 9;
const byte P_CONFIG = 10;
const byte P_REINICIO = 11;
const byte P_P1 = 12;
const byte P_P2 = 13;
const byte P_P3 = 14;
const byte P_P4 = 15;
// Procesos
const byte Pr_PonerFicha = 1;
const byte Pr_ColumnaLlena = 2;
const byte Pr_InicioTablero = 3;
const byte Pr_MuestraGanador = 4;

byte numeros[10][8] = {{28, 34, 50, 42, 38, 34, 28, 0}, {8, 12, 8, 8, 8, 8, 28, 0}, {28, 34, 32, 16, 8, 4, 62, 0}, {28, 34, 32, 24, 32, 34, 28, 0}, {16, 24, 20, 18, 62, 16, 16, 0}, {62, 2, 30, 32, 32, 34, 28, 0}, {56, 4, 2, 30, 34, 34, 28, 0}, {62, 32, 32, 16, 8, 8, 8, 0}, {28, 34, 34, 28, 34, 34, 28, 0}, {28, 34, 34, 60, 32, 34, 28, 0}};

// ########### Variables programa #############################
const byte columnas = 7; 				// Nº de columnas
const byte filas = 6;	 				// Nº de filas
byte contador = 0;		 				// Contador.
byte fila;				 				// Fila seleccionada.
byte columna;			 				// Columna seleccionada.
Config4eR config;		 				// Cofiguración del programa.
Config4eR configTemp;	 				// Valores temporales (mientras el usuario cambia la configuración)
byte salvaColumna[columnas] = {0};
byte salvaFila[filas] = {0};
byte tablero[columnas][filas] = {0};	// Control de las jugadas.
bool turno1 = true;						// Turno jugador 1
bool enProceso = false;					// Indica si hay un proceso activo.
bool comprobarFinProceso = false;		// Realiza las combrobaciones oportunas al finalizar un proceso.
bool enModoConfig = false;				// Indica si estamos en modo configuración.
bool juegoEnMarcha = false;				// Indica si ya ha empezado el juego.
volatile bool siguientePaso = false;	// Activa el siguiente paso de un proceso.
MatrizLed led;							// Clase para cálculo posición led.
Ganador ganador;						// Contiene los datos de la jugada ganadora.
MakAlc_Alarma Procesos;					// Generador de interrupciones para el control de procesos.
unsigned int repeticiones = 0;			// Nº de veces que se ejecuta un proceso.
unsigned int contadorPasos = 0;			// Lleva la cuenta de los pasos ejecutados en un proceso.
byte proceso = 0;						// Proceso activo.
unsigned int tiempo_PorPaso = 0;		// Tiempo entre un paso y el siguiente (en milisegundos)
unsigned int tiempo_PonerFicha = 250;	// Tiempo animación "Poner una ficha".
unsigned int tiempo_ColumnaLlena = 500; // Tiempo animación "Error columna llena".
int avanceFila = 0;
int avanceColumna = 0;
ModoConfiguracion modoConfig = ModoConfiguracion::Ninguna; // indica que opción está configurando el usuario.

// ######## VARIABLES CONTROL LEDS ##################
const int numeroLeds = (FILAS * COLUMNAS) + 8 + 8;	// Nº total de leds (Filas * columnas + pulsadores)
// RgbColor leds[numeroLeds];						// Array leds.
RgbColor colores[7];								// Posibles colores a utilizar. (El 0 es el de fondo)
RgbColor color[2][3];								// Array colores
byte nivel = 0;
byte letra = 255;
byte numero = 255;

// ############# Configuración Pins Nano Every ###############
const byte pinAltavoz = P10;
const byte pinInterrupciones = DD2;
const byte pinLeds = DD4;

// ############ Variables control expansor de puertos ##############
PCF8575 expansor(0x20);				 // Configuración expansor de puertos para los pulsadores
byte pulsador = 255;				 // Nº del último pulsador accionado.
volatile bool teclaPulsada = false;	 // Indica si se ha accionado algún pulsador.
PCF8575::DigitalInput pulsadores;	 // Variable para la lectura de los pulsadores.