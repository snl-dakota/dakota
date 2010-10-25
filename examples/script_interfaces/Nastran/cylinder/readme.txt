The NASTRAN command line to run this modal model of a cylinder in
interactive mode is (first login to sass2889-atm)...

nast707 jid=modal.bdf mem=2m scr=yes bat=no

The nastran ascii-text output is contained in 

modal.f06

The nastran binary results are contained in

modal.op2

The nastran model of the cylinder is contained in

modal.bdf

A picture of the model can be viewed from

modal.gif

Description:

This hollow cylinder model is all shells, made of aluminum, .2"
thick. The bottom edge of the cylinder is rigidly attached to a large
seismic mass of 1000 lb. The top of the cylinder is attached to a
"load node" for applying lateral static loads. It is not used in the
modal solution. This deck requests 20 modes, including the 1st 6 rigid
body modes.



