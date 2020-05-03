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
  foundItem^ ? Ok(newList) : Error(Util.NotFound);
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
    | Update(Util.id, update)
    | Transaction(list(operation));

  let string_of_operation =
    fun
    | Append(_t) => "Append"
    | Prepend(_t) => "Prepend"
    | AddAfter(_, _) => "AddAfter"
    | AddBefore(_, _) => "AddBefore"
    | Remove(_) => "Remove"
    | Replace(_, _) => "Replace"
    | Update(_, _) => "Update"
    | Transaction(_) => "Transaction";

  type collection = list(t);
  let internalId = "__internal__" |> Util.idOfString;
  let wrapper: ref(collection) = ref([]);

  let getCollection = () => wrapper^;

  let __resetCollection__ = () => wrapper := [];

  let get = id =>
    Stdlib.List.find(item => getId(item) === id, getCollection());

  let handleOperation = (~handleUndo, data) => {
    fun
    | Remove(id) => {
        handleUndo(Append(get(id))); // Todo put in correct place
        listRemove(item => getId(item) !== id, data);
      }
    | Append(t) => {
        handleUndo(Remove(t |> getId));
        Ok(data @ [t]);
      }
    | Prepend(t) => {
        handleUndo(Remove(t |> getId));
        Ok([t] @ data);
      }
    | AddBefore(_id, t) => {
        // Todo implement
        let id = t |> getId;
        handleUndo(Remove(id));
        Ok(data @ [t]);
      }
    | AddAfter(_id, t) => {
        // Todo implement
        let id = t |> getId;
        handleUndo(Remove(id));
        Ok(data @ [t]);
      }
    | Update(id, update) => {
        // TODO Handle error
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
        | None => Error(Util.NotFound)
        };
      }
    | Replace(id, t) => {
        // TODO Handle error
        let prevItem = Stdlib.List.find(item => getId(item) === id, data);
        handleUndo(Replace(id, prevItem));
        Ok(
          data |> Stdlib.List.map(item => {item |> getId === id ? t : item}),
        );
      }
    | Transaction(_) => raise(NotImplemented);
  };

  let setData = updatedInternalData => {
    wrapper := updatedInternalData;
  };

  /**
   * Apply a list of operations
   */
  let baseApply = (~handleUndo, ops) =>
    Stdlib.List.fold_left(
      ((collection, errors), op) => {
        print_endline(
          (op |> string_of_operation)
          ++ ": "
          ++ (collection |> Stdlib.List.length |> string_of_int),
        );
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

  let length = () => getCollection() |> Stdlib.List.length;

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