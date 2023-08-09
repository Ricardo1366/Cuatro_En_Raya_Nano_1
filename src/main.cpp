#include <Arduino.h>
// #include <MatrizLed.h>
// #include <MakAlc_Alarma.h>
// #include <NeoPixelBus.h>
#include <Config.h>
#define SIMULA_PULSADOR
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> tiraLeds(numeroLeds, pinLeds);

#if defined(SIMULA_PULSADOR)
MakAlc_Alarma pulsadorAuto(5000, ComprobarLectura, 21);
#endif
void setup()
{
  byte intensidad1 = 0;
  
  byte intensidad2 = 0;
  byte intensidadFondo = 0;

  // Si estamos en modo "Depurar" activamos la comunicación con el monitor serial.
#if defined(DEBUG)
  Serial.begin(115200);
  delay(1);
  Serial.println(F("Inicio configuración. "));
#endif
  // Leemos la configuración almacenada. Si algun valor no es correcto es porque no se ha configurado
  // y cargamos los valores por defecto.
  EEPROM.begin();
  config = EEPROM.get(0, config);
  if (config.configurado != 1 || config.color1 > 6 || config.color2 > 6 || config.NivelIntensidad > 8)
  {
#ifdef DEBUG
    Serial.println("Valores incorrectos: ");
    Serial.print(config.color1);
    Serial.print(", ");
    Serial.print(config.color2);
    Serial.print(", ");
    Serial.print(config.configurado);
    Serial.print(", ");
    Serial.print(config.NivelIntensidad);
    Serial.print(", ");
    Serial.println(config.Turno);
#endif // DEBUG

    ValoresxDefecto();
  }
#ifdef DEBUG
  else
  {
    Serial.println("Configurado correctamente.");
  }
#endif // DEBUG
  EEPROM.end();
#ifdef DEBUG
  Serial.println("Valores Leidos: ");
  Serial.print("Color J1: ");Serial.println(config.color1);
  Serial.print("Color J2: ");  Serial.println(config.color2);
  Serial.print("Configurado: "); Serial.println(config.configurado);
  Serial.print("Intensidad: ");  Serial.println(config.NivelIntensidad);
  Serial.print("Turno: ");  Serial.println(config.Turno);
#endif // DEBUG

  // Los leds son de tipo GRB
  colores[0] = RgbColor(0xFF, 0xFF, 0xFF); // Fondo (Blanco)
  colores[1] = RgbColor(0xFF, 0x00, 0x00); // Rojo
  colores[2] = RgbColor(0xFF, 0xFF, 0x00); // Amarillo
  colores[3] = RgbColor(0x00, 0xFF, 0x00); // Verde
  colores[4] = RgbColor(0x00, 0x00, 0xFF); // Azul
  colores[5] = RgbColor(0x00, 0xFF, 0xFF); // Verde + Azul
  colores[6] = RgbColor(0xFF, 0x00, 0xFF); // Violeta

  // Definimos los colores;
  intensidad1 = config.NivelIntensidad;

  if (intensidad1 < 5)
  {
    intensidad2 = intensidad1 * 2;
    intensidadFondo = intensidad1 > 1 ? intensidad1 / 2 : 1;
  }
  else
  {
    intensidad2 = intensidad1 / 2;
    intensidadFondo = intensidad2 - 1;
  }
  color[0][0] = DameColor(0, intensidadFondo);         // Fondo
  color[0][1] = DameColor(config.color1, intensidad1); // Jugador 1
  color[0][2] = DameColor(config.color2, intensidad1); // Jugador 2
  color[1][0] = DameColor(0, intensidadFondo);         // Fondo
  color[1][1] = DameColor(config.color1, intensidad2); // Jugador 1
  color[1][2] = DameColor(config.color2, intensidad2); // Jugador 2

#if defined(DEBUG)
  Serial.print(color[0][0].R);
  Serial.print("|");
  Serial.print(color[0][0].G);
  Serial.print("|");
  Serial.print(color[0][0].B);

  Serial.print("  -  ");

  Serial.print(color[1][0].R);
  Serial.print("|");
  Serial.print(color[1][0].G);
  Serial.print("|");
  Serial.println(color[1][0].B);

  Serial.print(color[0][1].R);
  Serial.print("|");
  Serial.print(color[0][1].G);
  Serial.print("|");
  Serial.print(color[0][1].B);

  Serial.print("  -  ");

  Serial.print(color[1][1].R);
  Serial.print("|");
  Serial.print(color[1][1].G);
  Serial.print("|");
  Serial.println(color[1][1].B);

  Serial.print(color[0][2].R);
  Serial.print("|");
  Serial.print(color[0][2].G);
  Serial.print("|");
  Serial.print(color[0][2].B);

  Serial.print("  -  ");

  Serial.print(color[1][2].R);
  Serial.print("|");
  Serial.print(color[1][2].G);
  Serial.print("|");
  Serial.println(color[1][2].B);
#endif

  // Asignamos turno.
  turno1 = config.Turno;

  // Habilitamos la matriz de leds.

  // LEDS.addLeds<WS2812B, pinLeds, RGB>(leds, numeroLeds);

  // Configuramos los pines I/O del ESP8266
  pinMode(pinAltavoz, OUTPUT);
  pinMode(pinInterrupciones, INPUT);

  // Habilitamos todas las patillas (16) como entradas.
  for (byte i = 0; i < 16; i++)
  {
    expansor.pinMode(i, INPUT);
  }
  expansor.begin();

  led.begin(COLUMNAS, FILAS, ml_Inicio::Inicio_Inferior_Izquierda, ml_Direccion::Direccion_Vertical);

  // Antes de activar las interrupciones del ESP8266 leemos el expansor por si hay alguna interrupción
  // pendiente, borrarla y empezar "limpios".
  pulsadores = expansor.digitalReadAll();

  // ### AQUÍ ###
  // Comprobar si es FALLING u otro modo.
  // Habilita las interrupciones.
  attachInterrupt(digitalPinToInterrupt(pinInterrupciones), ComprobarLectura, FALLING);

  // Iniciamos el tablero.
  IniciaTablero();
  pulsadorAuto.Start();
}

