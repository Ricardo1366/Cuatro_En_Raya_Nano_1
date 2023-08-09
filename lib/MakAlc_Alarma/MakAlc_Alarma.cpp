#include <MakAlc_Alarma.h>

// Inicializa sin parámetros.
// Es necesario configurarla antes de utilizarla.
MakAlc_Alarma::MakAlc_Alarma()
{
	_estado = estado::Sin_definir;
}

// Inicializa la alarma para que devuelva 'True' una vez que pase el plazo establecido.
// Parámetros:
// tiempo -> Tiempo asignado a la alarma (obligatorio)
// repeticiones -> Número de veces que se ejecuta antes de desactivarse (opcional)
//    Por defecto 1. Cero o -1 indica que siempre está activa.
// intervalo -> Unidad de medida del parámetro tiempo (opcional)
//    Se pueden utilizar segundos, milisegundos (por defecto) o microsegundos.
MakAlc_Alarma::MakAlc_Alarma(unsigned long tiempo, int repeticiones = 1, resolucion intervalo = MILISEGUNDOS)
{
	_estado = estado::Preparado;
	_tiempo = tiempo;
	_repeticiones = repeticiones;
	_cuentaRepeticiones = repeticiones;
	_resolucion = intervalo;
	_llamarFuncion = false;
}

// Inicializa la alarma para que ejecute una función una vez que pase el plazo establecido.
// Parámetros:
// tiempo -> Tiempo asignado a la alarma (obligatorio)
// funcion -> Función a jecutar (obligatorio)
//    La función tiene que ser de tipo "void" sin parámetros.
// repeticiones -> Número de veces que se ejecuta antes de desactivarse (opcional)
//    Por defecto 1. Cero o -1 indica que siempre está activa.
// intervalo -> Unidad de medida del parámetro tiempo (opcional)
//    Se pueden utilizar segundos, milisegundos (por defecto) o microsegundos.
MakAlc_Alarma::MakAlc_Alarma(unsigned long tiempo, void (*funcion)(), int repeticiones = 1, resolucion intervalo = MILISEGUNDOS)
{
	_estado = estado::Preparado;
	_tiempo = tiempo;
	_repeticiones = repeticiones;
	_cuentaRepeticiones = repeticiones;
	_resolucion = intervalo;
	_funcion = funcion;
	_llamarFuncion = true;
}

// Configura la alarma para que devuelva 'True' una vez que pase el plazo establecido.
// Parámetros:
// tiempo -> Tiempo asignado a la alarma (obligatorio)
// repeticiones -> Número de veces que se ejecuta antes de desactivarse (opcional)
//    Por defecto 1. Cero o -1 indica que siempre está activa.
// intervalo -> Unidad de medida del parámetro tiempo (opcional)
//    Se pueden utilizar segundos, milisegundos (por defecto) o microsegundos.
void MakAlc_Alarma::Define(unsigned long tiempo, int repeticiones = 1, resolucion intervalo = MILISEGUNDOS)
{
	_estado = estado::Preparado;
	_tiempo = tiempo;
	_repeticiones = repeticiones;
	_cuentaRepeticiones = repeticiones;
	_resolucion = intervalo;
	_llamarFuncion = false;
}

// Configura la alarma para que ejecute una función una vez que pase el plazo establecido.
// Parámetros:
// tiempo -> Tiempo asignado a la alarma (obligatorio)
// funcion -> Función a jecutar (obligatorio)
//    La función tiene que ser de tipo "void" sin parámetros.
// repeticiones -> Número de veces que se ejecuta antes de desactivarse (opcional)
//    Por defecto 1. Cero o -1 indica que siempre está activa.
// intervalo -> Unidad de medida del parámetro tiempo (opcional)
//    Se pueden utilizar segundos, milisegundos (por defecto) o microsegundos.
void MakAlc_Alarma::Define(unsigned long tiempo, void (*funcion)(), int repeticiones = 1, resolucion intervalo = MILISEGUNDOS)
{
	_estado = estado::Preparado;
	_tiempo = tiempo;
	_repeticiones = repeticiones;
	_cuentaRepeticiones = repeticiones;
	_resolucion = intervalo;
	_llamarFuncion = true;
	_funcion = funcion;
}

// Inicia una "Alarma".
// El primer evento sucede al pasar el tiempo establecido.
void MakAlc_Alarma::Start()
{
	switch (_estado)
	{
	case estado::Preparado:
		// Alarma configurada y pendiente de iniciarse. La activamos.
		asignaInicio();
		break;
	case estado::Finalizado:
		// Alarma configurada y finalizada. Iniciamos el contador de repeticiones y la activamos.
		_cuentaRepeticiones = _repeticiones;
		asignaInicio();
		break;
	case estado::Pausado:
		// Alarma configurada y pausada de iniciarse. La reactivamos.
		asignaInicio();
		break;
	default:
		// Resto de estados, no hacemos nada.
		break;
	}
}

