module type ListConfig = {
  type t;
  type update;
  let getId: t => Util.id;
  let print: t => string;
  let reducer: (t, update) => (t, update);
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

  type collection = list(t);
  let internalId = "__internal__" |> Util.idOfString;
  let wrapper: ref(collection) = ref([]);

  let getCollection = () => wrapper^;
  let get = id =>
    Stdlib.List.find(item => getId(item) === id, getCollection());

  let handleOperation = (~handleUndo, data) => {
    fun
    | Remove(id) => {
        handleUndo(Append(get(id))); // Todo put in correct place
        Stdlib.List.filter(item => getId(item) !== id, data);
      }
    | Append(t) => {
        let id = t |> getId;
        handleUndo(Remove(id));
        data @ [t];
      }
    | Prepend(t) => {
        let id = t |> getId;
        handleUndo(Remove(id));
        [t] @ data;
      }
    | AddBefore(_id, t) => {
        // Todo implement
        let id = t |> getId;
        handleUndo(Remove(id));
        data @ [t];
      }
    | AddAfter(_id, t) => {
        // Todo implement
        let id = t |> getId;
        handleUndo(Remove(id));
        data @ [t];
      }
    | Update(id, update) => {
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
        | Some(undo) => handleUndo(Update(id, undo))
        | None => ()
        };
        l;
      }
    | Replace(id, t) => {
        let prevItem = Stdlib.List.find(item => getId(item) === id, data);
        handleUndo(Replace(id, prevItem));
        data |> Stdlib.List.map(item => {item |> getId === id ? t : item});
      };
  };

  let setData = updatedInternalData => {
    wrapper := updatedInternalData;
  };

  /**
   * Apply a list of operations
   */
  let baseApply = (~handleUndo, updates) =>
    Stdlib.List.fold_left(
      handleOperation(~handleUndo),
      getCollection(),
      updates,
    )
    |> setData;

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