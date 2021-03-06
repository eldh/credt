open TestFramework;
module UserMap = {
  type t = {
    id: Credt.Util.id,
    name: string,
    email: string,
    age: int,
  };

  type update =
    | SetEmail(string)
    | SetName(string)
    | SetAge(int);

  include Credt.Map.Make({
    type nonrec t = t;
    type nonrec update = update;
    let moduleId = "UserMap" |> Credt.Util.idOfString;
    let getId = u => u.id;
    let reducer = user =>
      fun
      | SetEmail(email) => ({...user, email}, SetEmail(user.email))
      | SetName(name) => ({...user, name}, SetName(user.name))
      | SetAge(age) => ({...user, age}, SetAge(user.age));
  });
};
let makeUser = i => {
  UserMap.id: Credt.Util.makeId(),
  name: "Name" ++ (i |> string_of_int),
  email: "email@" ++ (i |> string_of_int),
  age: i * 2,
};
let {describe, describeSkip, describeOnly} =
  describeConfig
  |> withLifecycle(testLifecycle =>
       testLifecycle
       |> beforeAll(() => ())
       |> afterAll(() => ())
       |> beforeEach(() => {
            Credt.Manager.__reset__();
            UserMap.__resetCollection__();
          })
       |> afterEach(() => ())
     )
  |> build;

describe("Map", ({test, testOnly}) => {
  test("should update item", ({expect}) => {
    let me =
      UserMap.{
        id: Credt.Util.makeId(),
        name: "Andreas",
        email: "andreas@eldh.co",
        age: 35,
      };

    expect.result(
      UserMap.[Add(me), Update(me.id, SetEmail("a@eldh.co"))]
      |> UserMap.apply,
    ).
      toBeOk();

    expect.equal(me.email, "andreas@eldh.co");
    expect.equal(UserMap.getExn(me.id).email, "a@eldh.co");
  });
  test("should listen", ({expect}) => {
    let mock = Mock.mock1(_ => ());
    expect.mock(mock).toBeCalledTimes(0);
    UserMap.addChangeListener(Mock.fn(mock));
    [makeUser(2), makeUser(1)]
    |> Stdlib.List.map(u => UserMap.Add(u))
    |> UserMap.apply
    |> ignore;
    expect.equal(
      UserMap.getSnapshot() |> UserMap.toList |> Stdlib.List.length,
      2,
    );
    expect.mock(mock).toBeCalledTimes(1);
  });

  test("should remove listener", ({expect}) => {
    let mock = Mock.mock1(_ => ());
    let mock2 = Mock.mock1(_ => ());
    expect.mock(mock).toBeCalledTimes(0);
    let listener = Mock.fn(mock);
    let listener2 = Mock.fn(mock2);

    UserMap.addChangeListener(listener);
    UserMap.addChangeListener(listener2);

    [makeUser(2), makeUser(1)]
    |> Stdlib.List.map(u => UserMap.Add(u))
    |> UserMap.apply
    |> ignore;
    expect.mock(mock).toBeCalledTimes(1);
    expect.mock(mock2).toBeCalledTimes(1);

    UserMap.removeChangeListener(listener);

    [makeUser(3), makeUser(4)]
    |> Stdlib.List.map(u => UserMap.Add(u))
    |> UserMap.apply
    |> ignore;
    expect.mock(mock).toBeCalledTimes(1);
    expect.mock(mock2).toBeCalledTimes(2);
  });

  test("should handle illegal operation", ({expect}) => {
    let user = makeUser(1);
    let addOp = UserMap.[Add(user)];
    let removeOp = UserMap.[Remove(user.id)];
    let editOp = UserMap.[Update(user.id, SetName("New name"))];

    expect.result(addOp |> UserMap.apply).toBeOk();
    expect.result(removeOp |> UserMap.apply).toBeOk();
    expect.result(editOp |> UserMap.apply).toBeError();
  });

  test("should handle multiple items", ({expect}) => {
    let me =
      UserMap.{
        id: Credt.Util.makeId(),
        name: "Andreas",
        email: "andreas@eldh.co",
        age: 35,
      };

    let miniMe =
      UserMap.{
        id: Credt.Util.makeId(),
        name: "Sixten",
        email: "sixten@eldh.co",
        age: 2,
      };

    expect.result(
      UserMap.[
        Add(me),
        Update(me.id, SetEmail("a@eldh.co")),
        Add(miniMe),
        Update(miniMe.id, SetName("Sixten Eldh")),
      ]
      |> UserMap.apply,
    ).
      toBeOk();

    expect.equal(UserMap.getExn(miniMe.id).name, "Sixten Eldh");
    expect.equal(UserMap.getExn(me.id).email, "a@eldh.co");
    expect.equal(UserMap.getExn(miniMe.id).age, 2);
  });

  test("should handle ok transaction", ({expect}) => {
    let user1 = makeUser(1);
    let user2 = makeUser(2);
    UserMap.([Add(user1), Add(user2)] |> addToTransaction);
    expect.result(Credt.Manager.commitTransaction()).toBeOk();
    expect.equal(user1.name, UserMap.getExn(user1.id).name);
    expect.equal(user2.name, UserMap.getExn(user2.id).name);
  });

  test("should handle error transaction", ({expect}) => {
    let user1 = makeUser(1);
    let user2 = makeUser(2);
    let user3 = makeUser(3);
    UserMap.(
      [Add(user1), Add(user2), Remove(user3.id)] |> addToTransaction
    );
    expect.result(Credt.Manager.commitTransaction()).toBeError();
    expect.equal(
      UserMap.(getSnapshot() |> toList |> Tablecloth.List.length),
      0,
    );
  });
  test("should return error when removing non-existing record", ({expect}) => {
    let user3 = makeUser(3);
    expect.result(UserMap.([Remove(user3.id)] |> apply)).toBeError();
  });

  test("should handle undo & redo", ({expect}) => {
    let miniMe =
      UserMap.{
        id: Credt.Util.makeId(),
        name: "Sixten",
        email: "sixten@eldh.co",
        age: 2,
      };
    expect.result(
      UserMap.[Add(miniMe), Update(miniMe.id, SetName("Sixten Eldh"))]
      |> UserMap.apply,
    ).
      toBeOk();
    expect.equal(UserMap.getExn(miniMe.id).name, "Sixten Eldh");

    // Undo queue
    let previousUndoLength =
      Credt.Manager.getUndoHistory() |> Stdlib.List.length;
    expect.result(Credt.Manager.undo()).toBeOk();
    let newUndoLength = Credt.Manager.getUndoHistory() |> Stdlib.List.length;
    expect.equal(newUndoLength, previousUndoLength - 1);

    expect.result(
      UserMap.applyRemoteOperations([
        Add({
          id: Credt.Util.makeId(),
          name: "Alien",
          email: "alien@space.co",
          age: 21111111,
        }),
        Update(miniMe.id, SetName("Sxtn")),
      ]),
    ).
      toBeOk();

    // Remote operations should not affect undo queue
    expect.equal(
      newUndoLength,
      Credt.Manager.getUndoHistory() |> Stdlib.List.length,
    );

    // But values should update
    expect.equal(UserMap.getExn(miniMe.id).name, "Sxtn");

    // Redo still works, acts as a new update
    expect.result(Credt.Manager.redo()).toBeOk();
    expect.equal(UserMap.getExn(miniMe.id).name, "Sixten Eldh");
    expect.equal(UserMap.getExn(miniMe.id).age, 2);
  });
});
