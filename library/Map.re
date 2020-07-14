exception NotImplemented;
module type Config = {
  type t;
  type update;
  let moduleId: Util.id;
  let getId: t => Util.id;
  let reducer: (t, update) => (t, update);
};

module Make = (Config: Config) => {
  open Config;
  module IMap =
    Stdlib.Map.Make({
      type t = Util.id;
      let compare = Pervasives.compare;
    });
  let mapRemove = (id, data) => {
    switch (IMap.find_opt(id, data)) {
    | Some(_) => Ok(IMap.remove(id, data))
    | None => Error(`NotFound)
    };
  };
  let mapFind = (id, data) => {
    switch (IMap.find_opt(id, data)) {
    | Some(a) => Ok(a)
    | None => Error(`NotFound)
    };
  };

  type operation =
    | Add(t)
    | Remove(Util.id)
    | Update(Util.id, update);

  type collection = IMap.t(t);
  let internalId = "__internal__" |> Util.idOfString;
  let internalData = IMap.empty;
  let wrapper = ref(internalData);

  let getSnapshot = () => wrapper^;
  let get = id => IMap.find(id, getSnapshot());

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
    // | Transaction(_) => raise(NotImplemented)
    };
  };

  let setMap = updatedInternalData => {
    wrapper := updatedInternalData;
  };

  /**
   * Apply a list of operations
   */
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
        setMap(list);
        switch (errors) {
        | [] => Ok()
        | s => Error(s)
        };
      }
    );

  Manager.register(moduleId, baseApply);

  /* Apply operations that should not be part of the undo/redo handling */
  let applyRemoteOperations = baseApply(~handleUndo=ignore);

  let apply = ops => {
    let res = Manager.apply(moduleId, ops);
    callChangeListeners(ops);
    res;
  };
  let addToTransaction = ops => {
    let prevCollection = getSnapshot();
    Manager.addToTransaction(moduleId, ops, () => {setMap(prevCollection)});
  };

  let __resetCollection__ = () => {
    wrapper := IMap.empty;
    changeListeners := [];
  };

  let toList = m =>
    m
    |> IMap.to_seq
    |> Stdlib.List.of_seq
    |> Tablecloth.List.map(~f=((_id, item: t)) => item);
};
