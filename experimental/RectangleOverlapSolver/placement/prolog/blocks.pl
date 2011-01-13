


prob1(
	[block(x1,1,3),
	block(x2,1,2),
	block(x3,1,3),
	block(x4,3,2),
	block(x5,3,1)],
	[x2 >= x1 + 1,
	x3 >= x1 + 1,
	x4 >= x2 + 1,
	x4 >= x3 + 1,
	x5 >= x4 + 1]
    ).
%[x5>=x4+1]: [ (x4, 2.66667), (x5, 3.66667)]
%[x3>=x1+1, x2>=x1+1]: [ (x1, 0.375), (x2, 1.375), (x3, 1.375)]

prob1a(
	[
	block(x3,1,3),
        block(x1,1,3),
	block(x4,3,2),
	block(x5,3,1),
	block(x2,1,2)
        ],
	[x2 >= x1 + 1,
	x3 >= x1 + 1,
	x4 >= x2 + 1,
	x4 >= x3 + 1,
	x5 >= x4 + 1]
    ).

prob2(
	[block(x1,0,1),
	block(x2,1,1),
	block(x3,4,1),
	block(x4,4,1),
	block(x5,3,1),
	block(x6,6,1),
	block(x7,5,1),
	block(x8,0,1),
	block(x9,7,1)],
	[x2 >= x1 + 1,
	x3 >= x1 + 1,
	x4 >= x1 + 1,
	x5 >= x2 + 3,
	x5 >= x3 + 1,
	x6 >= x3 + 1,
	x5 >= x4 + 1,
	x8 >= x4 + 1,
	x7 >= x5 + 1,
	x9 >= x6 + 4,
	x9 >= x7 + 2,
	x9 >= x8 + 3]
    ).

%[x9>=x6+4]: [ (x6, 4.5), (x9, 8.5)]
%[ (x7, 5)]
%[x5>=x3+1]: [ (x3, 3), (x5, 4)]
%[x8>=x4+1]: [ (x4, 1.5), (x8, 2.5)]
%[x2>=x1+1]: [ (x1, 0), (x2, 1)]

prob3(
	[block(x1,0,1),
	block(x2,0,1),
	block(x3,0,1),
	block(x4,0,1),
	block(x5,8,1),
	block(x6,8,1),
	block(x7,8,1),
	block(x8,8,1),
	block(x9,4,20)
        ],
	[x2 >= x1 + 1,
	x3 >= x2 + 1,
	x4 >= x3 + 1,
	x6 >= x5 + 1,
	x7 >= x6 + 1,
	x8 >= x7 + 1,
	x3 >= x9 + 1
        ]
     ).

	
prob4(
	[block(x1,1,3),
	 block(x4,3,2),
	 block(x5,3,1),
	 block(x2,1,2),
	 block(x3,1,2)
        ],
	[x2 >= x1 + 1,
	 x3 >= x1 + 1,
	 x4 >= x2 + 1,
	 x4 >= x3 + 1,
	 x5 >= x4 + 1
        ]
    ).

prob5(
	[block(x1,1,1),
	 block(x2,2,1),
	 block(x3,2,1),
	 block(x4,3,1),
	 block(x5,5,1),
	 block(x6,6,1),
	 block(x7,7,1)]
     ,
	[x2 >= x1 + 2,
	x3 >= x1 + 2,
	x5 >= x4 + 2,
	x6 >= x2 + 2,
	x7 >= x5 + 2,
	x4 >= x3 + 1,
	x7 >= x6 + 2
    ]).

prob6([
        block(x1,0,1),
	block(x2,2,1),
	block(x3,1,8),
	block(x4,3,1)
      ],
      [
        x2 >= x1 + 3,
	x3 >= x1 + 3,
	x4 >= x2 + 1.5
      ]).


