{ECHO(OFF)}
{include(params.in)}
{ECHO(ON)}
*HEADING
PARIS FATIGUE CRACK PROPAGATION DEMONSTRATION (BHT 9/90) USING ABAQUS
*NODE
     1,          0.0,          0.0
     2,        {S/4},          0.0
     3,        {S/2},          0.0
     4,          0.0,        {B/2}
     5,        {S/4},        {B/2}
     6,        {S/2},        {B/2}
     7,          0.0,          {B}
     8,        {S/4},          {B}
     9,        {S/2},          {B}
**
** Use imcompatible mode element to improve bending response
**
*ELEMENT,TYPE=CPS4,ELSET=BEAM
 1, 1, 2, 5, 4
 2, 2, 3, 6, 5
 3, 4, 5, 8, 7
 4, 5, 6, 9, 8
*SOLID SECTION,ELSET=BEAM,MATERIAL=AL2024T3
1.0
*MATERIAL,NAME=AL2024T3
*ELASTIC
30.000000e3,     0.3
*BOUNDARY
 1, 2, , 0.0
 3, 1, , 0.0
 6, 1, , 0.0
 9, 1, , 0.0
*STEP
*STATIC
*CLOAD
** 9, 2,    3.3333333,
 9, 2,    {P/2},
**EL FILE, POSITION=AVERAGED AT NODES
**S
**END STEP
*EL PRINT, POSITION=AVERAGED AT NODES
S
*END STEP
