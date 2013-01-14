lines(0);
warning_old = warning('query');

NbBlocs = 8; // The number of blocks for the bridge.

global IndexNodeVarSup;
global IndexNodeVarInf;

IndexNodeVarSup = []; // This list will handles the number of the nodes we are allowed to move
IndexNodeVarInf = []; // This list will handles the number of the nodes where we compute the deformation of the deck of the bridge

// Definition of the initial solution. Depends on the number of blocks
x0 = zeros(2*NbBlocs+2,3);

//IndexNodeVarSup = [IndexNodeVarSup 4*(i-1)+3+5 4*(i-1)+3+7];

NbPts = size(x0(1:2:$,1),1);

// i+1 1 0;  // 4*(i-1)+3+5
x0(1:2:$,1) = (1:NbPts)';
x0(1:2:$,2) = 1;
x0(1:2:$,3) = 0;

// i+1 1 1]; // 4*(i-1)+3+7
x0(2:2:$,1) = (1:NbPts)';
x0(2:2:$,2) = 1;
x0(2:2:$,3) = 1;

x0 = matrix(x0',length(x0),1);

function [t,p,e,A,E,rho,F] = bridge_optim_3d(x)
global IndexNodeVarSup;
global IndexNodeVarInf;

// t : donnees en lignes regroupees par 2 (connection extremite 1 - connection extremite 2)
// p : autant de lignes que de noeuds. Sur chaque ligne n on retrouve les coordonnees 2d du noeud n
// e : liste des noeuds d'appui
// A : liste des sections des elements. Vecteur contenant autant d'entrees que d'elements
// E : liste des modules d'elasticite des elements. Vecteur contenant autant d'entrees que d'elements
// rho : liste des masses volumiques des elements. Vecteur contenant autant d'entrees que d'elements
// F   : liste des forces appliquees aux noeuds. Vecteur colonne comprenant la coordonnes X du noeud 1 en premiere ligne, la coordonnees Y du noeud 1 en seconde 
//       ligne, etc ...

IndexNodeVarSup = [];
IndexNodeVarInf = [];

//      4       6
//      +       +
//     /       /
//    /       / 
//   /       /  
// -+---+  -+---+
//  |1  3   |2  5

p = [0 0 0;
     0 0 1;
     1 0 0;
     1 1 0;
     1 0 1;
     1 1 1];
     
t = [1 3;
     1 4;
     2 5;
     2 6;
     1 2;
     3 5;
     4 6;
     1 5;
     2 3;
     3 6;
     4 5];

IndexNodeVarSup = [4 6];
IndexNodeVarInf = [3 5];

for i=1:NbBlocs
  //  4*(i-1)+3+1 4*(i-1)+3+5  4*(i-1)+3+3 4*(i-1)+3+7    
  //  +----+                      +----+
  //  |\  /                       |\  /
  //  | /\                        | /\ 
  //  |/  \                       |/  \
  //  +----+                      +----+
  //  4*(i-1)+3+0 4*(i-1)+3+4  4*(i-1)+3+2 4*(i-1)+3+6

  p = [p; 
       i+1 0 0;  // 4*(i-1)+3+4
       i+1 1 0;  // 4*(i-1)+3+5
       i+1 0 1;  // 4*(i-1)+3+6
       i+1 1 1]; // 4*(i-1)+3+7
       
  t = [t; 
       4*(i-1)+3+0 4*(i-1)+3+4; // First face
       4*(i-1)+3+1 4*(i-1)+3+5;
       4*(i-1)+3+0 4*(i-1)+3+1;
       4*(i-1)+3+0 4*(i-1)+3+5;
       4*(i-1)+3+1 4*(i-1)+3+4;
       4*(i-1)+3+2 4*(i-1)+3+6; // Second face
       4*(i-1)+3+3 4*(i-1)+3+7;
       4*(i-1)+3+2 4*(i-1)+3+3;
       4*(i-1)+3+3 4*(i-1)+3+6;
       4*(i-1)+3+2 4*(i-1)+3+7;
       4*(i-1)+3+1 4*(i-1)+3+7; // Above cross
       4*(i-1)+3+3 4*(i-1)+3+5;
       4*(i-1)+3+0 4*(i-1)+3+6; // Below cross
       4*(i-1)+3+4 4*(i-1)+3+2;
       4*(i-1)+3+4 4*(i-1)+3+7; // Cross inside
       4*(i-1)+3+5 4*(i-1)+3+6];
       
  IndexNodeVarSup = [IndexNodeVarSup 4*(i-1)+3+5 4*(i-1)+3+7];
  IndexNodeVarInf = [IndexNodeVarInf 4*(i-1)+3+4 4*(i-1)+3+6];
end

//  4*(i-1)+1               4*(i-1)+3   
//  +                          +       
//   \                          \   
//    \                          \
//     \                          \ 
// +---+                       +---+
// 4*(i-1)+0 4*i(i-1)4      4*(i-1)+2 4*(i-1)+5

p = [p; 
     NbBlocs+2 0 0;
     NbBlocs+2 0 1;];

t = [t;
     4*(NbBlocs)+3+0 4*(NbBlocs)+3+1;
     4*(NbBlocs)+3+2 4*(NbBlocs)+3+3;
     4*(NbBlocs)+3+0 4*(NbBlocs)+3+4;
     4*(NbBlocs)+3+2 4*(NbBlocs)+3+5;
     4*(NbBlocs)+3+1 4*(NbBlocs)+3+4;
     4*(NbBlocs)+3+3 4*(NbBlocs)+3+5;
     4*(NbBlocs)+3+4 4*(NbBlocs)+3+5;
     4*(NbBlocs)+3+0 4*(NbBlocs)+3+5;
     4*(NbBlocs)+3+2 4*(NbBlocs)+3+4]
    
e = [
     [1, 1, 1] .* localise3d(1);
     [1, 1, 1] .* localise3d(2);
     [1, 1, 1] .* localise3d(4*NbBlocs+3+4);
     [1, 1, 1] .* localise3d(4*NbBlocs+3+5);
    ]; 

A   = ones(1,size(t,1)) * 25e-4; // sections des elements
E   = ones(1,size(t,1)) * 210e9; // module d'elasticite des elements 
rho = ones(1,size(t,1)) * 7.8e3; // masse volumique des elements

F = zeros(length(p),1);
F(3*(IndexNodeVarInf-1)+2) = -3.4e5;

p(IndexNodeVarSup,1) = x(1:3:$);
p(IndexNodeVarSup,2) = x(2:3:$);
p(IndexNodeVarSup,3) = x(3:3:$);
endfunction

function plot_fobj_truss(x)
[t,p,e,A,E,rho,F] = bridge_optim_3d(x);
[U,P,R]= femtruss(bridge_optim_3d, %F, x);
plotdeforme(U,p,t,10);
endfunction

