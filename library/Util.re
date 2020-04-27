let now = () => 0.; // TODO obviously

type id;
let lastId = ref(0);
let idOfString: string => id = Obj.magic;
let stringOfId: id => string = Obj.magic;

let makeId: unit => id =
  () => {
    let id = lastId^ + 1;
    lastId := id;
    id |> string_of_int |> idOfString;
  };