void loop()
{
  pulsadorAuto.Comprueba();
  Procesos.Comprueba();
  if (teclaPulsada && !enProceso)
  {
    // La ponemos a false para entrar solo una vez.
    teclaPulsada = false;
#if defined(DEBUG)
    Serial.println(F("Detectada tecla pulsada. "));
#endif
    // Lee el valor del pulsador (0 - 15)

    leePulsador();
#if defined(DEBUG)
    Serial.print(F("Has pulsado el pulsador n. "));
    Serial.println(pulsador);
#endif

    // Comprobamos que la lectura sea buena.
    if (pulsador > 15)
    {
      pulsador = 255;
    }

    if (pulsador < 8)
    {
      // El pulsador corresponde a una letra/columna.
      letra = pulsador;

      // Si estamos jugando vamos a comprobar.
      if (juegoEnMarcha)
      {
        AsignaLetra();
#if defined(DEBUG)
        Serial.print(F("Han seleccionado columna "));
        Serial.println(letra);
#endif
      }
      if (enModoConfig)
      {
        // Estamos en modo configuración.
        LetraEnConfig();
      }
    }

    // Han pulsado una tecla de acción. #########
    if (pulsador >= 8 && pulsador < 16)
    {
#if defined(DEBUG)
      Serial.println(F("Asignamos modo. "));
#endif
      switch (pulsador)
      {
      case P_OK:
#if defined(DEBUG)
        Serial.println(F("Han pulsado OK. "));
#endif
        if (juegoEnMarcha)
        {
          OkEnJuego();
        };
        if (enModoConfig)
        {
          SalvaConfiguracion();
          enModoConfig = false;
        }
        break;
      case P_START:
        if (enModoConfig)
        {
          // Activamos el modo juego sin salvar la configuración.
          enModoConfig = false;
        }

        juegoEnMarcha = true;
#ifdef DEBUG
        Serial.println(F("\nJuego iniciado."));
#endif // DEBUG \
	// Borramos el tablero por si este no es el primer juego.
        memset(tablero, 0, sizeof(tablero));
        PintaTablero();
        break;
      case P_CONFIG:
        // Si estamos en una partida no se accede al modo configuración.
        if (!juegoEnMarcha)
        {
          // Si no estamos en modo configuración copiamos los valores a la variable temporal.
          if (!enModoConfig)
          {
            configTemp = config;
          }
          enModoConfig = true;
        }
        break;
      default:
        // Comprobamos el resto de pulsadores.
        if (enModoConfig)
        {
          if (pulsador == P_P1)
          {
            modoConfig = ModoConfiguracion::Turno;
          }
          if (pulsador == P_P2)
          {
            modoConfig = ModoConfiguracion::Color1;
          }
          if (pulsador == P_P3)
          {
            modoConfig = ModoConfiguracion::Color2;
          }
          if (pulsador == P_P4)
          {
            modoConfig = ModoConfiguracion::Intensidad;
          }
        }
        break;
      }
    }
  }

  // Estamos en un proceso.
  if (enProceso)
  {
    if (siguientePaso)
    {
      siguientePaso = false;
      contador++;
      switch (proceso)
      {
      case Pr_PonerFicha:
        tablero[columna][filas - contador] = turno1 ? 1 : 2;
#if defined(DEBUG)
        Serial.print(F("Iluminando columna/fila = valor "));
        Serial.print(columna);
        Serial.print("/");
        Serial.print(filas - contador);
        Serial.print(" = ");
        Serial.println(turno1 ? 1 : 2);
#endif
        if (contador > 1)
        {
          tablero[columna][filas - contador + 1] = 0;
#if defined(DEBUG)
          Serial.print(F("Apagando columna/fila "));
          Serial.print(columna);
          Serial.print("/");
          Serial.println(filas - contador + 1);
#endif
        }
        // Visualizamos el tablero.
        PintaTablero();
        break;
      case Pr_ColumnaLlena:
        for (byte i = 0; i < filas; i++)
        {
          tablero[columna][i] = contador % 2 == 1 ? 0 : salvaColumna[i];
        }
        // Visualizamos el tablero.
        PintaTablero();
        break;
      case Pr_InicioTablero:
#if defined(DEBUG)
        Serial.println(F("Iniciando tablero"));
#endif
        for (byte i = 0; i < 8; i++)
        {
          for (byte j = 0; j < 8; j++)
          {
            tiraLeds.SetPixelColor(led.posicion(i, j), (i + 1) == contador ? color[nivel][0] : RgbColor(0, 0, 0));
#if defined(DEBUG)
          Serial.print((i + 1) == contador ? color[nivel][0].R : 0);
          Serial.print(", ");
          Serial.print((i + 1) == contador ? color[nivel][0].G : 0);
          Serial.print(", ");
          Serial.print((i + 1) == contador ? color[nivel][0].B : 0);
          Serial.print("| ");
#endif
          }
          Serial.println();
        }
#if defined(DEBUG)
        Serial.println(F("Tablero encendido"));
#endif
        tiraLeds.Show();
        break;
      case Pr_MuestraGanador:
        nivel++;
        if (nivel > 1)
        {
          nivel = 0;
        }
        switch (ganador.modo)
        {
        case Modo::Vertical:
          avanceFila = 1;
          avanceColumna = 0;
          break;
        case Modo::Horizontal:
          avanceFila = 0;
          avanceColumna = 1;
          break;
        case Modo::Diagonal_derecha:
          avanceFila = 1;
          avanceColumna = 1;
          break;
        case Modo::Diagonal_izquierda:
          avanceFila = 1;
          avanceColumna = -1;
          break;
        default:
          break;
        }
        for (byte i = 0; i < 4; i++)
        {
          tiraLeds.SetPixelColor(led.posicion(ganador.columna + i * avanceColumna, ganador.fila + i * avanceFila), color[nivel][ganador.jugador]);
          // leds[led.posicion(ganador.columna + i * avanceColumna, ganador.fila + i * avanceFila)] = color[nivel][ganador.jugador];
        }
        tiraLeds.Show();
        juegoEnMarcha = false;
        break;
      default:
        break;
      }

      // Comprobamos si es la última repetición.
      if (Procesos.QuedanRepeticiones() == 0)
      {

        // Informamos "Fin del proceso".
        enProceso = false;
        contador = 0;
        tiempo_PorPaso = 0;

        // Borramos cualquier pulsación de tecla que pueda haber sucedido durante el proceso.
        teclaPulsada = false;
        pulsadores = expansor.digitalReadAll();

        // Realizamos las comprobaciones de fin de proceso.
        comprobarFinProceso = true;
      }
    }
  }

  if (comprobarFinProceso)
  {
#if defined(DEBUG)
    Serial.print(F("Comprobamos final proceso "));
    Serial.println(proceso);
#endif
    comprobarFinProceso = false;
    switch (proceso)
    {
    case Pr_PonerFicha:
      if (ComprobarGanador())
      {
        // El jugador del turno actual ha ganado.
#if defined(DEBUG)
        Serial.print(F("Ha ganado el jugador: "));
        Serial.println(turno1 ? 1 : 2);
#endif
        tiempo_PorPaso = tiempo_ColumnaLlena;
        repeticiones = 10;
        proceso = Pr_MuestraGanador;
        enProceso = true;
        siguientePaso = true;
        IniciaProceso();
      }
      else
      {
        // Cambiamos de turno.
        turno1 = !turno1;
#if defined(DEBUG)
        Serial.println(F("Cambio de turno "));
#endif
        // Apagamos la letra que esté encendida.
        letra = 255;
        AsignaLetra();
      }
      break;
    case Pr_ColumnaLlena:
      // No se comprueba nada (por ahora).
      break;
    case Pr_InicioTablero:
      tiraLeds.ClearTo(RgbColor(0, 0, 0));
      PintaTablero();
      break;
    case Pr_MuestraGanador:
      // Borramoa la letra.
      letra = 255;
      AsignaLetra();

      PintaTablero();
      break;
    default:
      break;
    }
  }
  pulsador = 255;
}

