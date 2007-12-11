(*
	computation of the stress function for straightening a bendy edge,
        and it's derivative and hessian.
        various test cases.
*)

(* euclidean distance between two points *)
l[a_,b_]:=Sqrt[(a[[1]]-b[[1]])^2+(a[[2]]-b[[2]])^2];
(* stress function *)
d=.;
f[X_,Y_]:=d^-2 (d-Sum[
    l[{X[[i-1]],Y[[i-1]]},{X[[i]],Y[[i]]}],{i,2,Length[X]}
])^2;

(*
X=Array[x,3];
Y=Array[y,3];
f[X,Y]
from=Table[l[{X[[i - 1]], Y[[i - 1]]}, {X[[i]], Y[[i]]}], {i, 2, Length[X]}]
to=Table[Subscript[l,i-1,i],{i,2,Length[X]}]
dfrom=Table[X[[i-1]]-X[[i]],{i,2,Length[X]}]
dto=Table[Subscript[dx,i-1,i],{i,2,Length[X]}]
rules=Join[
   {PowerExpand[d Sqrt[f[X, Y]]]->dl},
   Thread[from->to],
   Thread[1/from->1/to],
   Thread[from^(-2)->to^(-2)],
   Thread[from^(-3)->to^(-3)],
   Thread[dfrom->dto]]
g=D[f[X,Y],{X,1}]//.rules
h=D[f[X,Y],{X,2}]//.rules
g/.{x[i_] -> Xv[[i]], y[i_] -> Yv[[i]]}
*)

(* 
	extract points from rectangles
	a rectangle is an array: {bottom left point, bottom right, width, height
*)
tl[r_]:={r[[1]],r[[2]]+r[[4]]};
tr[r_]:={r[[1]]+r[[3]],r[[2]]+r[[4]]};
bl[r_]:={r[[1]],r[[2]]};
br[r_]:={r[[1]]+r[[3]],r[[2]]};
centre[r_]:={r[[1]]+r[[3]]/2.0,r[[2]]+r[[4]]/2.0};

(*
	test case: 3 rectangles, 1 path, 1 bend
*)
d=100.000000; (* ideal length *)
v0={571.500000,363.500000,63.000000,43.000000};
v1={620.000000,469.500000,63.000000,43.000000};
v2={541.500000,300.500000,63.000000,43.000000};

(* Path defined in Xv, Yv *)
Xv={centre[v1][[1]],br[v0][[1]],centre[v2][[1]]};
Yv={centre[v1][[2]],br[v0][[2]],centre[v2][[2]]};
(* order in which nodes are visited in path *)
Xi={1,0,2};
n=Length[Xi];

expected=1.05721;
Abs[f[Xv,Yv]-expected]<10^(-5)

X=Array[x,n];
Y=Array[y,n];
g=D[f[X,Y],{X,1}]/.{x[i_]:>Xv[[i]], y[i_]:>Yv[[i]]};
h=D[f[X,Y],{X,2}]/.{x[i_]:>Xv[[i]], y[i_]:>Yv[[i]]};

gg=Table[g[[Xi[[i]]+1]],{i,1,n}];
hh=Table[h[[Xi[[i]]+1]][[Xi[[j]]+1]],{i,1,n},{j,1,n}];

