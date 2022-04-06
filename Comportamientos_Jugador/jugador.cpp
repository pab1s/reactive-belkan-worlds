#include "../Comportamientos_Jugador/jugador.hpp"

#include <cmath>
#include <iostream>
#include <random>
#include <algorithm>
using namespace std;

Action ComportamientoJugador::think(Sensores sensores) {
    Action accion = actIDLE;
    accionDecidida = false;
    mensaje(sensores);
    if (sensores.reset) reset();
    ++pasos;

    // Establecer la posicion y orientacion anterior
    switch (ultimaAccion) {
        case actFORWARD:
            if (!sensores.colision) {
                switch (brujula) {
                    case 0:  // Norte
                        fil--;
                        auxFil--;
                        break;
                    case 1:  // Este
                        col++;
                        auxCol++;
                        break;
                    case 2:  // Sur
                        fil++;
                        auxFil++;
                        break;
                    case 3:  // Oeste
                        col--;
                        auxCol--;
                        break;
                }
            }
            break;
        case actTURN_L:
            brujula = (brujula + 3) % 4;
            girar_derecha = (rand() % 2 == 0);
            break;
        case actTURN_R:
            brujula = (brujula + 1) % 4;
            girar_derecha = (rand() % 2 == 0);
            break;
    }

    // Establecemos la información actual en variables de estado
    if (sensores.nivel == 1 and !bien_situado) {
        brujula = sensores.sentido;
    }
    if ((sensores.terreno[0] == 'G' or sensores.nivel == 0) and !bien_situado) {
        brujula = sensores.sentido;
        fil = sensores.posF;
        col = sensores.posC;
        bien_situado = true;
    }

    if (bien_situado) {
        incluirMapa();
        mapearVision(mapaResultado, sensores.terreno, brujula, fil, col);
    } else {
        mapearVision(mapaAux, sensores.terreno, brujula, auxFil, auxCol);
    }

    setEntorno();
    evaluarTerreno(sensores.terreno[0]);

    // Decidir la nueva acción
    // 1 - Evaluamos las condiciones para elegir la fase que determinará el
    // movimiento
    if (!caminoASeguir) {
        caminoASeguir = determinaCamino(encontrarEspecial(sensores));
    }

    if (sensores.terreno[0] == 'D' and !zapatillas) {
        zapatillas = true;
    } else if (sensores.terreno[0] == 'K' and !bikini) {
        bikini = true;
    }

    if (pasos >= 750) {
        if (bien_situado) {
            perDescPrev = perDesc;
        } else {
            perDescPrev = 0;
        }
        perDesc = calcularPerDesc();
        if ((perDesc - perDescPrev) < DISCOVER_PERC) {
            etapa = morir;
        }
        pasos = 0;
    }

    if (etapa != morir) {
        if (sensores.terreno[0] == 'X' and
            ((float(sensores.bateria) / float(sensores.vida)) < CHARGE_PROP)) {
            etapa = recargar;
        } else if (esFrontera(entorno[frente]) and esFrontera(entorno[derecha]) and
                esFrontera(entorno[izquierda]) and esFrontera(entorno[atras])) {
            if (entorno[frente] != 'M') {
                accion = actFORWARD;
            } else {
                accion = actTURN_L;
            }
            etapa = simple;
            accionDecidida = true;
        } else if (evitarAldeano(sensores.superficie[2])) {
            etapa = esquivar;
            caminoASeguir = false;
            ruta.clear();
        } else if (caminoASeguir and entorno[frente] != 'M') {
            etapa = especial;
        } else if (etapa != especial) {
            etapa = esFrontera(entorno[frente]) ? muro : etapa;
        } else {
            etapa = simple;
        }
    }

    if (randomGenerator() < LEAVE_WALL_PROB and pasosFrontera > MIN_STEPS_LEAVE_WALL and
        etapa == muro) {
        etapa = dejarMuro;
    }

    // 2 - Realizamos las operaciones necesarias para ejecutar la fase
    // seleccionada
    if (!accionDecidida) {
        switch (etapa) {
            case inicio:
                accion = inicioAgente(sensores);
                break;
            case muro:
                ++pasosFrontera;
                accion = seguirFrontera(sensores);
                break;
            case dejarMuro:
                pasosFrontera = 0;
                fronteraEncontrada = false;
                puerta = posiblePuerta = false;
                accion = actTURN_L;
                etapa = simple;
                break;
            case simple:
                accion = accionSimple(sensores);
                break;
            case morir:
                if (entorno[frente] == 'P') {
                    accion = actFORWARD;
                } else {
                    accion = accionSimple(sensores);
                }
                break;
            case esquivar:
                fronteraEncontrada = false;
                accion = actTURN_L;
                etapa = simple;
                break;
            case especial:
                pasosFrontera = 0;
                fronteraEncontrada = false;
                puerta = posiblePuerta = false;
                accion = sigueCamino();
                break;
            case recargar:
                accion = actIDLE;
                break;
        }
    }

    /* 3 - Si detecta que en el siguiente ciclo muere, infiere las casillas 
    no descubiertas. */
    if (voyAMorir(accion, sensores)) {
        rellenar();
    }

    // 4 - Determinar el efecto de la ultima accion enviada 
    ultimaAccion = accion;
    return accion;
}

