module Make =
       (
         C: {
           type operation;
           let apply:
             (~handleUndo: operation => unit, list(operation)) => unit;
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
    | [] => ()
    | [a, ...rest] =>
      [a] |> C.apply(~handleUndo=addRedo);
      undoHistory := rest;
    };
  };

  let redo = () => {
    switch (redoHistory^) {
    | [] => ()
    | [a, ...rest] =>
      [a] |> C.apply(~handleUndo=addUndo);
      redoHistory := rest;
    };
  };
};