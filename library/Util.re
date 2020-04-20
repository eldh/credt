let foo = () => print_endline("Hello");
let one = () => 1;
module type ListConfig = {type t;};

let now = () => 0.;

type id;
type timestamp = float;

let makeId: string => id = Obj.magic;

module RList = (Config: ListConfig) => {
  open Config;
  type diff = t;
  type operation =
    | Append(timestamp, t)
    | Prepend(timestamp, t)
    | AddAfter(timestamp, id, t)
    | AddBefore(timestamp, id, t)
    | Remove(timestamp, id)
    | Replace(timestamp, id, t);
};

module type MapConfig = {
  type t;
  type update;
  let getId: t => id;
  let reducer: (t, update) => (t, update);
};

module RMap = (Config: MapConfig) => {
  open Config;
  module IMap =
    Map.Make({
      type t = id;
      let compare = Pervasives.compare;
    });
  let internalId = "__internal__" |> makeId;
  let internalData = IMap.empty;
  let wrapper = ref(internalData);
  type operation =
    | Add(timestamp, t)
    | Remove(timestamp, id)
    | Update(timestamp, id, update);

  let add = item => Add(now(), item);

  let update = (item, action) => Update(now(), item |> getId, action);

  let remove = item => Remove(now(), item |> getId);
  let handleUpdate = data => {
    fun
    | Remove(timestamp, id) => IMap.remove(id, data)
    | Add(timestamp, t) => IMap.add(t |> getId, t, data)
    | Update(timestamp, id, update) => {
        let (newData, _) = reducer(IMap.find(id, data), update);
        IMap.add(id, newData, data);
      };
  };
  let getMap_ = () => wrapper^;
  let setMap = updatedInternalData => {
    wrapper := updatedInternalData;
  };
  let toList = m =>
    m |> IMap.to_seq |> List.of_seq |> List.map(((id, item: t)) => item);
  let save = updates => {
    List.fold_left(handleUpdate, getMap_(), updates) |> setMap;
  };
  let get = id => IMap.find(id, getMap_());
};