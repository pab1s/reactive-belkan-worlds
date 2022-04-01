#ifndef COMPORTAMIENTOJUGADOR_H
#define COMPORTAMIENTOJUGADOR_H

#include <vector>

#include "comportamientos/comportamiento.hpp"
using namespace std;

enum Entorno {frenteIzq, frente, frenteDer, derecha, atrasDer, atras, atrasIzq, izquierda};
enum Etapa {simple, inicio, muro, dejarMuro, objeto, morir, recargar};

class ComportamientoJugador : public Comportamiento {
   public:
    ComportamientoJugador(unsigned int size) : Comportamiento(size) {
        // Constructor de la clase
        fil = col = auxFil = auxCol = 99;
        contIni = 0;
        fib_n0 = 0;
        fib_n1 = 1;
        brujula = norte;
        girar_derecha = false;
        bien_situado = false;
        zapatillas = bikini = false;
        fronteraEncontrada = posiblePuerta = puerta = false;
        perDescPrev = perDesc = 0;
        ultimaAccion = actIDLE;
        etapa = inicio;
        vector<unsigned char> vacio(2 * MAX, '?');

        for (unsigned int i = 0; i < 2 * MAX; i++) {
            mapaAux.push_back(vacio);
        }

        for (int i=frenteIzq; i<=izquierda; i++) {
            entorno.push_back('?');
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
    const float LEAVE_WALL_PROB = 0.05;
    int fil, col, auxFil, auxCol, brujula, fib_n0, fib_n1, contIni;
    float perDescPrev, perDesc;
    bool girar_derecha, bien_situado, zapatillas, bikini;
    bool fronteraEncontrada, posiblePuerta, puerta;
    Etapa etapa;
    Action ultimaAccion;
    vector<unsigned char> entorno;
    vector<vector<unsigned char>> mapaAux;

    // Métodos privados
    void mensaje(Sensores sensores);
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
    void setEntorno();
    bool esFrontera(char casilla);
    Action accionSimple(Sensores sensores);
    Action inicioAgente(Sensores sensores);
    Action seguirFrontera(Sensores sensores);
    float randomGenerator();
    float calcularPerDesc();
};

#endif
