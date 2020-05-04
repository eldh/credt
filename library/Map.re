exception NotImplemented;
module type Config = {
  type t;
  type update;
  let getId: t => Util.id;
  let print: t => string;
  let reducer: (t, update) => (t, update);
};

module Make = (Config: Config) => {
  open Config;
  module IMap =
    Stdlib.Map.Make({
      type t = Util.id;
      let compare = Pervasives.compare;
    });
  let mapRemove = (id, data) =>
    try(Ok(IMap.remove(id, data))) {
    | Not_found => Error(`NotFound)
    };
  let mapFind = (id, data) =>
    try(Ok(IMap.find(id, data))) {
    | Not_found => Error(`NotFound)
    };

  type operation =
    | Add(t)
    | Remove(Util.id)
    | Update(Util.id, update)
    | Transaction(list(operation));

  type collection = IMap.t(t);
  let internalId = "__internal__" |> Util.idOfString;
  let internalData = IMap.empty;
  let wrapper = ref(internalData);

  let getCollection = () => wrapper^;
  let get = id => IMap.find(id, getCollection());

  let handleOperation = (~handleUndo, data, op) => {
    switch (op) {
    | Remove(id) =>
      switch (mapRemove(id, data)) {
      | Ok(_) as a =>
        handleUndo(Add(get(id)));
        a;
      | Error(_) => Error(Util.NotFound(op))
      }
    | Add(t) =>
      let id = t |> getId;
      handleUndo(Remove(id));
      Ok(IMap.add(id, t, data));
    | Update(id, update) =>
      switch (mapFind(id, data)) {
      | Ok(item) =>
        let (newData, undo) = reducer(item, update);
        handleUndo(Update(id, undo));
        Ok(IMap.add(id, newData, data));
      | Error(`NotFound) => Error(Util.NotFound(op))
      }
    | Transaction(_) => raise(NotImplemented)
    };
  };

  let setMap = updatedInternalData => {
    wrapper := updatedInternalData;
  };

  /**
   * Apply a list of operations
   */
  let baseApply = (~handleUndo, ops) =>
    Stdlib.List.fold_left(
      ((collection, errors), op) => {
        let res = handleOperation(~handleUndo, collection, op);
        switch (res) {
        | Ok(d) => (d, errors)
        | Error(s) => (collection, [s, ...errors])
        };
      },
      (getCollection(), []),
      ops,
    )
    |> (
      ((list, errors)) => {
        setMap(list);
        switch (errors) {
        | [] => Ok()
        | s => Error(s)
        };
      }
    );

  // let baseApply = (~handleUndo, updates) =>
  //   Stdlib.List.fold_left(
  //     handleOperation(~handleUndo),
  //     getCollection(),
  //     updates,
  //   )
  //   |> setMap;

  /* Apply operations that should not be part of the undo/redo handling */
  let applyRemoteOperations = baseApply(~handleUndo=ignore);

  module Undo =
    UndoRedo.Make({
      type nonrec operation = operation;
      let apply = baseApply;
    });

  let apply = baseApply(~handleUndo=Undo.addUndo);

  let undo = Undo.undo;
  let getUndoHistory = Undo.getUndoHistory;
  let redo = Undo.redo;

  let printCollection = () => {
    print_newline();
    print_newline();
    print_endline(
      "Collection length:"
      ++ (getCollection() |> IMap.cardinal |> string_of_int),
    );
    print_endline("Items:");
    getCollection() |> IMap.iter((_, item) => print(item) |> print_endline);
  };

  let toList = m =>
    m
    |> IMap.to_seq
    |> Stdlib.List.of_seq
    |> Stdlib.List.map(((id, item: t)) => item);
};