prob7([
       block(x1,1,7),
       block(x2,-1,2),
       block(x3,1,2),
       block(x4,-1,2),
       block(x5,1,2),
       block(x6,-1,2),
       block(x7,1,2),
       block(x8,-1,2),
       block(x9,1,2),
       block(x10,-1,7)
      ],[
       x2 >= x1 + 0,
       x3 >= x1 + 0, 
       x4 >= x1 + 0, 
       x5 >= x1 + 0, 
       x6 >= x1 + 0, 
       x7 >= x1 + 0, 
       x8 >= x1 + 0, 
       x9 >= x1 + 0, 
       x10 >= x2 + 0, 
       x10 >= x3 + 0, 
       x10 >= x4 + 0, 
       x10 >= x5 + 0, 
       x10 >= x6 + 0, 
       x10 >= x7 + 0, 
       x10 >= x8 + 0, 
       x10 >= x9 + 0
   ]).

      
prob8([
       block(x1,0,2),
	block(x2,0,1),
	block(x3,3,2),
	block(x4,2,5),
	block(x5,5,5),
        block(x6,4,2),
	block(x7,7,1),
	block(x8,7,2),
	block(x9,1,4)
    ],[
        x2 >= x1 + 1,
        x3 >= x2 + 1,
        x4 >= x3 + 1,
        x5 >= x4 + 1,
        x6 >= x5 + 1,
        x7 >= x6 + 1,
        x8 >= x7 + 1,
	x9 >= x1 + 1,
	x8 >= x9 + 6
    ]).



init_block(B,Offset) :- init_block_e(B,Offset).
merge_blocks(PlaceA,PlaceB,Min,Active,PlaceAB) :-
	merge_blocks_e(PlaceA,PlaceB,Min,Active,PlaceAB).
desired_position(Offsets, WPosn, Weight, Pos) :-
	desired_position_e(Offsets, WPosn, Weight, Pos).

%% init_block(B,Offset) :- init_block_m(B,Offset).
%% merge_blocks(PlaceA,PlaceB,Min,Active,PlaceAB) :-
%% 	merge_blocks_m(PlaceA,PlaceB,Min,Active,PlaceAB).
%% desired_position(Offsets, WPosn, Weight, Pos) :-
%% 	desired_position_m(Offsets, WPosn, Weight, Pos).


%%% euclidean distance %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 
init_block_e(Block,Offset) :-
	Block = block(_Name,Pos,Weight),
	WPosn is Pos * Weight,
	Offset = offsetb(0,WPosn,Block).
merge_blocks_e(PlaceA,PlaceB,Min,Active,PlaceAB) :-
	PlaceA = placed(PosA,WeightA,WPosnA,ConstraintsA,OffsetsA),
	PlaceB = placed(PosB,WeightB,WPosnB,ConstraintsB,OffsetsB),
	WeightAB is WeightA + WeightB,
	OffA is PosA - PosB + Min,
	WPosnAB is WPosnA + WPosnB - OffA * WeightA,
	PosAB is WPosnAB / WeightAB,
	
	move_offsets(OffsetsA, OffA, OffsetsAM),
	append(OffsetsAM, OffsetsB, OffsetsAB),
	append(ConstraintsA,[Active|ConstraintsB],ConstraintsAB),
	PlaceAB = placed(PosAB,WeightAB,WPosnAB,ConstraintsAB,OffsetsAB).
desired_position_e(Offsets, WPosn, Weight, Pos) :-
	desired_position_e(Offsets, 0, 0, WPosn, Weight),
	Pos is WPosn/Weight.

desired_position_e([], WPosn, Weight, WPosn, Weight).
desired_position_e([Offset|Os], WPosn0, Weight0, WPosn, Weight) :-
	Offset = offsetb(PosB,_,Block),
	Block = block(_,DesB,WeightB),
	WPosn1 is WPosn0 + (DesB - PosB) * WeightB,
	Weight1 is Weight0 + WeightB,
	desired_position_e(Os, WPosn1, Weight1, WPosn, Weight).
%%% manhattan distance %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 
init_block_m(Block,Offset) :-
	Block = block(_Name,Pos,Weight),
	WPosn = [(Pos,Weight)],
	Offset = offsetb(0,WPosn,Block).
