type op;
let toOp: 'a => op = Obj.magic;
let fromOp: op => 'a = Obj.magic;
type undoOperation = (Util.id, op);
type apply =
  (~handleUndo: op => unit, list(op)) =>
  result(unit, list(Util.operationError(op)));

let applyFns: ref(list((Util.id, apply))) = ref([]);
let register = (id, apply) => {
  applyFns := [(id, apply), ...applyFns^];
};
let getApplyFn = inputId =>
  applyFns^
  |> Stdlib.List.find(((id, _)) => id === inputId)
  |> (((_, apply)) => apply);

let mapError = fn =>
  fun
  | Ok(_) as a => a
  | Error(err) => fn(err);
let baseApply = (~handleUndo, ops: list(undoOperation)) => {
  // TODO Maybe find all ops with same if and apply in one call
  applyFns^
  |> Tablecloth.List.map(~f=((id, fn)) => {
       ops
       |> Tablecloth.List.filterMap(~f=((opId, op)) =>
            id === opId ? Some(op) : None
          )
       |> fn(~handleUndo=op => handleUndo((id, op)))
       |> mapError(errs =>
            Error(errs |> Tablecloth.List.map(~f=err => (id, err)))
          )
     })
  |> Tablecloth.List.fold_left(~initial=Ok(), ~f=(res, memo) => {
       //TODO Merge errors,
       switch (memo, res) {
       | (Ok (), Ok ()) => Ok()
       | (Error(errs) as e, Ok ())
       | (Ok (), Error(errs) as e) => e
       | (Error(memoErrs), Error(errs)) =>
         Error(Stdlib.List.concat([memoErrs, errs]))
       }
     });
       // |> Stdlib.List.concat;
};
let handleUndo = ((id, op)) => {
  []; // TODO FIX
};

module Undo =
  ManagerUndoRedo.Make({
    type operation = op;
    let apply = baseApply;
  });

let apply = (id: Util.id, applyFn) => {
  // TODO FIX
  print_endline("id" ++ (id |> Util.stringOfId));
  ops => {
    Undo.apply(ops);
  };
};

let applyTransaction = (id: Util.id, applyTransactionFn) => {
  // TODO FIX
  print_endline("id" ++ (id |> Util.stringOfId));
  ops => {
    Undo.applyTransaction(ops);
  };
};
