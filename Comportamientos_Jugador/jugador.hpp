#ifndef COMPORTAMIENTOJUGADOR_H
#define COMPORTAMIENTOJUGADOR_H

#include "comportamientos/comportamiento.hpp"
#include <vector>
using namespace std;

class ComportamientoJugador : public Comportamiento{

  public:
    ComportamientoJugador(unsigned int size) : Comportamiento(size){
      // Constructor de la clase
      fil = col = 99;
      brujula = 0;
      girar_derecha = false;
      bien_situado = false;
      ultimaAccion = actIDLE;
      
      // Dar el valor inicial a las variables de estado
    }

    ComportamientoJugador(const ComportamientoJugador & comport) : Comportamiento(comport){}
    ~ComportamientoJugador(){}

    Action think(Sensores sensores);
    int interact(Action accion, int valor);
    void mapearVision(vector<vector<unsigned char> >& mapaResultado, Sensores sensores);

  private:
  
  // Declarar aquí las variables de estado
  const int VISION_DEPTH=2;
  int fil, col, brujula;
  bool girar_derecha, bien_situado;
  Action ultimaAccion;
};

#endif
