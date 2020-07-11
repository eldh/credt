exception NotImplemented;
// TODO Perhaps handle cases where undo/redo cannot be applied in a better way. Now we just pop from the list and return the apply error.
module Make =
       (
         C: {
           type operation;
           let apply:
             (
               ~handleUndo: ((Util.id, operation)) => unit,
               list((Util.id, operation))
             ) =>
             result(unit, list((Util.id, Util.operationError(operation))));
         },
       ) => {
  type undoOperation =
    | Op((Util.id, C.operation))
    | Transaction(list((Util.id, C.operation)));

  let undoHistory: ref(list(undoOperation)) = ref([]);
  let redoHistory: ref(list(undoOperation)) = ref([]);

  let getUndoHistory = () => undoHistory^;
  let getRedoHistory = () => redoHistory^;

  let addUndo = ((id, op)) => {
    undoHistory := [Op((id, op))] @ undoHistory^;
  };

  let addRedo = ((id, op)) => {
    redoHistory := [Op((id, op))] @ redoHistory^;
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
    | [Op((id, op)), ...rest] =>
      undoHistory := rest;
      [(id, op)] |> C.apply(~handleUndo=addRedo);
    };
  };

  let redo = () => {
    switch (redoHistory^) {
    | [] => Ok()
    | [Transaction(ops), ...rest] =>
      redoHistory := rest;
      addUndoTransaction(ops);
      ops |> C.apply(~handleUndo=ignore);
    | [Op((id, op)), ...rest] =>
      redoHistory := rest;
      [(id, op)] |> C.apply(~handleUndo=addUndo);
    };
  };

  let __reset__ = () => {
    undoHistory := [];
    redoHistory := [];
  };
};
