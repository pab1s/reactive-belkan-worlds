#include "../Comportamientos_Jugador/jugador.hpp"

#include <iostream>
using namespace std;

Action ComportamientoJugador::think(Sensores sensores) {
    Action accion = actIDLE;

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
        mapearVision(mapaAux, sensores.terreno, norte, auxFil, auxCol);
    }

    // Decidir la nueva accion
    // 1 - Encontrar objetos
    if (sensores.terreno[0] == 'D' and !zapatillas) {
        zapatillas = true;
    }
    if (sensores.terreno[0] == 'K' and !bikini) {
        bikini = true;
    }
    /*
    objetivo = encontrarObjetos(sensores.terreno);
    if (objetivo != ninguno) {
            accion = sigPaso(objetivo);
    }
    */

    // Permitimos el avance en terreno de gasto base
    accion = accionSimple(sensores);

    // Determinar el efecto de la ultima accion enviada
    ultimaAccion = accion;
    return accion;
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