// Interrupción al presionar un pulsador.
void ComprobarLectura()
{
  // Ha saltado la interrupción. El pin 1 del expansor se han puesto a cero.
  teclaPulsada = true;
}

// Animación inicio al encender el tablero.
void IniciaTablero()
{
  // Configuramos y activamos el proceso de animación.
  proceso = Pr_InicioTablero;
  repeticiones = 8;
  tiempo_PorPaso = 500;
  enProceso = true;
  siguientePaso = true;
  IniciaProceso();
}

// Activa un proceso determinado.
void IniciaProceso()
{
  // contadorPasos = 0;
  Procesos.Start(tiempo_PorPaso, ActivaPaso, repeticiones);
}

// Activa el siguiente paso de un proceso.
void ActivaPaso()
{
  siguientePaso = true;
}

// Lee la tecla pulsada.
void leePulsador()
{
  
#if defined(SIMULA_PULSADOR)
  static byte pulsaciones;
  if(pulsaciones == 0) {pulsador = P_START;}
  if(pulsaciones == 1) {pulsador = 2;}
  if(pulsaciones == 2) {pulsador = P_OK;}
  if(pulsaciones == 3) {pulsador = 3;}
  if(pulsaciones == 4) {pulsador = P_OK;}
  if(pulsaciones == 5) {pulsador = 3;}
  if(pulsaciones == 6) {pulsador = P_OK;}
  if(pulsaciones == 7) {pulsador = 2;}
  if(pulsaciones == 8) {pulsador = P_OK;}
  if(pulsaciones == 9) {pulsador = 4;}
  if(pulsaciones == 10) {pulsador = P_OK;}
  if(pulsaciones == 11) {pulsador = 2;}
  if(pulsaciones == 12) {pulsador = P_OK;}
  if(pulsaciones == 13) {pulsador = 4;}
  if(pulsaciones == 14) {pulsador = P_OK;}
  if(pulsaciones == 15) {pulsador = 2;}
  if(pulsaciones == 16) {pulsador = P_OK;}
  if(pulsaciones == 17) {pulsador = 5;}
  if(pulsaciones == 18) {pulsador = P_OK;}
  if(pulsaciones == 19) {pulsador = 2;}
  if(pulsaciones == 20) {pulsador = P_OK;}
  if(pulsaciones == 21) {pulsador = 0;}
  if(pulsaciones == 22) {pulsador = P_OK;}
  if(pulsaciones == 23) {pulsador = 0;}
  if(pulsaciones == 24) {pulsador = P_OK;}
  if(pulsaciones == 25) {pulsador = 0;}
  if(pulsaciones == 26) {pulsador = P_OK;}
  if(pulsaciones == 27) {pulsador = 0;}
  pulsaciones++;
#else
  pulsadores = expansor.digitalReadAll();

  if (pulsadores.p0 == HIGH)
    pulsador = 0;
  if (pulsadores.p1 == HIGH)
    pulsador = 1;
  if (pulsadores.p2 == HIGH)
    pulsador = 2;
  if (pulsadores.p3 == HIGH)
    pulsador = 3;
  if (pulsadores.p4 == HIGH)
    pulsador = 4;
  if (pulsadores.p5 == HIGH)
    pulsador = 5;
  if (pulsadores.p6 == HIGH)
    pulsador = 6;
  if (pulsadores.p7 == HIGH)
    pulsador = 7;
  if (pulsadores.p8 == HIGH)
    pulsador = 8;
  if (pulsadores.p9 == HIGH)
    pulsador = 9;
  if (pulsadores.p10 == HIGH)
    pulsador = 10;
  if (pulsadores.p11 == HIGH)
    pulsador = 11;
  if (pulsadores.p12 == HIGH)
    pulsador = 12;
  if (pulsadores.p13 == HIGH)
    pulsador = 13;
  if (pulsadores.p14 == HIGH)
    pulsador = 14;
  if (pulsadores.p15 == HIGH)
    pulsador = 15;

#endif
}