merge_blocks_m(PlaceA,PlaceB,Min,Active,PlaceAB) :-
	PlaceA = placed(PosA,WeightA,WPosnA,ConstraintsA,OffsetsA),
	PlaceB = placed(PosB,WeightB,WPosnB,ConstraintsB,OffsetsB),
	WeightAB is WeightA + WeightB,
	OffA is PosA - PosB + Min,
	translate(WPosnA, OffA, NWPosnA),
	append(NWPosnA,WPosnB,UnsortedWPosnAB),
	msort(UnsortedWPosnAB, WPosnAB),
	compute_opt(WPosnAB,WeightAB,PosAB),
	move_offsets(OffsetsA, OffA, OffsetsAM),
	append(OffsetsAM, OffsetsB, OffsetsAB),
	append(ConstraintsA,[Active|ConstraintsB],ConstraintsAB),
	PlaceAB = placed(PosAB,WeightAB,WPosnAB,ConstraintsAB,OffsetsAB).
desired_position_m(Offsets, WPosn, Weight, Pos) :-
	desired_position_m(Offsets, [], 0, UnsortedWPosn, Weight),
	msort(UnsortedWPosn,WPosn),
	compute_opt(WPosn,Weight,Pos).

desired_position_m([], WPosn, Weight, WPosn, Weight).
desired_position_m([Offset|Os], WPosn0, Weight0, WPosn, Weight) :-
	Offset = offsetb(PosB,_,Block),
	Block = block(_,DesB,WeightB),
	AdjPosB is (DesB - PosB),
	WPosn1 = [(AdjPosB,WeightB)| WPosn0],
	Weight1 is Weight0 + WeightB,
	desired_position_m(Os, WPosn1, Weight1, WPosn, Weight).

translate([], _, []).
translate([(P,W)|PWs], Off, [(PO,W)|Ts]) :-
	PO is P - Off,
	translate(PWs, Off, Ts).

compute_opt([(P,W)|PWs], Weight, Pos) :-
	Weight1 is Weight - 2 * W,
	(Weight1 < 0 ->
	    Pos = P
	;
	    compute_opt(PWs, Weight1, Pos)
	).


% :- type block --> block(name,desired-position,weight)

% :- type placed --> placed(position, weight, wposn, [constraints], [offsets]).

% :- type offset --> offsetb(offset,wposn,block)
%                  ; offsetp(offset,placed)

initial([],[]).
initial([First|Blocks], 
	       [placed(Des,Weight,WPosn,[],[Offset])|Rest]) :-
	init_block(First,Offset),
	First = block(_Name,Des,Weight),
	Offset = offsetb(_,WPosn,_),
	initial(Blocks, Rest).

overlaps(Placed0, Constraints, Placed) :-
%% change line below for different overlap
	(overlap2(Placed0,Constraints,Place1,Place2,Rest,Violated) ->
	    merge(Place1,Place2,Violated,Place12),
	    %write_all_pos([Place12]),
	    split(Place12, Places12),
	    %write_all_pos(Places12),
	    append(Places12, Rest, Placed1),
	    overlaps(Placed1, Constraints, Placed)
	;
	    Placed = Placed0
	).

overlap([Placea|Placed0], Constraints, Place1, Place2, Rest, Violated) :-
	(overlapc(Placed0, Constraints, Placea, Placeb, Restab, Violated) ->
	    Place1 = Placea,
	    Place2 = Placeb,
	    Rest = Restab
	;
	    Rest = [Placea|Rest1],
	    overlap(Placed0, Constraints, Place1, Place2, Rest1, Violated)
	).

overlapc([Placeb|Placed0], Constraints, Place1, Place2, Rest, Violated) :-
	(overlapcc(Constraints, Placeb, Place1,Violated) ->
	    Place2 = Placeb,
	    Rest = Placed0
	;
	    Rest = [Placeb|Rest1],
	    overlapc(Placed0, Constraints, Place1, Place2, Rest1, Violated)
	).

%% 
overlapcc(Constraints, Place1, Place2, Violated) :-
	get_positions(Place1, [], List1),
	get_positions(Place2, [], List2),
	append(List1,List2,All),
	check_constraints(Constraints, All, Violated),
	Violated = [_|_].

