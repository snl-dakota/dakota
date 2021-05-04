NbBlocs = 8; // The number of blocks for the bridge.

global IndexNodeVarSup;
global IndexNodeVarInf;

IndexNodeVarSup = [];
IndexNodeVarInf = [];

// Definition of the initial solution. Depends on the number of blocks
x0 = zeros(2*(NbBlocs+1),1);
x0(2:2:$) = 1;
x0(1:2:$) = (1:NbBlocs+1)';

function [t,p,e,A,E,rho,F] = bridge_optim(x)
// t : donnees en lignes regroupees par 2 (connection extremite 1 - connection extremite 2)
// p : autant de lignes que de noeuds. Sur chaque ligne n on retrouve les coordonnees 2d du noeud n
// e : liste des noeuds d'appui
// A : liste des sections des elements. Vecteur contenant autant d'entrees que d'elements
// E : liste des modules d'elasticite des elements. Vecteur contenant autant d'entrees que d'elements
// rho : liste des masses volumiques des elements. Vecteur contenant autant d'entrees que d'elements
// F   : liste des forces appliquees aux noeuds. Vecteur colonne comprenant la coordonnes X du noeud 1 en premiere ligne, la coordonnees Y du noeud 1 en seconde 
//       ligne, etc ...

global IndexNodeVarSup;
global IndexNodeVarInf;

IndexNodeVarSup = [];
IndexNodeVarInf = [];

//      3
//      +
//     /
//    / 
//   /  
// -+---+
//  |1  2

p = [0 0;
     1 0;
     1 1];
     
t = [1 2;
     1 3];

IndexNodeVarSup = 3;
IndexNodeVarInf = 2;

for i=1:NbBlocs
  //  2*i+1 2*i+3    
  //  +----+
  //  |\  /
  //  | /\ 
  //  |/  \
  //  +----+
  //  2*i+0 2*i+2

  p = [p; 
       i+1 0;
       i+1 1];
       
  t = [t; 
       2*i+0 2*i+1;
       2*i+1 2*i+3;
       2*i+0 2*i+2;
       2*i+0 2*i+3;
       2*i+1 2*i+2];
       
  IndexNodeVarSup = [IndexNodeVarSup 2*i+3];
  IndexNodeVarInf = [IndexNodeVarInf 2*i+2];
end

//  2*i+1   
//  +       
//   \   
//    \
//     \ 
// +---+
// 2*i+0 2*i+2   
p = [p; 
     NbBlocs+2 0];

t = [t; 
     2*(NbBlocs+1)+0 2*(NbBlocs+1)+2;
     2*(NbBlocs+1)+1 2*(NbBlocs+1)+2;
     2*(NbBlocs+1)+0 2*(NbBlocs+1)+1];

e = [
     [1, 1] .* localise2d(1) // On localise les positions du noeud 6 dans les matrices et on immobilise les deux degrees de liberte du noeud 5
     [1, 1] .* localise2d(2*(NbBlocs-1)+6) // On localise les positions du noeud 6 dans les matrices et on immobilise les deux degrees de liberte du noeud 2
    ]; 

A   = ones(1,size(t,1)) * 25e-4; // sections des elements
E   = ones(1,size(t,1)) * 210e9; // module d'elasticite des elements 
rho = ones(1,size(t,1)) * 7.8e3; // masse volumique des elements

F = zeros(length(p),1);
F(2*(IndexNodeVarInf-1)+2) = -3.4e5;

p(IndexNodeVarSup,1) = x(1:2:$);
p(IndexNodeVarSup,2) = x(2:2:$);
endfunction

function plot_fobj_truss(x)
[t,p,e,A,E,rho,F] = bridge_optim(x);
[U,P,R]= femtruss(bridge_optim, %F, x);
plotdeforme(U,p,t,10);
endfunction