// Comprueba si hay huecos libres en la columna elegida.
bool Comprobar(byte _columna)
{

#if defined(DEBUG)
  Serial.println(F("Comprobamos hay un hueco libre."));
#endif
  if (_columna >= columnas)
  {
#if defined(DEBUG)
    Serial.print(F("Columna incorrecta "));
    Serial.println(_columna);
#endif

    return false;
  }
  fila = DameHueco(_columna);
  if (fila < filas)
  {
    columna = _columna;
#if defined(DEBUG)
    Serial.print(F("Seleción columna/fila "));
    Serial.print(columna);
    Serial.print("/");
    Serial.println(fila);
#endif
    return true;
  }
  // La columna está llena. Mostramos error.
#if defined(DEBUG)
  Serial.println(F("Error columna llena."));
#endif
  letra = 255;
  return false;
}

// Comprobamos si el jugador actual ha conseguido la victoria.
bool ComprobarGanador()
{
  // Comprobamos Lineas horizontales.
  byte turno = turno1 ? 1 : 2;
  for (byte f = 0; f < 6; f++) // Se comprueban todas las filas.
  {
    for (byte c = 0; c < 4; c++) // En cada fila pueden haber 4 posiciones ganadoras
    {
      if (tablero[c][f] == turno &&
          tablero[c + 1][f] == turno &&
          tablero[c + 2][f] == turno &&
          tablero[c + 3][f] == turno)
      {
        ganador.columna = c;
        ganador.fila = f;
        ganador.jugador = turno;
        ganador.modo = Modo::Horizontal;
        return true;
      }
    }
  }

  // Comprobamos líneas verticales.
  for (byte c = 0; c < 7; c++) // Se comprueban las columnas.
  {
    for (byte f = 0; f < 3; f++) // En cada columna pueden haber 3 posiciones ganadoras
    {
      if (tablero[c][f] == turno &&
          tablero[c][f + 1] == turno &&
          tablero[c][f + 2] == turno &&
          tablero[c][f + 3] == turno)
      {
        ganador.columna = c;
        ganador.fila = f;
        ganador.jugador = turno;
        ganador.modo = Modo::Vertical;
        return true;
      }
    }
  }

  // Comprobamos diagonal derecha
  for (byte f = 0; f < 3; f++) // Se comprueban las filas.
  {
    for (byte c = 0; c < 4; c++) // Se avanza por columna
    {
      if (tablero[c][f] == turno &&
          tablero[c + 1][f + 1] == turno &&
          tablero[c + 2][f + 2] == turno &&
          tablero[c + 3][f + 3] == turno)
      {
        ganador.columna = c;
        ganador.fila = f;
        ganador.jugador = turno;
        ganador.modo = Modo::Horizontal;
        return true;
      }
    }
  }
  // Comprobamos diagonal izquierda
  for (byte f = 0; f < 3; f++) // Se comprueban las filas.
  {
    for (byte c = 3; c < 7; c++) // Se avanza por columna
    {
      if (tablero[c][f] == turno &&
          tablero[c - 1][f + 1] == turno &&
          tablero[c - 2][f + 2] == turno &&
          tablero[c - 3][f + 3] == turno)
      {
        ganador.columna = c;
        ganador.fila = f;
        ganador.jugador = turno;
        ganador.modo = Modo::Horizontal;
        return true;
      }
    }
  }
  return false;
}

