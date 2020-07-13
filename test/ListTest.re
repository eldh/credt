open TestFramework;
module UserList = {
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

  include Credt.List.Make({
    type nonrec t = t;
    type nonrec update = update;
    let getId = u => u.id;
    let moduleId = "UserList" |> Credt.Util.idOfString;
    let reducer = user =>
      fun
      | SetEmail(email) => ({...user, email}, SetEmail(user.email))
      | SetName(name) => ({...user, name}, SetName(user.name))
      | SetAge(age) => ({...user, age}, SetAge(user.age));
  });
};

let makeUser = i => {
  UserList.id: Credt.Util.makeId(),
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
       |> beforeEach(() => {
            Credt.Manager.__reset__();
            UserList.__resetCollection__();
          })
       |> afterEach(() => ())
     )
  |> build;

describe("List", t => {
  let {test, testOnly, testSkip} = t;
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
      [
        UserList.AddAfter(
          List.nth(UserList.getSnapshot(), 3) |> (u => u.id),
          testUser,
        ),
      ]
      |> UserList.apply,
    ).
      toBeOk();
    expect.equal(
      List.nth(UserList.getSnapshot(), 4) |> (u => u.id),
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
      [
        UserList.AddBefore(
          List.nth(UserList.getSnapshot(), 3) |> (u => u.id),
          testUser,
        ),
      ]
      |> UserList.apply,
    ).
      toBeOk();
    expect.equal(
      List.nth(UserList.getSnapshot(), 3) |> (u => u.id),
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
    expect.result(
      [UserList.AddAfter(notAddedUser.id, testUser)] |> UserList.apply,
    ).
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
      [UserList.AddBefore(notAddedUser.id, testUser)] |> UserList.apply,
    ).
      toBeError();
  });

  test("should handle illegal operation", ({expect}) => {
    let user = makeUser(1);
    let addOp = [UserList.Append(user)];
    let removeOp = [UserList.Remove(user.id)];
    let editOp = [UserList.Update(user.id, SetName("New name"))];
    let replaceOp = [UserList.Replace(user.id, makeUser(2))];

    expect.result(addOp |> UserList.apply).toBeOk();
    expect.result(removeOp |> UserList.apply).toBeOk();
    expect.result(editOp |> UserList.apply).toBeError();
    expect.result(replaceOp |> UserList.apply).toBeError();
  });

  testSkip("should correctly undo remove operation", ({expect}) => {
    userStdList
    |> Stdlib.List.map(u => UserList.Append(u))
    |> UserList.apply
    |> ignore;
    let unluckyUser = Stdlib.List.nth(userStdList, 3);
    let removeOp = [UserList.Remove(unluckyUser.id)];

    expect.result(removeOp |> UserList.apply).toBeOk();
    expect.result(Credt.Manager.undo()).toBeOk();
    expect.equal(
      unluckyUser.id,
      Stdlib.List.nth(UserList.getSnapshot(), 3).id,
    );
  });

  test("should handle multiple items", ({expect}) => {
    let me = {
      UserList.id: Credt.Util.makeId(),
      name: "Andreas",
      email: "andreas@eldh.co",
      age: 35,
    };

    let miniMe = {
      UserList.id: Credt.Util.makeId(),
      name: "Sixten",
      email: "sixten@eldh.co",
      age: 2,
    };

    expect.result(
      [
        UserList.Append(me),
        Update(me.id, SetEmail("a@eldh.co")),
        UserList.Append(miniMe),
        Update(miniMe.id, SetName("Sixten Eldh")),
      ]
      |> UserList.apply,
    ).
      toBeOk();

    expect.equal(UserList.getExn(miniMe.id).name, "Sixten Eldh");
    expect.equal(UserList.getExn(me.id).email, "a@eldh.co");
    expect.equal(UserList.getExn(miniMe.id).age, 2);
  });

  test("should handle ok transaction", ({expect}) => {
    let me = {
      UserList.id: Credt.Util.makeId(),
      name: "Andreas",
      email: "andreas@eldh.co",
      age: 35,
    };

    let miniMe = {
      UserList.id: Credt.Util.makeId(),
      name: "Sixten",
      email: "sixten@eldh.co",
      age: 2,
    };

    [
      UserList.Append(me),
      Update(me.id, SetEmail("a@eldh.co")),
      UserList.Append(miniMe),
      Update(miniMe.id, SetName("Sixten Eldh")),
    ]
    |> UserList.addToTransaction;
    expect.result(Credt.Manager.commitTransaction()).toBeOk();
    expect.equal(UserList.getExn(miniMe.id).name, "Sixten Eldh");
    expect.equal(UserList.getExn(me.id).email, "a@eldh.co");
    expect.equal(UserList.getExn(miniMe.id).age, 2);
    expect.equal(Credt.Manager.getUndoHistory() |> Stdlib.List.length, 1);
    expect.equal(Credt.Manager.getRedoHistory() |> Stdlib.List.length, 0);
  });

  test("should roll back transaction", ({expect}) => {
    let me = {
      UserList.id: Credt.Util.makeId(),
      name: "Andreas",
      email: "andreas@eldh.co",
      age: 35,
    };

    let miniMe = {
      UserList.id: Credt.Util.makeId(),
      name: "Sixten",
      email: "sixten@eldh.co",
      age: 2,
    };

    [
      UserList.Append(me),
      Remove(me.id),
      Update(me.id, SetEmail("a@eldh.co")),
      UserList.Append(miniMe),
      Update(miniMe.id, SetName("Sixten Eldh")),
    ]
    |> UserList.addToTransaction;
    expect.result(Credt.Manager.commitTransaction()).toBeError();
    expect.equal(UserList.getSnapshot() |> Stdlib.List.length, 0);
    expect.equal(Credt.Manager.getUndoHistory() |> Stdlib.List.length, 0);
    expect.equal(Credt.Manager.getRedoHistory() |> Stdlib.List.length, 0);
  });

  test("should handle undo & redo", ({expect}) => {
    let miniMe = {
      UserList.id: Credt.Util.makeId(),
      name: "Sixten",
      email: "sixten@eldh.co",
      age: 2,
    };
    expect.result(
      [UserList.Append(miniMe), Update(miniMe.id, SetName("Sixten Eldh"))]
      |> UserList.apply,
    ).
      toBeOk();
    expect.equal(UserList.getExn(miniMe.id).name, "Sixten Eldh");

    // Undo queue
    let previousUndoLength =
      Credt.Manager.getUndoHistory() |> Stdlib.List.length;
    expect.result(Credt.Manager.undo()).toBeOk();
    let newUndoLength = Credt.Manager.getUndoHistory() |> Stdlib.List.length;
    expect.equal(newUndoLength, previousUndoLength - 1);
    expect.result(
      UserList.applyRemoteOperations([
        UserList.Append({
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
    expect.equal(UserList.getExn(miniMe.id).name, "Sxtn");

    // Redo still works, acts as a new update
    expect.result(Credt.Manager.redo()).toBeOk();
    expect.equal(UserList.getExn(miniMe.id).name, "Sixten Eldh");
    expect.equal(UserList.getExn(miniMe.id).age, 2);
  });
});