// Declaracion metodos privados utilizados
void ComportamientoJugador::mensaje(Sensores sensores) {
    cout << "Posicion: fila " << sensores.posF << " columna " << sensores.posC
         << " ";
    switch (sensores.sentido) {
        case 0:
            cout << "Norte" << endl;
            break;
        case 1:
            cout << "Este" << endl;
            break;
        case 2:
            cout << "Sur " << endl;
            break;
        case 3:
            cout << "Oeste" << endl;
            break;
    }
    cout << "Terreno: ";
    for (int i = 0; i < sensores.terreno.size(); i++)
        cout << sensores.terreno[i];
    cout << endl;

    cout << "Superficie: ";
    for (int i = 0; i < sensores.superficie.size(); i++)
        cout << sensores.superficie[i];
    cout << endl;

    cout << "Colisión: " << sensores.colision << endl;
    cout << "Reset: " << sensores.reset << endl;
    cout << "Vida: " << sensores.vida << endl;
    cout << endl;
}

int ComportamientoJugador::interact(Action accion, int valor) { return false; }

void ComportamientoJugador::mapearVision(vector<vector<unsigned char>>& mapa,
                                         vector<unsigned char> vision,
                                         int orientacion, int f, int c) {
    int n = 0, aux = 0, x = 0, y = 0;

    for (int i = 0; i >= -VISION_DEPTH; i--) {
        for (int j = i; j <= -i; j++) {
            x = i;
            y = j;
            for (int k = 0; k < orientacion; k++) {
                aux = (-1) * x;
                x = y;
                y = aux;
            }
            mapa[f + x][c + y] = vision[n];
            n++;
        }
    }
}

void ComportamientoJugador::incluirMapa() {
    for (int i = 0; i < 2 * MAX; i++) {
        for (int j = 0; j < 2 * MAX; j++) {
            if (mapaAux[i][j] != '?') {
                mapaResultado[fil + (i - auxFil)][col + (j - auxCol)] =
                    mapaAux[i][j];
            }
        }
    }
}

int ComportamientoJugador::encontrarEspecial(Sensores sensores) {
    int i = 0, pos = UNKNOWN;
    bool encontrado = false,
         pocaBateria =
             ((float(sensores.bateria) / float(sensores.vida)) < CHARGE_PROP);
    vector<unsigned char> vision = sensores.terreno;

    if (!zapatillas or !bikini or !bien_situado or pocaBateria) {
        while (!encontrado and i < vision.size()) {
            if (!bien_situado and vision[i] == 'G') {
                encontrado = true;
            } else if (vision[i] == 'D' and !zapatillas) {
                encontrado = true;
            } else if (vision[i] == 'K' and !bikini) {
                encontrado = true;
            } else if (vision[i] == 'X' and pocaBateria) {
                encontrado = true;
            }
            i++;
        }
    }
    pos = encontrado ? --i : UNKNOWN;
    return pos;
}

bool ComportamientoJugador::determinaCamino(int dest) {
    int n = 1, x = 0;
    bool terminado = false;
    if (dest != UNKNOWN and dest != 0) {
        while (n <= VISION_DEPTH and !terminado) {
            x = x + 2 * n;
            ruta.push_back('F');
            if (dest == x) {
                terminado = true;
            } else if (dest <= (x + n) and dest > x) {
                ruta.push_back('R');
                for (int i = 0; i < (dest - x); i++) {
                    ruta.push_back('F');
                }
                terminado = true;
            } else if (dest >= (x - n) and dest < x) {
                ruta.push_back('L');
                for (int i = 0; i < (x - dest); i++) {
                    ruta.push_back('F');
                }
                terminado = true;
            }
            n++;
        }
    }
    return terminado;
}

Action ComportamientoJugador::sigueCamino() {
    Action ret = actIDLE;
    if (!ruta.empty()) {
        switch (ruta.front()) {
            case 'F':
                ret = actFORWARD;
                break;
            case 'R':
                ret = actTURN_R;
                break;
            case 'L':
                ret = actTURN_L;
                break;
        }
        ruta.pop_front();
    } else {
        caminoASeguir = false;
    }
    return ret;
}

