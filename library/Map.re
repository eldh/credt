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
  type operation =
    | Add(t)
    | Remove(Util.id)
    | Update(Util.id, update);

  type collection = IMap.t(t);
  let internalId = "__internal__" |> Util.idOfString;
  let internalData = IMap.empty;
  let wrapper = ref(internalData);

  let getCollection = () => wrapper^;
  let get = id => IMap.find(id, getCollection());

  let handleOperation = (~handleUndo, data) => {
    fun
    | Remove(id) => {
        handleUndo(Add(get(id)));
        IMap.remove(id, data);
      }
    | Add(t) => {
        let id = t |> getId;
        handleUndo(Remove(id));
        IMap.add(id, t, data);
      }
    | Update(id, update) => {
        let (newData, undo) = reducer(IMap.find(id, data), update);
        handleUndo(Update(id, undo));
        IMap.add(id, newData, data);
      };
  };

  let setMap = updatedInternalData => {
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
    |> setMap;

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