// Enciende el led de la letra seleccionada.
void AsignaLetra()
{
#if defined(DEBUG)
  Serial.print(F("Encendemos letra "));
  Serial.println(letra);
#endif
  // Iluminamos solo la letra selecciomada.
  for (byte i = 0; i < 8; i++)
  {
    tiraLeds.SetPixelColor(led.posicionP1(i), i == letra ? color[nivel][turno1 ? 1 : 2] : RgbColor(0, 0, 0));
  }
  tiraLeds.Show();
}

// Enciende el led del número seleccionado.
void AsignaNumero()
{
  //  Iluminamos solo la letra selecciomada.
  for (byte i = 0; i < 8; i++)
  {
    tiraLeds.SetPixelColor(led.posicionP2(i), i == numero ? color[nivel][turno1 ? 1 : 2] : RgbColor(0, 0, 0));
  }
  tiraLeds.Show();
}

// Ilumina cada celda del tablero con su color correspondiente.
void PintaTablero()
{
  // Pintamos tablero.
  for (byte c = 0; c < columnas; c++)
  {
    for (byte f = 0; f < filas; f++)
    {
      tiraLeds.SetPixelColor(led.posicion(c, f), color[nivel][tablero[c][f]]);
    }
  }
  // Pintamos letra.
  AsignaLetra();
}