Action ComportamientoJugador::accionSimple(Sensores sensores) {
    Action paso = actIDLE;

    if ((sensores.terreno[2] == 'T' or sensores.terreno[2] == 'S' or
         sensores.terreno[2] == 'G' or sensores.terreno[2] == 'X' or
         sensores.terreno[2] == 'D' or sensores.terreno[2] == 'K' or
         (sensores.terreno[2] == 'B' and zapatillas) or
         (sensores.terreno[2] == 'A' and bikini) or
         (sensores.terreno[2] == 'B' and !zapatillas and !terrenoIdeal) or
         (sensores.terreno[2] == 'A' and !bikini and !terrenoIdeal)) and
        (sensores.superficie[2] == '_')) {
        paso = actFORWARD;
    } else if (!girar_derecha) {
        paso = actTURN_L;
    } else {
        paso = actTURN_R;
    }

    return paso;
}

Action ComportamientoJugador::inicioAgente(Sensores sensores) {
    Action ret = actIDLE;
    int fib_n = fib_n1 + fib_n0;
    if (!terrenoIdeal) {
        ret = actFORWARD;
    } else {
        if (contIni < fib_n) {
            contIni++;
            ret = accionSimple(sensores);
        } else {
            contIni = 0;
            fib_n0 = fib_n1;
            fib_n1 = fib_n;
            ret = actTURN_L;
        }
    }
    return ret;
}

Action ComportamientoJugador::seguirFrontera(Sensores sensores) {
    Action ret = actIDLE;

    if (esFrontera(entorno[frente]) and fronteraEncontrada == false) {
        fronteraEncontrada = true;
        ret = actTURN_L;
    } else if (esFrontera(entorno[derecha]) and !esFrontera(entorno[frente])) {
        ret = actFORWARD;
        if (esFrontera(entorno[izquierda])) {
            posiblePuerta = true;
        }
    } else if (esFrontera(entorno[derecha]) and esFrontera(entorno[frente])) {
        ret = actTURN_L;
    } else if (!esFrontera(entorno[derecha]) and
               esFrontera(entorno[atrasDer])) {
        if (puerta) {
            puerta = posiblePuerta = false;
            ret = actFORWARD;
        } else if (posiblePuerta) {
            puerta = true;
            ret = actTURN_R;
        } else {
            ret = actTURN_R;
        }
    } else if (!esFrontera(entorno[frente]) and
               esFrontera(entorno[frenteDer])) {
        ret = actFORWARD;
    }

    if (posiblePuerta) {
        controladorPuerta++;
        if (controladorPuerta > 3) {
            posiblePuerta = false;
            controladorPuerta = 0;
        }
    }
    return ret;
}

void ComportamientoJugador::setEntorno() {
    int x1 = -1, y1 = 0, aux1 = 0, x2 = -1, y2 = -1, aux2 = 0;

    for (int i = 0; i < brujula; i++) {
        aux1 = (-1) * x1;
        x1 = y1;
        y1 = aux1;

        aux2 = (-1) * x2;
        x2 = y2;
        y2 = aux2;
    }

    if (bien_situado) {
        for (int i = 0; i < 4; i++) {
            entorno[i * 2 + 1] = mapaResultado[fil + x1][col + y1];
            aux1 = (-1) * x1;
            x1 = y1;
            y1 = aux1;

            entorno[i * 2] = mapaResultado[fil + x2][col + y2];
            aux2 = (-1) * x2;
            x2 = y2;
            y2 = aux2;
        }
    } else {
        for (int i = 0; i < 4; i++) {
            entorno[i * 2 + 1] = mapaAux[auxFil + x1][auxCol + y1];
            aux1 = (-1) * x1;
            x1 = y1;
            y1 = aux1;

            entorno[i * 2] = mapaAux[auxFil + x2][auxCol + y2];
            aux2 = (-1) * x2;
            x2 = y2;
            y2 = aux2;
        }
    }
}

bool ComportamientoJugador::esFrontera(char terreno) {
    bool softFrontier = false, hardFrontier = false;

    if ((terreno == 'A' and !bikini) or (terreno == 'B' and !zapatillas)) {
        softFrontier = true;
    } else if (terreno == 'P' or terreno == 'M') {
        hardFrontier = true;
    }

    return (softFrontier and terrenoIdeal) or hardFrontier;
}

float ComportamientoJugador::randomGenerator() {
    float random_num = 0;
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(0, 1.0);
    random_num = dis(gen);
    return random_num;
}

float ComportamientoJugador::calcularPerDesc() {
    float porcentaje = 0, contador = 0;

    if (bien_situado) {
        for (int i = 0; i < mapaResultado.size(); i++) {
            for (int j = 0; j < mapaResultado.size(); j++) {
                if (mapaResultado[i][j] != '?') {
                    contador++;
                }
            }
        }
    } else {
        for (int i = 0; i < 2 * MAX; i++) {
            for (int j = 0; j < 2 * MAX; j++) {
                if (mapaAux[i][j] != '?') {
                    contador++;
                }
            }
        }
    }
    porcentaje = contador / (mapaResultado.size() * mapaResultado.size());
    return porcentaje;
}

