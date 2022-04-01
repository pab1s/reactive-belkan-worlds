#include "../Comportamientos_Jugador/jugador.hpp"

#include <iostream>
#include <random>
using namespace std;

Action ComportamientoJugador::think(Sensores sensores) {
    Action accion = actIDLE;
    mensaje(sensores);

    // Establecer la posicion y orientacion anterior
    switch (ultimaAccion) {
        case actFORWARD:
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
    etapa = esFrontera(entorno[frente]) ? muro : etapa;
    if (randomGenerator() < LEAVE_WALL_PROB and etapa == muro) {
        etapa = dejarMuro;
    }

    // Decidir la nueva accion
    // 1 - Encontrar objetos
    if (sensores.terreno[0] == 'D' and !zapatillas) {
        zapatillas = true;
    }
    if (sensores.terreno[0] == 'K' and !bikini) {
        bikini = true;
    }

    // 2 - Elegimos la accion
    switch (etapa) {
        case inicio:
            accion = inicioAgente(sensores);
            break;
        case muro:
            accion = seguirFrontera(sensores);
            break;
        case dejarMuro:
            fronteraEncontrada = false;
            accion = actTURN_L;
            etapa = simple;
            break;
        case simple:
            accion = accionSimple(sensores);
            break;
    }

    // Determinar el efecto de la ultima accion enviada
    ultimaAccion = accion;
    return accion;
}

// Declaracion metodos
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

int ComportamientoJugador::encontrarObjeto(vector<unsigned char> vision) {
    int i = 0, pos = UNKNOWN;
    bool encontrado = false;

    if (!zapatillas and !bikini) {
        while (!encontrado and i < vision.size()) {
            if (vision[i] == 'D' and !zapatillas) {
                encontrado = true;
            } else if (vision[i] == 'K' and !bikini) {
                encontrado = true;
            }
            i++;
        }
    }
    pos = encontrado ? --i : UNKNOWN;
    return pos;
}

/*
Action ComportamientoJugador::sigPaso(int dest) {
        Action paso = actIDLE;
        int n = 0, x = 0;
        bool terminado = false;

        while (n <= VISION_DEPTH and !terminado) {
                x = x + 2*n;
                if (dest == x or ) {
                        paso = actFORWARD;
                } else if (dest <= (x+n)) {

                }
        }
}
*/

Action ComportamientoJugador::accionSimple(Sensores sensores) {
    Action paso = actIDLE;

    if ((sensores.terreno[2] == 'T' or sensores.terreno[2] == 'S' or
         sensores.terreno[2] == 'G' or sensores.terreno[2] == 'X' or
         sensores.terreno[2] == 'D' or sensores.terreno[2] == 'K' or
         (sensores.terreno[2] == 'B' and zapatillas) or
         (sensores.terreno[2] == 'A' and bikini)) and
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

    if (contIni < fib_n) {
        contIni++;
        ret = accionSimple(sensores);
    } else {
        contIni = 0;
        fib_n0 = fib_n1;
        fib_n1 = fib_n;
        ret = actTURN_L;
    }
    return ret;
}

// INCOMPLETO: FALTAN CASOS
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

    /*else {
       ret = accionSimple(sensores);
   }*/
    cout << "ESTOY EN FRONTERA Y HAGO ESTO: " << ret << endl
         << "FRENTE: " << entorno[frente] << endl
         << "DER: " << entorno[derecha] << endl
         << "ATRASDER: " << entorno[atrasDer] << endl
         << "FRENTEDER: " << entorno[frenteDer] << endl;
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

bool ComportamientoJugador::esFrontera(char casilla) {
    bool softFrontier = false, hardFrontier = false;

    if ((casilla == 'A' and !bikini) or (casilla == 'B' and !zapatillas)) {
        softFrontier = true;
    } else if (casilla == 'P' or casilla == 'M') {
        hardFrontier = true;
    }
    return softFrontier or hardFrontier;
}

float ComportamientoJugador::randomGenerator() {
    float random_num = 0;
    random_device rd; 
    mt19937 gen(rd()); 
    uniform_real_distribution<> dis(0,1.0);
    random_num = dis(gen);
    return random_num;
}

float ComportamientoJugador::calcularPerDesc() {
    float porcentaje = 0, contador = 0;

    if (bien_situado) {
        for (int i = 0; i < MAX; i++) {
            for (int j = 0; j < MAX; j++) {
                if (mapaResultado[i][j] != '?') {
                    contador++;
                }
            }
        }
    } else {
        for (int i = 0; i < 2*MAX; i++) {
            for (int j = 0; j < 2*MAX; j++) {
                if (mapaAux[i][j] != '?') {
                    contador++;
                }
            }
        }
    }
    porcentaje = contador / (MAX*MAX);
    return porcentaje;
}