// Busca la primera posición libre en una columna.
byte DameHueco(byte columna)
{
  fila = 255;
  // Buscamos la primera fila libre.
  for (byte i = 0; i < filas; i++)
  {
    if (tablero[columna][i] == 0)
    {
      fila = i;
      break;
    }
  }
  return fila;
}

// Cargamos los valores por defecto de la configuración.
void ValoresxDefecto()
{
  // Cargamos valores por defecto.
  config.color1 = 4;
  config.color2 = 1;
  config.Turno = true;
  config.NivelIntensidad = 4;
  config.configurado = 1;
  // Grabamos configuración.
  EEPROM.put(0, config);
}

// Calculamos el valor del color.
RgbColor DameColor(byte _color, byte _intensidad)
{
  RgbColor _Color = colores[_color];
  _Color.R >>= (8 - _intensidad);
  _Color.G >>= (8 - _intensidad);
  _Color.B >>= (8 - _intensidad);

  return _Color;
}

// Han pulsado OK estando en modo Juego.
void OkEnJuego()
{
  if (letra < 8)
  {
    if (Comprobar(letra))
    {
      // Hay hueco en la columna. Ponemos la ficha.
      tiempo_PorPaso = tiempo_PonerFicha;
      repeticiones = filas - fila;
      proceso = Pr_PonerFicha;
      enProceso = true;
      siguientePaso = true;
      IniciaProceso();
    }
    else
    {
      // Columna llena.
      // Salvamos los valores de la columna para poder hacerla parpadear;
      for (byte i = 0; i < filas; i++)
      {
        salvaColumna[i] = tablero[columna][i];
      }
      tiempo_PorPaso = tiempo_ColumnaLlena;
      repeticiones = 10;
      proceso = Pr_ColumnaLlena;
      enProceso = true;
      siguientePaso = true;
      IniciaProceso();
    }
  }
  else
  {
#if defined(DEBUG)
    Serial.println(F("No hay letra seleccionada. No hacemos nada."));
#endif
  }
}

