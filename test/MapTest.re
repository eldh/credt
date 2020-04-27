open TestFramework;
module UserMap = {
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

  include Rupp.Map.Make({
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

describe("Map", ({test}) => {
  test("should update item", ({expect}) => {
    let me =
      UserMap.{
        id: Rupp.Util.makeId(),
        name: "Andreas",
        email: "andreas@eldh.co",
        age: 35,
      };

    UserMap.[Add(me), Update(me.id, SetEmail("a@eldh.co"))]
    |> UserMap.apply;

    expect.equal(me.email, "andreas@eldh.co");
    expect.equal(UserMap.get(me.id).email, "a@eldh.co");
  });

  test("should handle multiple items", ({expect}) => {
    let me =
      UserMap.{
        id: Rupp.Util.makeId(),
        name: "Andreas",
        email: "andreas@eldh.co",
        age: 35,
      };

    let miniMe =
      UserMap.{
        id: Rupp.Util.makeId(),
        name: "Sixten",
        email: "sixten@eldh.co",
        age: 2,
      };

    UserMap.[
      Add(me),
      Update(me.id, SetEmail("a@eldh.co")),
      Add(miniMe),
      Update(miniMe.id, SetName("Sixten Eldh")),
    ]
    |> UserMap.apply;

    expect.equal(UserMap.get(miniMe.id).name, "Sixten Eldh");
    expect.equal(UserMap.get(me.id).email, "a@eldh.co");
    expect.equal(UserMap.get(miniMe.id).age, 2);
  });
  test("should handle undo & redo", ({expect}) => {
    let miniMe =
      UserMap.{
        id: Rupp.Util.makeId(),
        name: "Sixten",
        email: "sixten@eldh.co",
        age: 2,
      };
    UserMap.[Add(miniMe), Update(miniMe.id, SetName("Sixten Eldh"))]
    |> UserMap.apply;
    expect.equal(UserMap.get(miniMe.id).name, "Sixten Eldh");

    // Undo queue
    let previousUndoLength = UserMap.getUndoHistory() |> Stdlib.List.length;
    UserMap.undo();
    let newUndoLength = UserMap.getUndoHistory() |> Stdlib.List.length;
    expect.equal(newUndoLength, previousUndoLength - 1);

    UserMap.applyRemoteOperations([
      Add({
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
      UserMap.getUndoHistory() |> Stdlib.List.length,
    );

    // But values should update
    expect.equal(UserMap.get(miniMe.id).name, "Sxtn");

    // Redo still works, acts as a new update
    UserMap.redo();
    expect.equal(UserMap.get(miniMe.id).name, "Sixten Eldh");
    expect.equal(UserMap.get(miniMe.id).age, 2);
    UserMap.printCollection();
  });
});