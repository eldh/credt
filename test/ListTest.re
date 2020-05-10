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
open UserList;

let makeUser = i => {
  id: Rupp.Util.makeId(),
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
       |> beforeEach(() => {__resetCollection__()})
       |> afterEach(() => ())
     )
  |> build;

describe("List", t => {
  let {test, testOnly, testSkip} = t;
  test("should add", ({expect}) => {
    userStdList |> Stdlib.List.map(u => Append(u)) |> apply |> ignore;
    expect.equal(length(), 10);
  });
  test("should add after", ({expect}) => {
    userStdList
    |> Stdlib.List.map(u => Append(u))
    |> Stdlib.List.rev
    |> apply
    |> ignore;
    expect.equal(length(), 10);
    let testUser = makeUser(122);
    expect.result(
      [AddAfter(List.nth(getSnapshot(), 3) |> (u => u.id), testUser)]
      |> apply,
    ).
      toBeOk();
    expect.equal(List.nth(getSnapshot(), 4) |> (u => u.id), testUser.id);
  });

  test("should add before", ({expect}) => {
    userStdList
    |> Stdlib.List.map(u => Append(u))
    |> Stdlib.List.rev
    |> apply
    |> ignore;
    expect.equal(length(), 10);
    let testUser = makeUser(122);
    expect.result(
      [AddBefore(List.nth(getSnapshot(), 3) |> (u => u.id), testUser)]
      |> apply,
    ).
      toBeOk();
    expect.equal(List.nth(getSnapshot(), 3) |> (u => u.id), testUser.id);
  });

  test("should fail to add after invalid id", ({expect}) => {
    userStdList
    |> Stdlib.List.map(u => Append(u))
    |> Stdlib.List.rev
    |> apply
    |> ignore;
    expect.equal(length(), 10);
    let notAddedUser = makeUser(11232323);
    let testUser = makeUser(122);
    expect.result([AddAfter(notAddedUser.id, testUser)] |> apply).toBeError();
  });

  test("should fail to add before invalid id", ({expect}) => {
    userStdList
    |> Stdlib.List.map(u => Append(u))
    |> Stdlib.List.rev
    |> apply
    |> ignore;
    expect.equal(length(), 10);
    let notAddedUser = makeUser(11232323);
    let testUser = makeUser(122);
    expect.result([AddBefore(notAddedUser.id, testUser)] |> apply).toBeError();
  });

  test("should handle illegal operation", ({expect}) => {
    let user = makeUser(1);
    let addOp = [Append(user)];
    let removeOp = [Remove(user.id)];
    let editOp = [Update(user.id, SetName("New name"))];
    let replaceOp = [Replace(user.id, makeUser(2))];

    expect.result(addOp |> apply).toBeOk();
    expect.result(removeOp |> apply).toBeOk();
    expect.result(editOp |> apply).toBeError();
    expect.result(replaceOp |> apply).toBeError();
  });

  testSkip("should correctly undo remove operation", ({expect}) => {
    userStdList |> Stdlib.List.map(u => Append(u)) |> apply |> ignore;
    let unluckyUser = Stdlib.List.nth(userStdList, 3);
    let removeOp = [Remove(unluckyUser.id)];

    expect.result(removeOp |> apply).toBeOk();
    expect.result(undo()).toBeOk();
    expect.equal(unluckyUser.id, Stdlib.List.nth(getSnapshot(), 3).id);
  });

  test("should handle multiple items", ({expect}) => {
    let me = {
      id: Rupp.Util.makeId(),
      name: "Andreas",
      email: "andreas@eldh.co",
      age: 35,
    };

    let miniMe = {
      id: Rupp.Util.makeId(),
      name: "Sixten",
      email: "sixten@eldh.co",
      age: 2,
    };

    expect.result(
      [
        Append(me),
        Update(me.id, SetEmail("a@eldh.co")),
        Append(miniMe),
        Update(miniMe.id, SetName("Sixten Eldh")),
      ]
      |> apply,
    ).
      toBeOk();

    expect.equal(get(miniMe.id).name, "Sixten Eldh");
    expect.equal(get(me.id).email, "a@eldh.co");
    expect.equal(get(miniMe.id).age, 2);
  });

  test("should handle ok transaction", ({expect}) => {
    let me = {
      id: Rupp.Util.makeId(),
      name: "Andreas",
      email: "andreas@eldh.co",
      age: 35,
    };

    let miniMe = {
      id: Rupp.Util.makeId(),
      name: "Sixten",
      email: "sixten@eldh.co",
      age: 2,
    };

    expect.result(
      [
        Append(me),
        Update(me.id, SetEmail("a@eldh.co")),
        Append(miniMe),
        Update(miniMe.id, SetName("Sixten Eldh")),
      ]
      |> applyTransaction,
    ).
      toBeOk();

    expect.equal(get(miniMe.id).name, "Sixten Eldh");
    expect.equal(get(me.id).email, "a@eldh.co");
    expect.equal(get(miniMe.id).age, 2);
    expect.equal(getUndoHistory() |> Stdlib.List.length, 1);
    expect.equal(getRedoHistory() |> Stdlib.List.length, 0);
  });

  test("should roll back transaction", ({expect}) => {
    let me = {
      id: Rupp.Util.makeId(),
      name: "Andreas",
      email: "andreas@eldh.co",
      age: 35,
    };

    let miniMe = {
      id: Rupp.Util.makeId(),
      name: "Sixten",
      email: "sixten@eldh.co",
      age: 2,
    };

    expect.result(
      [
        Append(me),
        Remove(me.id),
        Update(me.id, SetEmail("a@eldh.co")),
        Append(miniMe),
        Update(miniMe.id, SetName("Sixten Eldh")),
      ]
      |> applyTransaction,
    ).
      toBeError();
    expect.equal(getSnapshot() |> Stdlib.List.length, 0);
    expect.equal(getUndoHistory() |> Stdlib.List.length, 0);
    expect.equal(getRedoHistory() |> Stdlib.List.length, 0);
  });

  test("should handle undo & redo", ({expect}) => {
    let miniMe = {
      id: Rupp.Util.makeId(),
      name: "Sixten",
      email: "sixten@eldh.co",
      age: 2,
    };
    expect.result(
      [Append(miniMe), Update(miniMe.id, SetName("Sixten Eldh"))] |> apply,
    ).
      toBeOk();
    expect.equal(get(miniMe.id).name, "Sixten Eldh");

    // Undo queue
    let previousUndoLength = getUndoHistory() |> Stdlib.List.length;
    expect.result(undo()).toBeOk();
    let newUndoLength = getUndoHistory() |> Stdlib.List.length;
    expect.equal(newUndoLength, previousUndoLength - 1);
    expect.result(
      applyRemoteOperations([
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
    expect.equal(newUndoLength, getUndoHistory() |> Stdlib.List.length);

    // But values should update
    expect.equal(get(miniMe.id).name, "Sxtn");

    // Redo still works, acts as a new update
    expect.result(redo()).toBeOk();
    expect.equal(get(miniMe.id).name, "Sixten Eldh");
    expect.equal(get(miniMe.id).age, 2);
  });
});