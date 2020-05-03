module Make =
       (
         C: {
           type operation;
           let apply:
             (~handleUndo: operation => unit, list(operation)) =>
             result(unit, list(Util.operationError));
         },
       ) => {
  let undoHistory: ref(list(C.operation)) = ref([]);
  let redoHistory: ref(list(C.operation)) = ref([]);

  let getUndoHistory = () => undoHistory^;
  let getRedoHistory = () => redoHistory^;

  let addUndo = undo => {
    undoHistory := [undo] @ undoHistory^;
  };

  let addRedo = redo => {
    redoHistory := [redo] @ redoHistory^;
  };

  let canUndo = getUndoHistory() |> Stdlib.List.length > 0;
  let canRedo = getRedoHistory() |> Stdlib.List.length > 0;
  let undo = () => {
    switch (undoHistory^) {
    | [] => Ok()
    | [a, ...rest] =>
      switch ([a] |> C.apply(~handleUndo=addRedo)) {
      | Ok(_) as ok =>
        undoHistory := rest;
        ok;
      | Error(_) as err => err
      }
    };
  };

  let redo = () => {
    switch (redoHistory^) {
    | [] => Ok()
    | [a, ...rest] =>
      switch ([a] |> C.apply(~handleUndo=addUndo)) {
      | Ok(_) as ok =>
        redoHistory := rest;
        ok;
      | Error(_) as err => err
      }
    };
  };
};