check_constraints([], _, []).
check_constraints([X >= Y + D|Constraints], PosList, Violated) :-
	( member((X,XP), PosList),
	  member((Y,YP), PosList),
	  XP + 0.0001 < YP + D ->
	      Violated = [X >= Y + D|Violated1]
	  ;
	      Violated = Violated1
	),
	check_constraints(Constraints, PosList,Violated1).

%%% second version of "overlap" returns the pair that overlap in the
%%% first constraint that is violated
overlap2(Placed, Constraints, Place1, Place2, Rest, Violated) :-
	get_all_positions(Placed, [], List),
	check_constraints(Constraints, List, [Violate|_]),
	Violate = (X >= Y + _),
	remove_place(Placed, X, Place1, Rest1),
	remove_place(Rest1, Y, Place2, Rest),
	overlapcc(Constraints, Place1, Place2, Violated).

get_all_positions([], L, L).
get_all_positions([Place|Placed], L0, L) :-
	get_positions(Place, L0, L1),
	get_all_positions(Placed, L1, L).

remove_place([PlaceA|Placed], Var, Place, Rest) :-
	PlaceA = placed(_PosA,_WeightA,_WPosnA,_ConstraintsA,OffsetsA),
	(member(offsetb(_,_,block(Var,_,_)), OffsetsA) ->
	    Place= PlaceA,
	    Rest = Placed
	;
	    Rest = [PlaceA|Rest1],
	    remove_place(Placed,Var,Place,Rest1)
	).
	

write_all_pos(Ps) :-
	canonify(Ps,CPs),
	sort(CPs,SCPs),
	write_all_pos_actual(SCPs).

canonify([],[]).
canonify([P|Ps],[ConstraintsS - LS|Rest]) :-
	P = placed(_,_,_,Constraints,_), 
	sort(Constraints,ConstraintsS),
	get_positions(P,[],L),
	sort(L,LS),
	canonify(Ps, Rest).

write_all_pos_actual([]).
write_all_pos_actual([ConstraintsS - LS |Ps]) :-
	(ConstraintsS = [] -> true ; write(ConstraintsS), write(': ')),
	write(LS), nl,
	write_all_pos_actual(Ps).

get_positions(placed(Pos, _, _,_, Offsets), ListOfPos0, ListOfPos) :-
	get_positions_offsets(Offsets, Pos, ListOfPos0, ListOfPos).

get_positions(placed(_, _, _, _, Offsets), BPos, ListOfPos0, ListOfPos) :-
	get_positions_offsets(Offsets, BPos, ListOfPos0, ListOfPos).
get_positions_offsets([], _, ListOfPos, ListOfPos).
get_positions_offsets([Offset|Offsets], BPos, ListOfPos0, ListOfPos) :-
	(Offset = offsetb(Off,_,block(Name,_,_)) ->
	    NPos is BPos + Off,
	    ListOfPos1 = [(Name,NPos)|ListOfPos0]
	;
	    Offset = offsetp(Off,Placed),
	    BPos1 is BPos + Off, 
	    get_positions(Placed, BPos1, ListOfPos0, ListOfPos1)
	),
	get_positions_offsets(Offsets, BPos, ListOfPos1, ListOfPos).


merge(Place1, Place2, Violated, Place12) :-
	get_positions(Place1, [], List1),
	get_positions(Place2, [], List2),
	( get_diffs(Violated, List1, List2, Diffs) ->
	    PlaceA = Place1,
	    PlaceB = Place2
	;
	    get_diffs(Violated, List2, List1, Diffs),
	    PlaceA = Place2,
	    PlaceB = Place1
        ),
	maximum(Diffs, Violated, Min, Active),
	merge_blocks(PlaceA,PlaceB,Min,Active,Place12),
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% diagnostics %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	get_positions(Place12,[],List12),
	write('merge: '),
	write(List1), write(' '), write(List2), write(' -> '), write(Active),
	write(' -> '), write(List12), nl,
	%write('check: '),write(Old_PosAB is PosAB), nl,
	true.

