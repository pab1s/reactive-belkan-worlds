#!/bin/bash
#Author: Nasr El Farissi

mkdir -p resultados

rm tests/resultados/porcentajes50.txt
rm tests/resultados/soloporcentajes50.txt
rm tests/resultados/sumatotal50.txt

for i in {3..46}
    do 
        for j in {3..46}
            do
                echo "$i $j" >> tests/resultados/porcentajes50.txt
                ./practica1SG mapas/mapa50.map 0 2 $i $j 0 |  grep "Porcentaje de mapa descubierto:" >> tests/resultados/porcentajes50.txt
            done
    done


grep -oP '(?<=Porcentaje de mapa descubierto: )[0-9]+'.'*[0-9]*' tests/resultados/porcentajes50.txt > tests/resultados/soloporcentajes50.txt

awk '{sum += $1 } END { print sum }' tests/resultados/soloporcentajes50.txt > tests/resultados/sumatotal50.txt

var1=$(wc -l < tests/resultados/soloporcentajes50.txt)
var2=$(cat tests/resultados/sumatotal50.txt)
echo "scale=2 ; $var2 / $var1" | bc

