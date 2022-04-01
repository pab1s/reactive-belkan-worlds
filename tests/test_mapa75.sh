#!/bin/bash
#Author: Nasr El Farissi

mkdir -p resultados

rm tests/resultados/porcentajes75.txt
rm tests/resultados/soloporcentajes75.txt
rm tests/resultados/sumatotal75.txt

for i in {3..71}
    do 
        for j in {3..71}
            do
                echo "$i $j" >> tests/resultados/porcentajes75.txt
                ./practica1SG mapas/mapa75.map 0 2 $i $j 0 |  grep "Porcentaje de mapa descubierto:" >> tests/resultados/porcentajes75.txt
            done
    done


grep -oP '(?<=Porcentaje de mapa descubierto: )[0-9]+'.'*[0-9]*' resultados/porcentajes75.txt > tests/resultados/soloporcentajes75.txt

awk '{sum += $1 } END { print sum }' tests/resultados/soloporcentajes75.txt > tests/resultados/sumatotal75.txt

var1=$(wc -l < tests/resultados/soloporcentajes75.txt)
var2=$(cat tests/resultados/sumatotal75.txt)
echo "scale=2 ; $var2 / $var1" | bc