move_offsets([],_,[]).
move_offsets([offsetb(Off0,WPosn,Block)|Os], OffA, [offsetb(Off,WPosn,Block)|MOs]) :-
	Off is Off0 + OffA,
	move_offsets(Os,OffA,MOs).
move_offsets([offsetp(Off0,Placed)|Os], OffA, [offsetp(Off,Placed)|MOs]) :-
	Off is Off0 + OffA,
	move_offsets(Os,OffA,MOs).

%% diffs are how much each constraint is violated
get_diffs([], _, _, []).
get_diffs([X >= Y + D|Constraints], List1, List2, [Diff|Diffs]) :-
	member((X,XP), List1), !,
	member((Y,YP), List2), !,
	Diff is YP + D - XP,
	get_diffs(Constraints, List1,List2,Diffs).

%% Min will be bound to the maximum difference 
%% and active to the corresponding constraint
maximum([Diff|Diffs], [C|Cs], Min, Active) :-
	maximum(Diffs,Cs,Diff,Min,C,Active).
maximum([],[],Min,Min,Active,Active).
maximum([D|Ds], [C|Cs], Min0, Min, Active0, Active) :-
	(D > Min0 ->
	    Min1 = D,
	    Active1 = C
	;
	    Min1 = Min0,
	    Active1 = Active0
	),
	maximum(Ds,Cs,Min1,Min,Active1,Active).

split(Place, Places) :-
	Place = placed(_Pos,_Weight,_WPosn,Constraints,_Offsets), 
	check_split(Constraints, Place, Places).
	

check_split([], Place, [Place]).
check_split([C|Cs], Place, ListPlaced) :-
	C = (VA >= VB + D),
	Place = placed(_Pos,_Weight,_WPosn,Constraints,Offsets),
	delete(Constraints,C,RConstraints),
	reachable(RConstraints, [VA], VarsA),
	reachable(RConstraints, [VB], VarsB),
	% check disjoint
	(intersect(VarsA,VarsB) -> error(disjoint) ; true),
	split_constraints(RConstraints, VarsA, ConstraintsA, ConstraintsB),
	split_offsets(Offsets, VarsA, OffsetsA, OffsetsB),
        member(offsetb(OffA,_,block(VA,_,_)),OffsetsA),!,
        member(offsetb(OffB,_,block(VB,_,_)),OffsetsB),!,
	desired_position(OffsetsA, WPosnA, WeightA, PosA),
	desired_position(OffsetsB, WPosnB, WeightB, PosB),
	((PosA + OffA) >= 0.0001 + (PosB + OffB) + D ->
	    PlacedA = placed(PosA, WeightA, WPosnA, ConstraintsA, OffsetsA),
	    PlacedB = placed(PosB, WeightB, WPosnB, ConstraintsB, OffsetsB),
	    get_positions(PlacedA,[],List1),
	    get_positions(PlacedB,[],List2),
	    get_positions(Place,[],List12),
	    Active = C,
	    write('split: '),
	     write(List12), write(' -> '), write(Active),
	    write(' -> '), write(List1), write(' '), write(List2), nl,
	    split(PlacedA, ListPlacedA),
	    split(PlacedB, ListPlacedB),
	    append(ListPlacedA,ListPlacedB,ListPlaced)
	;
	    check_split(Cs, Place, ListPlaced)
	).

intersect(L1,L2) :- member(A,L1), member(A,L2).


% reachable(Cs,Vs0,Vs) - Vs is the subset of variables Vs0 
%    that are referenced by the constraints Cs
reachable(Cs, Vs0, Vs) :-
	sort(Vs0,SVs0),
	reachable2(Cs, SVs0, Vs1),
	sort(Vs1,SVs1),
	(SVs0 = SVs1 ->
	    Vs = SVs1
	;
	    reachable(Cs,SVs1,Vs)
	).
reachable2([], Vs, Vs).
reachable2([VA >= VB + _|Cs], Vs0, Vs) :-
	(member(VA,Vs0) ->
	    Vs1 = [VB|Vs0]
	; (member(VB,Vs0) ->
	    Vs1 = [VA|Vs0]
	;
	    Vs1 = Vs0
	)),
	reachable2(Cs, Vs1, Vs).


