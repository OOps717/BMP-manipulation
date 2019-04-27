#!/bin/bash

a=psf

gcc resize.c -o resize 


for i in *{0..9}.bmp
do 
    if [ -f $i ]
    then 
        ./resize $i $a 
    else
        echo "No .bmp files found"
    fi 
done   

if [ -d Resized ]
then

    cp crop.c glue.c Resized  
    cd Resized

    gcc crop.c -o crop

    for j in *Resized.bmp
    do  
        if [ -f $j ]
        then
            ./crop $j
        else 
            echo "No files with the end Resized.bmp found"
        fi
    done

    rm crop.c crop

    if [ -d ResizedAndCropped ] 
    then

        cp glue.c ResizedAndCropped
        rm glue.c
        cd ResizedAndCropped

        gcc glue.c -o glue

        ./glue
        
        rm glue.c glue
    fi

fi