exception NotImplemented;

// TODO Perhaps handle cases where undo/redo cannot be applied in a better way. Now we just pop from the list and return the apply error.
module Make =
       (
         C: {
           type operation;
           let apply:
             (~handleUndo: operation => unit, list(operation)) =>
             result(unit, list(Util.operationError(operation)));
         },
       ) => {
  type undoOperation =
    | Op(C.operation)
    | Transaction(list(C.operation));

  let undoHistory: ref(list(undoOperation)) = ref([]);
  let redoHistory: ref(list(undoOperation)) = ref([]);

  let getUndoHistory = () => undoHistory^;
  let getRedoHistory = () => redoHistory^;

  let addUndo = op => {
    undoHistory := [Op(op)] @ undoHistory^;
  };

  let addRedo = op => {
    redoHistory := [Op(op)] @ redoHistory^;
  };

  let addUndoTransaction = ops => {
    undoHistory := [Transaction(ops)] @ undoHistory^;
  };

  let addRedoTransaction = ops => {
    redoHistory := [Transaction(ops)] @ redoHistory^;
  };

  let apply = C.apply(~handleUndo=addUndo);

  let applyTransaction = ops => {
    switch (ops |> C.apply(~handleUndo=ignore)) {
    | Ok(_) as ok =>
      addUndoTransaction(ops);
      ok;
    | Error(_) as err => err
    };
  };

  let canUndo = getUndoHistory() |> Tablecloth.List.length > 0;
  let canRedo = getRedoHistory() |> Tablecloth.List.length > 0;
  let undo = () => {
    switch (undoHistory^) {
    | [] => Ok()
    | [Transaction(ops), ...rest] =>
      undoHistory := rest;
      addRedoTransaction(ops);
      ops |> C.apply(~handleUndo=ignore);
    | [Op(op), ...rest] =>
      undoHistory := rest;
      [op] |> C.apply(~handleUndo=addRedo);
    };
  };

  let redo = () => {
    switch (redoHistory^) {
    | [] => Ok()
    | [Transaction(ops), ...rest] =>
      redoHistory := rest;
      addUndoTransaction(ops);
      ops |> C.apply(~handleUndo=ignore);
    | [Op(op), ...rest] =>
      redoHistory := rest;
      [op] |> C.apply(~handleUndo=addUndo);
    };
  };

  let __reset__ = () => {
    undoHistory := [];
    redoHistory := [];
  };
};