partition(Cs, XA >= _YA + _D, ConstraintsA, ConstraintsB) :-
	deleteall(Cs, [], [XA], ConstraintsA, ConstraintsB).

deleteall([], Rest, _, [], Rest).
deleteall([X >= Y + D|Cs], Seen, Vars, Cin, Cout) :-
	(member(X,Vars) ->
	    Cin = [X >= Y + D|Cin1],
	    append(Seen,Cs,All),
	    deleteall(All,[],[Y|Vars],Cin1,Cout)
	; (member(Y,Vars) ->
	    Cin = [X >= Y + D|Cin1],
	    append(Seen,Cs,All),
	    deleteall(All,[],[X|Vars],Cin1,Cout)
	;
	    deleteall(Cs,[X + Y >= D|Seen], Vars, Cin, Cout)
	)).
	    
split_constraints([], _, [], []).
split_constraints([VA >= VB + D|Cs], Vars, CA, CB) :-
	(member(VA,Vars), member(VB,Vars) ->
	    CA = [VA >= VB + D|CA1],
	    CB = CB1
	;
	    CA = CA1,
	    CB = [VA >= VB + D|CB1]
	),
	split_constraints(Cs,Vars,CA1,CB1).

	    
split_offsets([], _, [], []).
split_offsets([Offset|Cs], Vars, CA, CB) :-
	Offset = offsetb(_,_,Block),
	Block = block(Name,_,_),
	(member(Name,Vars) ->
	    CA = [Offset|CA1],
	    CB = CB1
	;
	    CA = CA1,
	    CB = [Offset|CB1]
	),
	split_offsets(Cs,Vars,CA1,CB1).


append([],Y,Y).
append([A|X],Y,[A|Z]) :- append(X,Y,Z).

member(X, [X|_]).
member(X, [_|R]) :- member(X,R).

output_solution(Placed,Constraints) :-
	write_all_pos(Placed),
	calculate_sum(Placed,0,Sum),
	write('Objective: '), write(Sum),
	calculate_active(Constraints,Placed), nl.

calculate_sum([],S,S).
calculate_sum([P|Ps],S0,S) :-
	get_positions(P,[],L),
	P = placed(_,_,_,_,Offsets),
	sum_offsets(Offsets,L,S0,S1),
	calculate_sum(Ps,S1,S).

sum_offsets([],_,S,S).
sum_offsets([offsetb(_,_,block(V,D,W))|Os], L, S0, S) :-
	member((V,VP),L), !,
	S1 is S0 + W * (D - VP) * (D - VP),
	sum_offsets(Os,L,S1,S).

calculate_active(Constraints,Placed) :-
	get_active(Placed,[],Active),
	write('['),
	output_active(Constraints,Active),
	write(']').

output_active([],_).
output_active([C|Cs],A) :-
	(member(C,A) ->
	    write('X')
	;
	    write(' ')
	),
	output_active(Cs,A).

get_active([],A,A).
get_active([P|Ps], A0, A) :-
	P = placed(_,_,_,C,_),
	append(C,A0,A1),
	get_active(Ps,A1,A).


g(E):- prob1(B,_C), initial(B,P), P = [F|_R], get_positions(F,[],E).

g1:- prob1(B,C), initial(B,P), overlaps(P,C,Pn), write(Pn),
	nl, write_all_pos(Pn).
g1a:- prob1a(B,C), initial(B,P), overlaps(P,C,Pn), write(Pn),
	nl, write_all_pos(Pn).

g2:- prob2(B,C), initial(B,P), overlaps(P,C,Pn), write(Pn),
	nl, write_all_pos(Pn).

g3:- prob3(B,C), initial(B,P), overlaps(P,C,Pn), write(Pn),
	nl, write_all_pos(Pn).



g4:- prob4(B,C), initial(B,P), overlaps(P,C,Pn), write(Pn),
	nl, write_all_pos(Pn).


