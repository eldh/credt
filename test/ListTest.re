open TestFramework;
module UserList = {
  type t = {
    id: Rupp.Util.id,
    name: string,
    email: string,
    age: int,
  };

  type update =
    | SetEmail(string)
    | SetName(string)
    | SetAge(int);

  include Rupp.List.Make({
    type nonrec t = t;
    type nonrec update = update;
    let getId = u => u.id;
    let print = u =>
      "{\n  id: "
      ++ (u.id |> Rupp.Util.stringOfId)
      ++ ",\n  name: "
      ++ u.name
      ++ ",\n  email: "
      ++ u.email
      ++ ",\n  age: "
      ++ (u.age |> string_of_int)
      ++ "\n}";
    let reducer = user =>
      fun
      | SetEmail(email) => ({...user, email}, SetEmail(user.email))
      | SetName(name) => ({...user, name}, SetName(user.name))
      | SetAge(age) => ({...user, age}, SetAge(user.age));
  });
};
let makeUser = i => {
  UserList.id: Rupp.Util.makeId(),
  name: "Name" ++ (i |> string_of_int),
  email: "email@" ++ (i |> string_of_int),
  age: i * 2,
};

let userStdList = Stdlib.List.init(10, makeUser);

describe("List", ({test}) => {
  test("should add", ({expect}) => {
    userStdList |> Stdlib.List.map(u => UserList.Append(u)) |> UserList.apply;

    expect.equal(UserList.length(), 10);
  });

  test("should handle multiple items", ({expect}) => {
    let me =
      UserList.{
        id: Rupp.Util.makeId(),
        name: "Andreas",
        email: "andreas@eldh.co",
        age: 35,
      };

    let miniMe =
      UserList.{
        id: Rupp.Util.makeId(),
        name: "Sixten",
        email: "sixten@eldh.co",
        age: 2,
      };

    UserList.[
      Append(me),
      Update(me.id, SetEmail("a@eldh.co")),
      Append(miniMe),
      Update(miniMe.id, SetName("Sixten Eldh")),
    ]
    |> UserList.apply;

    expect.equal(UserList.get(miniMe.id).name, "Sixten Eldh");
    expect.equal(UserList.get(me.id).email, "a@eldh.co");
    expect.equal(UserList.get(miniMe.id).age, 2);
  });
  test("should handle undo & redo", ({expect}) => {
    let miniMe =
      UserList.{
        id: Rupp.Util.makeId(),
        name: "Sixten",
        email: "sixten@eldh.co",
        age: 2,
      };
    UserList.[Append(miniMe), Update(miniMe.id, SetName("Sixten Eldh"))]
    |> UserList.apply;
    expect.equal(UserList.get(miniMe.id).name, "Sixten Eldh");

    // Undo queue
    let previousUndoLength = UserList.getUndoHistory() |> Stdlib.List.length;
    UserList.undo();
    let newUndoLength = UserList.getUndoHistory() |> Stdlib.List.length;
    expect.equal(newUndoLength, previousUndoLength - 1);

    UserList.applyRemoteOperations([
      Append({
        id: Rupp.Util.makeId(),
        name: "Alien",
        email: "alien@space.co",
        age: 21111111,
      }),
      Update(miniMe.id, SetName("Sxtn")),
    ]);

    // Remote operations should not affect undo queue
    expect.equal(
      newUndoLength,
      UserList.getUndoHistory() |> Stdlib.List.length,
    );

    // But values should update
    expect.equal(UserList.get(miniMe.id).name, "Sxtn");

    // Redo still works, acts as a new update
    UserList.redo();
    expect.equal(UserList.get(miniMe.id).name, "Sixten Eldh");
    expect.equal(UserList.get(miniMe.id).age, 2);
    UserList.printCollection();
  });
});