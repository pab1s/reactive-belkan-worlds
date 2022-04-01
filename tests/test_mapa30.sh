#!/bin/bash
#Author: Nasr El Farissi

mkdir -p tests/resultados

rm tests/resultados/porcentajes30.txt
rm tests/resultados/soloporcentajes30.txt
rm tests/resultados/sumatotal30.txt

for i in {3..26}
    do 
        for j in {3..26}
            do
                echo "$i $j" >> tests/resultados/porcentajes30.txt
                ./practica1SG mapas/mapa30.map 0 2 $i $j 0 |  grep "Porcentaje de mapa descubierto:" >> tests/resultados/porcentajes30.txt
            done
    done


grep -oP '(?<=Porcentaje de mapa descubierto: )[0-9]+'.'*[0-9]*' tests/resultados/porcentajes30.txt > tests/resultados/soloporcentajes30.txt

awk '{sum += $1 } END { print sum }' tests/resultados/soloporcentajes30.txt > tests/resultados/sumatotal30.txt

var1=$(wc -l < tests/resultados/soloporcentajes30.txt)
var2=$(cat tests/resultados/sumatotal30.txt)
echo "scale=2 ; $var2 / $var1" | bc