test :-
	Offsets = [offsetb(0,3,block(x1,1,3)),
	           offsetb(1,2,block(x2,1,2)),
		   offsetb(1,2,block(x3,1,2)),
		   offsetb(2,6,block(x4,3,2)),
		   offsetb(3,3,block(x5,3,1))
	          ],
	Constraints = 
           [x3 >= x1 + 1,
	    x4 >= x2 + 1,
	    x4 >= x3 + 1,
	    x5 >= x4 + 1
           ],
	desired_position(Offsets,WPosn,Weight,Pos),
	Placed = placed(Pos, Weight, WPosn, Constraints, Offsets),
        write(Placed), nl,
	split(Placed,Places0),
%	Places0 = [Placed],
	overlaps(Places0,[x2 >= x1 + 1|Constraints],Places),
	write_all_pos(Places),
	true.

test2 :-
	Offsets = [offsetb(0,3,block(x1,1,3)),
	           offsetb(1,2,block(x2,1,2)),
		   offsetb(1,2,block(x3,1,2)),
		   offsetb(2,6,block(x4,3,2)),
		   offsetb(3,3,block(x5,3,1))
	          ],
	Constraints = 
           [x2 >= x1 + 1, 
	    x3 >= x1 + 1,
	    %x4 >= x2 + 1,
	    x4 >= x3 + 1,
	    x5 >= x4 + 1
           ],
	desired_position(Offsets,WPosn,Weight,Pos),
	Placed = placed(Pos, Weight, WPosn, Constraints, Offsets),
        write(Placed), nl,
	split(Placed,Places0),
%	Places0 = [Placed],
	overlaps(Places0,[x4 >= x2 + 1|Constraints],Places),
	write_all_pos(Places),
	true.

test7 :-
	P0 = placed(0,30,0,       
	 [x2 >= x1 + 0,
          x3 >= x1 + 0, 
          x4 >= x1 + 0, 
          x5 >= x1 + 0, 
          x6 >= x1 + 0, 
          x7 >= x1 + 0, 
          x8 >= x1 + 0, 
          x9 >= x1 + 0, 
          x10 >= x2 + 0],
         [
          offsetb(0,7,block(x1,1,7)),
       offsetb(0,-2,block(x2,-1,2)),
       offsetb(0,2,block(x3,1,2)),
       offsetb(0,-2,block(x4,-1,2)),
       offsetb(0,2,block(x5,1,2)),
       offsetb(0,-2,block(x6,-1,2)),
       offsetb(0,2,block(x7,1,2)),
       offsetb(0,-2,block(x8,-1,2)),
       offsetb(0,2,block(x9,1,2)),
       offsetb(0,-7,block(x10,-1,7))
        ]),
	C = [
       x2 >= x1 + 0,
       x3 >= x1 + 0, 
       x4 >= x1 + 0, 
       x5 >= x1 + 0, 
       x6 >= x1 + 0, 
       x7 >= x1 + 0, 
       x8 >= x1 + 0, 
       x9 >= x1 + 0, 
       x10 >= x2 + 0, 
       x10 >= x3 + 0, 
       x10 >= x4 + 0, 
       x10 >= x5 + 0, 
       x10 >= x6 + 0, 
       x10 >= x7 + 0, 
       x10 >= x8 + 0, 
       x10 >= x9 + 0
   ],
	merge_repeat([P0],C,P1),	
	output_solution(P1,C),
	alg11(P1,C,_P).

test8 :-
	P0 = [placed(0,[],14,[
         x2 >= x1 + 1,
        x3 >= x2 + 1,
        x4 >= x3 + 1,
        x5 >= x4 + 1,
        x6 >= x5 + 1,
        x7 >= x6 + 1,
	x9 >= x1 + 1,
	x8 >= x9 + 6
    ],[
        offsetb(0,[],block(x1,0,2)),
	offsetb(1,[],block(x2,0,1)),
	offsetb(2,[],block(x3,3,2)),
	offsetb(3,[],block(x4,2,5)),
	offsetb(4,[],block(x5,5,5)),
        offsetb(5,[],block(x6,4,2)),
	offsetb(6,[],block(x7,7,1)),
	offsetb(7,[],block(x8,7,2)),
	offsetb(1,[],block(x9,1,4))
    ])],
	C = [
        x2 >= x1 + 1,
        x3 >= x2 + 1,
        x4 >= x3 + 1,
        x5 >= x4 + 1,
        x6 >= x5 + 1,
        x7 >= x6 + 1,
        x8 >= x7 + 1,
	x9 >= x1 + 1,
	x8 >= x9 + 6
    ],
	merge_repeat(P0,C,P1),	
	output_solution(P1,C),
	alg11(P1,C,_P).
	


