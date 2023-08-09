#pragma once
#include <Arduino.h>

enum resolucion {
	SEGUNDOS,
	MILISEGUNDOS,
	MICROSEGUNDOS
};

enum estado : byte {
	Sin_definir,
	Preparado,
	Iniciado,
	Pausado,
	Finalizado
};

class MakAlc_Alarma
{
private:
	int _repeticiones = 1;
	int _cuentaRepeticiones = 1;
	resolucion _resolucion = MILISEGUNDOS;
	byte _estado = estado::Sin_definir;
	unsigned long _tiempo = 0;
	unsigned long _inicio = 0;
	bool _llamarFuncion = false;

	void actualizaEstado();
	void asignaInicio();
	void (*_funcion)();
	
public:
	~MakAlc_Alarma();
	MakAlc_Alarma();
	MakAlc_Alarma(unsigned long, int repeticiones = 1, resolucion intervalo = MILISEGUNDOS);
	MakAlc_Alarma(unsigned long, void (*funcion)(), int repeticiones = 1, resolucion intervalo = MILISEGUNDOS);
	void Start();
	void Start(unsigned long, int repeticiones = 1, resolucion intervalo = MILISEGUNDOS);
	void Start(unsigned long, void (*funcion)(), int repeticiones = 1, resolucion intervalo = MILISEGUNDOS);
	void Define(unsigned long, int repeticiones = 1, resolucion intervalo = MILISEGUNDOS);
	void Define(unsigned long, void (*funcion)(), int repeticiones = 1, resolucion intervalo = MILISEGUNDOS);
	void ProximoEvento();	// Pendiente definir.
	void Finaliza();
	void Borra();
	void Pausa();
	bool Comprueba();
	int QuedanRepeticiones();
};

