#ifndef COMPORTAMIENTOJUGADOR_H
#define COMPORTAMIENTOJUGADOR_H

#include <vector>

#include "comportamientos/comportamiento.hpp"
using namespace std;

class ComportamientoJugador : public Comportamiento {
   public:
    ComportamientoJugador(unsigned int size) : Comportamiento(size) {
        // Constructor de la clase
        fil = col = auxFil = auxCol = 99;
        brujula = norte;
        girar_derecha = false;
        bien_situado = false;
        zapatillas = bikini = false;
        ultimaAccion = actIDLE;
        vector<unsigned char> vacio(2 * MAX, '?');

        for (unsigned int i = 0; i < 2 * MAX; i++) {
            mapaAux.push_back(vacio);
        }
        // Dar el valor inicial a las variables de estado
    }

    ComportamientoJugador(const ComportamientoJugador& comport)
        : Comportamiento(comport) {}
    ~ComportamientoJugador() {}

    Action think(Sensores sensores);
    int interact(Action accion, int valor);

   private:
    // Declarar aquí las variables de estado
    static const int VISION_DEPTH = 3, UNKNOWN = -1, MAX = 100;
    int fil, col, auxFil, auxCol, brujula;
    bool girar_derecha, bien_situado, zapatillas, bikini;
    Action ultimaAccion;
    vector<vector<unsigned char>> mapaAux;

    // Métodos privados

    // Inserta en el mapa indicado todo lo que el jugador ve
    void mapearVision(vector<vector<unsigned char>>& mapa,
                      vector<unsigned char> vision, int orientacion, int f,
                      int c);

    // Incluye lo descubierto del mapa origen en el map destino
    void incluirMapa();

    // Devuelve el objeto más cercano que no dispone el jugador
    int encontrarObjeto(vector<unsigned char> vision);

    // Devuelve los datos para ir al destino indicado
    // Action sigPaso(int dest);
    Action accionSimple(Sensores sensores);
};

#endif
