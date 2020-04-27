let foo = () => print_endline("Hello");
let one = () => 1;
module type ListConfig = {type t;};

let now = () => 0.;

type id;
type float;
let lastId = ref(0);
let castId: string => id = Obj.magic;

let makeId: unit => id =
  () => {
    let id = lastId^ + 1;
    lastId := id;
    id |> string_of_int |> castId;
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
  type collection = IMap.t(t);
  let internalId = "__internal__" |> castId;
  let internalData = IMap.empty;
  let wrapper = ref(internalData);
  type operation =
    | Add(t)
    | Remove(id)
    | Update(id, update);

  let handleUpdate = data => {
    fun
    | Remove(id) => IMap.remove(id, data)
    | Add(t) => IMap.add(t |> getId, t, data)
    | Update(id, update) => {
        print_endline("herer___1: " ++ (id |> Obj.magic));
        let (newData, _) = reducer(IMap.find(id, data), update);
        IMap.add(id, newData, data);
      };
  };

  let getCollection = () => wrapper^;

  let printCollection = () => {
    print_endline(
      "Print collection:"
      ++ (getCollection() |> IMap.cardinal |> string_of_int),
    );
    getCollection() |> IMap.iter((_, item) => print(item) |> print_endline);
  };

  let setMap = updatedInternalData => {
    wrapper := updatedInternalData;
  };

  let toList = m =>
    m |> IMap.to_seq |> List.of_seq |> List.map(((id, item: t)) => item);
  let save = updates => {
    List.fold_left(handleUpdate, getCollection(), updates) |> setMap;
  };
  let get = id => IMap.find(id, getCollection());
};