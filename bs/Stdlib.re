// Just the parts needed for library to function.
// Using Tablecloth when possible.

module Map = {
  module Make = (C: {
                   type t;
                   let compare: (t, t) => int;
                 }) => {
    // Using Belt.Map.String since it's most performant. Util.id is opaque, but actually a string, so it works.
    include Belt_MapString;

    // This depends on Util.id being a string
    external stringOfId: C.t => string = "%identity";

    // The rest is just implmeneting Stdlib.Map

    let empty = Belt_MapString.empty;

    let add = (id: C.t, t, data) =>
      Belt_MapString.set(data, id->stringOfId, t);

    let cardinal = Belt_MapString.size;

    let remove = (id: C.t, data) =>
      Belt_MapString.remove(data, id |> stringOfId);

    let iter = (fn, data) => Belt_MapString.forEach(data, fn);

    // There's no bucklescript equivalent of Seq, so convert to list. List.from_seq is then just %identity
    let to_seq = Belt_MapString.toList;

    let find = (id, data) => Belt_MapString.getExn(data, id |> stringOfId);
  };
};

module List = {
  // In this universe, Seq === List
  external of_seq: 'a => 'a = "%identity";
};