// Inicia la "Alarma" y lanza el primer evento.
void MakAlc_Alarma::Start(unsigned long tiempo, int repeticiones = 1, resolucion intervalo = MILISEGUNDOS)
{
	_tiempo = tiempo;
	_repeticiones = repeticiones;
	_cuentaRepeticiones = repeticiones;
	_resolucion = intervalo;
	_llamarFuncion = false;
	_estado = estado::Iniciado;
	actualizaEstado();
}

// Inicia la "Alarma" y lanza el primer evento.
void MakAlc_Alarma::Start(unsigned long tiempo, void (*funcion)(), int repeticiones = 1, resolucion intervalo = MILISEGUNDOS)
{
	_tiempo = tiempo;
	_repeticiones = repeticiones;
	_cuentaRepeticiones = repeticiones;
	_resolucion = intervalo;
	_funcion = funcion;
	_llamarFuncion = true;
	_estado = estado::Iniciado;
	_funcion();
	_inicio = millis();
	actualizaEstado();
}

// Comprueba si se ha cumplido el plazo definido. Si es así devuelve true.
// Si se ha informado nº de repeticiones la alarma se ejecutará tantas veces
// como se haya indicado. Luego se desactiva.
// Si se ha informado una función además de devolver true, ejecuta la función.
bool MakAlc_Alarma::Comprueba()
{
	// Si no se ha iniciado se devuelve "false".
	if (_estado != estado::Iniciado)
	{
		return false;
	}

	// En función de la resolución hacemos unas comprobaciones u otras.
	switch (_resolucion)
	{
	case MILISEGUNDOS:

		if (millis() - _inicio >= _tiempo)
		{
			_inicio = millis();
			// Se ha igualado o superado el tiempo de espera.
			actualizaEstado();
			if (_llamarFuncion)
			{
				_funcion();
			}
			return true;
		}
		else
		{
			return false;
		}
		break;

	case MICROSEGUNDOS:
		if (micros() - _inicio >= _tiempo)
		{
			// Se ha igualado o superado el tiempo de espera.
			_inicio = micros();
			actualizaEstado();
			if (_llamarFuncion)
			{
				_funcion();
			}
			return true;
		}
		else
		{
			return false;
		}
		break;

	case SEGUNDOS:

		if ((millis() - _inicio) / 1000 >= _tiempo)
		{
			// Se ha igualado o superado el tiempo de espera.
			_inicio = millis();
			actualizaEstado();
			if (_llamarFuncion)
			{
				_funcion();
			}
			return true;
		}
		else
		{
			return false;
		}
		break;

	default:

		break;
	}
}

// Elimina la configuración de una alarma.
// Es necesario volver a definirla para utilizarla de nuevo.
// No puede borrarse la definición de una alarma activa. Primero hay que pararla o finalizarla.
void MakAlc_Alarma::Borra()
{
	if (_estado == estado::Iniciado)
	{
		return;
	}
	_estado = estado::Sin_definir;
}

// Si la "Alarma" está iniciada la pausa.
// Con Start se vuelve a activar y continua.
void MakAlc_Alarma::Pausa()
{
	if (_estado != estado::Iniciado)
	{
		return;
	}
	_estado = estado::Pausado;
}

// Controla si se ha llegado a la última repetición.
void MakAlc_Alarma::actualizaEstado()
{
	if (_cuentaRepeticiones > 0)
	{
		// Actualizamos las repeticiones que faltan para finalizar el clclo.
		_cuentaRepeticiones--;
		// Si hemos alcanzado el número de repeticiones marcamos como finalizado.
		if (_cuentaRepeticiones == 0)
		{
			_estado = estado::Finalizado;
		}
	}
}

// Marca una "Alarma" como finalizda.
// Con Start() vuelve a ejecutarse de nuevo.
void MakAlc_Alarma::Finaliza()
{
	_estado = estado::Finalizado;
	_cuentaRepeticiones = 0;
}

// Asigna el "tiempo" a partir del cual contamos.
void MakAlc_Alarma::asignaInicio()
{
	_estado = estado::Iniciado;
	switch (_resolucion)
	{
	case SEGUNDOS:
		_inicio = millis();
		break;

	case MILISEGUNDOS:
		_inicio = millis();
		break;

	case MICROSEGUNDOS:
		_inicio = micros();
		break;

	default:
		break;
	}
}

// Devuelve el número de repeticiones pendientes de ejecutar;
int MakAlc_Alarma::QuedanRepeticiones()
{
	if (_estado != 2)
	{
		return 0;
	}
	return _cuentaRepeticiones;
}

MakAlc_Alarma::~MakAlc_Alarma()
{
	
}