%%%%%%%%%%%%%%%%%%%% ALGORITHM 1 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%
%% Merge until satisfiable
%% repeat
%%     split once
%%     merge until satisfiable

alg1(B,C,P) :-
	initial(B,P0),
	merge_repeat(P0,C,P1),
	output_solution(P1,C),
	alg11(P1, C, P).

alg11(P0,C,P) :-
	(find_split(P0, P1) ->
	    merge_repeat(P1,C,P2),
	    output_solution(P2,C),
	    alg11(P2,C,P)
	;
	    P = P0
	).

merge_repeat(Placed0,Constraints,Placed) :-
	(overlap(Placed0,Constraints,Place1,Place2,Rest,Violated) ->
	    merge(Place1,Place2,Violated,Place12),
	    length(Violated,VS),
	    (VS>1->	
	    	write('**** VIOLATED SIZE ='),write(VS),
			write(':'),write(Violated)
		;
			VS=1
		),
	    Placed1 = [Place12|Rest],
	    merge_repeat(Placed1, Constraints, Placed)
	;
	    Placed = Placed0
	).
	

find_split([PlaceA|Places],Placed) :-
	PlaceA = placed(_Pos,_Weight,_WPosn,Constraints,_Offsets), 
	(single_split(Constraints,PlaceA,PlacesA) ->
	    append(PlacesA,Places,Placed)
	;
	    Placed = [PlaceA|Placed1],
	    find_split(Places,Placed1)
	).


single_split([C|Cs], Place, ListPlaced) :-
	C = (VA >= VB + D),
	Place = placed(_Pos,_Weight,_WPosn,Constraints,Offsets),
	delete(Constraints,C,RConstraints),
	reachable(RConstraints, [VA], VarsA),
	reachable(RConstraints, [VB], VarsB),
	% check disjoint
	(intersect(VarsA,VarsB) -> error(disjoint) ; true),
	split_constraints(RConstraints, VarsA, ConstraintsA, ConstraintsB),
	split_offsets(Offsets, VarsA, OffsetsA, OffsetsB),
        member(offsetb(OffA,_,block(VA,_,_)),OffsetsA),!,
        member(offsetb(OffB,_,block(VB,_,_)),OffsetsB),!,
	desired_position(OffsetsA, WPosnA, WeightA, PosA),
	desired_position(OffsetsB, WPosnB, WeightB, PosB),
	((PosA + OffA) >= 0.0001 + (PosB + OffB) + D ->
	    PlacedA = placed(PosA, WeightA, WPosnA, ConstraintsA, OffsetsA),
	    PlacedB = placed(PosB, WeightB, WPosnB, ConstraintsB, OffsetsB),
	    get_positions(PlacedA,[],List1),
	    get_positions(PlacedB,[],List2),
	    get_positions(Place,[],List12),
	    Active = C,
	    write('split: '),
	     write(List12), write(' -> '), write(Active),
	    write(' -> '), write(List1), write(' '), write(List2), nl,
	    ListPlaced = [PlacedA,PlacedB]
	;
	    single_split(Cs, Place, ListPlaced)
	).


	
a1 :- prob1(B,C), alg1(B,C,_P).
a2 :- prob2(B,C), alg1(B,C,_P).
a3 :- prob3(B,C), alg1(B,C,_P).
a4 :- prob4(B,C), alg1(B,C,_P).
a5 :- prob5(B,C), alg1(B,C,_P).
a6 :- prob6(B,C), alg1(B,C,_P).
a7 :- prob7(B,C), alg1(B,C,_P).
a8 :- prob8(B,C), alg1(B,C,_P).
