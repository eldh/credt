exception NotImplemented;

module type ListConfig = {
  type t;
  type update;
  let getId: t => Util.id;
  let print: t => string;
  let reducer: (t, update) => (t, update);
};

let listRemove = (fn, list) => {
  let foundItem = ref(false);
  let newList =
    Tablecloth.List.filter(
      ~f=
        item => {
          let res = fn(item);
          if (!res) {
            foundItem := true;
          };
          res;
        },
      list,
    );
  foundItem^ ? Ok(newList) : Error(`NotFound);
};
let listFind = (fn, data) =>
  try(Ok(Tablecloth.List.find(~f=fn, data))) {
  | Not_found => Error(`NotFound)
  };

let rec __findExn = (index, fn, lst) =>
  switch (lst) {
  | [] => raise(Failure("Not Found"))
  | [h, ...t] =>
    if (fn(h)) {
      (index, h);
    } else {
      __findExn(1 + index, fn, t);
    }
  };

let __insertAtExn = (index, item, _currentIndex, lst) =>
  if (index >= Tablecloth.List.length(lst)) {
    lst @ [item];
  } else {
    lst @ [item]; // Todo fix
  };

let find = (x, lst) =>
  try(Some(__findExn(0, x, lst))) {
  | _ => None
  };

module Make = (Config: ListConfig) => {
  open Config;
  type diff = t;
  type operation =
    | Append(t)
    | Prepend(t)
    | InsertAt(int, t)
    | AddAfter(Util.id, t)
    | AddBefore(Util.id, t)
    | Remove(Util.id)
    | Replace(Util.id, t)
    | Update(Util.id, update);

  let string_of_operation =
    fun
    | Append(_t) => "Append"
    | Prepend(_t) => "Prepend"
    | InsertAt(_, _) => "InsertAt"
    | AddAfter(_, _) => "AddAfter"
    | AddBefore(_, _) => "AddBefore"
    | Remove(_) => "Remove"
    | Replace(_, _) => "Replace"
    | Update(_, _) => "Update";

  type collection = list(t);
  let internalId = "__internal__" |> Util.idOfString;
  let wrapper: ref(collection) = ref([]);

  let getSnapshot = () => wrapper^;

  let get = id =>
    Tablecloth.List.find(~f=item => getId(item) === id, getSnapshot())
    |> Tablecloth.Result.fromOption(~error="Item not found");

  let getExn = id =>
    id |> get |> Tablecloth.Result.toOption |> Tablecloth.Option.getExn;

  let setData = updatedInternalData => {
    wrapper := updatedInternalData;
  };

  let handleOperation = (~handleUndo, data, op) => {
    switch (op) {
    | Remove(id) =>
      let findRes = find(item => getId(item) === id, data);
      switch (findRes, listRemove(item => getId(item) !== id, data)) {
      | (Some((index, item)), Ok(_) as a) =>
        handleUndo(InsertAt(index, item));
        a;
      | (None, Ok(_))
      | (Some(_), Error(_))
      | (None, Error(_)) => Error(Util.NotFound(op))
      };
    | InsertAt(_index, t) =>
      // TODO fix

      handleUndo(Remove(t |> getId));
      Ok(data @ [t]);
    | Append(t) =>
      handleUndo(Remove(t |> getId));
      Ok(data @ [t]);
    | Prepend(t) =>
      handleUndo(Remove(t |> getId));
      Ok([t] @ data);
    | AddBefore(id, t) =>
      let newItemId = t |> getId;
      handleUndo(Remove(newItemId));
      let foundItem = ref(false);
      let res =
        Tablecloth.List.foldRight(
          ~f=
            (item, newList) => {
              item |> getId === id
                ? {
                  foundItem := true;
                  [t, item, ...newList];
                }
                : [item, ...newList]
            },
          ~initial=[],
          data,
        );
      foundItem^ ? Ok(res) : Error(Util.NotFound(op));
    | AddAfter(id, t) =>
      let newItemId = t |> getId;
      handleUndo(Remove(newItemId));
      let foundItem = ref(false);
      let res =
        Tablecloth.List.foldRight(
          ~f=
            (item, newList) => {
              item |> getId === id
                ? {
                  foundItem := true;
                  [item, t, ...newList];
                }
                : [item, ...newList]
            },
          ~initial=[],
          data,
        );
      foundItem^ ? Ok(res) : Error(Util.NotFound(op));
    | Update(id, update) =>
      let (l, undo) =
        Tablecloth.List.foldLeft(
          ~f=
            (item, (l, undoOp)) => {
              item |> getId === id
                ? {
                  let (newData, undo) = reducer(item, update);
                  ([newData, ...l], Some(undo));
                }
                : ([item, ...l], undoOp)
            },
          ~initial=([], None),
          data,
        );
      switch (undo) {
      | Some(undo) =>
        handleUndo(Update(id, undo));
        Ok(l);
      | None => Error(Util.NotFound(op))
      };
    | Replace(id, t) =>
      switch (Tablecloth.List.find(~f=item => getId(item) === id, data)) {
      | Some(item) =>
        handleUndo(Replace(id, item));
        Ok(
          data
          |> Tablecloth.List.map(~f=item => {item |> getId === id ? t : item}),
        );
      | None => Error(Util.NotFound(op))
      }
    };
  };
  let baseApply:
    (~handleUndo: operation => unit, list(operation)) =>
    result(unit, list(Util.operationError(operation))) =
    (~handleUndo, ops) =>
      Tablecloth.List.foldLeft(
        ~f=
          (op, (collection, errors)) => {
            let res = handleOperation(~handleUndo, collection, op);
            switch (res) {
            | Ok(d) => (d, errors)
            | Error(s) => (collection, [s, ...errors])
            };
          },
        ~initial=(getSnapshot(), []),
        ops,
      )
      |> (
        ((list, errors)) => {
          setData(list);
          switch (errors) {
          | [] => Ok()
          | s => Error(s)
          };
        }
      );

  /**
   * Apply operations that should not be part of the undo/redo handling
   * */
  let applyRemoteOperations = baseApply(~handleUndo=ignore);

  module Undo =
    UndoRedo.Make({
      type nonrec operation = operation;
      let apply = baseApply;
    });

  let apply = Undo.apply;
  let applyTransaction = ops => {
    let prevCollection = getSnapshot();
    switch (Undo.applyTransaction(ops)) {
    | Ok(_data) as ok => ok
    | Error(_) as err =>
      setData(prevCollection);
      err;
    };
  };

  let undo = Undo.undo;
  let getUndoHistory = Undo.getUndoHistory;
  let redo = Undo.redo;
  let getRedoHistory = Undo.getRedoHistory;

  let length = () => getSnapshot() |> Tablecloth.List.length;
  let __resetCollection__ = () => {
    wrapper := [];
    Undo.__reset__();
  };

  let print = Config.print;
  let printCollection = () => {
    print_newline();
    print_endline("List: ");
    getSnapshot()
    |> Tablecloth.List.mapi(~f=(i, item) =>
         (i |> string_of_int) ++ ": " ++ Config.print(item)
       )
    |> Tablecloth.List.iter(~f=print_endline);
  };
};