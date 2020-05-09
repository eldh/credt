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
    Stdlib.List.filter(
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
  try(Ok(Stdlib.List.find(fn, data))) {
  | Not_found => Error(`NotFound)
  };

module Make = (Config: ListConfig) => {
  open Config;
  type diff = t;
  type operation =
    | Append(t)
    | Prepend(t)
    | AddAfter(Util.id, t)
    | AddBefore(Util.id, t)
    | Remove(Util.id)
    | Replace(Util.id, t)
    | Update(Util.id, update);

  let string_of_operation =
    fun
    | Append(_t) => "Append"
    | Prepend(_t) => "Prepend"
    | AddAfter(_, _) => "AddAfter"
    | AddBefore(_, _) => "AddBefore"
    | Remove(_) => "Remove"
    | Replace(_, _) => "Replace"
    | Update(_, _) => "Update";

  type collection = list(t);
  let internalId = "__internal__" |> Util.idOfString;
  let wrapper: ref(collection) = ref([]);

  let getCollection = () => wrapper^;

  let get = id =>
    Stdlib.List.find(item => getId(item) === id, getCollection());

  let setData = updatedInternalData => {
    wrapper := updatedInternalData;
  };

  let handleOperation = (~handleUndo, data, op) => {
    switch (op) {
    | Remove(id) =>
      let item = listFind(item => getId(item) === id, data);
      switch (item, listRemove(item => getId(item) !== id, data)) {
      | (Ok(i), Ok(_) as a) =>
        handleUndo(Append(i)); // Todo put in correct place
        a;
      | (Error(_), Ok(_))
      | (Ok(_), Error(_))
      | (Error(_), Error(_)) => Error(Util.NotFound(op))
      };
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
        Stdlib.List.fold_right(
          (item, newList) => {
            item |> getId === id
              ? {
                foundItem := true;
                [t, item, ...newList];
              }
              : [item, ...newList]
          },
          data,
          [],
        );
      foundItem^ ? Ok(res) : Error(Util.NotFound(op));
    | AddAfter(id, t) =>
      let newItemId = t |> getId;
      handleUndo(Remove(newItemId));
      let foundItem = ref(false);
      let res =
        Stdlib.List.fold_right(
          (item, newList) => {
            item |> getId === id
              ? {
                foundItem := true;
                [item, t, ...newList];
              }
              : [item, ...newList]
          },
          data,
          [],
        );
      foundItem^ ? Ok(res) : Error(Util.NotFound(op));
    | Update(id, update) =>
      let (l, undo) =
        data
        |> Stdlib.List.fold_left(
             ((l, undoOp), item) => {
               item |> getId === id
                 ? {
                   let (newData, undo) = reducer(item, update);

                   ([newData, ...l], Some(undo));
                 }
                 : ([item, ...l], undoOp)
             },
             ([], None),
           );
      switch (undo) {
      | Some(undo) =>
        handleUndo(Update(id, undo));
        Ok(l);
      | None => Error(Util.NotFound(op))
      };
    | Replace(id, t) =>
      switch (Stdlib.List.find_opt(item => getId(item) === id, data)) {
      | Some(item) =>
        handleUndo(Replace(id, item));
        Ok(
          data |> Stdlib.List.map(item => {item |> getId === id ? t : item}),
        );
      | None => Error(Util.NotFound(op))
      }
    };
  };
  let baseApply:
    (~handleUndo: operation => unit, list(operation)) =>
    result(unit, list(Util.operationError(operation))) =
    (~handleUndo, ops) =>
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
    let prevCollection = getCollection();
    switch (Undo.applyTransaction(ops)) {
    | Ok(data) as ok => ok
    | Error(_) as err =>
      setData(prevCollection);
      err;
    };
  };

  let undo = Undo.undo;
  let getUndoHistory = Undo.getUndoHistory;
  let redo = Undo.redo;
  let getRedoHistory = Undo.getRedoHistory;

  let length = () => getCollection() |> Stdlib.List.length;
  let __resetCollection__ = () => {
    wrapper := [];
    Undo.__reset__();
  };

  let print = Config.print;
  let printCollection = () => {
    print_newline();
    print_endline("List: ");
    getCollection()
    |> Stdlib.List.mapi((i, item) =>
         (i |> string_of_int) ++ ": " ++ Config.print(item)
       )
    |> Stdlib.List.iter(print_endline);
  };
};