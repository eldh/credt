let foo = () => print_endline("Hello");
let one = () => 1;
module type ListConfig = {type t;};

let now = () => 0.;

type id;
type float;
let lastId = ref(0);
let idOfString: string => id = Obj.magic;
let stringOfId: id => string = Obj.magic;

let makeId: unit => id =
  () => {
    let id = lastId^ + 1;
    lastId := id;
    id |> string_of_int |> idOfString;
  };

module RList = (Config: ListConfig) => {
  open Config;
  type diff = t;
  type operation =
    | Append(t)
    | Prepend(t)
    | AddAfter(id, t)
    | AddBefore(id, t)
    | Remove(id)
    | Replace(id, t);
};

module type MapConfig = {
  type t;
  type update;
  let getId: t => id;
  let print: t => string;
  let reducer: (t, update) => (t, update);
};

module RMap = (Config: MapConfig) => {
  open Config;
  module IMap =
    Map.Make({
      type t = id;
      let compare = Pervasives.compare;
    });
  type operation =
    | Add(t)
    | Remove(id)
    | Update(id, update);

  let undoHistory: ref(list(operation)) = ref([]);
  let redoHistory: ref(list(operation)) = ref([]);

  let addUndo = undo => {
    undoHistory := [undo] @ undoHistory^;
  };

  let addRedo = redo => {
    redoHistory := [redo] @ redoHistory^;
  };

  let stringOfOperation =
    fun
    | Remove(id) => {
        "Remove: " ++ (id |> stringOfId);
      }
    | Add(t) => {
        "Add: " ++ (t |> print);
      }
    | Update(id, update) => {
        "Update: " ++ (id |> stringOfId);
      };

  let printUndoHistory = () =>
    undoHistory^
    |> List.map(stringOfOperation)
    |> List.fold_left((a, b) => {a ++ ", " ++ b}, "Undo: ")
    |> print_endline;

  let printRedoHistory = () =>
    redoHistory^
    |> List.map(stringOfOperation)
    |> List.fold_left((a, b) => {a ++ ", " ++ b}, "Redo: ")
    |> print_endline;

  type collection = IMap.t(t);
  let internalId = "__internal__" |> idOfString;
  let internalData = IMap.empty;
  let wrapper = ref(internalData);

  let getCollection = () => wrapper^;
  let get = id => IMap.find(id, getCollection());

  let handleOperation = (~handleUndo=addUndo, data) => {
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
  let apply = (~handleUndo=?, updates) =>
    List.fold_left(handleOperation(~handleUndo?), getCollection(), updates)
    |> setMap;

  /* Apply operations that should not be part of the undo/redo handling */
  let applyRemoteOperations = apply(~handleUndo=ignore);

  let undo = () => {
    switch (undoHistory^) {
    | [] => ()
    | [a, ...rest] =>
      [a] |> apply(~handleUndo=addRedo);
      undoHistory := rest;
    };
  };

  let redo = () => {
    switch (redoHistory^) {
    | [] => ()
    | [a, ...rest] =>
      [a] |> apply;
      redoHistory := rest;
    };
  };

  let printCollection = () => {
    print_endline(
      "Print collection:"
      ++ (getCollection() |> IMap.cardinal |> string_of_int),
    );
    getCollection() |> IMap.iter((_, item) => print(item) |> print_endline);
  };

  let toList = m =>
    m |> IMap.to_seq |> List.of_seq |> List.map(((id, item: t)) => item);
};