// Han pulsado una letra/columna en modo configuración.
void LetraEnConfig()
{
  switch (modoConfig)
  {
  case ModoConfiguracion::Ninguna:
    // Si no hay seleccionado un modo de configuración no hacemos nada.
    break;
  case ModoConfiguracion::Turno:
    // Si no es el pulsador 1 o 2 no hacemos nada.
    if (letra > 1)
    {
      break;
    }
    configTemp.Turno = letra == 0 ? true : false;
    MuestraConfiguracion();
    break;
  case ModoConfiguracion::Color1:
    if (letra > 5)
    {
      break;
    }
    if (letra + 1 == configTemp.color2)
    {
      break;
    }
    configTemp.color1 = letra + 1;
    MuestraConfiguracion();
    break;
  case ModoConfiguracion::Color2:
    if (letra > 5)
    {
      break;
    }
    if (letra + 1 == configTemp.color1)
    {
      break;
    }
    configTemp.color2 = letra + 1;
    MuestraConfiguracion();
    break;
  case ModoConfiguracion::Intensidad:
    configTemp.NivelIntensidad = letra + 1;
    MuestraConfiguracion();
    break;
  default:
    break;
  }
}

// Guardamos la configuración actual.
void SalvaConfiguracion()
{
  config = configTemp;
  EEPROM.begin();
  EEPROM.put(0, config);
  EEPROM.end();
}

