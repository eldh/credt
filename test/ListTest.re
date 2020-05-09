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

let {describe, describeSkip, describeOnly} =
  describeConfig
  |> withLifecycle(testLifecycle =>
       testLifecycle
       |> beforeAll(() => ())
       |> afterAll(() => ())
       |> beforeEach(() => {UserList.__resetCollection__()})
       |> afterEach(() => ())
     )
  |> build;

describe("List", ({test, testOnly}) => {
  test("should add", ({expect}) => {
    userStdList
    |> Stdlib.List.map(u => UserList.Append(u))
    |> UserList.apply
    |> ignore;
    expect.equal(UserList.length(), 10);
  });
  test("should add after", ({expect}) => {
    userStdList
    |> Stdlib.List.map(u => UserList.Append(u))
    |> Stdlib.List.rev
    |> UserList.apply
    |> ignore;
    expect.equal(UserList.length(), 10);
    let testUser = makeUser(122);
    expect.result(
      UserList.(
        [
          AddAfter(
            List.nth(UserList.getCollection(), 3) |> (u => u.id),
            testUser,
          ),
        ]
        |> apply
      ),
    ).
      toBeOk();
    expect.equal(
      List.nth(UserList.getCollection(), 4) |> (u => u.id),
      testUser.id,
    );
  });
  test("should add before", ({expect}) => {
    userStdList
    |> Stdlib.List.map(u => UserList.Append(u))
    |> Stdlib.List.rev
    |> UserList.apply
    |> ignore;
    expect.equal(UserList.length(), 10);
    let testUser = makeUser(122);
    expect.result(
      UserList.(
        [
          AddBefore(
            List.nth(UserList.getCollection(), 3) |> (u => u.id),
            testUser,
          ),
        ]
        |> apply
      ),
    ).
      toBeOk();
    expect.equal(
      List.nth(UserList.getCollection(), 3) |> (u => u.id),
      testUser.id,
    );
  });
  test("should fail to add after invalid id", ({expect}) => {
    userStdList
    |> Stdlib.List.map(u => UserList.Append(u))
    |> Stdlib.List.rev
    |> UserList.apply
    |> ignore;
    expect.equal(UserList.length(), 10);
    let notAddedUser = makeUser(11232323);
    let testUser = makeUser(122);
    expect.result(UserList.([AddAfter(notAddedUser.id, testUser)] |> apply)).
      toBeError();
  });
  test("should fail to add before invalid id", ({expect}) => {
    userStdList
    |> Stdlib.List.map(u => UserList.Append(u))
    |> Stdlib.List.rev
    |> UserList.apply
    |> ignore;
    expect.equal(UserList.length(), 10);
    let notAddedUser = makeUser(11232323);
    let testUser = makeUser(122);
    expect.result(
      UserList.([AddBefore(notAddedUser.id, testUser)] |> apply),
    ).
      toBeError();
  });

  test("should handle illegal operation", ({expect}) => {
    let user = makeUser(1);
    let addOp = UserList.[Append(user)];
    let removeOp = UserList.[Remove(user.id)];
    let editOp = UserList.[Update(user.id, SetName("New name"))];
    let replaceOp = UserList.[Replace(user.id, makeUser(2))];

    expect.result(addOp |> UserList.apply).toBeOk();
    expect.result(removeOp |> UserList.apply).toBeOk();
    expect.result(editOp |> UserList.apply).toBeError();
    expect.result(replaceOp |> UserList.apply).toBeError();
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

    expect.result(
      UserList.[
        Append(me),
        Update(me.id, SetEmail("a@eldh.co")),
        Append(miniMe),
        Update(miniMe.id, SetName("Sixten Eldh")),
      ]
      |> UserList.apply,
    ).
      toBeOk();

    expect.equal(UserList.get(miniMe.id).name, "Sixten Eldh");
    expect.equal(UserList.get(me.id).email, "a@eldh.co");
    expect.equal(UserList.get(miniMe.id).age, 2);
  });

  test("should handle ok transaction", ({expect}) => {
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

    expect.result(
      UserList.[
        Append(me),
        Update(me.id, SetEmail("a@eldh.co")),
        Append(miniMe),
        Update(miniMe.id, SetName("Sixten Eldh")),
      ]
      |> UserList.applyTransaction,
    ).
      toBeOk();

    expect.equal(UserList.get(miniMe.id).name, "Sixten Eldh");
    expect.equal(UserList.get(me.id).email, "a@eldh.co");
    expect.equal(UserList.get(miniMe.id).age, 2);
    expect.equal(UserList.getUndoHistory() |> Stdlib.List.length, 1);
    expect.equal(UserList.getRedoHistory() |> Stdlib.List.length, 0);
  });

  test("should roll back transaction", ({expect}) => {
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

    expect.result(
      UserList.[
        Append(me),
        Remove(me.id),
        Update(me.id, SetEmail("a@eldh.co")),
        Append(miniMe),
        Update(miniMe.id, SetName("Sixten Eldh")),
      ]
      |> UserList.applyTransaction,
    ).
      toBeError();
    expect.equal(UserList.getCollection() |> Stdlib.List.length, 0);
    expect.equal(UserList.getUndoHistory() |> Stdlib.List.length, 0);
    expect.equal(UserList.getRedoHistory() |> Stdlib.List.length, 0);
  });

  test("should handle undo & redo", ({expect}) => {
    let miniMe =
      UserList.{
        id: Rupp.Util.makeId(),
        name: "Sixten",
        email: "sixten@eldh.co",
        age: 2,
      };
    expect.result(
      UserList.[Append(miniMe), Update(miniMe.id, SetName("Sixten Eldh"))]
      |> UserList.apply,
    ).
      toBeOk();
    expect.equal(UserList.get(miniMe.id).name, "Sixten Eldh");

    // Undo queue
    let previousUndoLength = UserList.getUndoHistory() |> Stdlib.List.length;
    expect.result(UserList.undo()).toBeOk();
    let newUndoLength = UserList.getUndoHistory() |> Stdlib.List.length;
    expect.equal(newUndoLength, previousUndoLength - 1);
    expect.result(
      UserList.applyRemoteOperations([
        Append({
          id: Rupp.Util.makeId(),
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
      UserList.getUndoHistory() |> Stdlib.List.length,
    );

    // But values should update
    expect.equal(UserList.get(miniMe.id).name, "Sxtn");

    // Redo still works, acts as a new update
    expect.result(UserList.redo()).toBeOk();
    expect.equal(UserList.get(miniMe.id).name, "Sixten Eldh");
    expect.equal(UserList.get(miniMe.id).age, 2);
  });
});