void ComportamientoJugador::reset() {
    fil = col = auxFil = auxCol = 99;
    pasos = 0;
    contIni = 0;
    fib_n0 = 0;
    fib_n1 = 1;
    brujula = norte;
    pasosFrontera = 0;
    girar_derecha = false;
    bien_situado = false;
    terrenoIdeal = false;
    accionDecidida = false;
    zapatillas = bikini = false;
    controladorPuerta = 0;
    fronteraEncontrada = posiblePuerta = puerta = false;
    perDescPrev = perDesc = 0;
    ultimaAccion = actIDLE;
    etapa = inicio;

    for (unsigned int i = 0; i < 2 * MAX; i++) {
        for (unsigned int j = 0; j < 2 * MAX; j++) {
            mapaAux[i][j] = '?';
        }
    }

    for (int i = frenteIzq; i <= izquierda; i++) {
        entorno[i] = '?';
    }
}

void ComportamientoJugador::evaluarTerreno(char casilla) {
    if (casilla == 'T' or casilla == 'S' or casilla == 'G' or casilla == 'X' or
        casilla == 'D' or casilla == 'K' or (casilla == 'B' and zapatillas) or
        (casilla == 'A' and bikini)) {
        terrenoIdeal = true;
    } else {
        terrenoIdeal = false;
    }
}

bool ComportamientoJugador::evitarAldeano(char superficie) {
    return superficie == 'a';
}

void ComportamientoJugador::rellenar() {
    int size = mapaResultado.size(), maxElementIndex = 0;
    vector <unsigned char> casillas =   {'P', 'T', 'S', 'G', 'X', 'D', 'K', 'B', 'A', 'M'};
    vector <int> num_casillas =         {0,   0,   0,   0,   0,   0,   0,   0,   0,   0};

    for (int n = 0; n < size/2; n++) {
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                if (i == 0+n || j == 0+n || i == size -1-n || j == size -1-n) {
                    for (int k = 0; k < casillas.size(); k++) {
                        if (mapaResultado[i][j] == casillas[k]) {
                            num_casillas[k]++;
                        }
                    }
                }
            }
        }

        maxElementIndex = distance(num_casillas.begin(), max_element(num_casillas.begin(), num_casillas.end()));

        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                if (mapaResultado[i][j] == '?' and (i == 0+n || j == 0+n || i == size -1-n || j == size -1-n)) {
                    if (num_casillas[maxElementIndex] == 0) {
                        mapaResultado[i][j] = 'T';
                    } else {
                    mapaResultado[i][j] = casillas[maxElementIndex];
                    }
                }
            }
        }

        fill(num_casillas.begin(), num_casillas.end(), 0);
    }
}

void ComportamientoJugador::rellenarV2() {
    int size = mapaResultado.size();

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (i == 0) {
                if (mapaResultado[i][j] == '?') {
                    mapaResultado[i][j] = 'P';
                }
            } else if (mapaResultado[i-1][j] != '?' and mapaResultado[i][j] == '?') {
                mapaResultado[i][j] = mapaResultado[i-1][j];
            }
        }
    }
}

bool ComportamientoJugador::voyAMorir(Action accion, Sensores sensores) {
  int nextBateria = 1;
  bool fin = false;
  unsigned char celda = '?';

  if (accion == actFORWARD) {
      celda = sensores.terreno[2];
  } else {
        celda = sensores.terreno[0];
  }

  switch (accion) {
    case actIDLE:
        nextBateria = 0;
        break;
    case actFORWARD:
        switch (celda) {
            case 'A':
            if (bikini) // Bikini
                nextBateria = 10;
            else
                nextBateria = 200;
            break;
        case 'B':
            if (zapatillas) // Zapatillas
                nextBateria = 15;
            else
                nextBateria = 100;
            break;
        case 'T':
            nextBateria = 2;
            break;
        default:
            nextBateria = 1;
            break;
        } // Fin switch celda
        break;
    case actTURN_L:
    case actTURN_R:
        switch (celda) {
            case 'A':
                if (bikini) // Bikini
                    nextBateria = 5;
                else
                    nextBateria = 500;
                break;
            case 'B':
                if (zapatillas) // Zapatillas
                    nextBateria = 1;
                else
                    nextBateria = 3;
            break;
            case 'T':
                nextBateria = 2;
                break;
            default:
                nextBateria = 1;
                break; 
        } // Fin switch celda
        break;
    }

    if (sensores.bateria - nextBateria <= 0 or sensores.vida - 1 <= 0) {
        fin = true;
    }

    return fin;
}