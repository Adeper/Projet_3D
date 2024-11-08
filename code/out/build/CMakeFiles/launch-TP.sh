#!/bin/sh
bindir=$(pwd)
cd /home/e20170005389/Cours/Master-Imagine/M1/S2/Moteur_jeu/Cameras_TP6/TP/
export 

if test "x$1" = "x--debugger"; then
	shift
	if test "xYES" = "xYES"; then
		echo "r  " > $bindir/gdbscript
		echo "bt" >> $bindir/gdbscript
		/usr/bin/gdb -batch -command=$bindir/gdbscript --return-child-result /home/e20170005389/Cours/Master-Imagine/M1/S2/Moteur_jeu/Cameras_TP6/out/build/TP 
	else
		"/home/e20170005389/Cours/Master-Imagine/M1/S2/Moteur_jeu/Cameras_TP6/out/build/TP"  
	fi
else
	"/home/e20170005389/Cours/Master-Imagine/M1/S2/Moteur_jeu/Cameras_TP6/out/build/TP"  
fi