// Muetra el "valor" del modo de configuración seleccionado.
void MuestraConfiguracion()
{
  // Borramos todos los leds.
  tiraLeds.ClearTo(RgbColor(0, 0, 0));

  switch (modoConfig)
  {
  case ModoConfiguracion::Ninguna:
    // Iluminamos todos los leds con el color de fondo.
    tiraLeds.ClearTo(DameColor(0, DameIntensidadFondo(configTemp.NivelIntensidad)));
    break;

  case ModoConfiguracion::Turno:

    DibujaTurno(configTemp.Turno ? 1 : 2);

    // Iluminamos el pulsador 1 y 2 con los colores de cada turno.
    tiraLeds.SetPixelColor(led.posicionP1(0), DameColor(configTemp.color1, configTemp.NivelIntensidad));
    tiraLeds.SetPixelColor(led.posicionP1(1), DameColor(configTemp.color2, configTemp.NivelIntensidad));

    tiraLeds.Show();
    break;
  case ModoConfiguracion::Color1:
    // Es el color del turno 1
    DibujaTurno(1);

    // Iluminamos las teclas con los distintos colores excepto el que tiene el turno 2.
    for (byte i = 1; i < 7; i++)
    {
      tiraLeds.SetPixelColor(led.posicionP1(i - 1), i != configTemp.color2 ? DameColor(i, configTemp.NivelIntensidad) : RgbColor(0, 0, 0));
    }

    // Los últimos dos pulsadores no tienen color asignado.
    tiraLeds.SetPixelColor(led.posicionP1(6), RgbColor(0, 0, 0));
    tiraLeds.SetPixelColor(led.posicionP1(7), RgbColor(0, 0, 0));

    tiraLeds.Show();
    break;
  case ModoConfiguracion::Color2:
    // Es el color del turno 1
    DibujaTurno(2);

    // Iluminamos las teclas con los distintos colores excepto el que tiene el turno 2.
    for (byte i = 1; i < 7; i++)
    {
      tiraLeds.SetPixelColor(led.posicionP1(i - 1), i != configTemp.color1 ? DameColor(i, configTemp.NivelIntensidad) : RgbColor(0, 0, 0));
    }

    // Los últimos dos pulsadores no tienen color asignado.
    tiraLeds.SetPixelColor(led.posicionP1(6), RgbColor(0, 0, 0));
    tiraLeds.SetPixelColor(led.posicionP1(7), RgbColor(0, 0, 0));

    tiraLeds.Show();
    break;

  case ModoConfiguracion::Intensidad:
    RgbColor color1, color2, colorFondo, colorFila;
    color1 = DameColor(configTemp.color1, configTemp.NivelIntensidad);
    color2 = DameColor(configTemp.color2, configTemp.NivelIntensidad);
    colorFondo = DameColor(0, DameIntensidadFondo(configTemp.NivelIntensidad));

    // Ya tenemos los colores, ahora dibujamos el tablero mostrando los tres colores.
    for (byte f = 0; f < 8; f++)
    {
      colorFila = f % 2 == 0 ? color1 : color2;
      for (byte c = 0; c < 8; c++)
      {
        tiraLeds.SetPixelColor(led.posicion(f, c), c <= f ? colorFila : colorFondo);
      }
    }

    // Iluminamos la fila de letras con los dos colores cambiando la intensidad.
    for (byte i = 0; i < 8; i++)
    {
      tiraLeds.SetPixelColor(led.posicionP1(i), DameColor(i % 2 == 0 ? config.color1 : config.color2, i));
    }
    // Iluminamos la fila de los números con el color de fondo para la intensidad correspondiente.
    for (byte i = 0; i < 8; i++)
    {
      tiraLeds.SetPixelColor(led.posicionP1(i), DameColor(0, DameIntensidadFondo(i)));
    }

    break;
  default:
    break;
  }
}

// Dibuja en el tablero el número de jugador que empieza con su color.
void DibujaTurno(byte turnoActual)
{
  byte posicion = 0;
  RgbColor color_ = turnoActual == 1 ? DameColor(configTemp.color1, configTemp.NivelIntensidad) : DameColor(configTemp.color2, configTemp.NivelIntensidad);
  RgbColor colorFondo_ = DameColor(0, DameIntensidadFondo(configTemp.NivelIntensidad));
  // Dibujamos el turno en el tablero.
  for (byte f = 0; f < 8; f++)
  {
    posicion = 1;
    for (byte p = 0; p < 8; p++)
    {
      tiraLeds.SetPixelColor(led.posicion(f, p), (numeros[turnoActual][7 - f] && posicion) ? color_ : colorFondo_);
      tiraLeds.SetPixelColor(led.posicion(f, p), (numeros[turnoActual][7 - f] && posicion) ? color_ : colorFondo_);
      posicion << 1;
    }
  }
}

// Calcula la intensidad del color de fondo.
byte DameIntensidadFondo(byte nivel)
{
  if (nivel < 5)
  {
    return nivel > 1 ? nivel / 2 : 1;
  }
  else
  {
    return nivel / 2 - 1;
  }
}