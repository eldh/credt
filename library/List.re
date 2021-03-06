exception NotImplemented;

module type ListConfig = {
  type t;
  type update;
  let moduleId: Util.id;
  let getId: t => Util.id;
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

  // let toResult: Tablecloth.Result.t('a, 'b) => result('b, 'a) =
  //   fun
  //   | Ok(a) => Ok(a)
  //   | Error(a) => Error(a);

  let get = id =>
    Tablecloth.List.find(~f=item => getId(item) === id, getSnapshot());

  let getExn = id => id |> get |> Tablecloth.Option.getExn;

  let setData = updatedInternalData => {
    wrapper := updatedInternalData;
  };

  type changeListener = list(operation) => unit;
  let changeListeners: ref(list(changeListener)) = ref([]);

  let addChangeListener = fn => {
    changeListeners := [fn, ...changeListeners^];
  };

  let removeChangeListener = fn => {
    changeListeners :=
      changeListeners^
      |> Tablecloth.List.filter(~f=changeFn => changeFn !== fn);
  };

  let callChangeListeners = ops => {
    changeListeners^ |> Tablecloth.List.iter(~f=fn => fn(ops));
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
  let baseApply = (~handleUndo, ops) =>
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

  Manager.register(moduleId, baseApply);

  let apply = ops => {
    let res = Manager.apply(moduleId, ops);
    callChangeListeners(ops);
    res;
  };
  let addToTransaction = ops => {
    let prevCollection = getSnapshot();
    Manager.addToTransaction(moduleId, ops, () => {setData(prevCollection)});
  };

  let length = () => getSnapshot() |> Tablecloth.List.length;
  let __resetCollection__ = () => {
    wrapper := [];
    changeListeners := [];
  };

  type instance = {
    get: Util.id => option(Config.t),
    getSnapshot: unit => collection,
    apply:
      list(operation) =>
      result(unit, list((Util.id, Util.operationError(Manager.op)))),
    applyRemoteOperations:
      list(operation) => result(unit, list(Util.operationError(operation))),
    length: unit => int,
    addToTransaction: list(operation) => unit,
    addChangeListener: changeListener => unit,
    removeChangeListener: changeListener => unit,
  };

  let instance = {
    get,
    getSnapshot,
    apply,
    applyRemoteOperations,
    length,
    addToTransaction,
    addChangeListener,
    removeChangeListener,
  };
};
