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
module PostList = {
  type t = {
    id: Credt.Util.id,
    title: string,
    userId: Credt.Util.id,
    content: string,
  };

  type update =
    | SetTitle(string)
    | SetContent(string)
    | SetUserId(Credt.Util.id);

  include Credt.List.Make({
    type nonrec t = t;
    type nonrec update = update;
    let moduleId = "PostList" |> Credt.Util.idOfString;
    let getId = u => u.id;
    let reducer = post =>
      fun
      | SetTitle(title) => ({...post, title}, SetTitle(post.title))
      | SetContent(content) => (
          {...post, content},
          SetContent(post.content),
        )
      | SetUserId(userId) => ({...post, userId}, SetUserId(post.userId));
  });
};
let makeUser = i => {
  UserMap.id: Credt.Util.makeId(),
  name: "Name" ++ (i |> string_of_int),
  email: "email@" ++ (i |> string_of_int),
  age: i * 2,
};
let makePost = i => {
  PostList.id: Credt.Util.makeId(),
  title: "Title" ++ (i |> string_of_int),
  userId: Credt.Util.makeId(),
  content: "Content" ++ (i |> string_of_int),
};
let me =
  UserMap.{
    id: Credt.Util.makeId(),
    name: "Andreas",
    email: "andreas@eldh.co",
    age: 35,
  };

module Tests = {
  let {describe, describeSkip, describeOnly} =
    describeConfig
    |> withLifecycle(testLifecycle =>
         testLifecycle
         |> beforeAll(() => ())
         |> afterAll(() => ())
         |> beforeEach(() => {
              Credt.Manager.__reset__();
              PostList.__resetCollection__();
              UserMap.__resetCollection__();
            })
         |> afterEach(() => ())
       )
    |> build;

  describe("Manager", ({test, testOnly}) => {
    test("should undo update", ({expect}) => {
      UserMap.[Add(me), Update(me.id, SetEmail("a@eldh.co"))]
      |> UserMap.apply
      |> ignore;

      expect.bool(Credt.Manager.canUndo()).toBeTrue();
      expect.result(Credt.Manager.undo()).toBeOk();
      expect.equal(UserMap.get(me.id).email, "andreas@eldh.co");
    });

    test("canUndo should be false if no available undo", ({expect}) => {
      expect.bool(Credt.Manager.canUndo()).toBeFalse();
      expect.result(Credt.Manager.undo()).toBeOk();
    });

    test("canRedo should be false if no available redo", ({expect}) => {
      UserMap.[Add(me), Update(me.id, SetEmail("a@eldh.co"))]
      |> UserMap.apply
      |> ignore;
      expect.bool(Credt.Manager.canRedo()).toBeFalse();
      expect.result(Credt.Manager.redo()).toBeOk();
    });

    test("canRedo should work", ({expect}) => {
      UserMap.[Add(me), Update(me.id, SetEmail("a@eldh.co"))]
      |> UserMap.apply
      |> ignore;
      expect.result(Credt.Manager.undo()).toBeOk();
      expect.bool(Credt.Manager.canRedo()).toBeTrue();
      expect.result(Credt.Manager.redo()).toBeOk();
    });

    test("transaction should apply", ({expect}) => {
      [UserMap.Add(me), UserMap.Update(me.id, SetEmail("a@eldh.co"))]
      |> UserMap.addToTransaction;
      [PostList.Append(makePost(1))] |> PostList.addToTransaction;
      expect.result(Credt.Manager.commitTransaction()).toBeOk();
    });

    test("undo transaction should work", ({expect}) => {
      UserMap.[Add(me), Update(me.id, SetEmail("a@eldh.co"))]
      |> UserMap.addToTransaction
      |> ignore;
      expect.result(Credt.Manager.commitTransaction()).toBeOk();
      expect.result(Credt.Manager.undo()).toBeOk();
      expect.bool(Credt.Manager.canUndo()).toBeFalse();
    });

    test("getUndoHistory should work", ({expect}) => {
      expect.equal(Credt.Manager.getUndoHistory() |> Stdlib.List.length, 0);
      UserMap.[Add(me), Update(me.id, SetEmail("a@eldh.co"))]
      |> UserMap.apply
      |> ignore;
      expect.equal(Credt.Manager.getUndoHistory() |> Stdlib.List.length, 2);
      Credt.Manager.undo() |> ignore;
      expect.equal(Credt.Manager.getUndoHistory() |> Stdlib.List.length, 1);
      Credt.Manager.undo() |> ignore;
      expect.equal(Credt.Manager.getUndoHistory() |> Stdlib.List.length, 0);
      Credt.Manager.redo() |> ignore;
      expect.equal(Credt.Manager.getUndoHistory() |> Stdlib.List.length, 1);
    });

    test("getUndoHistory should work with transactions", ({expect}) => {
      expect.equal(Credt.Manager.getUndoHistory() |> Stdlib.List.length, 0);
      UserMap.[Add(me), Update(me.id, SetEmail("a@eldh.co"))]
      |> UserMap.addToTransaction
      |> ignore;
      Credt.Manager.commitTransaction() |> ignore;
      expect.equal(Credt.Manager.getUndoHistory() |> Stdlib.List.length, 1);
      Credt.Manager.undo() |> ignore;
      expect.equal(Credt.Manager.getUndoHistory() |> Stdlib.List.length, 0);
      Credt.Manager.redo() |> ignore;
      expect.equal(Credt.Manager.getUndoHistory() |> Stdlib.List.length, 1);
    });

    test("getRedoHistory should work", ({expect}) => {
      expect.equal(Credt.Manager.getRedoHistory() |> Stdlib.List.length, 0);
      UserMap.[Add(me), Update(me.id, SetEmail("a@eldh.co"))]
      |> UserMap.apply
      |> ignore;
      expect.equal(Credt.Manager.getRedoHistory() |> Stdlib.List.length, 0);
      Credt.Manager.undo() |> ignore;
      expect.equal(Credt.Manager.getRedoHistory() |> Stdlib.List.length, 1);
      Credt.Manager.redo() |> ignore;
      expect.equal(Credt.Manager.getRedoHistory() |> Stdlib.List.length, 0);
    });

    test("should listen", ({expect}) => {
      let mock = Mock.mock1(_ => ());
      expect.mock(mock).toBeCalledTimes(0);
      Credt.Manager.addChangeListener(Mock.fn(mock));
      UserMap.[Add(me)] |> UserMap.apply |> ignore;
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

      UserMap.[Add(makeUser(2))] |> UserMap.apply |> ignore;
      expect.mock(mock).toBeCalledTimes(1);
      expect.mock(mock2).toBeCalledTimes(1);

      UserMap.removeChangeListener(listener);

      UserMap.[Add(makeUser(1))] |> UserMap.apply |> ignore;

      expect.mock(mock).toBeCalledTimes(1);
      expect.mock(mock2).toBeCalledTimes(2);
    });
  });
};
