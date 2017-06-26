#!/bin/bash

# number images
for n in 0 1 2 3 4 5 6 7 8 9 blank; do
    echo "const uint8_t number_$n [] PROGMEM = {"
    ../tools/image2hex.pl ../images/number_$n.png
    echo "};"
    echo
done > numbers.h

# cybercom images
for n in 1 2 3; do
    echo "const uint8_t cybercom_$n [] PROGMEM = {";
    ../tools/image2hex.pl ../images/cybercom_$n.png
    echo "};"
    echo
done > cybercom.h

# cunning cybercom
for n in 0 1 2 3 4 5 6 7 8 9; do
    echo "const uint8_t cybercom_running_$n [] PROGMEM = {";
    ../tools/image2hex.pl ../images/cybercom_running_$n.png
    echo "};"
    echo
done > cybercom_running.h
