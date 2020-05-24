module Map = {
  module Make = (C: {
                   type t;
                   let compare: (t, t) => int;
                 }) => {
    include Belt_MapString;
    external stringOfId: C.t => string = "%identity";

    let empty = Belt_MapString.empty;
    let add = (id: C.t, t, data) =>
      Belt_MapString.set(data, id->stringOfId, t);
    let cardinal = Belt_MapString.size;
    let remove = (id: C.t, data) =>
      Belt_MapString.remove(data, id |> stringOfId);
    let iter = (fn, data) => Belt_MapString.forEach(data, fn);
    // TODO
    let to_seq = Belt_MapString.toList;
    let find = (id, data) => Belt_MapString.getExn(data, id |> stringOfId);
  };
};
module List = {
  let of_seq = a => a;
};