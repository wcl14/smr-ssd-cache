#!/bin/sh

fifo_nums=(9513 9900 3754 21207 13054 3988 7646 10273 6731 4941 7451)
strategys=(1 2 3 5 6 7)
zone_sizes=(20971520 2097152 10485760 52428800)     # 20MB, 2MB, 10MB, 5MB

for i in "${!fifo_nums[@]}";
do
    for strategy in ${strategys[@]};
    do
        if [ $strategy -ge 6 ]
        then
            j=0
            while [ $j -lt ${#zone_sizes[@]} ]
            do
                echo ./smr-ssd-cache ${fifo_nums[$i]}*5 ${fifo_nums[$i]} 0 4096 ${zone_sizes[$j]} $strategy $i
                ./smr-ssd-cache ${fifo_nums[$i]}*5 ${fifo_nums[$i]} 0 4096 ${zone_sizes[$j]} $strategy $i
                let j++
            done
        else
            echo ./smr-ssd-cache ${fifo_nums[$i]}*5 ${fifo_nums[$i]} 0 4096 ${zone_sizes[0]} $strategy $i
            ./smr-ssd-cache ${fifo_nums[$i]}*5 ${fifo_nums[$i]} 0 4096 ${zone_sizes[0]} $strategy $i
        fi
    done
done
